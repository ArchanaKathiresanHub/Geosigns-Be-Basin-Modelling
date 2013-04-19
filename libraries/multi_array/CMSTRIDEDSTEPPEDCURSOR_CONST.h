namespace multi_array {

template <int DIM, typename ENTRY>
class CMSTRIDEDSTEPPEDCURSOR
{
public:

  enum { dim = DIM, dim_min_1 = DIM-1, dim_plus_1 = DIM+1 };
  typedef ENTRY Entry;

private:

  CONST char* m_start;
  CONST char* m_here;
  CONST char* m_finish;
  int m_charStep;
  int m_arraySizeMin1;

  CMSTRIDEDSTEPPEDCURSOR<dim_min_1,ENTRY> m_nextDim;

public:

  /// For simplicity purposes, the following constructors are public, even though they are not 
  /// supposed to be called directly. However, making them private involves including a lot of 
  /// templatized friend class declarations.

  /// Constructor
  ///
  /// @param[in] data: the start of the array.
  /// @param[in] size: the size of the array in each dimension.
  /// @param[in] steps: the steps between consecutive entries of each dimension.  steps ultimately determines 
  /// the iteration order of the array.  If steps[0] != 1, either the iteration order differs from the natural 
  /// order or we iterate not over the whole array but, for instance, only over each second element.
  inline CMSTRIDEDSTEPPEDCURSOR(const Tuple<DIM,int>& size, const Tuple<DIM,int>& steps, 
    CONST ENTRY* data):
    m_arraySizeMin1( size[0]-1 ),
    m_charStep( steps[0] * sizeof(ENTRY) ),
    m_start( (CONST char*)data ),
    // Because of the way how advance() works, we must go to the imaginary element before the first element
    // (just like end() refers to the imaginary element behind all elements of an stl container):
    m_here( m_start - m_charStep ),
    m_finish( m_start + m_charStep * m_arraySizeMin1 ),
    m_nextDim( size.cdr(), steps.cdr(), data )
  {
    // We have gone to the imaginary element before the first element, but this is only necessary for the 
    // first dimension. In addition we must check whether any of the dimensions in empty, so do:
    if (!m_nextDim.advance() )
      // One of the dimensions is empty, so make in this case all dimensions empty:
      m_finish = m_here;
  }

  /// Constructor
  ///
  /// @param[in] data: the start of the array. 
  /// @param[in] size: the size of the array in each dimension.
  /// @param[in] arrayPos: the current position in the array.
  /// @param[in] steps: the steps between consecutive entries in each dimension.  steps ultimately determines 
  /// in what order we iterate over the array.  If steps[0] != 1, either the iteration order differs from the 
  /// natural order or we iterate not over the whole array but only, for instance, only over each second element.
  inline CMSTRIDEDSTEPPEDCURSOR(const Tuple<DIM,int>& size, const Tuple<DIM,int>& steps,
    CONST ENTRY* data, const Tuple<DIM,int>& arrayPos):
    m_arraySizeMin1( size[0]-1 ),
    m_charStep( steps[0] * sizeof(ENTRY) ),
    m_start( (CONST char*)data ),
    // In this case, we may assume that none of the dimensions are empty, for else the position would not be 
    // valid, so we don't need to go to the first imaginary element before the first element:
    m_here( m_start + m_charStep * arrayPos[0] ),
    m_finish( m_start + m_charStep * m_arraySizeMin1 ),
    m_nextDim( size.cdr(), steps.cdr(), data, arrayPos.cdr() )
  {
    assert(arrayPos[0] < size[0]);
  }

  inline CONST ENTRY& operator*() CONST { 
    return (CONST ENTRY&)*m_here;
  }

  inline bool advance() {
    if (m_here >= m_finish) {
      if (!m_nextDim.advance() )
        return false;
      m_start = (CONST char*)&(*m_nextDim);
      m_here = m_start;
      m_finish = m_start + m_charStep * m_arraySizeMin1;
    }
    else
      m_here += m_charStep;
    return true;
  }

  inline bool recede() {
    if (m_here <= m_start) {
      if (!m_nextDim.recede() )
        return false;
      m_start = (CONST char*)&(*m_nextDim);
      m_finish = m_start + m_charStep * m_arraySizeMin1;
      m_here = m_finish;
    }
    else
      m_here -= m_charStep;
    return true;
  }
};

#include "multi_array/CMSTRIDEDSTEPPEDCURSOR_CONST_1.h"

} // namespace multi_array


