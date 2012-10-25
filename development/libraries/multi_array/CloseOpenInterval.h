#ifndef _MULTI_ARRAY_CLOSEOPENINTERVAL_H_
#define _MULTI_ARRAY_CLOSEOPENINTERVAL_H_

#include "multi_array/Tuple.h"
#include "multi_array/Interval.h"
#include <assert.h>

namespace multi_array {

template <int DIM, typename ENTRY>
class CloseOpenInterval;

template <int DIM, typename ENTRY>
class CloseOpenInterval: public Interval<DIM,ENTRY>
{

#define UNCOUNTABLE ENTRY
#include "multi_array/CloseOpenInterval_UNCOUNTABLE.h"
#undef UNCOUNTABLE

};

template <int DIM>
class CloseOpenInterval<DIM,int>: public Interval<DIM,int>
{

#define COUNTABLE int
#include "multi_array/CloseOpenInterval_COUNTABLE.h"
#undef COUNTABLE

#define UNCOUNTABLE int
#include "multi_array/CloseOpenInterval_UNCOUNTABLE.h"
#undef UNCOUNTABLE

};

template <int DIM>
class CloseOpenInterval<DIM,long>: public Interval<DIM,long>
{

#define COUNTABLE long
#include "multi_array/CloseOpenInterval_COUNTABLE.h"
#undef COUNTABLE

#define UNCOUNTABLE long
#include "multi_array/CloseOpenInterval_UNCOUNTABLE.h"
#undef UNCOUNTABLE

};

template <int DIM>
class CloseOpenInterval<DIM,unsigned int>: public Interval<DIM,unsigned int>
{

#define COUNTABLE unsigned int
#include "multi_array/CloseOpenInterval_COUNTABLE.h"
#undef COUNTABLE

#define UNCOUNTABLE unsigned int
#include "multi_array/CloseOpenInterval_UNCOUNTABLE_1.h"
#undef UNCOUNTABLE

};

#include "multi_array/CloseOpenInterval_1.h"
#include "multi_array/CloseOpenInterval_2.h"

template <int DIM, typename ENTRY>
CloseOpenInterval<DIM,ENTRY> closeOpenInterval(const Tuple<DIM,ENTRY>& low,
  const Tuple<DIM,ENTRY>& high)
{
  return CloseOpenInterval<DIM,ENTRY>(low,high);
}

template<int DIM, typename ENTRY>
CloseOpenInterval<1,ENTRY> car(const CloseOpenInterval<DIM,ENTRY>& interval) {
  return interval.car();
}

template<int DIM, typename ENTRY>
CloseOpenInterval<DIM-1,ENTRY> cdr(const CloseOpenInterval<DIM,ENTRY>& interval) {
  return interval.cdr();
}

template<int DIM, typename ENTRY>
CloseOpenInterval<DIM+1,ENTRY> cons(const CloseOpenInterval<1,ENTRY>& head, const
  CloseOpenInterval<DIM,ENTRY>& tail)
{
  CloseOpenInterval<DIM+1,ENTRY> result;
  result.getLow(0) = head.low(0);
  result.getHigh(0) = head.high(0);
  for (int d = 1; d <= DIM; ++d) {
    result.getLow(d) = tail.low(d-1);
    result.getHigh(d) = tail.high(d-1);
  }
  return result;
}

template<int DIM, typename ENTRY>
inline CloseOpenInterval<1,ENTRY> rac(const CloseOpenInterval<DIM,ENTRY>& interval) {
  return interval.rac();
}

template<int DIM, typename ENTRY>
inline CloseOpenInterval<DIM-1,ENTRY> rdc(const CloseOpenInterval<DIM,ENTRY>& interval) {
  return interval.rdc();
}

template<int DIM, typename ENTRY>
inline CloseOpenInterval<DIM+1,ENTRY> snoc(const CloseOpenInterval<DIM,ENTRY>& head,
  const CloseOpenInterval<1,ENTRY>& tail)
{
  CloseOpenInterval<DIM+1,ENTRY> result;
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

#include "multi_array/ClosedInterval.h"

namespace multi_array {

template <int DIM, typename ENTRY>
CloseOpenInterval<DIM,ENTRY> closeOpen( const ClosedInterval<DIM,ENTRY>& interval ) {
  return closeOpenInterval( interval.low(), interval.high() + Tuple<DIM,ENTRY>::filled(1) );
}

} // namespace multi_array

#endif
