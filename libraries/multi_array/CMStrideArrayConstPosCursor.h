#ifndef _MULTI_ARRAY_CMSTRIDEARRAYCONSTPOSCURSOR_H_
#define _MULTI_ARRAY_CMSTRIDEARRAYCONSTPOSCURSOR_H_

#include "multi_array/CMStridedSteppedConstCursor.h"

#define CMPOSCURSOR CMStrideArrayConstPosCursorBase
#define CURSOR CMStridedSteppedConstCursor<DIM,ENTRY>
#define CONST const
#include "multi_array/CMPOSCURSOR_CONST.h"
#undef CONST
#undef CURSOR
#undef CMPOSCURSOR

namespace multi_array {

template <int DIM, typename ENTRY>
class CMStrideArrayConstPosCursor: 
  public CMStrideArrayConstPosCursorBase<DIM,ENTRY>
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
  inline CMStrideArrayConstPosCursor(ENTRY* data, const Tuple<DIM,int>& arraySteps, 
    const CloseOpenInterval<DIM,int>& domain ):
    CMStrideArrayConstPosCursorBase<DIM,ENTRY>( CMStridedSteppedConstCursor<DIM,ENTRY>( 
      arraySteps, domain.size(), data ), domain )
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
  /// @param[in] domainPos: the current position in the array.
  inline CMStrideArrayConstPosCursor(ENTRY* data, const Tuple<DIM,int>& arraySteps, 
    const CloseOpenInterval<DIM,int>& domain, const Tuple<DIM,int>& domainPos):
    CMStrideArrayConstPosCursorBase<DIM,ENTRY>( CMStridedSteppedConstCursor<DIM,ENTRY>( 
      arraySteps, domain.size(), data, domainPos - domainPos.low() ), domain )
  {}
};					    
					    
} // namespace multi_array

#endif
