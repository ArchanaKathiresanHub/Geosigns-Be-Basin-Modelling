template <typename ENTRY>
class CMSTRIDEDSTEPPEDCURSOR<1,ENTRY>
{
public:

  enum { dim = 1, dim_plus_1 = 2 };
  typedef ENTRY Entry;

private:

  CONTIGUOUSSTEPPEDCURSOR<ENTRY> m_impl;

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
  inline CMSTRIDEDSTEPPEDCURSOR(const Tuple<1,int>& size, const Tuple<1,int>& steps,
    CONST ENTRY* data):
    m_impl( size[0], steps[0], data )
  {}

  /// Constructor
  ///
  /// @param[in] data: the start of the array. 
  /// @param[in] size: the size of the array in each dimension.
  /// @param[in] arrayPos: the current position in the array.
  /// @param[in] steps: the steps between consecutive entries in each dimension.  steps ultimately determines 
  /// in what order we iterate over the array.  If steps[0] != 1, either the iteration order differs from the 
  /// natural order or we iterate not over the whole array but only, for instance, only over each second element.
  inline CMSTRIDEDSTEPPEDCURSOR(const Tuple<1,int>& size, const Tuple<1,int>& steps, 
    CONST ENTRY* data, const Tuple<1,int>& arrayPos):
    m_impl( size[0], steps[0], arrayPos[0] )
  {}

  inline CONST ENTRY& operator*() CONST { 
    return m_impl.operator*();
  }

  inline bool advance() {
    return m_impl.advance();
  }

  inline bool recede() {
    return m_impl.recede();
  }
};
