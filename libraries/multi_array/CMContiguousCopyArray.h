#ifndef _MULTI_ARRAY_CMCONTIGUOUSCOPYARRAY_H_
#define _MULTI_ARRAY_CMCONTIGUOUSCOPYARRAY_H_

#include "multi_array/CopyArrayBase.h"
#include "multi_array/CMContiguousRefArray.h"

namespace multi_array {

template <int DIM, typename ENTRY>
class CMContiguousCopyArray:
  public virtual CopyArrayBase<ENTRY>,
  public virtual CMContiguousRefArray<DIM,ENTRY>
{
public:

  inline CMContiguousCopyArray()
  {}

  inline CMContiguousCopyArray(const CMContiguousRefArray<DIM,ENTRY>& other):
    CopyArrayBase<ENTRY>(other.domain().volume()),
    CMContiguousRefArray<DIM,ENTRY>(other.domain)
  {
    fillFrom(other);
  }

  inline CMContiguousCopyArray(const CloseOpenInterval<DIM,int>& dmn):
    CopyArrayBase<ENTRY>(dmn.volume()),
    CMContiguousRefArray<DIM,ENTRY>(dmn)
  {}

  inline CMContiguousCopyArray(const CloseOpenInterval<DIM,int>& dmn, const ENTRY* dt):
    CopyArrayBase<ENTRY>(dmn.volume()),
    CMContiguousRefArray<DIM,ENTRY>(dmn)
  {
    fillFromData(dt);
  }

  template <typename ARRAY>
  inline CMContiguousCopyArray(const ARRAY& other):
    CopyArrayBase<ENTRY>(other.domain().volume()),
    CMContiguousRefArray<DIM,ENTRY>(other.domain())
  {
    fillFrom(other);
  }

  /// Include the following method in order to disambiguate the getData() method
  /// from CMContiguousRefArray and CopyArrayBase:
  inline ENTRY* getData() {
    return CopyArrayBase<ENTRY>::getData();
  }

  /// Include the following method in order to disambiguate the data() method
  /// from CMContiguousRefArray and CopyArrayBase:
  inline const ENTRY* data() const {
    return CopyArrayBase<ENTRY>::data();
  }
};

} // namespace multi_array

#endif
