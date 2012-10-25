public:

  inline void expandToCover(const Tuple<1,COUNTABLE>& crd) {
    if (empty()) {
      m_low = crd;
      m_high[0] = crd[0] + 1;
    }
    else {
      if (crd[0] < m_low[0]) m_low = crd;
      if (crd[0] >= m_high[0]) m_high[0] = crd[0] + 1;
    }
  }
