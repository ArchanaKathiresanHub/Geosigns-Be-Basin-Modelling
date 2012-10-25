#include "multi_array/RMAdvancer.h"

namespace multi_array {

template <int DIM, typename ENTRY>
class RMFORWARDPOSCURSOR
{
private:

  CURSOR m_cursor;
  RMAdvancer<DIM> m_calcCrd;
  Tuple<DIM,int> m_domainCrd;

public:

  /// For simplicity purposes, the following constructors are public, even though they are not
  /// supposed to be called directly. However, making them private involves including a lot of
  /// templatized friend class declarations.

  /// Constructor
  ///
  /// @param[in] crsr: the cursor over the array data.
  /// @param[in] dmn: the domain of the array.
  inline RMFORWARDPOSCURSOR(const CURSOR& crsr, const CloseOpenInterval<DIM,int>& dmn):
    m_cursor(crsr),
    m_calcCrd(dmn),
    m_domainCrd(dmn.low())
  {
    --m_domainCrd[DIM-1];
  }

  /// Constructor
  ///
  /// @param[in] crsr: the cursor over the array data.
  /// @param[in] dmn: the domain of the array.
  /// @param[in] domainCrd: the current position in the array.
  inline RMFORWARDPOSCURSOR(const CURSOR& crsr, const CloseOpenInterval<DIM,int>& dmn,
    const Tuple<DIM,int>& domainCrd):
    m_cursor(crsr),
    m_calcCrd(dmn),
    m_domainCrd(domainCrd)
  {
    assert(m_calcCrd.domain().contains(domainCrd));
  }

  inline bool advance() {
    if (m_cursor.advance()) {
      bool result = m_calcCrd.advance(m_domainCrd);
      assert(result);
      return true;
    }
    else
      return false;
  }

  inline CONST ENTRY& operator*() CONST {
    return m_cursor.operator*();
  }

  const Tuple<DIM,int>& pos() const {
    return m_domainCrd;
  }
};

} // namespace multi_array

