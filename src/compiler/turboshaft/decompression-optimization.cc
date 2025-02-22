// Copyright 2022 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/compiler/turboshaft/decompression-optimization.h"

#include "src/base/v8-fallthrough.h"
#include "src/codegen/machine-type.h"
#include "src/compiler/turboshaft/operations.h"
#include "src/compiler/turboshaft/optimization-phase.h"

namespace v8::internal::compiler::turboshaft {

namespace {

// Analyze the uses of values to determine if a compressed value has any uses
// that need it to be decompressed. Since this analysis looks at uses, we
// iterate the graph backwards, updating the analysis state for the inputs of an
// operation. Due to loop phis, we need to compute a fixed-point. Therefore, we
// re-visit the loop if a loop phi backedge changes something. As a performance
// optimization, we keep track of operations (`candidates`) that need to be
// updated potentially, so that we don't have to walk the whole graph again.
struct DecompressionAnalyzer {
  const Graph& graph;
  Zone* phase_zone;
  // We use `uint8_t` instead of `bool` here to avoid the bitvector optimization
  // of std::vector.
  FixedSidetable<uint8_t> needs_decompression;
  ZoneVector<OpIndex> candidates;

  DecompressionAnalyzer(const Graph& graph, Zone* phase_zone)
      : graph(graph),
        phase_zone(phase_zone),
        needs_decompression(graph.op_id_count(), phase_zone),
        candidates(phase_zone) {
    candidates.reserve(graph.op_id_count() / 8);
  }

  void Run() {
    for (uint32_t next_block_id = graph.block_count() - 1; next_block_id > 0;) {
      BlockIndex block_index = BlockIndex(next_block_id);
      --next_block_id;
      const Block& block = graph.Get(block_index);
      if (block.IsLoop()) {
        ProcessBlock<true>(block, &next_block_id);
      } else {
        ProcessBlock<false>(block, &next_block_id);
      }
    }
  }

  bool NeedsDecompression(OpIndex op) { return needs_decompression[op]; }
  bool NeedsDecompression(const Operation& op) {
    return NeedsDecompression(graph.Index(op));
  }
  bool MarkAsNeedsDecompression(OpIndex op) {
    return (needs_decompression[op] = true);
  }

  template <bool is_loop>
  void ProcessBlock(const Block& block, uint32_t* next_block_id) {
    for (const Operation& op : base::Reversed(graph.operations(block))) {
      if (is_loop && op.Is<PhiOp>() && NeedsDecompression(op)) {
        const PhiOp& phi = op.Cast<PhiOp>();
        if (!NeedsDecompression(phi.input(1))) {
          Block* backedge = block.LastPredecessor();
          *next_block_id =
              std::max<uint32_t>(*next_block_id, backedge->index().id());
        }
      }
      ProcessOperation(op);
    }
  }
  void ProcessOperation(const Operation& op);
};

void DecompressionAnalyzer::ProcessOperation(const Operation& op) {
  switch (op.opcode) {
    case Opcode::kStore: {
      auto& store = op.Cast<StoreOp>();
      MarkAsNeedsDecompression(store.base());
      if (store.index().valid()) {
        MarkAsNeedsDecompression(store.index().value());
      }
      if (!store.stored_rep.IsTagged()) {
        MarkAsNeedsDecompression(store.value());
      }
      break;
    }
    case Opcode::kFrameState:
      // The deopt code knows how to handle compressed inputs.
      break;
    case Opcode::kPhi: {
      // Replicate the phi's state for its inputs.
      auto& phi = op.Cast<PhiOp>();
      if (NeedsDecompression(op)) {
        for (OpIndex input : phi.inputs()) {
          MarkAsNeedsDecompression(input);
        }
      } else {
        candidates.push_back(graph.Index(op));
      }
      break;
    }
    case Opcode::kEqual: {
      auto& equal = op.Cast<EqualOp>();
      if (equal.rep == WordRepresentation::Word64()) {
        MarkAsNeedsDecompression(equal.left());
        MarkAsNeedsDecompression(equal.right());
      }
      break;
    }
    case Opcode::kComparison: {
      auto& comp = op.Cast<ComparisonOp>();
      if (comp.rep == WordRepresentation::Word64()) {
        MarkAsNeedsDecompression(comp.left());
        MarkAsNeedsDecompression(comp.right());
      }
      break;
    }
    case Opcode::kWordBinop: {
      auto& binary_op = op.Cast<WordBinopOp>();
      if (binary_op.rep == WordRepresentation::Word64()) {
        MarkAsNeedsDecompression(binary_op.left());
        MarkAsNeedsDecompression(binary_op.right());
      }
      break;
    }
    case Opcode::kShift: {
      auto& shift_op = op.Cast<ShiftOp>();
      if (shift_op.rep == WordRepresentation::Word64()) {
        MarkAsNeedsDecompression(shift_op.left());
      }
      break;
    }
    case Opcode::kChange: {
      auto& change = op.Cast<ChangeOp>();
      if (change.to == WordRepresentation::Word64() && NeedsDecompression(op)) {
        MarkAsNeedsDecompression(change.input());
      }
      break;
    }
    case Opcode::kTaggedBitcast: {
      auto& bitcast = op.Cast<TaggedBitcastOp>();
      if (NeedsDecompression(op)) {
        MarkAsNeedsDecompression(bitcast.input());
      } else {
        candidates.push_back(graph.Index(op));
      }
      break;
    }
    case Opcode::kLoad:
    case Opcode::kConstant:
      if (!NeedsDecompression(op)) {
        candidates.push_back(graph.Index(op));
      }
      V8_FALLTHROUGH;
    default:
      for (OpIndex input : op.inputs()) {
        MarkAsNeedsDecompression(input);
      }
      break;
  }
}

}  // namespace

// Instead of using `OptimizationPhase`, we directly mutate the operations after
// the analysis. Doing it in-place is possible because we only modify operation
// options.
void RunDecompressionOptimization(Graph& graph, Zone* phase_zone) {
  DecompressionAnalyzer analyzer(graph, phase_zone);
  analyzer.Run();
  for (OpIndex op_idx : analyzer.candidates) {
    Operation& op = graph.Get(op_idx);
    if (analyzer.NeedsDecompression(op)) continue;
    switch (op.opcode) {
      case Opcode::kConstant: {
        auto& constant = op.Cast<ConstantOp>();
        if (constant.kind == ConstantOp::Kind::kHeapObject) {
          constant.kind = ConstantOp::Kind::kCompressedHeapObject;
        }
        break;
      }
      case Opcode::kPhi: {
        auto& phi = op.Cast<PhiOp>();
        if (phi.rep == RegisterRepresentation::Tagged()) {
          phi.rep = RegisterRepresentation::Compressed();
        }
        break;
      }
      case Opcode::kLoad: {
        auto& load = op.Cast<LoadOp>();
        if (load.loaded_rep.IsTagged()) {
          DCHECK_EQ(load.result_rep,
                    any_of(RegisterRepresentation::Tagged(),
                           RegisterRepresentation::Compressed()));
          load.result_rep = RegisterRepresentation::Compressed();
        }
        break;
      }
      case Opcode::kTaggedBitcast: {
        auto& bitcast = op.Cast<TaggedBitcastOp>();
        if (bitcast.from == RegisterRepresentation::Tagged() &&
            bitcast.to == RegisterRepresentation::PointerSized()) {
          bitcast.from = RegisterRepresentation::Compressed();
          bitcast.to = RegisterRepresentation::Word32();
        }
        break;
      }
      default:
        break;
    }
  }
}

}  // namespace v8::internal::compiler::turboshaft
