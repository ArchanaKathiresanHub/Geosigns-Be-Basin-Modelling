public:

  inline void expandToCover(const Tuple<2,COUNTABLE>& crd) {
    if (empty()) {
      m_low = crd;
      m_high[0] = crd[0] + 1;
      m_high[1] = crd[1] + 1;
    }
    else {
      if (crd[0] < m_low[0]) m_low[0] = crd[0];
      if (crd[0] >= m_high[0]) m_high[0] = crd[0] + static_cast<COUNTABLE>(1);
      if (crd[1] < m_low[1]) m_low[1] = crd[1];
      if (crd[1] >= m_high[1]) m_high[1] = crd[1] + static_cast<COUNTABLE>(1);
    }
  }
