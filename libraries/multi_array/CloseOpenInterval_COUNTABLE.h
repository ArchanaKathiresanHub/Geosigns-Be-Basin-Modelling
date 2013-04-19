public:

  inline void expandToCover(const Tuple<DIM,COUNTABLE>& crd) {
    if (empty()) {
      m_low = crd;
      m_high = crd + Tuple<DIM,COUNTABLE>::filled(static_cast<COUNTABLE>(1));
    }
    else {
      for (int d = 0; d < DIM; ++d) {
        if (crd[d] < m_low[d]) m_low[d] = crd[d];
        if (crd[d] >= m_high[d]) m_high[d] = crd[d] + static_cast<COUNTABLE>(1);
      }
    }
  }
