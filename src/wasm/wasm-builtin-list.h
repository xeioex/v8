// Copyright 2023 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_WASM_WASM_BUILTIN_LIST_H_
#define V8_WASM_WASM_BUILTIN_LIST_H_

#include "src/base/macros.h"
#include "src/builtins/builtins.h"
#include "src/common/globals.h"

#if !V8_ENABLE_WEBASSEMBLY
#error This header should only be included if WebAssembly is enabled.
#endif  // !V8_ENABLE_WEBASSEMBLY

namespace v8::internal::wasm {
// Convenience macro listing all builtins called from wasm. Note that the first
// few elements of the list coincide with {compiler::TrapId}, order matters.
#define WASM_BUILTIN_LIST(V, VTRAP) /*                                      */ \
  FOREACH_WASM_TRAPREASON(VTRAP)                                               \
  V(WasmCompileLazy)                                                           \
  V(WasmTriggerTierUp)                                                         \
  V(WasmLiftoffFrameSetup)                                                     \
  V(WasmDebugBreak)                                                            \
  V(WasmInt32ToHeapNumber)                                                     \
  V(WasmTaggedNonSmiToInt32)                                                   \
  V(WasmFloat32ToNumber)                                                       \
  V(WasmFloat64ToNumber)                                                       \
  V(WasmTaggedToFloat64)                                                       \
  V(WasmAllocateJSArray)                                                       \
  V(WasmI32AtomicWait)                                                         \
  V(WasmI64AtomicWait)                                                         \
  V(WasmGetOwnProperty)                                                        \
  V(WasmRefFunc)                                                               \
  V(WasmInternalFunctionCreateExternal)                                        \
  V(WasmMemoryGrow)                                                            \
  V(WasmTableInit)                                                             \
  V(WasmTableCopy)                                                             \
  V(WasmTableFill)                                                             \
  V(WasmTableGrow)                                                             \
  V(WasmTableGet)                                                              \
  V(WasmTableSet)                                                              \
  V(WasmTableGetFuncRef)                                                       \
  V(WasmTableSetFuncRef)                                                       \
  V(WasmStackGuard)                                                            \
  V(WasmStackOverflow)                                                         \
  V(WasmAllocateFixedArray)                                                    \
  V(WasmThrow)                                                                 \
  V(WasmRethrow)                                                               \
  V(WasmRethrowExplicitContext)                                                \
  V(WasmTraceEnter)                                                            \
  V(WasmTraceExit)                                                             \
  V(WasmTraceMemory)                                                           \
  V(BigIntToI32Pair)                                                           \
  V(BigIntToI64)                                                               \
  V(CallRefIC)                                                                 \
  V(DoubleToI)                                                                 \
  V(I32PairToBigInt)                                                           \
  V(I64ToBigInt)                                                               \
  V(RecordWriteSaveFP)                                                         \
  V(RecordWriteIgnoreFP)                                                       \
  V(ToNumber)                                                                  \
  V(ThrowDataViewGetBigInt64DetachedError)                                     \
  V(ThrowDataViewGetBigInt64OutOfBounds)                                       \
  V(ThrowDataViewGetBigInt64TypeError)                                         \
  V(ThrowDataViewGetBigUint64DetachedError)                                    \
  V(ThrowDataViewGetBigUint64OutOfBounds)                                      \
  V(ThrowDataViewGetBigUint64TypeError)                                        \
  V(ThrowDataViewGetFloat32DetachedError)                                      \
  V(ThrowDataViewGetFloat32OutOfBounds)                                        \
  V(ThrowDataViewGetFloat32TypeError)                                          \
  V(ThrowDataViewGetFloat64DetachedError)                                      \
  V(ThrowDataViewGetFloat64OutOfBounds)                                        \
  V(ThrowDataViewGetFloat64TypeError)                                          \
  V(ThrowDataViewGetInt8DetachedError)                                         \
  V(ThrowDataViewGetInt8OutOfBounds)                                           \
  V(ThrowDataViewGetInt8TypeError)                                             \
  V(ThrowDataViewGetInt16DetachedError)                                        \
  V(ThrowDataViewGetInt16OutOfBounds)                                          \
  V(ThrowDataViewGetInt16TypeError)                                            \
  V(ThrowDataViewGetInt32DetachedError)                                        \
  V(ThrowDataViewGetInt32OutOfBounds)                                          \
  V(ThrowDataViewGetInt32TypeError)                                            \
  V(ThrowDataViewGetUint8DetachedError)                                        \
  V(ThrowDataViewGetUint8OutOfBounds)                                          \
  V(ThrowDataViewGetUint8TypeError)                                            \
  V(ThrowDataViewGetUint16DetachedError)                                       \
  V(ThrowDataViewGetUint16OutOfBounds)                                         \
  V(ThrowDataViewGetUint16TypeError)                                           \
  V(ThrowDataViewGetUint32DetachedError)                                       \
  V(ThrowDataViewGetUint32OutOfBounds)                                         \
  V(ThrowDataViewGetUint32TypeError)                                           \
  V(ThrowDataViewSetBigInt64DetachedError)                                     \
  V(ThrowDataViewSetBigInt64OutOfBounds)                                       \
  V(ThrowDataViewSetBigInt64TypeError)                                         \
  V(ThrowDataViewSetBigUint64DetachedError)                                    \
  V(ThrowDataViewSetBigUint64OutOfBounds)                                      \
  V(ThrowDataViewSetBigUint64TypeError)                                        \
  V(ThrowDataViewSetFloat32DetachedError)                                      \
  V(ThrowDataViewSetFloat32OutOfBounds)                                        \
  V(ThrowDataViewSetFloat32TypeError)                                          \
  V(ThrowDataViewSetFloat64DetachedError)                                      \
  V(ThrowDataViewSetFloat64OutOfBounds)                                        \
  V(ThrowDataViewSetFloat64TypeError)                                          \
  V(ThrowDataViewSetInt8DetachedError)                                         \
  V(ThrowDataViewSetInt8OutOfBounds)                                           \
  V(ThrowDataViewSetInt8TypeError)                                             \
  V(ThrowDataViewSetInt16DetachedError)                                        \
  V(ThrowDataViewSetInt16OutOfBounds)                                          \
  V(ThrowDataViewSetInt16TypeError)                                            \
  V(ThrowDataViewSetInt32DetachedError)                                        \
  V(ThrowDataViewSetInt32OutOfBounds)                                          \
  V(ThrowDataViewSetInt32TypeError)                                            \
  V(ThrowDataViewSetUint8DetachedError)                                        \
  V(ThrowDataViewSetUint8OutOfBounds)                                          \
  V(ThrowDataViewSetUint8TypeError)                                            \
  V(ThrowDataViewSetUint16DetachedError)                                       \
  V(ThrowDataViewSetUint16OutOfBounds)                                         \
  V(ThrowDataViewSetUint16TypeError)                                           \
  V(ThrowDataViewSetUint32DetachedError)                                       \
  V(ThrowDataViewSetUint32OutOfBounds)                                         \
  V(ThrowDataViewSetUint32TypeError)                                           \
  IF_TSAN(V, TSANRelaxedStore8IgnoreFP)                                        \
  IF_TSAN(V, TSANRelaxedStore8SaveFP)                                          \
  IF_TSAN(V, TSANRelaxedStore16IgnoreFP)                                       \
  IF_TSAN(V, TSANRelaxedStore16SaveFP)                                         \
  IF_TSAN(V, TSANRelaxedStore32IgnoreFP)                                       \
  IF_TSAN(V, TSANRelaxedStore32SaveFP)                                         \
  IF_TSAN(V, TSANRelaxedStore64IgnoreFP)                                       \
  IF_TSAN(V, TSANRelaxedStore64SaveFP)                                         \
  IF_TSAN(V, TSANSeqCstStore8IgnoreFP)                                         \
  IF_TSAN(V, TSANSeqCstStore8SaveFP)                                           \
  IF_TSAN(V, TSANSeqCstStore16IgnoreFP)                                        \
  IF_TSAN(V, TSANSeqCstStore16SaveFP)                                          \
  IF_TSAN(V, TSANSeqCstStore32IgnoreFP)                                        \
  IF_TSAN(V, TSANSeqCstStore32SaveFP)                                          \
  IF_TSAN(V, TSANSeqCstStore64IgnoreFP)                                        \
  IF_TSAN(V, TSANSeqCstStore64SaveFP)                                          \
  IF_TSAN(V, TSANRelaxedLoad32IgnoreFP)                                        \
  IF_TSAN(V, TSANRelaxedLoad32SaveFP)                                          \
  IF_TSAN(V, TSANRelaxedLoad64IgnoreFP)                                        \
  IF_TSAN(V, TSANRelaxedLoad64SaveFP)                                          \
  V(WasmAllocateArray_Uninitialized)                                           \
  V(WasmArrayCopy)                                                             \
  V(WasmArrayCopyWithChecks)                                                   \
  V(WasmArrayNewSegment)                                                       \
  V(WasmArrayInitSegment)                                                      \
  V(WasmAllocateStructWithRtt)                                                 \
  V(WasmOnStackReplace)                                                        \
  V(WasmSuspend)                                                               \
  V(WasmStringNewWtf8)                                                         \
  V(WasmStringNewWtf16)                                                        \
  V(WasmStringConst)                                                           \
  V(WasmStringMeasureUtf8)                                                     \
  V(WasmStringMeasureWtf8)                                                     \
  V(WasmStringEncodeWtf8)                                                      \
  V(WasmStringEncodeWtf16)                                                     \
  V(WasmStringConcat)                                                          \
  V(WasmStringEqual)                                                           \
  V(WasmStringIsUSVSequence)                                                   \
  V(WasmStringAsWtf16)                                                         \
  V(WasmStringViewWtf16GetCodeUnit)                                            \
  V(WasmStringCodePointAt)                                                     \
  V(WasmStringViewWtf16Encode)                                                 \
  V(WasmStringViewWtf16Slice)                                                  \
  V(WasmStringNewWtf8Array)                                                    \
  V(WasmStringNewWtf16Array)                                                   \
  V(WasmStringEncodeWtf8Array)                                                 \
  V(WasmStringEncodeWtf16Array)                                                \
  V(WasmStringAsWtf8)                                                          \
  V(WasmStringViewWtf8Advance)                                                 \
  V(WasmStringViewWtf8Encode)                                                  \
  V(WasmStringViewWtf8Slice)                                                   \
  V(WasmStringAsIter)                                                          \
  V(WasmStringViewIterNext)                                                    \
  V(WasmStringViewIterAdvance)                                                 \
  V(WasmStringViewIterRewind)                                                  \
  V(WasmStringViewIterSlice)                                                   \
  V(StringCompare)                                                             \
  V(WasmStringFromCodePoint)                                                   \
  V(WasmStringHash)                                                            \
  V(WasmAnyConvertExtern)                                                      \
  V(WasmStringFromDataSegment)                                                 \
  V(StringAdd_CheckNone)                                                       \
  V(DebugPrintFloat64)                                                         \
  V(DebugPrintWordPtr)

namespace detail {
constexpr std::array<uint8_t, static_cast<int>(Builtin::kFirstBytecodeHandler)>
InitBuiltinToFarJumpTableIndex() {
  std::array<uint8_t, static_cast<int>(Builtin::kFirstBytecodeHandler)>
      result{};
  uint8_t next_index = 0;
#define DEF_INIT_LOOKUP(NAME) \
  result[static_cast<int>(Builtin::k##NAME)] = next_index++;
#define DEF_INIT_LOOKUP_TRAP(NAME) DEF_INIT_LOOKUP(ThrowWasm##NAME)
  WASM_BUILTIN_LIST(DEF_INIT_LOOKUP, DEF_INIT_LOOKUP_TRAP)
#undef DEF_INIT_LOOKUP_TRAP
#undef DEF_INIT_LOOKUP
  return result;
}
}  // namespace detail
class BuiltinLookup {
 public:
  static constexpr int JumptableIndexForBuiltin(Builtin builtin) {
    int result = kBuiltinToFarJumpTableIndex[static_cast<int>(builtin)];
    DCHECK_EQ(builtin, kFarJumpTableIndexToBuiltin[result]);
    return result;
  }

  static constexpr Builtin BuiltinForJumptableIndex(int index) {
    Builtin result = kFarJumpTableIndexToBuiltin[index];
    DCHECK_EQ(index, kBuiltinToFarJumpTableIndex[static_cast<int>(result)]);
    return result;
  }

  static constexpr int BuiltinCount() { return kBuiltinCount; }

 private:
#define BUILTIN_COUNTER(NAME) +1
  static constexpr int kBuiltinCount =
      0 WASM_BUILTIN_LIST(BUILTIN_COUNTER, BUILTIN_COUNTER);
#undef BUILTIN_COUNTER

  static constexpr auto kFarJumpTableIndexToBuiltin =
      base::make_array<static_cast<int>(kBuiltinCount)>([](size_t index) {
        size_t next_index = 0;
#define DEF_INIT_LOOKUP(NAME) \
  if (index == next_index) {  \
    return Builtin::k##NAME;  \
  }                           \
  ++next_index;
#define DEF_INIT_LOOKUP_TRAP(NAME) DEF_INIT_LOOKUP(ThrowWasm##NAME)
        WASM_BUILTIN_LIST(DEF_INIT_LOOKUP, DEF_INIT_LOOKUP_TRAP)
#undef DEF_INIT_LOOKUP_TRAP
#undef DEF_INIT_LOOKUP
        return Builtin::kNoBuiltinId;
      });

  static constexpr auto kBuiltinToFarJumpTableIndex =
      detail::InitBuiltinToFarJumpTableIndex();
};

}  // namespace v8::internal::wasm

#undef WASM_BUILTIN_LIST

#endif  // V8_WASM_WASM_BUILTIN_LIST_H_
