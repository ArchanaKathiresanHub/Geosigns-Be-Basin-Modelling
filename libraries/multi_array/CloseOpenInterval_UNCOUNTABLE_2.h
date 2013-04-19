public:

  inline static CloseOpenInterval<2,UNCOUNTABLE> everywhere() {
    return CloseOpenInterval(
      Tuple<2,UNCOUNTABLE>::filled(-numeric_limits<UNCOUNTABLE>::max()),
      Tuple<2,UNCOUNTABLE>::filled( numeric_limits<UNCOUNTABLE>::max()));
  }

  inline static CloseOpenInterval<2,UNCOUNTABLE> nowhere() {
    return CloseOpenInterval();
  }

  inline CloseOpenInterval()
  {}

  inline CloseOpenInterval(const Tuple<2,UNCOUNTABLE>& lw, const Tuple<2,UNCOUNTABLE>& hgh):
    Interval<2,UNCOUNTABLE>(lw,hgh)
  {}

  inline CloseOpenInterval(const Interval<1,UNCOUNTABLE>& head, const Interval<1,UNCOUNTABLE>& tail):
    Interval<2,UNCOUNTABLE>(head,tail)
  {}

  inline CloseOpenInterval<1,UNCOUNTABLE> operator[](int d) const {
    assert(d == 0 || d == 1);
    return CloseOpenInterval<1,UNCOUNTABLE>(tuple(low(d)),tuple(high(d)));
  }

  inline CloseOpenInterval<1,UNCOUNTABLE> car() const {
    return CloseOpenInterval<1,UNCOUNTABLE>(m_low.car(), m_high.car());
  }

  inline CloseOpenInterval<1,UNCOUNTABLE> cdr() const {
    return CloseOpenInterval<1,UNCOUNTABLE>(m_low.cdr(), m_high.cdr());
  }

  inline CloseOpenInterval<1,UNCOUNTABLE> rac() const {
    return CloseOpenInterval<1,UNCOUNTABLE>(m_low.rac(), m_high.rac());
  }

  inline CloseOpenInterval<1,UNCOUNTABLE> rdc() const {
    return CloseOpenInterval<1,UNCOUNTABLE>(m_low.rdc(), m_high.rdc());
  }

  inline bool contains(const Tuple<2,UNCOUNTABLE>& crd) const {
    if (crd[0] < m_low[0] || m_high[0] <= crd[0])
        return false;
    return !(crd[1] < m_low[1] || m_high[1] <= crd[1]);
  }

  inline bool contains(const CloseOpenInterval<2,UNCOUNTABLE>& other) const {
    return Interval<2,UNCOUNTABLE>::contains(other);
  }

  inline bool overlaps(const CloseOpenInterval<2,UNCOUNTABLE>& other) const {
    if (other.m_high[0] <= m_low[0] || m_high[0] <= other.m_low[0])
      return false;
    if (other.m_high[1] <= m_low[1] || m_high[1] <= other.m_low[1])
      return false;
    return !other.empty();
  }

  inline void expandToCover(const CloseOpenInterval<2,UNCOUNTABLE>& other) {
    Interval<2,UNCOUNTABLE>::expandToCover(other);
  }

  inline void intersectWith(const CloseOpenInterval<2,UNCOUNTABLE>& other) {
    Interval<2,UNCOUNTABLE>::intersectWith(other);
  }

  inline CloseOpenInterval<2,UNCOUNTABLE> supremum(const CloseOpenInterval<2,UNCOUNTABLE>& other) const {
    CloseOpenInterval result = *this;
    result.expandToCover(other);
    return result;
  }

  inline CloseOpenInterval<2,UNCOUNTABLE> infimum(const CloseOpenInterval<2,UNCOUNTABLE>& other) const {
    CloseOpenInterval result = *this;
    result.intersectWith(other);
    return result;
  }

  inline bool operator==(const CloseOpenInterval<2,UNCOUNTABLE>& rhs) const {
    return Interval<2,UNCOUNTABLE>::operator==(rhs);
  }

  inline bool operator!=(const CloseOpenInterval<2,UNCOUNTABLE>& rhs) const {
    return Interval<2,UNCOUNTABLE>::operator!=(rhs);
  }

  inline CloseOpenInterval<2,UNCOUNTABLE>& operator+=(const Tuple<2,UNCOUNTABLE>& crd) {
    m_low += crd; m_high += crd;
    return *this;
  }

  inline CloseOpenInterval<2,UNCOUNTABLE>& operator*=(const Tuple<2,UNCOUNTABLE>& crd) {
    m_low *= crd; m_high *= crd;
    return *this;
  }

  inline CloseOpenInterval<2,UNCOUNTABLE>& operator-=(const Tuple<2,UNCOUNTABLE>& crd) {
    m_low -= crd; m_high -= crd;
    return *this;
  }

  inline CloseOpenInterval<2,UNCOUNTABLE>& operator/=(const Tuple<2,UNCOUNTABLE>& crd) {
    m_low /= crd; m_high /= crd;
    return *this;
  }

  inline CloseOpenInterval<2,UNCOUNTABLE> operator+(const Tuple<2,UNCOUNTABLE>& crd) const {
    return CloseOpenInterval(low() + crd, high() + crd);
  }

  inline CloseOpenInterval<2,UNCOUNTABLE> operator*(const Tuple<2,UNCOUNTABLE>& crd) const {
    return CloseOpenInterval(low() * crd, high() * crd);
  }



