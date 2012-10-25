template <typename ENTRY>
class Tuple<3,ENTRY>
{
public:

  enum { dim = 3, dim_min_1 = 2 };
  typedef ENTRY Entry;

private:

  ENTRY m_first, m_second, m_third;

public:

  inline static Tuple<3,ENTRY> filled(const ENTRY& value) {
    return tuple(value,value,value);
  }

  inline Tuple() {
  }

  inline ~Tuple() {
  }

  inline explicit Tuple(const ENTRY* data) {
    m_first = data[0];
    m_second = data[1];
    m_third = data[2];
  }

  template <typename OTHER_ENTRY>
  inline Tuple(OTHER_ENTRY head, const Tuple<2,ENTRY>& tail)
  {
    m_first = head;
    m_second = tail[0];
    m_third = tail[1];
  }

/*    inline Tuple(const ENTRY& head, const Tuple<2,ENTRY>& tail) */
/*    { */
/*      m_first = head; */
/*      m_second = tail[0]; */
/*      m_third = tail[1]; */
/*    } */

  template <typename OTHER_ENTRY>
  inline Tuple(const Tuple<2,ENTRY>& head, OTHER_ENTRY tail)
  {
    m_first = head[0];
    m_second = head[1];
    m_third = tail;
  }

/*    inline Tuple(const Tuple<2,ENTRY>& head, const ENTRY& tail) */
/*    { */
/*      m_first = head[0]; */
/*      m_second = head[1]; */
/*      m_third = tail; */
/*    } */

  inline Tuple(const ENTRY& value1, const ENTRY& value2, const ENTRY& value3):
    m_first(value1),
    m_second(value2),
    m_third(value3)
  {}

  inline const ENTRY& operator[](int d) const {
    assert(d >= 0 && d < 3);
    return d == 0 ? m_first : d == 1 ? m_second : m_third;
  }

  inline ENTRY& operator[](int d) {
    assert(d >= 0 && d < 3);
    return d == 0 ? m_first : d == 1 ? m_second : m_third;
  }

  inline const ENTRY& car() const {
    return m_first;
  }

  inline Tuple<2,ENTRY> cdr() const {
    return tuple(m_second,m_third);
  }

/*    inline Tuple<4,ENTRY> cons(const ENTRY& value) const { */
/*      Tuple<4,ENTRY> result; */
/*      result[0] = value; */
/*      result[1] = m_first; */
/*      result[2] = m_second; */
/*      result[3] = m_third; */
/*      return result; */
/*    } */

  /// rac returns the last tuple element. rac is the opposite of car
  /// (and the name is chosen to reflect that fact).
  inline const ENTRY& rac() const {
    return m_third;
  }

  /// Like rac, rdc is the opposite of cdr. rdc returns a tuple of the
  /// remaining elements before the element returned by rac.
  inline Tuple<2,ENTRY> rdc() const {
    return tuple(m_first,m_second);
  }

/*    inline Tuple<4,ENTRY> snoc(const ENTRY& value) const { */
/*      Tuple<4,ENTRY> result; */
/*      result[0] = m_first; */
/*      result[1] = m_second; */
/*      result[2] = m_third; */
/*      result[3] = value; */
/*      return result; */
/*    } */

/*   inline Tuple<DIM2 + 3, ENTRY> construct(const Tuple<DIM2,ENTRY>& product) const { */
/*     Tuple<DIM2 + 3, ENTRY> result; */
/*     int d = 0; */
/*     for (; d < DIM2; ++d) */
/*       result[d] = product[d]; */
/*     result[d] = m_first; ++d; */
/*     result[d] = m_second; ++d; */
/*     result[d] = m_third; */
/*     assert(d == DIM2 + 2); */
/*     return result; */
/*   } */

  inline Tuple<3,ENTRY> reverse() const {
    Tuple<3,ENTRY> result;
    result[0] = m_third;
    result[1] = m_second;
    result[2] = m_first;
    return result;
  }

  inline Tuple<3,ENTRY>& operator+=(const Tuple<3,ENTRY>& rhs) {
    m_first += rhs[0];
    m_second += rhs[1];
    m_third += rhs[2];
    return *this;
  }

  inline Tuple<3,ENTRY>& operator-=(const Tuple<3,ENTRY>& rhs) {
    m_first -= rhs[0];
    m_second -= rhs[1];
    m_third += rhs[2];
    return *this;
  }

  inline Tuple<3,ENTRY>& operator*=(const Tuple<3,ENTRY>& rhs) {
    m_first *= rhs[0];
    m_second *= rhs[1];
    m_third += rhs[2];
    return *this;
  }

  inline Tuple<3,ENTRY>& operator/=(const Tuple<3,ENTRY>& rhs) {
    m_first /= rhs[0];
    m_second /= rhs[1];
    m_third += rhs[2];
    return *this;
  }
};

// Including this function leads to the compiler error: Multiple declaration for dim.

template <typename ENTRY>
inline Tuple<3,ENTRY> tuple(const ENTRY& value1, const ENTRY& value2, const ENTRY& value3) {
  return Tuple<3,ENTRY>(value1,value2,value3);
}
