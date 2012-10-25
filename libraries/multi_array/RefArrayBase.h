#ifndef _MULTI_ARRAY_REFARRAYBASE_H_
#define _MULTI_ARRAY_REFARRAYBASE_H_

namespace multi_array {

template <typename ENTRY>
class RefArrayBase
{
public:

  typedef ENTRY Entry;
  typedef ENTRY& EntryRef;
  typedef const ENTRY& ConstEntryRef;

protected:

  ENTRY* m_data;

  inline RefArrayBase():
    m_data(0)
  {}

public:

  inline RefArrayBase(const RefArrayBase& other):
    m_data(other.m_data)
  {}

  inline RefArrayBase(ENTRY* dt):
    m_data(dt)
  {}

  inline ENTRY* getData() {
    return m_data;
  }

  inline const ENTRY* data() const {
    return m_data;
  }
};

} // namespace multi_array

#endif
