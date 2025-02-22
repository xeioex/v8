// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_OBJECTS_FIXED_ARRAY_H_
#define V8_OBJECTS_FIXED_ARRAY_H_

#include "src/handles/maybe-handles.h"
#include "src/objects/heap-object.h"
#include "src/objects/instance-type.h"
#include "src/objects/objects.h"
#include "src/objects/smi.h"
#include "src/roots/roots.h"
#include "src/utils/memcopy.h"

// Has to be the last include (doesn't have include guards):
#include "src/objects/object-macros.h"

namespace v8 {
namespace internal {

#include "torque-generated/src/objects/fixed-array-tq.inc"

// Derived: must have a Smi slot at kCapacityOffset.
template <class Derived, class ShapeT>
class TaggedArrayBase : public HeapObject {
  OBJECT_CONSTRUCTORS(TaggedArrayBase, HeapObject);

  using ElementT = typename ShapeT::ElementT;
  static_assert(ShapeT::kElementSize == kTaggedSize);
  static_assert(is_subtype_v<ElementT, Object>);

  static constexpr bool kSupportsSmiElements =
      std::is_convertible_v<Smi, ElementT>;
  static constexpr WriteBarrierMode kDefaultMode =
      std::is_same_v<ElementT, Smi> ? SKIP_WRITE_BARRIER : UPDATE_WRITE_BARRIER;

 public:
  using Shape = ShapeT;

  inline int capacity() const;
  inline int capacity(AcquireLoadTag) const;
  inline void set_capacity(int value);
  inline void set_capacity(int value, ReleaseStoreTag);

  // For most arraylike objects, length equals capacity. Provide these
  // convenience accessors:
  template <typename = std::enable_if<Shape::kLengthEqualsCapacity>>
  inline int length() const;
  template <typename = std::enable_if<Shape::kLengthEqualsCapacity>>
  inline int length(AcquireLoadTag tag) const;
  template <typename = std::enable_if<Shape::kLengthEqualsCapacity>>
  inline void set_length(int value);
  template <typename = std::enable_if<Shape::kLengthEqualsCapacity>>
  inline void set_length(int value, ReleaseStoreTag tag);

  inline Tagged<ElementT> get(int index) const;
  inline Tagged<ElementT> get(int index, RelaxedLoadTag) const;
  inline Tagged<ElementT> get(int index, AcquireLoadTag) const;
  inline Tagged<ElementT> get(int index, SeqCstAccessTag) const;

  inline void set(int index, Tagged<ElementT> value,
                  WriteBarrierMode mode = kDefaultMode);
  template <typename = std::enable_if<kSupportsSmiElements>>
  inline void set(int index, Tagged<Smi> value);
  inline void set(int index, Tagged<ElementT> value, RelaxedStoreTag,
                  WriteBarrierMode mode = kDefaultMode);
  template <typename = std::enable_if<kSupportsSmiElements>>
  inline void set(int index, Tagged<Smi> value, RelaxedStoreTag);
  inline void set(int index, Tagged<ElementT> value, ReleaseStoreTag,
                  WriteBarrierMode mode = kDefaultMode);
  template <typename = std::enable_if<kSupportsSmiElements>>
  inline void set(int index, Tagged<Smi> value, ReleaseStoreTag);
  inline void set(int index, Tagged<ElementT> value, SeqCstAccessTag,
                  WriteBarrierMode mode = kDefaultMode);
  template <typename = std::enable_if<kSupportsSmiElements>>
  inline void set(int index, Tagged<Smi> value, SeqCstAccessTag);

  inline Tagged<ElementT> swap(int index, Tagged<ElementT> value,
                               SeqCstAccessTag,
                               WriteBarrierMode mode = kDefaultMode);
  inline Tagged<ElementT> compare_and_swap(
      int index, Tagged<ElementT> expected, Tagged<ElementT> value,
      SeqCstAccessTag, WriteBarrierMode mode = kDefaultMode);

  // Move vs. Copy behaves like memmove vs. memcpy: for Move, the memory
  // regions may overlap, for Copy they must not overlap.
  inline static void MoveElements(Isolate* isolate, Tagged<Derived> dst,
                                  int dst_index, Tagged<Derived> src,
                                  int src_index, int len,
                                  WriteBarrierMode mode = kDefaultMode);
  inline static void CopyElements(Isolate* isolate, Tagged<Derived> dst,
                                  int dst_index, Tagged<Derived> src,
                                  int src_index, int len,
                                  WriteBarrierMode mode = kDefaultMode);

  // Right-trim the array.
  // Invariant: 0 < new_length <= length()
  inline void RightTrim(Isolate* isolate, int new_capacity);

  inline int AllocatedSize() const;
  static inline constexpr int SizeFor(int capacity) {
    return Shape::kHeaderSize + capacity * Shape::kElementSize;
  }
  static inline constexpr int OffsetOfElementAt(int index) {
    return SizeFor(index);
  }

  // Gives access to raw memory which stores the array's data.
  inline ObjectSlot RawFieldOfFirstElement() const;
  inline ObjectSlot RawFieldOfElementAt(int index) const;

  // Maximal allowed capacity, in number of elements. Chosen s.t. the size fits
  // into a Smi which is necessary for being able to create a free space
  // filler.
  // TODO(jgruber): The kMaxCapacity could be larger (`(Smi::kMaxValue -
  // Shape::kHeaderSize) / Shape::kElementSize`), but our tests rely on a
  // smaller maximum to avoid timeouts.
  static constexpr int kMaxCapacity =
      128 * MB - kHeaderSize / Shape::kElementSize;
  static_assert(Smi::IsValid(SizeFor(kMaxCapacity)));

  // Maximally allowed length for regular (non large object space) object.
  static constexpr int kMaxRegularCapacity =
      (kMaxRegularHeapObjectSize - Shape::kHeaderSize) / Shape::kElementSize;
  static_assert(kMaxRegularCapacity < kMaxCapacity);

  // Object layout.
  static constexpr int kCapacityOffset = Shape::kCapacityOffset;
  static constexpr int kHeaderSize = Shape::kHeaderSize;

 protected:
  template <class IsolateT>
  static Handle<Derived> Allocate(
      IsolateT* isolate, int capacity,
      base::Optional<DisallowGarbageCollection>* no_gc_out,
      AllocationType allocation = AllocationType::kYoung);

  static constexpr int NewCapacityForIndex(int index, int old_capacity);

  inline bool IsInBounds(int index) const;
  inline bool IsCowArray() const;
};

class TaggedArrayShape final : public AllStatic {
 public:
  static constexpr int kElementSize = kTaggedSize;
  using ElementT = Object;
  static constexpr RootIndex kMapRootIndex = RootIndex::kFixedArrayMap;
  static constexpr bool kLengthEqualsCapacity = true;

#define FIELD_LIST(V)                                                   \
  V(kCapacityOffset, kTaggedSize)                                       \
  V(kUnalignedHeaderSize, OBJECT_POINTER_PADDING(kUnalignedHeaderSize)) \
  V(kHeaderSize, 0)
  DEFINE_FIELD_OFFSET_CONSTANTS(HeapObject::kHeaderSize, FIELD_LIST)
#undef FIELD_LIST
};

// FixedArray describes fixed-sized arrays with element type Object.
class FixedArray : public TaggedArrayBase<FixedArray, TaggedArrayShape> {
  using Super = TaggedArrayBase<FixedArray, TaggedArrayShape>;
  OBJECT_CONSTRUCTORS(FixedArray, Super);

 public:
  template <class IsolateT>
  static inline Handle<FixedArray> New(
      IsolateT* isolate, int capacity,
      AllocationType allocation = AllocationType::kYoung);

  using Super::CopyElements;
  using Super::MoveElements;

  // TODO(jgruber): Only needed for FixedArrays used as JSObject elements.
  inline void MoveElements(Isolate* isolate, int dst_index, int src_index,
                           int len, WriteBarrierMode mode);
  inline void CopyElements(Isolate* isolate, int dst_index,
                           Tagged<FixedArray> src, int src_index, int len,
                           WriteBarrierMode mode);

  // Return a grown copy if the index is bigger than the array's length.
  V8_EXPORT_PRIVATE static Handle<FixedArray> SetAndGrow(
      Isolate* isolate, Handle<FixedArray> array, int index,
      Handle<Object> value);

  // Right-trim the array.
  // Invariant: 0 < new_length <= length()
  V8_EXPORT_PRIVATE void RightTrim(Isolate* isolate, int new_capacity);
  // Right-trims the array, and canonicalizes length 0 to empty_fixed_array.
  static Handle<FixedArray> RightTrimOrEmpty(Isolate* isolate,
                                             Handle<FixedArray> array,
                                             int new_length);

  // TODO(jgruber): Only needed for FixedArrays used as JSObject elements.
  inline void FillWithHoles(int from, int to);

  // For compatibility with FixedDoubleArray:
  // TODO(jgruber): Only needed for FixedArrays used as JSObject elements.
  inline bool is_the_hole(Isolate* isolate, int index);
  inline void set_the_hole(Isolate* isolate, int index);
  inline void set_the_hole(ReadOnlyRoots ro_roots, int index);

  static_assert(kHeaderSize == Internals::kFixedArrayHeaderSize);

  DECL_CAST(FixedArray)
  DECL_PRINTER(FixedArray)
  DECL_VERIFIER(FixedArray)

  class BodyDescriptor;

  static constexpr int kLengthOffset = FixedArray::Shape::kCapacityOffset;
  static constexpr int kMaxLength = FixedArray::kMaxCapacity;
  static constexpr int kMaxRegularLength = FixedArray::kMaxRegularCapacity;

 private:
  inline static Handle<FixedArray> Resize(
      Isolate* isolate, Handle<FixedArray> xs, int new_capacity,
      AllocationType allocation = AllocationType::kYoung,
      WriteBarrierMode mode = UPDATE_WRITE_BARRIER);
};

// FixedArray alias added only because of IsFixedArrayExact() predicate, which
// checks for the exact instance type FIXED_ARRAY_TYPE instead of a range
// check: [FIRST_FIXED_ARRAY_TYPE, LAST_FIXED_ARRAY_TYPE].
class FixedArrayExact final : public FixedArray {};

// Common superclass for FixedArrays that allow implementations to share common
// accessors and some code paths. Note that due to single-inheritance
// restrictions, it is not part of the actual type hierarchy. Instead, we slot
// it in with manual is_subtype specializations in tagged.h.
// TODO(jgruber): This class is really specific to FixedArrays used as
// elements backing stores and should not be part of the common FixedArray
// hierarchy.
class FixedArrayBase : public HeapObject {
  OBJECT_CONSTRUCTORS(FixedArrayBase, HeapObject);

 public:
  // TODO(jgruber): Remove these `length` accessors once all subclasses have
  // been ported to use TaggedArrayBase or similar.
  inline int length() const;
  inline int length(AcquireLoadTag tag) const;
  inline void set_length(int value);
  inline void set_length(int value, ReleaseStoreTag tag);
  static constexpr int kLengthOffset = HeapObject::kHeaderSize;
  static constexpr int kHeaderSize = kLengthOffset + kTaggedSize;
  static constexpr int kMaxLength = FixedArray::kMaxCapacity;
  static constexpr int kMaxRegularLength = FixedArray::kMaxRegularCapacity;

  static int GetMaxLengthForNewSpaceAllocation(ElementsKind kind);

  V8_EXPORT_PRIVATE bool IsCowArray() const;

  // Maximal allowed size, in bytes, of a single FixedArrayBase. Prevents
  // overflowing size computations, as well as extreme memory consumption.
  static constexpr int kMaxSize = 128 * kTaggedSize * MB;
  static_assert(Smi::IsValid(kMaxSize));

  DECL_CAST(FixedArrayBase)
  DECL_VERIFIER(FixedArrayBase)
};

// Derived: must have a Smi slot at kCapacityOffset.
template <class Derived, class ShapeT>
class PrimitiveArrayBase : public HeapObject {
  OBJECT_CONSTRUCTORS(PrimitiveArrayBase, HeapObject);

  using ElementT = typename ShapeT::ElementT;
  static_assert(!is_subtype_v<ElementT, Object>);

 public:
  using Shape = ShapeT;

  inline int length() const;
  inline int length(AcquireLoadTag) const;
  inline void set_length(int value);
  inline void set_length(int value, ReleaseStoreTag);

  inline ElementT get(int index) const;
  inline void set(int index, ElementT value);

  inline int AllocatedSize() const;
  static inline constexpr int SizeFor(int length) {
    return OBJECT_POINTER_ALIGN(OffsetOfElementAt(length));
  }
  static inline constexpr int OffsetOfElementAt(int index) {
    return Shape::kHeaderSize + index * Shape::kElementSize;
  }

  // Gives access to raw memory which stores the array's data.
  // Note that on 32-bit archs and on 64-bit platforms with pointer compression
  // the pointers to 8-byte size elements are not guaranteed to be aligned.
  inline ElementT* AddressOfElementAt(int index) const;
  inline ElementT* begin() const;
  inline ElementT* end() const;
  inline int DataSize() const;

  static inline Tagged<Derived> FromAddressOfFirstElement(Address address);

  // Maximal allowed length, in number of elements. Chosen s.t. the size fits
  // into a Smi which is necessary for being able to create a free space
  // filler.
  // TODO(jgruber): The kMaxLength could be larger (`(Smi::kMaxValue -
  // Shape::kHeaderSize) / Shape::kElementSize`), but our tests rely on a
  // smaller maximum to avoid timeouts.
  static constexpr int kMaxLength =
      (FixedArrayBase::kMaxSize - kHeaderSize) / Shape::kElementSize;
  static_assert(Smi::IsValid(SizeFor(kMaxLength)));

  // Maximally allowed length for regular (non large object space) object.
  static constexpr int kMaxRegularLength =
      (kMaxRegularHeapObjectSize - Shape::kHeaderSize) / Shape::kElementSize;
  static_assert(kMaxRegularLength < kMaxLength);

  // Object layout.
  static constexpr int kLengthOffset = Shape::kLengthOffset;
  static constexpr int kHeaderSize = Shape::kHeaderSize;

 protected:
  template <class IsolateT>
  static Handle<Derived> Allocate(
      IsolateT* isolate, int length,
      base::Optional<DisallowGarbageCollection>* no_gc_out,
      AllocationType allocation = AllocationType::kYoung);

  inline bool IsInBounds(int index) const;
};

class FixedDoubleArrayShape final : public AllStatic {
 public:
  static constexpr int kElementSize = kDoubleSize;
  using ElementT = double;
  static constexpr int kLengthOffset = HeapObject::kHeaderSize;
  static constexpr int kHeaderSize = kLengthOffset + kTaggedSize;
  static constexpr RootIndex kMapRootIndex = RootIndex::kFixedDoubleArrayMap;
};

// FixedDoubleArray describes fixed-sized arrays with element type double.
class FixedDoubleArray
    : public PrimitiveArrayBase<FixedDoubleArray, FixedDoubleArrayShape> {
  using Super = PrimitiveArrayBase<FixedDoubleArray, FixedDoubleArrayShape>;
  OBJECT_CONSTRUCTORS(FixedDoubleArray, Super);

 public:
  // Note this returns FixedArrayBase due to canonicalization to
  // empty_fixed_array.
  template <class IsolateT>
  static inline Handle<FixedArrayBase> New(
      IsolateT* isolate, int capacity,
      AllocationType allocation = AllocationType::kYoung);

  // Setter and getter for elements.
  inline double get_scalar(int index);
  inline uint64_t get_representation(int index);
  static inline Handle<Object> get(Tagged<FixedDoubleArray> array, int index,
                                   Isolate* isolate);
  inline void set(int index, double value);

  inline void set_the_hole(Isolate* isolate, int index);
  inline void set_the_hole(int index);
  inline bool is_the_hole(Isolate* isolate, int index);
  inline bool is_the_hole(int index);

  inline void MoveElements(Isolate* isolate, int dst_index, int src_index,
                           int len, WriteBarrierMode /* unused */);

  inline void FillWithHoles(int from, int to);

  DECL_CAST(FixedDoubleArray)
  DECL_PRINTER(FixedDoubleArray)
  DECL_VERIFIER(FixedDoubleArray)

  class BodyDescriptor;
};

// WeakFixedArray describes fixed-sized arrays with element type
// MaybeObject.
class WeakFixedArray
    : public TorqueGeneratedWeakFixedArray<WeakFixedArray, HeapObject> {
 public:
  inline MaybeObject Get(int index) const;
  inline MaybeObject Get(PtrComprCageBase cage_base, int index) const;

  inline void Set(
      int index, MaybeObject value,
      WriteBarrierMode mode = WriteBarrierMode::UPDATE_WRITE_BARRIER);

  static inline Handle<WeakFixedArray> EnsureSpace(Isolate* isolate,
                                                   Handle<WeakFixedArray> array,
                                                   int length);

  // Forward declare the non-atomic (set_)length defined in torque.
  using TorqueGeneratedWeakFixedArray::length;
  using TorqueGeneratedWeakFixedArray::set_length;
  DECL_RELEASE_ACQUIRE_INT_ACCESSORS(length)

  // Gives access to raw memory which stores the array's data.
  inline MaybeObjectSlot data_start();

  inline MaybeObjectSlot RawFieldOfElementAt(int index);

  inline void CopyElements(Isolate* isolate, int dst_index,
                           Tagged<WeakFixedArray> src, int src_index, int len,
                           WriteBarrierMode mode);

  DECL_PRINTER(WeakFixedArray)
  DECL_VERIFIER(WeakFixedArray)

  class BodyDescriptor;

  static const int kMaxLength =
      (FixedArrayBase::kMaxSize - kHeaderSize) / kTaggedSize;
  static_assert(Internals::IsValidSmi(kMaxLength),
                "WeakFixedArray maxLength not a Smi");

  inline int AllocatedSize() const;

  static int OffsetOfElementAt(int index) {
    static_assert(kHeaderSize == SizeFor(0));
    return SizeFor(index);
  }

 private:
  friend class Heap;

  static const int kFirstIndex = 1;

  TQ_OBJECT_CONSTRUCTORS(WeakFixedArray)
};

// WeakArrayList is like a WeakFixedArray with static convenience methods for
// adding more elements. length() returns the number of elements in the list and
// capacity() returns the allocated size. The number of elements is stored at
// kLengthOffset and is updated with every insertion. The array grows
// dynamically with O(1) amortized insertion.
class WeakArrayList
    : public TorqueGeneratedWeakArrayList<WeakArrayList, HeapObject> {
 public:
  NEVER_READ_ONLY_SPACE
  DECL_PRINTER(WeakArrayList)

  V8_EXPORT_PRIVATE static Handle<WeakArrayList> AddToEnd(
      Isolate* isolate, Handle<WeakArrayList> array, MaybeObjectHandle value);

  // A version that adds to elements. This ensures that the elements are
  // inserted atomically w.r.t GC.
  V8_EXPORT_PRIVATE static Handle<WeakArrayList> AddToEnd(
      Isolate* isolate, Handle<WeakArrayList> array, MaybeObjectHandle value1,
      Tagged<Smi> value2);

  // Appends an element to the array and possibly compacts and shrinks live weak
  // references to the start of the collection. Only use this method when
  // indices to elements can change.
  static Handle<WeakArrayList> Append(
      Isolate* isolate, Handle<WeakArrayList> array, MaybeObjectHandle value,
      AllocationType allocation = AllocationType::kYoung);

  // Compact weak references to the beginning of the array.
  V8_EXPORT_PRIVATE void Compact(Isolate* isolate);

  inline MaybeObject Get(int index) const;
  inline MaybeObject Get(PtrComprCageBase cage_base, int index) const;

  // Set the element at index to obj. The underlying array must be large enough.
  // If you need to grow the WeakArrayList, use the static AddToEnd() method
  // instead.
  inline void Set(int index, MaybeObject value,
                  WriteBarrierMode mode = UPDATE_WRITE_BARRIER);
  inline void Set(int index, Tagged<Smi> value);

  static constexpr int SizeForCapacity(int capacity) {
    return SizeFor(capacity);
  }

  static constexpr int CapacityForLength(int length) {
    return length + std::max(length / 2, 2);
  }

  // Gives access to raw memory which stores the array's data.
  inline MaybeObjectSlot data_start();

  inline void CopyElements(Isolate* isolate, int dst_index,
                           Tagged<WeakArrayList> src, int src_index, int len,
                           WriteBarrierMode mode);

  V8_EXPORT_PRIVATE bool IsFull() const;

  inline int AllocatedSize() const;

  class BodyDescriptor;

  static const int kMaxCapacity =
      (FixedArrayBase::kMaxSize - kHeaderSize) / kTaggedSize;

  static Handle<WeakArrayList> EnsureSpace(
      Isolate* isolate, Handle<WeakArrayList> array, int length,
      AllocationType allocation = AllocationType::kYoung);

  // Returns the number of non-cleaned weak references in the array.
  int CountLiveWeakReferences() const;

  // Returns the number of non-cleaned elements in the array.
  int CountLiveElements() const;

  // Returns whether an entry was found and removed. Will move the elements
  // around in the array - this method can only be used in cases where the user
  // doesn't care about the indices! Users should make sure there are no
  // duplicates.
  V8_EXPORT_PRIVATE bool RemoveOne(MaybeObjectHandle value);

  // Searches the array (linear time) and returns whether it contains the value.
  V8_EXPORT_PRIVATE bool Contains(MaybeObject value);

  class Iterator;

 private:
  static int OffsetOfElementAt(int index) {
    return kHeaderSize + index * kTaggedSize;
  }

  TQ_OBJECT_CONSTRUCTORS(WeakArrayList)
};

class WeakArrayList::Iterator {
 public:
  explicit Iterator(Tagged<WeakArrayList> array) : index_(0), array_(array) {}
  Iterator(const Iterator&) = delete;
  Iterator& operator=(const Iterator&) = delete;

  inline Tagged<HeapObject> Next();

 private:
  int index_;
  Tagged<WeakArrayList> array_;
  DISALLOW_GARBAGE_COLLECTION(no_gc_)
};

class ArrayListShape final : public AllStatic {
 public:
  static constexpr int kElementSize = kTaggedSize;
  using ElementT = Object;
  static constexpr RootIndex kMapRootIndex = RootIndex::kArrayListMap;
  static constexpr bool kLengthEqualsCapacity = false;

#define FIELD_LIST(V)                                                   \
  V(kCapacityOffset, kTaggedSize)                                       \
  V(kLengthOffset, kTaggedSize)                                         \
  V(kUnalignedHeaderSize, OBJECT_POINTER_PADDING(kUnalignedHeaderSize)) \
  V(kHeaderSize, 0)
  DEFINE_FIELD_OFFSET_CONSTANTS(HeapObject::kHeaderSize, FIELD_LIST)
#undef FIELD_LIST
};

// A generic array that grows dynamically with O(1) amortized insertion.
class ArrayList : public TaggedArrayBase<ArrayList, ArrayListShape> {
  using Super = TaggedArrayBase<ArrayList, ArrayListShape>;
  OBJECT_CONSTRUCTORS(ArrayList, Super);

 public:
  using Shape = ArrayListShape;

  template <class IsolateT>
  static inline Handle<ArrayList> New(
      IsolateT* isolate, int capacity,
      AllocationType allocation = AllocationType::kYoung);

  inline int length() const;
  inline void set_length(int value);

  V8_EXPORT_PRIVATE static Handle<ArrayList> Add(
      Isolate* isolate, Handle<ArrayList> array, Tagged<Smi> obj,
      AllocationType allocation = AllocationType::kYoung);
  V8_EXPORT_PRIVATE static Handle<ArrayList> Add(
      Isolate* isolate, Handle<ArrayList> array, Handle<Object> obj,
      AllocationType allocation = AllocationType::kYoung);
  V8_EXPORT_PRIVATE static Handle<ArrayList> Add(
      Isolate* isolate, Handle<ArrayList> array, Handle<Object> obj0,
      Handle<Object> obj1, AllocationType allocation = AllocationType::kYoung);

  V8_EXPORT_PRIVATE static Handle<FixedArray> ToFixedArray(
      Isolate* isolate, Handle<ArrayList> array,
      AllocationType allocation = AllocationType::kYoung);

  // Right-trim the array.
  // Invariant: 0 < new_length <= length()
  void RightTrim(Isolate* isolate, int new_capacity);

  DECL_CAST(ArrayList)
  DECL_PRINTER(ArrayList)
  DECL_VERIFIER(ArrayList)

  class BodyDescriptor;

 private:
  static Handle<ArrayList> EnsureSpace(
      Isolate* isolate, Handle<ArrayList> array, int length,
      AllocationType allocation = AllocationType::kYoung);
};

enum SearchMode { ALL_ENTRIES, VALID_ENTRIES };

template <SearchMode search_mode, typename T>
inline int Search(T* array, Tagged<Name> name, int valid_entries = 0,
                  int* out_insertion_index = nullptr,
                  bool concurrent_search = false);

class ByteArrayShape final : public AllStatic {
 public:
  static constexpr int kElementSize = kUInt8Size;
  using ElementT = uint8_t;
  static constexpr RootIndex kMapRootIndex = RootIndex::kByteArrayMap;

#define FIELD_LIST(V)                                                   \
  V(kLengthOffset, kTaggedSize)                                         \
  V(kUnalignedHeaderSize, OBJECT_POINTER_PADDING(kUnalignedHeaderSize)) \
  V(kHeaderSize, 0)

  DEFINE_FIELD_OFFSET_CONSTANTS(HeapObject::kHeaderSize, FIELD_LIST)
#undef FIELD_LIST
};

// ByteArray represents fixed sized arrays containing raw bytes that will not
// be scanned by the garbage collector.
class ByteArray : public PrimitiveArrayBase<ByteArray, ByteArrayShape> {
  using Super = PrimitiveArrayBase<ByteArray, ByteArrayShape>;
  OBJECT_CONSTRUCTORS(ByteArray, Super);

 public:
  using Shape = ByteArrayShape;

  template <class IsolateT>
  static inline Handle<ByteArray> New(
      IsolateT* isolate, int capacity,
      AllocationType allocation = AllocationType::kYoung);

  inline uint32_t get_int(int offset) const;
  inline void set_int(int offset, uint32_t value);

  // Given the full object size in bytes, return the length that should be
  // passed to New s.t. an object of the same size is created.
  static constexpr int LengthFor(int size_in_bytes) {
    DCHECK(IsAligned(size_in_bytes, kTaggedSize));
    DCHECK_GE(size_in_bytes, Shape::kHeaderSize);
    return size_in_bytes - Shape::kHeaderSize;
  }

  DECL_CAST(ByteArray)
  DECL_PRINTER(ByteArray)
  DECL_VERIFIER(ByteArray)

  class BodyDescriptor;
};

// Convenience class for treating a ByteArray as array of fixed-size integers.
template <typename T>
class FixedIntegerArray : public ByteArray {
  static_assert(std::is_integral<T>::value);

 public:
  static Handle<FixedIntegerArray<T>> New(
      Isolate* isolate, int length,
      AllocationType allocation = AllocationType::kYoung);

  // Get/set the contents of this array.
  T get(int index) const;
  void set(int index, T value);

  // Code Generation support.
  static constexpr int OffsetOfElementAt(int index) {
    return kHeaderSize + index * sizeof(T);
  }

  inline int length() const;

  DECL_CAST(FixedIntegerArray<T>)

  OBJECT_CONSTRUCTORS(FixedIntegerArray<T>, ByteArray);
};

using FixedInt8Array = FixedIntegerArray<int8_t>;
using FixedUInt8Array = FixedIntegerArray<uint8_t>;
using FixedInt16Array = FixedIntegerArray<int16_t>;
using FixedUInt16Array = FixedIntegerArray<uint16_t>;
using FixedInt32Array = FixedIntegerArray<int32_t>;
using FixedUInt32Array = FixedIntegerArray<uint32_t>;
using FixedInt64Array = FixedIntegerArray<int64_t>;
using FixedUInt64Array = FixedIntegerArray<uint64_t>;

// Use with care! Raw addresses on the heap are not safe in combination with
// the sandbox. Use an ExternalPointerArray instead. However, this can for
// example be used to store sandboxed pointers, which is safe.
class FixedAddressArray : public FixedIntegerArray<Address> {
 public:
  // Get/set a sandboxed pointer from this array.
  inline Address get_sandboxed_pointer(int offset) const;
  inline void set_sandboxed_pointer(int offset, Address value);

  static inline Handle<FixedAddressArray> New(
      Isolate* isolate, int length,
      AllocationType allocation = AllocationType::kYoung);

  DECL_CAST(FixedAddressArray)

  OBJECT_CONSTRUCTORS(FixedAddressArray, FixedIntegerArray<Address>);
};

// An array containing external pointers.
// When the sandbox is off, this will simply contain system-pointer sized words.
// Otherwise, it contains external pointer handles, i.e. indices into the
// external pointer table.
// This class uses lazily-initialized external pointer slots. As such, its
// content can simply be zero-initialized, and the external pointer table
// entries are only allocated when an element is written to for the first time.
class ExternalPointerArray : public FixedArrayBase {
 public:
  template <ExternalPointerTag tag>
  inline Address get(int index, Isolate* isolate);
  template <ExternalPointerTag tag>
  inline void set(int index, Isolate* isolate, Address value);
  inline void clear(int index);

  static inline Handle<ExternalPointerArray> New(
      Isolate* isolate, int length,
      AllocationType allocation = AllocationType::kYoung);

  static constexpr int SizeFor(int length) {
    return kHeaderSize + length * kExternalPointerSlotSize;
  }

  static constexpr int OffsetOfElementAt(int index) {
    return kHeaderSize + index * kExternalPointerSlotSize;
  }

  // Maximal length of a single ExternalPointerArray.
  static const int kMaxLength = FixedArrayBase::kMaxSize - kHeaderSize;
  static_assert(Internals::IsValidSmi(kMaxLength),
                "ExternalPointerArray maxLength not a Smi");

  class BodyDescriptor;

  DECL_CAST(ExternalPointerArray)
  DECL_PRINTER(ExternalPointerArray)
  DECL_VERIFIER(ExternalPointerArray)

  OBJECT_CONSTRUCTORS(ExternalPointerArray, FixedArrayBase);
};

// Wrapper class for ByteArray which can store arbitrary C++ classes, as long
// as they can be copied with memcpy.
template <class T>
class PodArray : public ByteArray {
 public:
  static Handle<PodArray<T>> New(
      Isolate* isolate, int length,
      AllocationType allocation = AllocationType::kYoung);
  static Handle<PodArray<T>> New(
      LocalIsolate* isolate, int length,
      AllocationType allocation = AllocationType::kOld);

  void copy_out(int index, T* result, int length) {
    MemCopy(result, AddressOfElementAt(index * sizeof(T)), length * sizeof(T));
  }

  void copy_in(int index, const T* buffer, int length) {
    MemCopy(AddressOfElementAt(index * sizeof(T)), buffer, length * sizeof(T));
  }

  bool matches(const T* buffer, int length) {
    DCHECK_LE(length, this->length());
    return memcmp(begin(), buffer, length * sizeof(T)) == 0;
  }

  bool matches(int offset, const T* buffer, int length) {
    DCHECK_LE(offset, this->length());
    DCHECK_LE(offset + length, this->length());
    return memcmp(begin() + sizeof(T) * offset, buffer, length * sizeof(T)) ==
           0;
  }

  T get(int index) {
    T result;
    copy_out(index, &result, 1);
    return result;
  }

  void set(int index, const T& value) { copy_in(index, &value, 1); }

  inline int length() const;
  DECL_CAST(PodArray<T>)

  OBJECT_CONSTRUCTORS(PodArray<T>, ByteArray);
};

}  // namespace internal
}  // namespace v8

#include "src/objects/object-macros-undef.h"

#endif  // V8_OBJECTS_FIXED_ARRAY_H_
