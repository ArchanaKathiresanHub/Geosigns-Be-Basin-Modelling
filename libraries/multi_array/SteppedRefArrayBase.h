#ifndef _MULTI_ARRAY_STEPPEDREFARRAYBASE_H_
#define _MULTI_ARRAY_STEPPEDREFARRAYBASE_H_

#include "multi_array/IndexMultiRefArrayBase.h"
#include "multi_array/Step2IndexMap.h"

namespace multi_array {

template<int DIM, typename ENTRY>
class RMContiguousRefArray;

template <int DIM, typename ENTRY>
class RMMultrideRefArray;

template <int DIM, typename ENTRY>
class SteppedRefArray;

/// SteppedRefArrayBase is the base class of
/// SteppedRefArray and inherits from IndexMultiRefArrayBase.
/// SteppedRefArrayBase specializes IndexMultiRefArrayBase with the
/// index map Step2IndexMap.  The index map is the map from a
/// multidimensional tuple of dimension DIM to a one dimensional
/// storage index.  The particular index map Step2IndexMap used here
/// organizes storage with steps such that:
/// - the steps between each consecutive value in any of the
/// dimensions are constant.
/// In contrast to RMStridedRefArrayBase and CMStridedRefArrayBase,
/// there are therefore steps associated with each of the dimensions,
/// so in comparison to those classes there are strides for each of
/// the DIM dimensions.  For the purpose of SteppedRefArray, it is
/// furthermore also irrelevant whether the last dimension (rows)
/// iterate fastest, or the first dimension (columns).  So
/// SteppedRefArray transcends row and column major ordering.  For
/// instance, SteppedRefArrayBase would therefore be the array one
/// would use if one would only be interested in every second element
/// of the array.  However, this functionality is currently not
/// provided for in multi_array.  The only place where
/// SteppedRefArrayBase is currently used is as the return type of the
/// transpose method of the RMStridedRefArrayBase and
/// CMStridedRefArrayBase classes.

template <int DIM, typename ENTRY>
class SteppedRefArrayBase: public IndexMultiRefArrayBase<DIM, ENTRY,
  Step2IndexMap<DIM> >
{
protected:

  inline SteppedRefArrayBase() {
    assert(DIM > 0);
  }

  inline SteppedRefArrayBase(const CloseOpenInterval<DIM,int>& domain):
    IndexMultiRefArrayBase<DIM,ENTRY, Step2IndexMap<DIM> >(domain,
    Step2IndexMap<DIM>( Tuple<DIM,int>::filled(0) ) )
  {
    assert(DIM > 0);
  }

  inline SteppedRefArrayBase(const CloseOpenInterval<DIM,int>& dmn,
    const Step2IndexMap<DIM>& indexMap):
    IndexMultiRefArrayBase<DIM,ENTRY, Step2IndexMap<DIM> >(dmn, indexMap)
  {
    assert(DIM > 0);
  }

public:

  inline SteppedRefArrayBase(const SteppedRefArrayBase& other):
    IndexMultiRefArrayBase<DIM,ENTRY,Step2IndexMap<DIM> >(other)
  {}

  inline SteppedRefArrayBase(const CloseOpenInterval<DIM,int>& dmn,
    const Step2IndexMap<DIM>& indexMap, ENTRY* dt):
    IndexMultiRefArrayBase<DIM,ENTRY,Step2IndexMap<DIM> >(dmn,
      indexMap,dt)
  {}

  SteppedRefArray<DIM,ENTRY> slice(const CloseOpenInterval<DIM,int>&
    sliceDomain)
  {
    assert(domain().contains(sliceDomain));
    return SteppedRefArray<DIM,ENTRY>(sliceDomain, m_indexMap, &operator[](
      sliceDomain.low()) );
  }

  const SteppedRefArray<DIM,ENTRY> slice(const CloseOpenInterval<DIM,int>&
    sliceDomain) const
  {
    assert(domain().contains(sliceDomain));
    return SteppedRefArray<DIM,ENTRY>(sliceDomain, m_indexMap, &operator[](
      sliceDomain.low()) );
  }

  SteppedRefArray<1,ENTRY> row(const Tuple<dim_min_1,int>& rowPos) {
    Tuple<1,int> strides(m_indexMap.strides()[DIM-1]);
    Tuple<DIM,int> offset = snoc(rowPos,domain().low(DIM-1));
    assert(domain().contains(offset));
    return SteppedRefArray<1,ENTRY>(domain()[DIM-1], Step2IndexMap<1>(
      domain.size()[0], strides), &operator[](offset) );
  }

  const SteppedRefArray<1,ENTRY> row(const Tuple<dim_min_1,int>& rowPos) const {
    Tuple<1,int> strides(m_indexMap.strides()[DIM-1]);
    Tuple<DIM,int> offset = snoc(rowPos,domain().low(DIM-1));
    assert(domain().contains(offset));
    return SteppedRefArray<1,ENTRY>(domain()[DIM-1], Step2IndexMap<1>(
      domain.size()[0], strides), &operator[](offset) );
  }

  SteppedRefArray<1,ENTRY> column(const Tuple<dim_min_1,int>& columnPos) {
    Tuple<1,int> strides(m_indexMap.strides()[0]);
    Tuple<DIM,int> offset = cons(domain().low(0),columnPos);
    assert(domain().contains(offset));
    return SteppedRefArray<1,ENTRY>(domain()[0], Step2IndexMap<1>(
      domain.size()[0], strides), &operator[](offset) );
  }

  SteppedRefArray<1,ENTRY> column(int columnPos) {
    Tuple<1,int> strides(m_indexMap.strides()[0]);
    Tuple<DIM,int> offset = domain().low(); offset[DIM-1] = columnPos;
    assert(domain().contains(offset));
    return SteppedRefArray<1,ENTRY>(domain()[0], Step2IndexMap<DIM>(
      domain.size()[DIM-1], strides), &operator[](offset) );
  }

  const SteppedRefArray<1,ENTRY> column(const Tuple<dim_min_1,int>& columnPos) const {
    Tuple<1,int> strides(m_indexMap.strides()[0]);
    Tuple<DIM,int> offset = cons(domain.low(0),columnPos);
    assert(domain().contains(offset));
    return SteppedRefArray<1,ENTRY>(domain()[0], Step2IndexMap<1>(
      domain.size()[0], strides), &operator[](offset) );
  }
};

} // namespace multi_array

#endif
