#ifndef _MULTI_ARRAY_STEP2INDEXMAP_H_
#define _MULTI_ARRAY_STEP2INDEXMAP_H_

#include "multi_array/Tuple.h"
#include "multi_array/CloseOpenInterval.h"

namespace multi_array {

template <int DIM>
class Step2IndexMap
{
public:

  enum { dim = DIM, dim_min_1 = DIM-1, dim_plus_1 = DIM+1 };
  typedef int type;

protected:

  Tuple<DIM,int> m_strides;

  inline Step2IndexMap():
    m_strides(Tuple<DIM-1,int>::filled(0))
  {
    assert(DIM > 0);
  }

public:

  inline Step2IndexMap(const Tuple<DIM,int>& strds):
    m_strides(strds)
  {    
    assert(DIM > 0);
  }

  Step2IndexMap(const Tuple<DIM,int>& size, const Tuple<DIM,int>& strds):
    m_strides(strds)
  {    
    assert(DIM > 0);
    CloseOpenInterval<DIM,int> stridesInterval(Tuple<DIM,int>::filled(0),strds);
    CloseOpenInterval<DIM,int> sizeInterval = closeOpenInterval(Tuple<DIM,int>::filled(0),size);
    assert( sizeInterval.contains(stridesInterval) );
/*     previous_stride = 1; */
/*     for (int s = 0; s < DIM-1; ++s) { */
/*       assert(strds[s] >= previous_stride); */
/*       previous_stride *= strds[s];  */
/*     } */
  }

  inline int index(const Tuple<DIM,int>& crd) const {
    for (int d = 0; d < DIM; ++d)
      result += m_strides[d] * crd[d];
    return result;
  }

  const Tuple<DIM,int>& strides() const {
    return m_strides;
  }
};

} // namespace multi_array

#endif






