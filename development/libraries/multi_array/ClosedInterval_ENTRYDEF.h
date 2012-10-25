  inline bool overlaps(const ClosedInterval<dim,ENTRYDEF>& other) const {
    for (int d = 0; d < dim; ++d)
      if (other.m_high[d] < m_low[d] || m_high[d] < other.m_low[d])
        return false;
    return !other.empty();
  }

  inline bool contains(const Tuple<dim,ENTRYDEF>& crd) const {
    for (int d = 0; d < dim; ++d)
      if (crd[d] < m_low[d] || m_high[d] < crd[d])
        return false;
    return true;
  }

  inline void expandToCover(const Tuple<dim,ENTRYDEF>& crd) {
    if (empty()) {
      m_low = crd;
      m_high = crd;
    }
    else {
      for (int d = 0; d < dim; ++d) {
        if (crd[d] < m_low[d]) m_low[d] = crd[d];
        if (crd[d] > m_high[d]) m_high[d] = crd[d];
      }
    }
  }


