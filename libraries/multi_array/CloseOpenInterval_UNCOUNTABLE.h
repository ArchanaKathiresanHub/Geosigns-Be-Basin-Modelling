public:

  inline static CloseOpenInterval<DIM,UNCOUNTABLE> everywhere() {
    return CloseOpenInterval(
      Tuple<DIM,UNCOUNTABLE>::filled(-numeric_limits<UNCOUNTABLE>::max()),
      Tuple<DIM,UNCOUNTABLE>::filled( numeric_limits<UNCOUNTABLE>::max()));
  }

  inline static CloseOpenInterval<DIM,UNCOUNTABLE> nowhere() {
    return CloseOpenInterval();
  }

  inline CloseOpenInterval()
  {}

  inline CloseOpenInterval(const Tuple<DIM,UNCOUNTABLE>& lw, const Tuple<DIM,UNCOUNTABLE>& hgh):
    Interval<DIM,UNCOUNTABLE>(lw,hgh)
  {}

  inline CloseOpenInterval<1,UNCOUNTABLE> operator[](int d) const {
    assert(0 <= d && d < DIM);
    return CloseOpenInterval<1,UNCOUNTABLE>(tuple(low(d)),tuple(high(d)));
  }

  inline CloseOpenInterval<1,UNCOUNTABLE> car() const {
    return CloseOpenInterval<1,UNCOUNTABLE>(m_low.car(), m_high.car());
  }

  inline CloseOpenInterval<dim_min_1,UNCOUNTABLE> cdr() const {
    return CloseOpenInterval<dim_min_1,UNCOUNTABLE>(low().cdr(), high().cdr());
  }

  inline CloseOpenInterval<1,UNCOUNTABLE> rac() const {
    return CloseOpenInterval<1,UNCOUNTABLE>(low().rac(), high().rac());
  }

  inline CloseOpenInterval<dim_min_1,UNCOUNTABLE> rdc() const {
    return CloseOpenInterval<dim_min_1,UNCOUNTABLE>(m_low.rdc(), m_high.rdc());
  }

  inline bool contains(const Tuple<DIM,UNCOUNTABLE>& crd) const {
    for (int d = 0; d < DIM; ++d)
      if (crd[d] < m_low[d] || m_high[d] <= crd[d])
        return false;
    return true;
  }

  inline bool contains(const CloseOpenInterval<DIM,UNCOUNTABLE>& other) const {
#ifdef sun
    for (int d = 0; d < DIM; ++d)
      if (other.m_low[d] < m_low[d] || m_high[d] < other.m_high[d])
        return false;
    return other.empty();
#else
    return Interval<DIM,UNCOUNTABLE>::contains(other);
#endif
  }

  inline bool overlaps(const CloseOpenInterval<DIM,UNCOUNTABLE>& other) const {
#ifdef sun
    for (int d = 0; d < DIM; ++d)
      if (other.m_high[d] <= m_low[d] || m_high[d] <= other.m_low[d])
        return false;
    return !other.empty();
#else
    return Interval<DIM,UNCOUNTABLE>::overlaps(other);
#endif
  }

  inline void expandToCover(const CloseOpenInterval<DIM,UNCOUNTABLE>& other) {
    Interval<DIM,UNCOUNTABLE>::expandToCover(other);
  }

  inline void intersectWith(const CloseOpenInterval<DIM,UNCOUNTABLE>& other) {
    Interval<DIM,UNCOUNTABLE>::intersectWith(other);
  }

  inline CloseOpenInterval<DIM,UNCOUNTABLE> supremum(const CloseOpenInterval<DIM,UNCOUNTABLE>& other) const {
    CloseOpenInterval result = *this;
    result.expandToCover(other);
    return result;
  }

  inline CloseOpenInterval<DIM,UNCOUNTABLE> infimum(const CloseOpenInterval<DIM,UNCOUNTABLE>& other) const {
    CloseOpenInterval result = *this;
    result.intersectWith(other);
    return result;
  }

  inline bool operator==(const CloseOpenInterval<DIM,UNCOUNTABLE>& rhs) const {
    return Interval<DIM,UNCOUNTABLE>::operator==(Interval<DIM,UNCOUNTABLE>(rhs));
  }

  inline bool operator!=(const CloseOpenInterval<DIM,UNCOUNTABLE>& rhs) const {
    return Interval<DIM,UNCOUNTABLE>::operator!=(rhs);
  }

  inline CloseOpenInterval<DIM,UNCOUNTABLE>& operator+=(const Tuple<DIM,UNCOUNTABLE>& crd) {
    m_low += crd; m_high += crd;
    return *this;
  }

  inline CloseOpenInterval<DIM,UNCOUNTABLE>& operator-=(const Tuple<DIM,UNCOUNTABLE>& crd) {
    m_low -= crd; m_high -= crd;
    return *this;
  }

  inline CloseOpenInterval<DIM,UNCOUNTABLE>& operator*=(const Tuple<DIM,UNCOUNTABLE>& crd) {
    m_low *= crd; m_high *= crd;
    return *this;
  }

  inline CloseOpenInterval<DIM,UNCOUNTABLE>& operator/=(const Tuple<DIM,UNCOUNTABLE>& crd) {
    m_low /= crd; m_high /= crd;
    return *this;
  }

  inline CloseOpenInterval<DIM,UNCOUNTABLE> operator+(const Tuple<DIM,UNCOUNTABLE>& crd) const {
    return CloseOpenInterval(low() + crd, high() + crd);
  }

  inline CloseOpenInterval<DIM,UNCOUNTABLE> operator*(const Tuple<DIM,UNCOUNTABLE>& crd) const {
    return CloseOpenInterval(low() * crd, high() * crd);
  }
