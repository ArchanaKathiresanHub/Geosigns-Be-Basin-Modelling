#ifndef _MULTI_ARRAY_COPYARRAYBASE_H_
#define _MULTI_ARRAY_COPYARRAYBASE_H_

#include "multi_array/RefArrayBase.h"

namespace multi_array {

template <typename ENTRY>
class CopyArrayBase: public RefArrayBase<ENTRY>
{
protected:

  inline CopyArrayBase()
  {}

  inline CopyArrayBase(int size) {
    m_data = new ENTRY[size];
  }

public:

  inline CopyArrayBase(ENTRY* data):
    RefArrayBase<ENTRY>(data)
  {}

  inline ~CopyArrayBase() {
    delete m_data;
  }

};

} // namespace multi_array

#endif
