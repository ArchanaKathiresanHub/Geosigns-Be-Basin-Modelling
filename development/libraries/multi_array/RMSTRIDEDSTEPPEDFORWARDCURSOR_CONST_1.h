template <typename ENTRY>
class RMSTRIDEDSTEPPEDFORWARDCURSOR<1,ENTRY>
{
public:

  enum { dim = 1, dim_min_1 = 0, dim_plus_1 = 2 };
  typedef ENTRY Entry;

private:

  CONTIGUOUSSTEPPEDFORWARDCURSOR<ENTRY> m_impl;

public:

  /// For simplicity purposes, the following constructors are public, even though they are not
  /// supposed to be called directly. However, making them private involves including a lot of
  /// templatized friend class declarations.

  /// Constructor
  ///
  /// @param[in] dt: the start of the array.
  /// @param[in] size: the size of the array in each dimension.
  /// @param[in] steps: the steps between consecutive entries of each dimension.  steps ultimately determines
  /// the iteration order of the array.  If steps[1] != 1, either the iteration order differs from the natural
  /// order or we iterate not over the whole array but, for instance, only over each second element.
  inline RMSTRIDEDSTEPPEDFORWARDCURSOR(const Tuple<1,int>& size, const Tuple<1,int>& steps, CONST ENTRY* dt):
    m_impl( size[0], steps[0], dt )
  {}

  /// Constructor
  ///
  /// @param[in] dt: the start of the array.
  /// @param[in] size: the size of the array in each dimension.
  /// @param[in] arrayPos: the current position in the array.
  /// @param[in] steps: the steps between consecutive entries in each dimension.  steps ultimately determines
  /// in what order we iterate over the array.  If steps[1] != 1, either the iteration order differs from the
  /// natural order or we iterate not over the whole array but only, for instance, only over each second element.
  inline RMSTRIDEDSTEPPEDFORWARDCURSOR(const Tuple<1,int>& size, const Tuple<1,int>& steps,
    CONST ENTRY* dt, const Tuple<1,int>& arrayPos):
    m_impl( size[0], steps[0], dt, arrayPos[0] )
  {}

  inline CONST ENTRY& operator*() CONST {
    return m_impl.operator*();
  }

  inline bool advance() {
    return m_impl.advance();
  }
};









