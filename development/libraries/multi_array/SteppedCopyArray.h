#ifndef _MULTI_ARRAY_STEPPEDCOPYARRAY_H_
#define _MULTI_ARRAY_STEPPEDCOPYARRAY_H_

#include "multi_array/CopyArrayBase.h"
#include "multi_array/SteppedRefArray.h"

namespace multi_array {

template <int DIM, typename ENTRY>
class SteppedCopyArray:
  public virtual CopyArrayBase<ENTRY>,
  public virtual SteppedRefArray<DIM,ENTRY>
{
public:

  inline SteppedCopyArray()
  {}

  inline SteppedCopyArray(const SteppedRefArray<DIM,ENTRY>& other):
    CopyArrayBase<ENTRY>(other.domain().volume()),
    SteppedRefArray<DIM,ENTRY>(other.domain)
  {
    fillFrom(other);
  }

  inline SteppedCopyArray(const CloseOpenInterval<DIM,int>& dmn,
    const Step2IndexMap<DIM>& indexMap):
    CopyArrayBase<ENTRY>(dmn.volume()),
    SteppedRefArray<DIM,ENTRY>(dmn, indexMap)
  {}

  inline SteppedCopyArray(const CloseOpenInterval<DIM,int>& dmn,
    const Step2IndexMap<DIM>& indexMap, const ENTRY* dt):
    CopyArrayBase<ENTRY>(dmn.volume()),
    SteppedRefArray<DIM,ENTRY>(dmn, indexMap)
  {
    fillFromData(dt);
  }

  template <typename ARRAY>
  inline SteppedCopyArray(const ARRAY& other, const Step2IndexMap<DIM>& indexMap):
    CopyArrayBase<ENTRY>(other.domain().volume()),
    SteppedRefArray<DIM,ENTRY>(other.domain(), indexMap)
  {
    fillFrom(other);
  }

  /// Include the following method in order to disambiguate the getData() method
  /// from SteppedRefArray and CopyArrayBase:
  inline ENTRY* getData() {
    return CopyArrayBase<ENTRY>::getData();
  }

  /// Include the following method in order to disambiguate the data() method
  /// from SteppedRefArray and CopyArrayBase:
  inline const ENTRY* data() const {
    return CopyArrayBase<ENTRY>::data();
  }
};

} // namespace multi_array

#endif
