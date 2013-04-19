#ifndef _MULTI_ARRAY_INTERVAL_H_
#define _MULTI_ARRAY_INTERVAL_H_

#include "multi_array/Tuple.h"
#include <limits>
#include <algorithm>

namespace multi_array {

using namespace std;

template <int DIM, typename ENTRY>
class Interval
{
public:

  enum { dim = DIM, dim_min_1 = DIM-1, dim_plus_1 = DIM+1 };
  typedef ENTRY Entry;

protected:

  Tuple<DIM,ENTRY> m_low, m_high;

  inline Interval():
    m_low (Tuple<DIM,ENTRY>::filled(-numeric_limits<ENTRY>::max())),
    m_high(Tuple<DIM,ENTRY>::filled(-numeric_limits<ENTRY>::max()))
  {}

  inline Interval(const Tuple<DIM,ENTRY>& lw, const Tuple<DIM,ENTRY>& hgh):
    m_low(lw),
    m_high(hgh)
  {}

public:

  inline const Tuple<DIM,ENTRY>& low() const {
    return m_low;
  }

  inline const ENTRY& low(int d) const {
    assert(d >= 0 && d < DIM);
    return m_low[d];
  }

  inline ENTRY& getLow(int d) {
    assert(d >= 0 && d < DIM);
    return m_low[d];
  }

  inline Tuple<DIM,ENTRY>& getLow() {
    return m_low;
  }

  inline Interval<DIM,ENTRY>& setLow(const Tuple<DIM,ENTRY>& newLow) {
    m_low = newLow;
    assert(m_high >= m_low);
    return this;
  }

  inline const Tuple<DIM,ENTRY>& high() const {
    return m_high;
  }

  inline const ENTRY& high(int d) const {
    assert(d >= 0 && d < DIM);
    return m_high[d];
  }

  inline ENTRY& getHigh(int d) {
    assert(d >= 0 && d < DIM);
    return m_high[d];
  }

  inline Tuple<DIM,ENTRY>& getHigh() {
    return m_high;
  }

  inline Interval<DIM,ENTRY>& setHigh(const Tuple<DIM,ENTRY>& newHigh) {
    m_high = newHigh;
    assert(m_high >= m_low);
    return this;
  }

  inline Tuple<DIM,ENTRY> size() const {
    Tuple<DIM,ENTRY> result;
    for (int d = 0; d < DIM; ++d)
      result[d] = size(d);
    return result;
  }

  inline ENTRY size(int d) const {
    assert(d >= 0 && d < DIM);
    return max(high(d) - low(d),0);
  }

  inline ENTRY volume() const {
    ENTRY result = 1;
    for (int d = 0; d < DIM; ++d)
      result *= size(d);
    return result;
  }

  inline bool empty() const {
    for (int d = 0; d < DIM; ++d)
      if (m_high[d] <= m_low[d]) return true;
    return false;
  }

protected:

  inline bool operator==(const Interval<DIM,ENTRY>& rhs) const
  {
    for (int d = 0; d < DIM; ++d)
      if (m_low[d] != rhs.m_low[d] || m_high[d] != rhs.m_high[d])
	return false;
    return true;
  }

  inline bool operator!=(const Interval<DIM,ENTRY>& rhs) const
  {
    return !operator==(rhs);
  }

  inline bool contains(const Interval<DIM,ENTRY>& other) const {
    for (int d = 0; d < DIM; ++d)
      if (other.m_low[d] < m_low[d] || m_high[d] < other.m_high[d])
        return false;
    return other.empty();
  }

  inline bool overlaps(const Interval<DIM,ENTRY>& other) const {
    for (int d = 0; d < DIM; ++d)
      if (other.m_high[d] <= m_low[d] || m_high[d] <= other.m_low[d])
        return false;
    return !other.empty();
  }

  inline void expandToCover(const Interval<DIM,ENTRY>& other) {
    if (other.empty()) return;
    if (empty())
      *this = other;
    else {
      for (int d = 0; d < DIM; ++d) {
        if (other.low(d) < m_low[d]) m_low[d] = other.low(d);
        if (other.high(d) > m_high[d]) m_high[d] = other.high(d);
      }
    }
  }

  inline void intersectWith(const Interval<DIM,ENTRY>& other) {
    for (int d = 0; d < DIM; ++d) {
      if (other.low(d) > m_low[d]) m_low[d] = other.low(d);
      if (other.high(d) < m_high[d]) m_high[d] = other.high(d);
    }
  }
};

} // namespace multi_array

#include "multi_array/Interval_1.h"
#include "multi_array/Interval_2.h"

#endif
