#ifndef _MULTI_ARRAY_MULTISTEPREFARRAYBASE_H_
#define _MULTI_ARRAY_MULTISTEPREFARRAYBASE_H_

#include "multi_array/MultiIndexRefArrayBase.h"
#include "multi_array/Step2IndexMap.h"

namespace multi_array {

/// MultiStepRefArrayBase is the base class of
/// MultiStepRefArray and inherits from MultiIndexRefArrayBase.
/// MultiStepRefArrayBase specializes MultiIndexRefArrayBase with the
/// index map Step2IndexMap.  The index map is the map from a
/// multidimensional tuple of dimension DIM to a one dimensional
/// storage index.  The particular index map Step2IndexMap used here
/// organizes storage with steps such that:
/// - the steps between each consecutive value in any of the
/// dimensions are constant.
/// In contrast to RMStridedRefArrayBase and CMStridedRefArrayBase,
/// there are therefore steps associated with each of the dimensions,
/// so in comparison to those classes there are strides for each of
/// the DIM dimensions.  For the purpose of MultiStepRefArray, it is
/// furthermore also irrelevant whether the last dimension (rows)
/// iterate fastest, or the first dimension (columns).  So
/// MultiStepRefArray transcends row and column major ordering.  For
/// instance, MultiStepRefArrayBase would therefore be the array one
/// would use if one would only be interested in every second element
/// of the array.  However, this functionality is currently not
/// provided for in multi_array.  The only place where
/// MultiStepRefArrayBase is currently used is as the return type of the
/// transpose method of the RMStridedRefArrayBase and
/// CMStridedRefArrayBase classes.  

template <int DIM, typename ENTRY>
class MultiStepRefArrayBase: public MultiIndexRefArrayBase<DIM, ENTRY, 
  Step2IndexMap<DIM> >
{
protected:

  inline MultiStepRefArrayBase() {
    assert(DIM > 0);
  }

public:

  inline MultiStepRefArrayBase(const MultiStepRefArrayBase& other):
    MultiIndexRefArrayBase<DIM,ENTRY,Step2IndexMap<DIM> >(other)
  {}

  inline MultiStepRefArrayBase(const CloseOpenInterval<DIM,int>& domain, 
    const Step2IndexMap<DIM>& indexMap, ENTRY* data):
    MultiIndexRefArrayBase<DIM,ENTRY,Step2IndexMap<DIM> >(domain,
      indexMap,data)
  {}

  MultiStepRefArray<DIM,ENTRY> slice(const CloseOpenInterval<DIM,int>& 
    sliceDomain) 
  {
    assert(domain().contains(sliceDomain));
    return MultiStepRefArray<DIM,ENTRY>(sliceDomain, m_indexMap, &operator[](
      sliceDomain.low()) );
  }

  const MultiStepRefArray<DIM,ENTRY> slice(const CloseOpenInterval<DIM,int>& 
    sliceDomain) const 
  {
    assert(domain().contains(sliceDomain));
    return MultiStepRefArray<DIM,ENTRY>(sliceDomain, m_indexMap, &operator[](
      sliceDomain.low()) );
  }

  MultiStepRefArray<1,ENTRY> row(const Tuple<dim_min_1,int>& rowPos) {
    Tuple<1,int> strides(m_indexMap.strides()[DIM-1]);
    Tuple<DIM,int> offset = snoc(rowPos,domain().low(DIM-1));
    return MultiStepRefArray<1,ENTRY>(domain()[DIM-1], Step2IndexMap<1>(
      domain.size()[0], strides), &operator[](offset) );
  } 

  const MultiStepRefArray<1,ENTRY> row(const Tuple<dim_min_1,int>& rowPos) const {
    Tuple<1,int> strides(m_indexMap.strides()[DIM-1]);
    Tuple<DIM,int> offset = snoc(rowPos,domain().low(DIM-1));
    return MultiStepRefArray<1,ENTRY>(domain()[DIM-1], Step2IndexMap<1>(
      domain.size()[0], strides), &operator[](offset) );
  } 

  MultiStepRefArray<1,ENTRY> column(const Tuple<dim_min_1,int>& columnPos) {
    Tuple<1,int> strides(m_indexMap.strides()[0]);
    Tuple<DIM,int> offset = cons(domain().low(0),columnPos);
    return MultiStepRefArray<1,ENTRY>(domain()[0], Step2IndexMap<1>(
      domain.size()[0], strides), &operator[](offset) );
  } 

  MultiStepRefArray<1,ENTRY> column(int columnPos) {
    Tuple<1,int> strides(m_indexMap.strides()[0]);
    Tuple<DIM,int> offset = domain().low(); offset[DIM-1] = columnPos;
    return MultiStepRefArray<1,ENTRY>(domain()[0], Step2IndexMap<DIM>(
      domain.size()[DIM-1], strides), &operator[](offset) );
  } 

  const MultiStepRefArray<1,ENTRY> column(const Tuple<dim_min_1,int>& columnPos) const {
    Tuple<1,int> strides(m_indexMap.strides()[0]);
    Tuple<DIM,int> offset = cons(domain.low(0),columnPos);
    return MultiStepRefArray<1,ENTRY>(domain()[0], Step2IndexMap<1>(
      domain.size()[0], strides), &operator[](offset) );
  } 
};

} // namespace multi_array

#endif
