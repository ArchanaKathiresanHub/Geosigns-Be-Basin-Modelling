namespace multi_array {

template <typename ENTRY>
class Interval<2,ENTRY>
{
public:

  enum { dim = 2, dim_min_1 = 1, dim_plus_1 = 3 };
  typedef ENTRY Entry;

protected:

  Tuple<2,ENTRY> m_low, m_high;

  inline Interval():
    m_low (Tuple<2,ENTRY>::filled(-numeric_limits<ENTRY>::max())),
    m_high(Tuple<2,ENTRY>::filled(-numeric_limits<ENTRY>::max()))
  {}

  inline Interval(const Tuple<2,ENTRY>& lw, const Tuple<2,ENTRY>& hgh):
    m_low(lw),
    m_high(hgh)
  {}

  inline Interval(const Interval<1,ENTRY>& head, const Interval<1,ENTRY>& tail) {
    m_low[0] = head.low(0);
    m_high[0] = head.high(0);
    m_low[1] = tail.low(0);
    m_high[1] = tail.high(0);
  }

public:

  inline const Tuple<2,ENTRY>& low() const {
    return m_low;
  }

  inline const ENTRY& low(int d) const {
    assert(d >= 0 && d < 2);
    return m_low[d];
  }

  inline ENTRY& low(int d) {
    assert(d >= 0 && d < 2);
    return m_low[d];
  }

  inline const Tuple<2,ENTRY>& high() const {
    return m_high;
  }

  inline const ENTRY& high(int d) const {
    assert(d >= 0 && d < 2);
    return m_high[d];
  }

  inline ENTRY& high(int d) {
    assert(d >= 0 && d < 2);
    return m_high[d];
  }

  inline Tuple<2,ENTRY> size() const {
    Tuple<2,ENTRY> result;
    result[0] = size(0);
    result[1] = size(1);
    return result;
  }

  inline ENTRY size(int d) const {
    assert(d >= 0 && d < 2);
    return max(high(d) - low(d),0);
  }

  inline ENTRY volume() const {
    ENTRY result = size(0);
    result *= size(1);
    return result;
  }

  inline bool empty() const {
    if (m_high[0] <= m_low[0]) return true;
    return m_high[1] <= m_low[1];
  }

protected:

  inline bool operator==(const Interval<2,ENTRY>& rhs) const
  {
    if (m_low[0] != rhs.m_low[0] || m_high[0] != rhs.m_high[0])
	return false;
    return m_low[1] == rhs.m_low[1] || m_high[1] == rhs.m_high[1];
  }

  inline bool operator!=(const Interval<2,ENTRY>& rhs) const
  {
    return !operator==(rhs);
  }

  inline bool contains(const Interval<2,ENTRY>& other) const {
    if (other.m_low[0] < m_low[0] || m_high[0] < other.m_high[0])
      return false;
    if (other.m_low[1] < m_low[1] || m_high[1] < other.m_high[1])
      return false;
    return other.empty();
  }

  inline bool overlaps(const Interval<2,ENTRY>& other) const {
    if (other.m_high[0] <= m_low[0] || m_high[0] <= other.m_low[0])
      return false;
    if (other.m_high[1] <= m_low[1] || m_high[1] <= other.m_low[1])
      return false;
    return !other.empty();
  }

  inline void expandToCover(const Interval<2,ENTRY>& other) {
    if (other.empty()) return;
    if (empty())
      *this = other;
    else {
      for (int d = 0; d < 2; ++d) {
        if (other.low(d) < m_low[d]) m_low[d] = other.low(d);
        if (other.high(d) > m_high[d]) m_high[d] = other.high(d);
      }
    }
  }

  inline void intersectWith(const Interval<2,ENTRY>& other) {
    if (other.low(0) > m_low[0]) m_low[0] = other.low(0);
    if (other.high(0) < m_high[0]) m_high[0] = other.high(0);
    if (other.low(1) > m_low[1]) m_low[1] = other.low(1);
    if (other.high(1) < m_high[1]) m_high[1] = other.high(1);
  }
};

} // namespace multi_array






