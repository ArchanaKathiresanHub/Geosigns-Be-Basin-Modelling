namespace multi_array {

template <typename ENTRY>
class Interval<1,ENTRY>
{
public:

  enum { dim = 1, dim_min_1 = 0, dim_plus_1 = 2 };
  typedef ENTRY Entry;

protected:

  Tuple<1,ENTRY> m_low, m_high;

  inline Interval():
    m_low (-numeric_limits<ENTRY>::max()),
    m_high(-numeric_limits<ENTRY>::max())
  {}

  inline Interval(const Tuple<1,ENTRY>& lw, const Tuple<1,ENTRY>& hgh):
    m_low(lw[0]),
    m_high(hgh[0])
  {}

  inline Interval(const ENTRY& lw, const ENTRY& hgh):
    m_low(lw),
    m_high(hgh)
  {}

public:

  inline const Tuple<1,ENTRY>& low() const {
    return m_low;
  }

  inline const ENTRY& low(int d) const {
    assert(d == 0);
    return m_low[0];
  }

  inline ENTRY& low(int d) {
    assert(d == 0);
    return m_low[0];
  }

  inline const Tuple<1,ENTRY>& high() const {
    return m_high;
  }

  inline const ENTRY& high(int d) const {
    assert(d == 0);
    return m_high[0];
  }

  inline ENTRY& high(int d) {
    assert(d == 0);
    return m_high[0];
  }

  inline Tuple<1,ENTRY> size() const {
    Tuple<1,ENTRY> result(tuple(max(high(0) - low(0),0)));
    return result;
  }

  inline ENTRY size(int d) const {
    assert(d == 0);
    return max(high(0) - low(0),0);
  }

  inline ENTRY volume() const {
    return size(0);
  }

  inline bool empty() const {
    return m_high[0] <= m_low[0];
  }

protected:

  inline bool operator==(const Interval<1,ENTRY>& rhs) const
  {
    return m_low[0] == rhs.m_low[0] && m_high[0] == rhs.m_high[0];
  }

  inline bool operator!=(const Interval<1,ENTRY>& rhs) const
  {
    return !operator==(rhs);
  }

  inline bool contains(const Interval<1,ENTRY>& other) const {
    if (other.m_low[0] < m_low[0] || m_high[0] < other.m_high[0])
      return false;
    return other.empty();
  }

  inline void expandToCover(const Interval<1,ENTRY>& other) {
    if (other.empty()) return;
    if (empty())
      *this = other;
    else {
      if (other.low(0) < m_low[0]) m_low[0] = other.low(0);
      if (other.high(0) > m_high[0]) m_high[0] = other.high(0);
    }
  }

  inline void intersectWith(const Interval<1,ENTRY>& other) {
    if (other.low(0) > m_low[0]) m_low[0] = other.low(0);
    if (other.high(0) < m_high[0]) m_high[0] = other.high(0);
  }
};

} // namespace multi_array

