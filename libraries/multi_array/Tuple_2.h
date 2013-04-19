template <typename ENTRY>
class Tuple<2,ENTRY>
{
public:

  enum { dim = 2, dim_min_1 = 1, dim_plus_1 = 3 };
  typedef ENTRY Entry;

  typedef Tuple<dim_plus_1,ENTRY> OneMore;

private:

  ENTRY m_first, m_second;

public:

  inline static Tuple<2,ENTRY> filled(const ENTRY& value) {
    return tuple(value,value);
  }

  inline Tuple() {
  }

  inline ~Tuple() {
  }

  inline Tuple(const ENTRY* data){
    m_first = data[0];
    m_second = data[1];
  }

  template <typename OTHER_ENTRY>
  inline Tuple(const OTHER_ENTRY& head, const Tuple<1,ENTRY>& tail)
  {
    m_first = head;
    m_second = tail[0];
  }

/*    inline Tuple(const ENTRY& head, const Tuple<1,ENTRY>& tail) */
/*    { */
/*      m_first = head; */
/*      m_second = tail[0]; */
/*    } */

  template <typename OTHER_ENTRY>
  inline Tuple(const Tuple<1,ENTRY>& head, const OTHER_ENTRY& tail)
  {
    m_first = head[0];
    m_second = tail;
  }

/*    inline Tuple(const Tuple<1,ENTRY>& head, const ENTRY& tail) */
/*    { */
/*      m_first = head[0]; */
/*      m_second = tail; */
/*    } */

  inline explicit Tuple(const ENTRY& value1, const ENTRY& value2):
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

  inline const ENTRY& car() const {
    return m_first;
  }

  inline Tuple<1,ENTRY> cdr() const {
    return tuple(m_second);
  }

/*    inline Tuple<3,ENTRY> cons(const ENTRY& value) const { */
/*      Tuple<3,ENTRY> result; */
/*      result[0] = value; */
/*      result[1] = m_first; */
/*      result[2] = m_second; */
/*      return result; */
/*    } */

  /// rac returns the last tuple element. rac is the opposite of car
  /// (and the name is chosen to reflect that fact).
  inline const ENTRY& rac() const {
    return m_second;
  }

  /// Like rac, rdc is the opposite of cdr. rdc returns a tuple of the
  /// remaining elements before the element returned by rac.
  inline Tuple<1,ENTRY> rdc() const {
    return tuple(m_first);
  }

/*    inline Tuple<3,ENTRY> snoc(const ENTRY& value) const { */
/*      Tuple<3,ENTRY> result; */
/*      result[0] = m_first; */
/*      result[1] = m_second; */
/*      result[2] = value; */
/*      return result; */
/*    } */

/*   inline Tuple<DIM2 + 2, ENTRY> construct(const Tuple<DIM2,ENTRY>& product) const { */
/*     Tuple<DIM2 + 1, ENTRY> result; */
/*     int d = 0; */
/*     for (; d < DIM2; ++d) */
/*       result[d] = product[d]; */
/*     result[d] = m_first; ++d */
/*     result[d] = m_second; */
/*     assert(d == DIM2 + 1); */
/*     return result;      */
/*   } */

  inline Tuple<1,ENTRY> reverse() const {
    Tuple<2,ENTRY> result;
    result[0] = m_second;
    result[1] = m_first;
    return result;
  }

  inline Tuple<2,ENTRY>& operator+=(const Tuple<2,ENTRY>& rhs) {
    m_first += rhs[0];
    m_second += rhs[1];
    return *this;
  }

  inline Tuple<2,ENTRY>& operator-=(const Tuple<2,ENTRY>& rhs) {
    m_first -= rhs[0];
    m_second -= rhs[1];
    return *this;
  }

  inline Tuple<2,ENTRY>& operator*=(const Tuple<2,ENTRY>& rhs) {
    m_first *= rhs[0];
    m_second *= rhs[1];
    return *this;
  }

  inline Tuple<2,ENTRY>& operator/=(const Tuple<2,ENTRY>& rhs) {
    m_first /= rhs[0];
    m_second /= rhs[1];
    return *this;
  }
};

#ifndef sun

// Including this function leads to the compiler error: Multiple declaration for dim.

template <typename ENTRY>
inline Tuple<2,ENTRY> tuple(const ENTRY& value1, const ENTRY& value2) {
  return Tuple<2,ENTRY>(value1,value2);
}

#endif


