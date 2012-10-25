template <typename ENTRY>
class ClosedInterval<2,ENTRY>: public Interval<2,ENTRY> 
{
  friend ClosedInterval<2,ENTRY> cons<>(const ClosedInterval<1,ENTRY>& head, const ClosedInterval<
    1,ENTRY>& tail);

  friend ClosedInterval<2,ENTRY> snoc<>(const ClosedInterval<1,ENTRY>& head, const ClosedInterval<
    1,ENTRY>& tail);
  
  inline static ClosedInterval<2,ENTRY> everywhere() {
    return ClosedInterval(
      m_low (Tuple<2,ENTRY>::filled(-numeric_limits<ENTRY>::max())),
      m_high(Tuple<2,ENTRY>::filled( numeric_limits<ENTRY>::max())));
  }

  inline static ClosedInterval<2,ENTRY> nowhere() {
    return ClosedInterval();
  }

  inline ClosedInterval()
  {}

  inline ClosedInterval(const Tuple<2,ENTRY>& low, const Tuple<2,ENTRY>& high):
    Interval<1,ENTRY>(low,high)
  {}

  inline ClosedInterval<1,ENTRY> operator[](int d) const {
    assert(domain().contains(d));
    return ClosedInterval<1,ENTRY>(tuple(low(d)),tuple(high(d)));
  }

  inline ClosedInterval<1,ENTRY> car() const {
    return ClosedInterval<1,ENTRY>(low().car(), high().car());
  }

  inline ClosedInterval<1,ENTRY> cdr() const {
    return ClosedInterval(low().cdr(), high().cdr());
  }

  inline ClosedInterval<1,ENTRY> rac() const {
    return ClosedInterval<1,ENTRY>(low().rac(), high().rac());
  }

  inline ClosedInterval<1,ENTRY> rdc() const {
    return ClosedInterval<1,ENTRY>(low().rdc(), high().rdc());
  }

  inline bool contains(const Tuple<2,ENTRY>& crd) const {
    if (crd[0] < m_low[0] || m_high[0] < crd[0])
      return false;
    if (crd[1] < m_low[1] || m_high[1] < crd[1])
      return false;
    return true;
  }

  inline bool contains(const ClosedInterval<2,ENTRY>& other) const {
    return Interval<2,ENTRY>::contains(other);
  }

  inline bool overlaps(const ClosedInterval<2,ENTRY>& other) const {
    if (other.m_high[0] < m_low[0] || m_high[0] < other.m_low[0])
      return false;
    if (other.m_high[1] < m_low[1] || m_high[1] < other.m_low[1])
      return false;
    return !other.empty();
  }

  inline void expandToCover(const Tuple<2,ENTRY>& crd) {
    if (empty()) {
      m_low = crd;
      m_high = crd;
    }
    else {
      if (crd[0] < m_low[0]) m_low[0] = crd[0];
      if (crd[0] > m_high[0]) m_high[0] = crd[0];
      if (crd[1] < m_low[1]) m_low[1] = crd[1];
      if (crd[1] > m_high[1]) m_high[1] = crd[1];
    }
  }

  inline void expandToCover(const ClosedInterval<2,ENTRY>& other) {
    Interval<2,ENTRY>::expandToCover(other);
  }

  inline void intersectWith(const ClosedInterval<2,ENTRY>& other) {
    Interval<2,ENTRY>::intersectWith(other);
  }

  inline ClosedInterval<2,ENTRY> supremum(const ClosedInterval<2,ENTRY>& other) const {
    ClosedInterval result = *this;
    result.expandToCover(other);
    return result;
  }

  inline ClosedInterval<2,ENTRY> infimum(const ClosedInterval<2,ENTRY>& other) const {
    ClosedInterval result = *this;
    result.intersectWith(other);
    return result;
  }

  inline bool operator==(const ClosedInterval<2,ENTRY>& rhs) const {
    return Interval<2,ENTRY>::operator==(rhs);
  }

  inline bool operator!=(const ClosedInterval<2,ENTRY>& rhs) const {
    return Interval<2,ENTRY>::operator!=(rhs);
  }

  inline ClosedInterval<2,ENTRY>& operator+=(const Tuple<2,ENTRY>& crd) {
    m_low += crd; m_high += crd;
    return ClosedInterval(*this);
  }

  inline ClosedInterval<2,ENTRY>& operator-=(const Tuple<2,ENTRY>& crd) {
    m_low -= crd; m_high -= crd;
    return ClosedInterval(*this);
  }

  inline ClosedInterval<2,ENTRY>& operator*=(const Tuple<2,ENTRY>& crd) {
    m_low *= crd; m_high *= crd;
    return ClosedInterval(*this);
  }

  inline ClosedInterval<2,ENTRY>& operator/=(const Tuple<2,ENTRY>& crd) {
    m_low /= crd; m_high /= crd;
    return ClosedInterval(*this);
  }

  inline ClosedInterval<2,ENTRY> operator+(const Tuple<2,ENTRY>& crd) const {
    return ClosedInterval(low() + crd, high() + crd);
  }

  inline ClosedInterval<2,ENTRY> operator*(const Tuple<2,ENTRY>& crd) const {
    return ClosedInterval(low() * crd, high() * crd);
  }
};



