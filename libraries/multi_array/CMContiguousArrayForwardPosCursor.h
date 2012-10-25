#ifndef _MULTI_ARRAY_CMCONTIGUOUSARRAYFORWARDPOSCURSOR_H_
#define _MULTI_ARRAY_CMCONTIGUOUSARRAYFORWARDPOSCURSOR_H_

#include "multi_array/ContiguousSteppedForwardCursor.h"

#define CMFORWARDPOSCURSOR CMContiguousArrayForwardPosCursorBase
#define CURSOR ContiguousSteppedForwardCursor<ENTRY>
#define CONST
#include "multi_array/CMFORWARDPOSCURSOR_CONST.h"
#undef CONST
#undef CURSOR
#undef CMFORWARDPOSCURSOR

namespace multi_array {

template <int DIM, typename ENTRY>
class CMContiguousArrayForwardPosCursor:
  public CMContiguousArrayForwardPosCursorBase<DIM,ENTRY>
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
  inline CMContiguousArrayForwardPosCursor(const CloseOpenInterval<DIM,int>& dmn,
     const Tuple<DIM,int>& arraySteps, ENTRY* dt):
    CMContiguousArrayForwardPosCursorBase<DIM,ENTRY>( ContiguousSteppedForwardCursor<
      ENTRY>( dmn.volume(), 1, dt ), dmn )
  {}

  /// Constructor
  ///
  /// @param[in] data: the start of the array.
  /// @param[in] arraySteps: the steps between consecutive entries in each dimension.  arraySteps
  /// ultimately determines in what order we iterate over the array.  If arraySteps[0] != 1, either
  /// the iteration order differs from the natural order or we iterate not over the whole array but
  /// only, for instance, only over each second element.
  /// @param[in] domain: the domain of the array.
  /// @param[in] domainSteps:
  /// @param[in] domainCrd: the current position in the array.
  inline CMContiguousArrayForwardPosCursor(const CloseOpenInterval<DIM,int>& dmn,
     const Tuple<DIM,int>& arraySteps, ENTRY* dt, const Tuple<DIM,int>& domainCrd):
    CMContiguousArrayForwardPosCursorBase<DIM,ENTRY>( ContiguousSteppedForwardCursor<
      ENTRY>( dmn.volume(), 1, dt, CMStrided2IndexMap<DIM>(dmn.size()).index(
      domainCrd-dmn.low()) ), dmn, domainCrd )
  {}
};

} // namespace multi_array

#endif
