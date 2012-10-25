#ifndef _MULTI_ARRAY_CMSTRIDEREFARRAYBASE_H_
#define _MULTI_ARRAY_CMSTRIDEREFARRAYBASE_H_

#include "multi_array/MultiIndexRefArrayBase.h"
#include "multi_array/CMStrided2IndexMap.h"
#include "multi_array/SteppedRefArray.h"

namespace multi_array {

template<int DIM, typename ENTRY>
class CMContiguousRefArray;

template<int DIM, typename ENTRY>
class CMStrideRefArray;

/// CMStrideRefArrayBase is the base class of
/// CMStrideRefArray and CMContinuousRefArray
/// and inherits from MultiIndexRefArrayBase.  CMStrideRefArrayBase
/// specializes MultiIndexRefArrayBase with the index map
/// CMStrided2IndexMap.  The index map is the map from a
/// multidimensional tuple of dimension DIM to a one dimensional
/// storage index.  The particular index map CMStrided2IndexMap used
/// here organizes storage with strides such that:
/// - the values in the first dimension (column) are adjacent and
/// therefore iterate fastest.
/// - dimension i iterate faster than dimension i+1 for each dimension
/// (column or row).
/// So there are only strides for DIM-1 dimensions, the first
/// dimension has no stride.  This also explains the name of the
/// class.  The prefix CM refers to Column Major ordering, the next
/// prefix Stride refers to the fact that apart from the first
/// dimension there are strides for each dimension.  (There is a
/// CMSteppedRefArrayBase array, which is more general in this
/// respect.  CMSteppedRefArrayBase adds to CMStrideRefArrayBase also
/// strides for the first dimension.)  In a normal contiguous array,
/// the strides correspond to the total volume of the columns
/// contained with that dimension (and that is the way how the strides
/// are initialized in CMContiguousRefArray), but that is not
/// necessary (and hence the definition of CMStrideRefArray).  So for
/// example, entries tuple(1,2,3) tuple(2,2,3) are adjacent.  Column
/// major ordering is the normal ordering of Fortran and, in a way, is
/// the most logical way in which to order storage.  One way in which
/// this shows up is that the strides of CMStrided2IndexMap increase
/// with dimension.  (For row major ordering they decrease.)  However,
/// most main stream languages use row major ordering.

template <int DIM, typename ENTRY>
class CMStrideRefArrayBase: public MultiIndexRefArrayBase<DIM, ENTRY,
  CMStrided2IndexMap<DIM> >
{
protected:

  inline CMStrideRefArrayBase() {
    assert(DIM > 0);
  }

  inline CMStrideRefArrayBase(const CloseOpenInterval<DIM,int>& dmn):
    MultiIndexRefArrayBase<DIM,ENTRY, CMStrided2IndexMap<DIM> >(dmn,
    CMStrided2IndexMap<DIM>(Tuple<DIM,int>(0)) )
  {
    assert(DIM > 0);
  }

  inline CMStrideRefArrayBase(const CloseOpenInterval<DIM,int>& dmn, ENTRY* dt):
    MultiIndexRefArrayBase<DIM,ENTRY, CMStrided2IndexMap<DIM> >(dmn,
    CMStrided2IndexMap<DIM>(dmn.size()), dt)
  {}

public:

  inline CMStrideRefArrayBase(const CMStrideRefArrayBase& other):
    MultiIndexRefArrayBase<DIM,ENTRY,CMStrided2IndexMap<DIM> >(other)
  {}

  inline CMStrideRefArrayBase(const CloseOpenInterval<DIM,int>& dmn,
    const CMStrided2IndexMap<DIM>& indxMp, ENTRY* dt):
    MultiIndexRefArrayBase<DIM,ENTRY,CMStrided2IndexMap<DIM> >(dmn,
      indxMp,dt)
  {}

  inline CMStrideRefArray<DIM,ENTRY> slice(const CloseOpenInterval<DIM,int>&
    sliceDomain)
  {
    assert(domain().contains(sliceDomain));
    return CMStrideRefArray<DIM,ENTRY>(sliceDomain, m_indexMap, &operator[](
      sliceDomain.low()) );
  }

  inline const CMStrideRefArray<DIM,ENTRY> slice(const CloseOpenInterval<DIM,int>&
    sliceDomain) const
  {
    assert(domain().contains(sliceDomain));
    return CMStrideRefArray<DIM,ENTRY>(sliceDomain, m_indexMap, &operator[](
      sliceDomain.low()) );
  }

  inline SteppedRefArray<DIM,ENTRY> transpose() {
    return SteppedRefArray<DIM,ENTRY>(domain(), m_indexMap.transpose(), data);
  }

  inline const SteppedRefArray<DIM,ENTRY> transpose() const {
    return SteppedRefArray<DIM,ENTRY>(domain(), m_indexMap.transpose(), data);
  }

  inline SteppedRefArray<1,ENTRY> row(const Tuple<DIM-1,int>& rowPos) {
    Tuple<1,int> strides(m_indexMap.strides()[DIM-2]);
    Tuple<DIM,int> offset = Tuple<DIM,int>(rowPos,domain().low(DIM-1));
    assert(domain().contains(offset));
    return SteppedRefArray<1,ENTRY>(domain(DIM-1), Step2IndexMap<1>(
      tuple(domain().size(0)), strides), &operator[](offset) );
  }

  inline const SteppedRefArray<1,ENTRY> row(const Tuple<DIM-1,int>& rowPos) const {
    Tuple<1,int> strides(m_indexMap.strides()[DIM-2]);
    Tuple<DIM,int> offset = Tuple<DIM,int>(rowPos,domain().low(DIM-1));
    assert(domain().contains(offset));
    return SteppedRefArray<1,ENTRY>(domain(DIM-1), Step2IndexMap<1>(
      tuple(domain().size(0)), strides), &operator[](offset) );
  }

  inline CMContiguousRefArray<1,ENTRY> column(const Tuple<DIM-1,int> columnPos) {
    Tuple<DIM,int> offset = Tuple<DIM,int>(domain().low(0),columnPos);
    assert(domain().contains(offset));
    return CMContiguousRefArray<1,ENTRY>(domain()[0], &operator[](offset) );
  }

  inline const CMContiguousRefArray<1,ENTRY> column(const Tuple<DIM-1,int> columnPos) const {
    Tuple<DIM,int> offset = Tuple<DIM,int>(domain().low(0),columnPos);
    assert(domain().contains(offset));
    return CMContiguousRefArray<1,ENTRY>(domain()[0], &operator[](offset) );
  }
};

} // namespace multi_array

#endif
