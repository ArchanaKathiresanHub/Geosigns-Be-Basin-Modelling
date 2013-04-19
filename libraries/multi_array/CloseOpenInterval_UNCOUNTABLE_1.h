public:

  friend class CloseOpenInterval<2,UNCOUNTABLE>;

  inline static CloseOpenInterval<1,UNCOUNTABLE> everywhere() {
    return CloseOpenInterval(
      -numeric_limits<UNCOUNTABLE>::max(),
       numeric_limits<UNCOUNTABLE>::max() );
  }

  inline static CloseOpenInterval<1,UNCOUNTABLE> nowhere() {
    return CloseOpenInterval();
  }

  inline CloseOpenInterval()
  {}

  inline CloseOpenInterval(const Tuple<1,UNCOUNTABLE>& lw, const Tuple<1,UNCOUNTABLE>& hgh):
    Interval<1,UNCOUNTABLE>(lw,hgh)
  {}

  inline CloseOpenInterval(const UNCOUNTABLE& lw, const UNCOUNTABLE& hgh):
    Interval<1,UNCOUNTABLE>(lw,hgh)
  {}

  inline CloseOpenInterval<1,UNCOUNTABLE> operator[](int d) const {
    assert(d == 0);
    return *this;
  }

  inline CloseOpenInterval<1,UNCOUNTABLE> car() const {
    return *this;
  }

  inline CloseOpenInterval<1,UNCOUNTABLE> rac() const {
    return *this;
  }

  inline bool contains(const Tuple<1,UNCOUNTABLE>& crd) const {
    return !(crd[0] < m_low[0] || m_high[0] <= crd[0]);
  }

  inline bool contains(const CloseOpenInterval<1,UNCOUNTABLE>& other) const {
    return Interval<1,UNCOUNTABLE>::contains(other);
  }

  inline bool overlaps(const CloseOpenInterval<1,UNCOUNTABLE>& other) const {
    if (other.m_high[0] <= m_low[0] || m_high[0] <= other.m_low[0])
      return false;
    return !other.empty();
  }

  inline void expandToCover(const CloseOpenInterval<1,UNCOUNTABLE>& other) {
    Interval<1,UNCOUNTABLE>::expandToCover(other);
  }

  inline void intersectWith(const CloseOpenInterval<1,UNCOUNTABLE>& other) {
    Interval<1,UNCOUNTABLE>::intersectWith(other);
  }

  inline CloseOpenInterval<1,UNCOUNTABLE> supremum(const CloseOpenInterval<1,UNCOUNTABLE>& other) const {
    CloseOpenInterval result = *this;
    result.expandToCover(other);
    return result;
  }

  inline CloseOpenInterval<1,UNCOUNTABLE> infimum(const CloseOpenInterval<1,UNCOUNTABLE>& other) const {
    CloseOpenInterval result = *this;
    result.intersectWith(other);
    return result;
  }

  inline bool operator==(const CloseOpenInterval<1,UNCOUNTABLE>& rhs) const {
    return Interval<1,UNCOUNTABLE>::operator==(rhs);
  }

  inline bool operator!=(const CloseOpenInterval<1,UNCOUNTABLE>& rhs) const {
    return Interval<1,UNCOUNTABLE>::operator!=(rhs);
  }

  inline CloseOpenInterval<1,UNCOUNTABLE>& operator+=(const Tuple<1,UNCOUNTABLE>& crd) {
    m_low += crd; m_high += crd;
    return *this;
  }

  inline CloseOpenInterval<1,UNCOUNTABLE>& operator*=(const Tuple<1,UNCOUNTABLE>& crd) {
    m_low *= crd; m_high *= crd;
    return *this;
  }

  inline CloseOpenInterval<1,UNCOUNTABLE>& operator-=(const Tuple<1,UNCOUNTABLE>& crd) {
    m_low -= crd; m_high -= crd;
    return *this;
  }

  inline CloseOpenInterval<1,UNCOUNTABLE>& operator/=(const Tuple<1,UNCOUNTABLE>& crd) {
    m_low /= crd; m_high /= crd;
    return *this;
  }

  inline CloseOpenInterval<1,UNCOUNTABLE> operator+(const Tuple<1,UNCOUNTABLE>& crd) const {
    return CloseOpenInterval(low() + crd, high() + crd);
  }

  inline CloseOpenInterval<1,UNCOUNTABLE> operator*(const Tuple<1,UNCOUNTABLE>& crd) const {
    return CloseOpenInterval(low() * crd, high() * crd);
  }

