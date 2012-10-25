namespace multi_array {

template <typename ENTRY>
class CONTIGUOUSSTEPPEDCURSOR
{
private:

  CONST char* m_start;
  CONST char* m_here;
  CONST char* m_finish;
  int m_charStep;

public:

  /// For simplicity purposes, the following constructors are public, even though they are not 
  /// supposed to be called directly. However, making them private involves including a lot of 
  /// templatized friend class declarations.

  /// Constructor
  ///
  /// @param[in] data: the start of the array.
  /// @param[in] size: the length of the array (excluding step).
  /// @param[in] step: the step between consecutive entries.  For instance, if only every second element 
  /// is wanted, step must be equal to 2.
  inline CONTIGUOUSSTEPPEDCURSOR(int size, int step, CONST ENTRY* data):
    m_charStep( step * sizeof(ENTRY) ),
    m_start( (CONST char*)data ),
    // Because of the way how advance() works, we must go to the imaginary element before the first element
    // (just like end() refers to the imaginary element behind all elements of an stl container):
    m_here( m_start - m_charStep ),
    m_finish( m_start + m_charStep * size )
  {}

  /// Constructor
  ///
  /// @param[in] data: the start of the array. 
  /// @param[in] size: the length of the array (excluding step).
  /// @param[in] pos: the current position in the array.
  /// @param[in] step: the step between consecutive entries.  For instance, if only every second element 
  /// is wanted, step must be equal to 2.
  inline CONTIGUOUSSTEPPEDCURSOR(int size, int step, CONST ENTRY* data, int pos):
    m_charStep( step * sizeof(ENTRY) ),
    m_start( (CONST char*)data ),
    m_here( m_start + m_charStep * pos ),
    m_finish( m_start + m_charStep * size )
  {}

  inline bool advance() {
    if (m_here < m_finish) {
      m_here += m_charStep;
      return true;
    }
    else
      return false;
  }

  inline bool recede() {
    if (m_here > m_start) {
      m_here -= m_charStep;
      return true;
    }
    else
      return false;
  }

  inline CONST ENTRY& operator*() CONST { 
    return (CONST ENTRY&)*m_here;
  }
};

} // namespace multi_array
