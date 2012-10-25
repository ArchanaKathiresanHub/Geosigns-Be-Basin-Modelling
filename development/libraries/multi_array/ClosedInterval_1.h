template <typename ENTRY>
class ClosedInterval<1,ENTRY>: public Interval<1,ENTRY> 
{
public:

  friend class ClosedInterval<2,ENTRY>;

  inline static ClosedInterval<1,ENTRY> everywhere() {
    return ClosedInterval(
      m_low (tuple(-numeric_limits<ENTRY>::max())),
      m_high(tuple( numeric_limits<ENTRY>::max())));
  }

  inline static ClosedInterval<1,ENTRY> nowhere() {
    return ClosedInterval();
  }

  inline ClosedInterval()
  {}

  inline ClosedInterval(const Tuple<1,ENTRY>& lw, const Tuple<1,ENTRY>& hgh):
    Interval<1,ENTRY>(lw,hgh)
  {}

  inline ClosedInterval<1,ENTRY> operator[](int d) const {
    assert(domain().contains(d));
    return ClosedInterval<1,ENTRY>(tuple(low(d)),tuple(high(d)));
  }

  inline const ClosedInterval<1,ENTRY>& car() const {
    return *this;
  }

  inline ClosedInterval<1,ENTRY> rac() const {
    return *this;
  }

  inline bool contains(const Tuple<1,ENTRY>& crd) const {
    return !(crd[0] < m_low[0] || m_high[0] < crd[0]);
  }

  inline bool contains(const ClosedInterval<1,ENTRY>& other) const {
    return Interval<1,ENTRY>::contains(other);
  }

  inline bool overlaps(const ClosedInterval<1,ENTRY>& other) const {
    if (other.m_high[0] < m_low[0] || m_high[0] < other.m_low[0])
      return false;
    return !other.empty();
  }

  inline void expandToCover(const Tuple<1,ENTRY>& crd) {
    if (empty()) {
      m_low = crd;
      m_high = crd;
    }
    else {
      if (crd[0] < m_low[0]) m_low[0] = crd[0];
      if (crd[0] > m_high[0]) m_high[0] = crd[0];
    }
  }

  inline void expandToCover(const ClosedInterval<1,ENTRY>& other) {
    Interval<1,ENTRY>::expandToCover(other);
  }

  inline void intersectWith(const ClosedInterval<1,ENTRY>& other) {
    Interval<1,ENTRY>::intersectWith(other);
  }

  inline ClosedInterval<1,ENTRY> supremum(const ClosedInterval<1,ENTRY>& other) const {
    ClosedInterval result = *this;
    result.expandToCover(other);
    return result;
  }

  inline ClosedInterval<1,ENTRY> infimum(const ClosedInterval<1,ENTRY>& other) const {
    ClosedInterval result = *this;
    result.intersectWith(other);
    return result;
  }

  inline bool operator==(const ClosedInterval<1,ENTRY>& rhs) const {
    return Interval<1,ENTRY>::operator==(rhs);
  }

  inline bool operator!=(const ClosedInterval<1,ENTRY>& rhs) const {
    return Interval<1,ENTRY>::operator!=(rhs);
  }

  inline ClosedInterval<1,ENTRY>& operator+=(const Tuple<1,ENTRY>& crd) {
    m_low += crd; m_high += crd;
    return ClosedInterval(*this);
  }

  inline ClosedInterval<1,ENTRY>& operator-=(const Tuple<1,ENTRY>& crd) {
    m_low -= crd; m_high -= crd;
    return ClosedInterval(*this);
  }

  inline ClosedInterval<1,ENTRY>& operator*=(const Tuple<1,ENTRY>& crd) {
    m_low *= crd; m_high *= crd;
    return ClosedInterval(*this);
  }

  inline ClosedInterval<1,ENTRY>& operator/=(const Tuple<1,ENTRY>& crd) {
    m_low /= crd; m_high /= crd;
    return ClosedInterval(*this);
  }

  inline ClosedInterval<1,ENTRY> operator+(const Tuple<1,ENTRY>& crd) const {
    return ClosedInterval(low() + crd, high() + crd);
  }

  inline ClosedInterval<1,ENTRY> operator*(const Tuple<1,ENTRY>& crd) const {
    return ClosedInterval(low() * crd, high() * crd);
  }
};




