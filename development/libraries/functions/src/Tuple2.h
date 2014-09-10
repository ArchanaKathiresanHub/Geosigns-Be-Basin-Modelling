#ifndef _FUNCTIONS_TUPLE2_H_
#define _FUNCTIONS_TUPLE2_H_

#include <assert.h>
#include <limits>

using std::numeric_limits;

namespace functions {

template <typename ENTRY>
class Tuple2
{
public:

  enum { dim = 2, dim_min_1 = 1, dim_plus_1 = 3 };
  typedef ENTRY Entry;

private:

  ENTRY m_first, m_second;

public:

  inline static Tuple2<ENTRY> filled(const ENTRY& value) {
    return tuple(value,value);
  }
 
  Tuple2()
  {
#ifdef _MSC_VER
#undef max
#endif
      m_first = -numeric_limits<ENTRY>::max();
      m_second = m_first;
  }

  inline ~Tuple2() {
  }

  Tuple2(const ENTRY* data) {
    m_first = data[0];
    m_second = data[1];
  }

  Tuple2(const ENTRY& value1, const ENTRY& value2):
    m_first(value1),
    m_second(value2)
  {}

  inline const ENTRY& operator[](int d) const {
    assert(d == 0 || d == 1);
    return d == 0 ? m_first : m_second;
  }

  inline ENTRY& operator[](int d) {
    assert(d == 0 || d == 1);
    return d == 0 ? m_first : m_second;
  }

  inline Tuple2<ENTRY> reverse() const {
    Tuple2<ENTRY> result;
    result[0] = m_second;
    result[1] = m_first;
    return result;
  }

  inline Tuple2<ENTRY>& operator+=(const Tuple2<ENTRY>& rhs) {
    m_first += rhs[0];
    m_second += rhs[1];
    return *this;
  }

  inline Tuple2<ENTRY>& operator-=(const Tuple2<ENTRY>& rhs) {
    m_first -= rhs[0];
    m_second -= rhs[1];
    return *this;
  }

  inline Tuple2<ENTRY>& operator*=(const Tuple2<ENTRY>& rhs) {
    m_first *= rhs[0];
    m_second *= rhs[1];
    return *this;
  }

  inline Tuple2<ENTRY>& operator/=(const Tuple2<ENTRY>& rhs) {
    m_first /= rhs[0];
    m_second /= rhs[1];
    return *this;
  }
};

// Including this function leads to the compiler error: Multiple declaration for dim.

template <typename ENTRY>
inline Tuple2<ENTRY> tuple(const ENTRY& value1, const ENTRY& value2) {
  return Tuple2<ENTRY>(value1,value2);
}

} // namespace functions

#endif
