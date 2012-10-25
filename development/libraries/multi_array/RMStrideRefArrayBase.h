#ifndef _MULTI_ARRAY_RMSTRIDEREFARRAYBASE_H_
#define _MULTI_ARRAY_RMSTRIDEREFARRAYBASE_H_

#include "multi_array/MultiIndexRefArrayBase.h"
#include "multi_array/RMStrided2IndexMap.h"
#include "multi_array/SteppedRefArray.h"

namespace multi_array {

template<int DIM, typename ENTRY>
class RMContiguousRefArray;

template<int DIM, typename ENTRY>
class RMStrideRefArray;

/// RMStrideRefArrayBase is the base class of
/// RMStrideRefArray and RMContinuousRefArray
/// and inherits from MultiIndexRefArrayBase.  RMStrideRefArrayBase
/// specializes MultiIndexRefArrayBase with the index map
/// RMStrided2IndexMap.  The index map is the map from a
/// multidimensional tuple of dimension DIM to a one dimensional
/// storage index.  The particular index map RMStrided2IndexMap used
/// here organizes storage with strides such that:
/// - the values in the last dimension (row) are adjacent and
/// therefore iterate fastest.
/// - dimension i iterate faster than dimension i+1 for each dimension
/// (column or row).
/// So there are only strides for DIM-1 dimensions, the last
/// dimension has no stride.  This also explains the name of the
/// class.  The prefix RM refers to Row Major ordering, the next
/// prefix Stride refers to the fact that apart from the last
/// dimension there are strides for each dimension.  (There is a
/// RMSteppedRefArrayBase array, which is more general in this
/// respect.  RMSteppedRefArrayBase adds to RMStrideRefArrayBase also
/// strides for the last dimension.)  In a normal contiguous array,
/// the strides correspond to the total volume of the rows contained
/// with that dimension (and that is the way how the strides are
/// initialized in RMContiguousRefArray), but that is not necessary
/// (and hence the definition of RMStrideRefArray).  So for example,
/// entries tuple(1,2,3) tuple(1,2,4) are adjacent.  Row major
/// ordering is the normal ordering of most main stream languages
/// (with the notable exception of Fortran).  However, it is not the
/// most logical way in which to order storage.  Column major ordering
/// is in a way more logical.  One way in which this shows up is that
/// the strides given decrease with dimension.

template <int DIM, typename ENTRY>
class RMStrideRefArrayBase: public MultiIndexRefArrayBase<DIM, ENTRY,
  RMStrided2IndexMap<DIM> >
{
protected:

  inline RMStrideRefArrayBase() {
    assert(DIM > 0);
  }

  inline RMStrideRefArrayBase(const CloseOpenInterval<DIM,int>& dmn):
    MultiIndexRefArrayBase<DIM,ENTRY, RMStrided2IndexMap<DIM> >(dmn,
    RMStrided2IndexMap<DIM>( Tuple<DIM,int>(0) ) )
  {
    assert(DIM > 0);
  }

  inline RMStrideRefArrayBase(const CloseOpenInterval<DIM,int>& dmn, ENTRY* dt):
    MultiIndexRefArrayBase<DIM,ENTRY, RMStrided2IndexMap<DIM> >(dmn,
    RMStrided2IndexMap<DIM>(dmn.size()), dt)
  {}

public:

  inline RMStrideRefArrayBase(const RMStrideRefArrayBase& other):
    MultiIndexRefArrayBase<DIM,ENTRY,RMStrided2IndexMap<DIM> >(other)
  {}

  inline RMStrideRefArrayBase(const CloseOpenInterval<DIM,int>& dmn,
    const RMStrided2IndexMap<DIM>& indxMp, ENTRY* dt):
    MultiIndexRefArrayBase<DIM,ENTRY,RMStrided2IndexMap<DIM> >(dmn,
      indxMp, dt)
  {}

  inline SteppedRefArray<DIM,ENTRY> transpose() {
    return SteppedRefArray<DIM,ENTRY>(domain(), m_indexMap.transpose(), data);
  }

  inline const SteppedRefArray<DIM,ENTRY> transpose() const {
    return SteppedRefArray<DIM,ENTRY>(domain(), m_indexMap.transpose(), data);
  }

  inline RMStrideRefArray<DIM,ENTRY> slice(const CloseOpenInterval<DIM,int>&
    sliceDomain)
  {
    assert(domain().contains(sliceDomain));
    return RMStrideRefArray<DIM,ENTRY>(sliceDomain, m_indexMap, &operator[](
      sliceDomain.low()) );
  }

  inline const RMStrideRefArray<DIM,ENTRY> slice(const CloseOpenInterval<DIM,int>&
    sliceDomain) const
  {
    assert(domain().contains(sliceDomain));
    return RMStrideRefArray<DIM,ENTRY>(sliceDomain, m_indexMap, &operator[](
      sliceDomain.low()) );
  }

  inline RMContiguousRefArray<1,ENTRY> row(const Tuple<DIM-1,int>& rowPos) {
    Tuple<DIM,int> offset = Tuple<DIM,int>(rowPos,domain().low(DIM-1));
    assert(domain().contains(offset));
    return RMContiguousRefArray<1,ENTRY>(domain()[DIM-1], &operator[](offset) );
  }

  inline const RMContiguousRefArray<1,ENTRY> row(const Tuple<DIM-1,int>& rowPos) const {
    Tuple<DIM,int> offset = Tuple<DIM,int>(rowPos,domain().low(DIM-1));
    assert(domain().contains(offset));
    return RMContiguousRefArray<1,ENTRY>(domain()[DIM-1], &operator[](offset) );
  }

  inline SteppedRefArray<1,ENTRY> column(const Tuple<DIM-1,int>& columnPos) {
    Tuple<1,int> strides(m_indexMap.strides()[0]);
    Tuple<DIM,int> offset = Tuple<DIM,int>(domain().low(0),columnPos);
    assert(domain().contains(offset));
    return SteppedRefArray<1,ENTRY>(domain(0), Step2IndexMap<1>(
      tuple(domain().size(0)), strides), &operator[](offset) );
  }

  inline const SteppedRefArray<1,ENTRY> column(const Tuple<DIM-1,int>& columnPos) const {
    Tuple<1,int> strides(m_indexMap.strides()[0]);
    Tuple<DIM,int> offset = Tuple<DIM,int>(domain.low(0),columnPos);
    assert(domain().contains(offset));
    return SteppedRefArray<1,ENTRY>(domain(0), Step2IndexMap<1>(
      tuple(domain().size(0)), strides), &operator[](offset) );
  }
};

} // namespace multi_array

#endif
