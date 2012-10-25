#ifndef _MULTI_ARRAY_MULTIARRAYBASE_H_
#define _MULTI_ARRAY_MULTIARRAYBASE_H_

#include "multi_array/CloseOpenInterval.h"

namespace multi_array {

template <int DIM, typename TYPE>
class MultiArrayBase
{
public:

  enum { dim = DIM, dim_min_1 = DIM-1, dim_plus_1 = DIM+1 };

  typedef TYPE Index;
  typedef Tuple<DIM,TYPE> Coord;

  typedef CloseOpenInterval<DIM,TYPE> Domain;

protected:

  Domain m_domain;

  inline MultiArrayBase()
  {}

public:

  inline MultiArrayBase(const MultiArrayBase& other):
    m_domain(other.m_domain)
  {}

  inline MultiArrayBase(const CloseOpenInterval<DIM,TYPE>& dmn):
    m_domain(dmn)
  {}

  inline const CloseOpenInterval<DIM,TYPE>& domain() const {
    return m_domain;
  }

  inline CloseOpenInterval<1,TYPE> domain(int d) const {
    return m_domain[d];
  }
};

} // namespace multi_array

#endif
