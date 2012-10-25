#include "multi_array/CMAdvancer.h"

namespace multi_array {

template <int DIM, typename ENTRY>
class CMPOSCURSOR
{
private:

  CURSOR m_cursor;
  Tuple<DIM,int> m_domainCrd;
  CMAdvancer<DIM> m_calcCrd;

public:

  /// For simplicity purposes, the following constructors are public, even though they are not 
  /// supposed to be called directly. However, making them private involves including a lot of 
  /// templatized friend class declarations.

  /// Constructor
  ///
  /// @param[in] cursor: the cursor over the array data. 
  /// @param[in] domain: the domain of the array.
  inline CMPOSCURSOR(const CURSOR& cursor, const CloseOpenInterval<DIM,int>& domain):
    m_cursor(cursor),
    m_calcCrd(domain),
    m_domainCrd(domain.low())
  {
    --m_domainCrd[DIM-1];
  }

  /// Constructor
  ///
  /// @param[in] cursor: the cursor over the array data. 
  /// @param[in] domain: the domain of the array.
  /// @param[in] domainCrd: the current position in the array.
  inline CMPOSCURSOR(const CURSOR& cursor, const CloseOpenInterval<DIM,int>& domain, 
    const Tuple<DIM,int>& domainCrd):
    m_cursor(cursor),
    m_calcCrd(domain),
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

  inline bool recede() {
    if (m_cursor.recede()) {
      bool result = m_domain.recede(m_domainCrd);
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
