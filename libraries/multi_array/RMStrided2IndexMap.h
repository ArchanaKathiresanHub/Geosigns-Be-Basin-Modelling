#ifndef _MULTI_ARRAY_RMSTRIDEDINDEXMAP_H_
#define _MULTI_ARRAY_RMSTRIDEDINDEXMAP_H_

#include "multi_array/Tuple.h"
#include "multi_array/CloseOpenInterval.h"
#include "multi_array/Step2IndexMap.h"

namespace multi_array {

template <int DIM>
class RMStrided2IndexMap
{
public:

  enum { dim = DIM, dim_min_1 = DIM-1, dim_plus_1 = DIM+1 };
  typedef int type;

protected:

  Tuple<DIM-1,int> m_strides;

  Tuple<DIM-1,int> strides(const Tuple<DIM,int>& size) {
    Tuple<DIM-1,int> result;
    result[DIM-2] = size[DIM-1];
    for (int d = DIM-3; d >= 0; --d)
      result[d] = result[d+1] * size[d+1];
    return result;
  }

  inline RMStrided2IndexMap():
    m_strides(Tuple<DIM-1,int>::filled(0))
  {
    assert(DIM > 0);
  }

public:

  inline RMStrided2IndexMap(const Tuple<DIM,int>& size):
    m_strides(strides(size))
  {    
    assert(DIM > 0);
  }

  RMStrided2IndexMap(const Tuple<DIM-1,int>& strds):
    m_strides(strds)
  {    
    assert(DIM > 0);
  }

  RMStrided2IndexMap(const Tuple<DIM,int>& size, const Tuple<DIM-1,int>& strds):
    m_strides(strds)
  {    
    assert(DIM > 0);
    Tuple<DIM,int> allStrides = strds.snoc(1);
    CloseOpenInterval<DIM,int> allStridesInterval(Tuple<DIM,int>::filled(0),allStrides);
    CloseOpenInterval<DIM,int> sizeInterval = closeOpenInterval(Tuple<DIM,int>::filled(0),size);
    assert( sizeInterval.contains(allStridesInterval) );
/*     previous_stride = 1; */
/*     for (int s = 0; s < DIM-1; ++s) { */
/*       assert(strds[s] >= previous_stride); */
/*       previous_stride *= strds[s];  */
/*     } */
  }

  inline int index(const Tuple<DIM,int>& crd) const {
    int result = crd[DIM-1];
    for (int d = DIM-2; d >= 0; --d)
      result += m_strides[d] * crd[d];
    return result;
  }

  Step2IndexMap<DIM> transpose() const {
    return Step2IndexMap<DIM>(snoc(strides(),1).reverse());
  }

  const Tuple<DIM-1,int>& strides() const {
    return m_strides;
  }
};

#include "multi_array/RMStrided2IndexMap_1.h"

} // namespace multi_array

#endif






