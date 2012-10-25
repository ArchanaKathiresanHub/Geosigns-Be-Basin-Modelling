template <typename ENTRY>
class Tuple<1,ENTRY>
{
public:

  enum { dim = 1, dim_min_1 = 0, dim_plus_1 = 2 };
  typedef ENTRY Entry;

private:

  ENTRY m_first;

public:

  inline static Tuple<1,ENTRY> filled(const ENTRY& value) {
    return Tuple<1,ENTRY>(value);
  }

  inline Tuple() {
  }

  inline ~Tuple() {
  }

  inline Tuple(const ENTRY* data) {
    m_first = data[0];
  }

  inline Tuple(const ENTRY& head, const Tuple<0,ENTRY>& tail)
  {
    m_first = head;
  }

  inline Tuple(const Tuple<0,ENTRY>& head, const ENTRY& tail)
  {
    m_first = tail;
  }

  inline explicit Tuple(const ENTRY& value):
    m_first(value)
  {}

  inline const ENTRY& operator[](int d) const {
    assert(d == 0);
    return m_first;
  }

  inline ENTRY& operator[](int d) {
    assert(d == 0);
    return m_first;
  }

  inline const ENTRY& car() const {
    return m_first;
  }

/*    inline Tuple<2,ENTRY> cons(const ENTRY& value) const { */
/*      Tuple<2,ENTRY> result; */
/*      result[0] = value; */
/*      result[1] = m_first; */
/*      return result; */
/*    } */

  /// rac returns the last tuple element. rac is the opposite of car
  /// (and the name is chosen to reflect that fact).
  inline const ENTRY& rac() const {
    return m_first;
  }

/*    inline Tuple<2,ENTRY> snoc(const ENTRY& value) const { */
/*      Tuple<2,ENTRY> result; */
/*      result[0] = m_first; */
/*      result[1] = value; */
/*      return result; */
/*    } */

/*   inline Tuple<DIM2 + 1, ENTRY> construct(const Tuple<DIM2,ENTRY>& product) const { */
/*     Tuple<DIM2 + 1, ENTRY> result; */
/*     int d = 0; */
/*     for (; d < DIM2; ++d) */
/*       result[d] = product[d]; */
/*     result[d] = m_first; */
/*     assert(d == DIM2); */
/*     return result;      */
/*   } */

  inline Tuple<1,ENTRY>& reverse() const {
    return *this;
  }

  inline Tuple<1,ENTRY>& operator+=(const Tuple<1,ENTRY>& rhs) {
    m_first += rhs[0];
    return *this;
  }

  inline Tuple<1,ENTRY>& operator-=(const Tuple<1,ENTRY>& rhs) {
    m_first -= rhs[0];
    return *this;
  }

  inline Tuple<1,ENTRY>& operator*=(const Tuple<1,ENTRY>& rhs) {
    m_first *= rhs[0];
    return *this;
  }

  inline Tuple<1,ENTRY>& operator/=(const Tuple<1,ENTRY>& rhs) {
    m_first /= rhs[0];
    return *this;
  }
};

// Including this function leads to the compiler error: Multiple declaration for dim.

template <typename ENTRY>
inline Tuple<1,ENTRY> tuple(const ENTRY& value) {
  return Tuple<1,ENTRY>::filled(value);
}
