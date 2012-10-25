#ifndef _MULTI_ARRAY_TUPLE_H_
#define _MULTI_ARRAY_TUPLE_H_

#include <limits>
#include <assert.h>

using namespace std;

namespace multi_array {

template <int DIM, typename ENTRY>
class Tuple
{
public:

  enum { dim = DIM, dim_min_1 = DIM-1, dim_plus_1 = DIM+1 };
  typedef ENTRY Entry;

  typedef Tuple<dim_plus_1,ENTRY> OneMore;

private:

  ENTRY* m_data;

public:

  inline static Tuple<DIM,ENTRY> filled(const ENTRY& value) {
    Tuple<DIM,ENTRY> result;
    for (int d = 0; d < DIM; ++d)
      result.m_data[d] = value;
    return result;
  }

  inline Tuple() {
    m_data = new ENTRY[DIM];
    ENTRY value = -numeric_limits<ENTRY>::max();
    for (ENTRY* p = m_data, *lim = m_data + DIM; p < lim; ++p)
      *p = value;
  }

  ~Tuple() {
    delete m_data;
  }

  inline explicit Tuple(const ENTRY* dt){
    m_data = new ENTRY[DIM];
    for (ENTRY* p = m_data, *p2 = dt, *lim = m_data + DIM; p < lim; ++p, ++p2)
      *p = *p2;
  }

  template< typename OTHER_ENTRY>
  inline Tuple(const OTHER_ENTRY& head, const Tuple<dim_min_1,ENTRY>& tail)
  {
    m_data = new ENTRY[DIM];
    ENTRY* p = m_data; *p = head; ++p;
    for (ENTRY* p2 = tail.m_data, *lim = tail.m_data + dim_min_1; p2 < lim; ++p2, ++p)
      *p = *p2;
  }

/*    inline Tuple(const ENTRY& head, const Tuple<dim_min_1,ENTRY>& tail) */
/*    { */
/*      m_data = new ENTRY[DIM]; */
/*      ENTRY* p = m_data; *p = head; ++p; */
/*      for (ENTRY* p2 = tail.m_data, *lim = tail.m_data + dim_min_1; p2 < lim; ++p2, ++p) */
/*        *p = *p2; */
/*    } */

  template< typename OTHER_ENTRY>
  inline Tuple(const Tuple<dim_min_1,ENTRY>& head, const OTHER_ENTRY& tail)
  {
    m_data = new ENTRY[DIM];
    ENTRY* p = m_data;
    for (ENTRY* p2 = head.m_data, *lim = head.m_data + dim_min_1; p2 < lim; ++p2, ++p)
      *p = *p2;
    *p = tail;
  }

/*    inline Tuple(const Tuple<dim_min_1,ENTRY>& head, const ENTRY& tail) */
/*    { */
/*      m_data = new ENTRY[DIM]; */
/*      ENTRY* p = m_data; */
/*      for (ENTRY* p2 = head.m_data, *lim = head.m_data + dim_min_1; p2 < lim; ++p2, ++p) */
/*        *p = *p2; */
/*      *p = tail; */
/*    } */

  inline const ENTRY& data() const {
    return *m_data;
  }

  inline ENTRY& getData() {
    return *m_data;
  }

  inline const ENTRY& operator[](int d) const {
    assert(0 <= d && d < DIM);
    return m_data[d];
  }

  inline ENTRY& operator[](int d) {
    assert(0 <= d && d < DIM);
    return m_data[d];
  }

  /// car returns the first tuple element. The name car originates
  /// from Lisp and refers to an analogous function applied to
  /// lisps. car, cdr and cons are the standard operations for lists
  /// in Lisp.
  inline const ENTRY& car() const {
    return m_data[0];
  }

  /// cdr returns the remaining elements after the element returned by
  /// car. car, cdr and cons form the standard operations for lists in
  /// Lisp.
  inline Tuple<dim_min_1,ENTRY> cdr() const {
    return Tuple<dim_min_1,ENTRY>(&m_data[1]);
  }

  inline Tuple<dim_plus_1,ENTRY> cons(const ENTRY& value) const {
    Tuple<dim_plus_1,ENTRY> result;
    result[0] = value;
    for (int d = 1; d1 <= DIM; ++d)
      result[d] = m_data[d-1];
    return result;
  }

  /// rac returns the last tuple element. rac is the opposite of car
  /// (and the name is chosen to reflect that fact).
  inline const ENTRY& rac() const {
    return m_data[dim_min_1];
  }

  /// Like rac, rdc is the opposite of cdr. rdc returns a tuple of the
  /// remaining elements before the element returned by rac.
  inline Tuple<dim_min_1,ENTRY> rdc() const {
    return Tuple<dim_min_1,ENTRY>(&m_data[0]);
  }

  inline Tuple<dim_plus_1,ENTRY> snoc(const ENTRY& value) const {
    Tuple<dim_plus_1,ENTRY> result;
    for (int d = 0; d < DIM; ++d)
      result[d] = m_data[d];
    result[DIM] = value;
    return result;
  }

  inline Tuple<dim,ENTRY> reverse() const {
    Tuple<dim,ENTRY> result;
    for (int d = 0, e = DIM-1; d < DIM; ++d, --e)
      result[e] = m_data[d];
    return result;
  }

  inline Tuple<dim,ENTRY>& operator+=(const Tuple<DIM,ENTRY>& rhs) {
    for (int d = 0; d < DIM; ++d)
      m_data[d] += rhs[d];
    return *this;
  }

  inline Tuple<dim,ENTRY>& operator-=(const Tuple<DIM,ENTRY>& rhs) {
    for (int d = 0; d < DIM; ++d)
      m_data[d] -= rhs[d];
    return *this;
  }

  inline Tuple<dim,ENTRY>& operator*=(const Tuple<DIM,ENTRY>& rhs) {
    for (int d = 0; d < DIM; ++d)
      m_data[d] *= rhs[d];
    return *this;
  }

  inline Tuple<dim,ENTRY>& operator/=(const Tuple<DIM,ENTRY>& rhs) {
    for (int d = 0; d < DIM; ++d)
      m_data[d] /= rhs[d];
    return *this;
  }
};

/* template<int DIM, typename ENTRY, int DIM_MIN_1> */
/* Tuple<DIM,ENTRY> cons(const ENTRY& head, const Tuple<DIM_MIN_1,ENTRY>& tail) { */
/*   return Tuple<DIM,ENTRY>(head,tail); */
/* } */

template <int DIM, typename ENTRY>
ENTRY car(const Tuple<DIM,ENTRY>& tuple) {
  return tuple.car();
}

template <int DIM, typename ENTRY>
Tuple<DIM-1,ENTRY> cdr(const Tuple<DIM,ENTRY>& tuple) {
  return tuple.cdr();
}

template <int DIM, typename ENTRY>
Tuple<DIM+1,ENTRY> cons(const ENTRY& head, const Tuple<DIM,ENTRY>& tail) {
  return Tuple<DIM+1,ENTRY>(head,tail);
}

template <int DIM, typename ENTRY>
inline ENTRY rac(const Tuple<DIM,ENTRY>& tuple) {
  return tuple.rac();
}

template <int DIM, typename ENTRY>
inline Tuple<DIM-1,ENTRY> rdc(const Tuple<DIM,ENTRY>& tuple) {
  return tuple.rdc();
}

template <int DIM, typename ENTRY>
Tuple<DIM+1,ENTRY> snoc(const Tuple<DIM,ENTRY>& head, const ENTRY& tail) {
  return Tuple<DIM+1,ENTRY>(head,tail);
}

template <int DIM, typename ENTRY>
Tuple<DIM,ENTRY> operator+(const Tuple<DIM,ENTRY>& first, const Tuple<DIM,ENTRY>& second) {
  Tuple<DIM,ENTRY> result(first);
  result += second;
  return result;
}

template <int DIM, typename ENTRY>
Tuple<DIM,ENTRY> operator-(const Tuple<DIM,ENTRY>& first, const Tuple<DIM,ENTRY>& second) {
  Tuple<DIM,ENTRY> result(first);
  result -= second;
  return result;
}

template <int DIM, typename ENTRY>
Tuple<DIM,ENTRY> operator*(const Tuple<DIM,ENTRY>& first, const Tuple<DIM,ENTRY>& second) {
  Tuple<DIM,ENTRY> result(first);
  result *= second;
  return result;
}

template <int DIM, typename ENTRY>
Tuple<DIM,ENTRY> operator/(const Tuple<DIM,ENTRY>& first, const Tuple<DIM,ENTRY>& second) {
  Tuple<DIM,ENTRY> result(first);
  result /= second;
  return result;
}

template <int DIM, typename ENTRY>
Tuple<DIM,ENTRY> operator%(const Tuple<DIM,ENTRY>& first, const Tuple<DIM,ENTRY>& second) {
  Tuple<DIM,ENTRY> result(first);
  result %= second;
  return result;
}

#include "multi_array/Tuple_0.h"
#include "multi_array/Tuple_1.h"
#include "multi_array/Tuple_2.h"
#include "multi_array/Tuple_3.h"

} // namespace multi_array

#endif
