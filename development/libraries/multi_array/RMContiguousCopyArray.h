#ifndef _MULTI_ARRAY_RMCONTIGUOUSCOPYARRAY_H_
#define _MULTI_ARRAY_RMCONTIGUOUSCOPYARRAY_H_

#include "multi_array/CopyArrayBase.h"
#include "multi_array/RMContiguousRefArray.h"

namespace multi_array {

template <int DIM, typename ENTRY>
class RMContiguousCopyArray:
  public virtual CopyArrayBase<ENTRY>,
  public virtual RMContiguousRefArray<DIM,ENTRY>
{
public:

  inline RMContiguousCopyArray()
  {}

  inline RMContiguousCopyArray(const RMContiguousRefArray<DIM,ENTRY>& other):
    CopyArrayBase<ENTRY>(other.domain().volume()),
    RMContiguousRefArray<DIM,ENTRY>(other.domain)
  {
    fillFrom(other);
  }

  inline RMContiguousCopyArray(const CloseOpenInterval<DIM,int>& dmn):
    CopyArrayBase<ENTRY>(dmn.volume()),
    RMContiguousRefArray<DIM,ENTRY>(dmn)
  {}

  inline RMContiguousCopyArray(const CloseOpenInterval<DIM,int>& domain, const ENTRY* dt):
    CopyArrayBase<ENTRY>(dmn.volume()),
    RMContiguousRefArray<DIM,ENTRY>(dmn)
  {
    fillFromData(dt);
  }

  template <typename ARRAY>
  inline RMContiguousCopyArray(const ARRAY& other):
    CopyArrayBase<ENTRY>(other.domain().volume()),
    RMContiguousRefArray<DIM,ENTRY>(other.domain())
  {
    fillFrom(other);
  }

  /// Include the following method in order to disambiguate the getData() method
  /// from RMContiguousRefArray and CopyArrayBase:
  inline ENTRY* getData() {
    return CopyArrayBase<ENTRY>::getData();
  }

  /// Include the following method in order to disambiguate the data() method
  /// from RMContiguousRefArray and CopyArrayBase:
  inline const ENTRY* data() const {
    return CopyArrayBase<ENTRY>::data();
  }
};

} // namespace multi_array

#endif
