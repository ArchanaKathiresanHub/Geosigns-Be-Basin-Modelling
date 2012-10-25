#ifndef _MULTI_ARRAY_CLOSEDINTERVAL_H_
#define _MULTI_ARRAY_CLOSEDINTERVAL_H_

#include "multi_array/Tuple.h"
#include "multi_array/Interval.h"

namespace multi_array {

template <int DIM, typename ENTRY>
class ClosedInterval;

template <int DIM, typename ENTRY>
class ClosedInterval: public Interval<DIM,ENTRY>
{
public:

  inline static ClosedInterval<dim,ENTRY> everywhere() {
    return ClosedInterval(
      m_low (Tuple<dim,ENTRY>::filled(-numeric_limits<ENTRY>::max())),
      m_high(Tuple<dim,ENTRY>::filled( numeric_limits<ENTRY>::max())));
  }

  inline static ClosedInterval<dim,ENTRY> nowhere() {
    return ClosedInterval();
  }

  inline ClosedInterval()
  {}

  inline ClosedInterval(const Tuple<DIM,ENTRY>& lw, const Tuple<DIM,ENTRY>& hgh):
    Interval<DIM,ENTRY>(lw,hgh)
  {}

  inline ClosedInterval<1,ENTRY> operator[](int d) const {
    assert(domain().contains(d));
    return ClosedInterval<1,ENTRY>(tuple(low(d)),tuple(high(d)));
  }

  inline ClosedInterval<1,ENTRY> car() const {
    return ClosedInterval<1,ENTRY>(low().car(), high().car());
  }

  inline ClosedInterval<dim_min_1,ENTRY> cdr() const {
    return ClosedInterval(low().cdr(), high().cdr());
  }

  inline ClosedInterval<1,ENTRY> rac() const {
    return ClosedInterval<1,ENTRY>(low().rac(), high().rac());
  }

  inline ClosedInterval<dim_min_1,ENTRY> rdc() const {
    return ClosedInterval<1,ENTRY>(low().rdc(), high().rdc());
  }

  inline bool contains(const Tuple<dim,ENTRY>& crd) const {
    for (int d = 0; d < dim; ++d)
      if (crd[d] < m_low[d] || m_high[d] < crd[d])
        return false;
    return true;
  }

  inline bool contains(const ClosedInterval<dim,ENTRY>& other) const {
    return Interval<dim,ENTRY>::contains(other);
  }

  inline bool overlaps(const ClosedInterval<dim,ENTRY>& other) const {
    for (int d = 0; d < dim; ++d)
      if (other.m_high[d] < m_low[d] || m_high[d] < other.m_low[d])
        return false;
    return !other.empty();
  }

  inline void expandToCover(const Tuple<dim,ENTRY>& crd) {
    if (empty()) {
      m_low = crd;
      m_high = crd;
    }
    else {
      for (int d = 0; d < dim; ++d) {
        if (crd[d] < m_low[d]) m_low[d] = crd[d];
        if (crd[d] > m_high[d]) m_high[d] = crd[d];
      }
    }
  }

  inline void expandToCover(const ClosedInterval<dim,ENTRY>& other) {
    Interval<dim,ENTRY>::expandToCover(other);
  }

  inline void intersectWith(const ClosedInterval<dim,ENTRY>& other) {
    Interval<dim,ENTRY>::intersectWith(other);
  }

  inline ClosedInterval<dim,ENTRY> supremum(const ClosedInterval<dim,ENTRY>& other) const {
    ClosedInterval result = *this;
    result.expandToCover(other);
    return result;
  }

  inline ClosedInterval<dim,ENTRY> infimum(const ClosedInterval<dim,ENTRY>& other) const {
    ClosedInterval result = *this;
    result.intersectWith(other);
    return result;
  }

  inline bool operator==(const ClosedInterval<dim,ENTRY>& rhs) const {
    return Interval<dim,ENTRY>::operator==(rhs);
  }

  inline bool operator!=(const ClosedInterval<dim,ENTRY>& rhs) const {
    return Interval<dim,ENTRY>::operator!=(rhs);
  }

  inline const ClosedInterval<dim,ENTRY>& operator+=(const Tuple<dim,ENTRY>& crd) {
    m_low += crd; m_high += crd;
    return *this;
  }

  inline const ClosedInterval<dim,ENTRY>& operator-=(const Tuple<dim,ENTRY>& crd) {
    m_low -= crd; m_high -= crd;
    return *this;
  }

  inline const ClosedInterval<dim,ENTRY>& operator*=(const Tuple<dim,ENTRY>& crd) {
    m_low *= crd; m_high *= crd;
    return *this;
  }

  inline const ClosedInterval<dim,ENTRY>& operator/=(const Tuple<dim,ENTRY>& crd) {
    m_low /= crd; m_high /= crd;
    return *this;
  }

  inline ClosedInterval<dim,ENTRY> operator+(const Tuple<dim,ENTRY>& crd) const {
    return ClosedInterval(low() + crd, high() + crd);
  }

  inline ClosedInterval<dim,ENTRY> operator*(const Tuple<dim,ENTRY>& crd) const {
    return ClosedInterval(low() * crd, high() * crd);
  }
};

#include "multi_array/ClosedInterval_1.h"
#include "multi_array/ClosedInterval_2.h"

template <int DIM, typename ENTRY>
ClosedInterval<DIM,ENTRY> closedInterval(const Tuple<DIM,ENTRY>& lw,
  const Tuple<DIM,ENTRY>& hgh)
{
  return ClosedInterval<DIM,ENTRY>(lw,hgh);
}

template<int DIM, typename ENTRY>
ClosedInterval<1,ENTRY> car(const ClosedInterval<DIM,ENTRY>& interval) {
  return interval.car();
}

#ifndef sun

// "../../multi_array/ClosedInterval.h", line 167: Error: complex expression not allowed in dummy array type declaration.
// "../../multi_array/ClosedInterval.h", line 172: Error: complex expression not allowed in dummy array type declaration.

template<int DIM, typename ENTRY>
ClosedInterval<DIM-1,ENTRY> cdr(const ClosedInterval<DIM,ENTRY>& interval) {
  return interval.cdr();
}

template<int DIM, typename ENTRY>
ClosedInterval<DIM+1,ENTRY> cons(const ClosedInterval<1,ENTRY>& head, const ClosedInterval<
  DIM,ENTRY>& tail)
{
  ClosedInterval<DIM+1,ENTRY> result;
  result.getLow(0) = head.low(0);
  result.getHigh(0) = head.high(0);
  for (int d = 1; d < DIM; ++d) {
    result.getLow(d) = tail.low(d-1);
    result.getHigh(d) = tail.high(d-1);
  }
  return result;
}

#endif

template<int DIM, typename ENTRY>
ClosedInterval<1,ENTRY> rac(const ClosedInterval<DIM,ENTRY>& interval) {
  return interval.rac();
}

template<int DIM, typename ENTRY>
ClosedInterval<DIM-1,ENTRY> rdc(const ClosedInterval<DIM,ENTRY>& interval) {
  return interval.rdc();
}

template<int DIM, typename ENTRY>
ClosedInterval<DIM+1,ENTRY> snoc(const ClosedInterval<DIM,ENTRY>& head, const
  ClosedInterval<1,ENTRY>& tail)
{
  ClosedInterval<DIM+1,ENTRY> result;
  int d = 0;
  for (; d < DIM; ++d) {
    result.getLow(d) = head.low(d);
    result.getHigh(d) = head.high(d);
  }
  result.getLow(d) = tail.low(0);
  result.getHigh(d) = tail.high(0);
  return result;
}

} // namespace multi_array

#include "multi_array/CloseOpenInterval.h"

namespace multi_array {

template <int DIM, typename ENTRY>
ClosedInterval<DIM,ENTRY> close( const CloseOpenInterval<DIM,ENTRY>& interval ) {
  return closedInterval( interval.low(), interval.high() - Tuple<DIM,int>::filled(1) );
}

} // namespace multi_array

#endif





