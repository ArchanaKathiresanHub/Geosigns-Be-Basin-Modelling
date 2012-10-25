#ifndef _MULTI_ARRAY_RMCONTIGUOUSARRAYFORWARDPOSCURSOR_H_
#define _MULTI_ARRAY_RMCONTIGUOUSARRAYFORWARDPOSCURSOR_H_

#include "multi_array/ContiguousSteppedForwardCursor.h"

#define RMFORWARDPOSCURSOR RMContiguousArrayForwardPosCursorBase
#define CURSOR ContiguousSteppedForwardCursor<ENTRY>
#define CONST
#include "multi_array/RMFORWARDPOSCURSOR_CONST.h"
#undef CONST
#undef CURSOR
#undef RMFORWARDPOSCURSOR

namespace multi_array {

template <int DIM, typename ENTRY>
class RMContiguousArrayForwardPosCursor:
  public RMContiguousArrayForwardPosCursorBase<DIM,ENTRY>
{
public:

  /// For simplicity purposes, the following constructors are public, even though they are not
  /// supposed to be called directly. However, making them private involves including a lot of
  /// templatized friend class declarations.

  /// Constructor
  ///
  /// @param[in] data: the start of the array.
  /// @param[in] arraySteps: the steps between consecutive entries of each dimension.  arraySteps
  /// ultimately determines the iteration order of the array.  If arraySteps[0] != 1, either the
  /// iteration order differs from the natural order or we iterate not over the whole array but,
  /// for instance, only over each second element.
  /// @param[in] domain: the domain of the array.
  /// @param[in] domainSteps:
  inline RMContiguousArrayForwardPosCursor(const CloseOpenInterval<DIM,int>& dmn,
     const Tuple<DIM,int>& arraySteps, ENTRY* dt):
    RMContiguousArrayForwardPosCursorBase<DIM,ENTRY>( ContiguousSteppedForwardCursor<
      ENTRY>( dmn.volume(), 1, dt ), dmn )
  {}

  /// Constructor
  ///
  /// @param[in] dt: the start of the array.
  /// @param[in] arraySteps: the steps between consecutive entries in each dimension.  arraySteps
  /// ultimately determines in what order we iterate over the array.  If arraySteps[0] != 1, either
  /// the iteration order differs from the natural order or we iterate not over the whole array but
  /// only, for instance, only over each second element.
  /// @param[in] dmn: the domain of the array.
  /// @param[in] domainSteps:
  /// @param[in] domainCrd: the current position in the array.
  inline RMContiguousArrayForwardPosCursor(const CloseOpenInterval<DIM,int>& dmn,
     const Tuple<DIM,int>& arraySteps, ENTRY* dt, const Tuple<DIM,int>& domainCrd):
    RMContiguousArrayForwardPosCursorBase<DIM,ENTRY>( ContiguousSteppedForwardCursor<
      ENTRY>( dmn.volume(), 1, dt, RMStrided2IndexMap<DIM>(domain.size()).index(
      domainCrd-domain.low()) ), dmn, domainCrd )
  {}
};

} // namespace multi_array

#endif
