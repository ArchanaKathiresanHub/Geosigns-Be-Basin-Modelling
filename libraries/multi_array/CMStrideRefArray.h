#ifndef _MULTI_ARRAY_CMSTRIDEREFARRAY_H_
#define _MULTI_ARRAY_CMSTRIDEREFARRAY_H_

#include "multi_array/CMStrideRefArrayBase.h"

#include "multi_array/CMStridedSteppedConstForwardCursor.h"
#include "multi_array/CMStridedSteppedForwardCursor.h"
#include "multi_array/CMStridedSteppedConstCursor.h"
#include "multi_array/CMStridedSteppedCursor.h"
#include "multi_array/CMStrideArrayConstForwardPosCursor.h"
#include "multi_array/CMStrideArrayForwardPosCursor.h"
#include "multi_array/CMStrideArrayConstPosCursor.h"
#include "multi_array/CMStrideArrayPosCursor.h"

namespace multi_array {

template <int DIM, typename ENTRY>
class CMStrideRefArray: public CMStrideRefArrayBase<DIM, ENTRY>
{
protected:

  inline CMStrideRefArray():
    CMStrideRefArrayBase<DIM,ENTRY>()
  {}

  inline CMStrideRefArray(const CloseOpenInterval<DIM,int>& domain):
    CMStrideRefArrayBase<DIM,ENTRY>(domain)
  {}

public:

  typedef CMStridedSteppedConstForwardCursor<DIM,ENTRY> ConstForwardCursor;
  typedef CMStridedSteppedForwardCursor<DIM,ENTRY> ForwardCursor;
  typedef CMStridedSteppedConstCursor<DIM,ENTRY> ConstCursor;
  typedef CMStridedSteppedCursor<DIM,ENTRY> Cursor;
  typedef CMStrideArrayConstForwardPosCursor<DIM,ENTRY> ConstForwardPosCursor;
  typedef CMStrideArrayForwardPosCursor<DIM,ENTRY> ForwardPosCursor;
  typedef CMStrideArrayConstPosCursor<DIM,ENTRY> ConstPosCursor;
  typedef CMStrideArrayPosCursor<DIM,ENTRY> PosCursor;

  inline CMStrideRefArray(const CMStrideRefArray& other):
    CMStrideRefArrayBase<DIM,ENTRY>(other)
  {}

  inline CMStrideRefArray(const CloseOpenInterval<DIM,int>& dmn,
    const CMStrided2IndexMap<DIM>& indxMp, ENTRY* dt):
    CMStrideRefArrayBase<DIM,ENTRY>(dmn, indxMp, dt)
  {}

  inline Cursor cursor() {
    return Cursor(domain().size(), Tuple<DIM,int>(1,m_indexMap.strides()), getData() );
  }

  inline ConstCursor constCursor() const {
    return ConstCursor(domain().size(), Tuple<DIM,int>(1,m_indexMap.strides()), data() );
  }

  inline ForwardCursor forwardCursor() {
    return ForwardCursor(domain().size(), Tuple<DIM,int>(1,m_indexMap.strides()), getData() );
  }

  inline ConstForwardCursor constForwardCursor() const {
    return ConstForwardCursor(domain().size(), Tuple<DIM,int>(1,m_indexMap.strides()), data() );
  }

  inline PosCursor posCursor() {
    return PosCursor(domain().size(), Tuple<DIM,int>(1,m_indexMap.strides()), getData(),
      Tuple<DIM,int>::filled(1) );
  }

  inline ConstPosCursor constPosCursor() const {
    return ConstPosCursor(domain().size(), Tuple<DIM,int>(1,m_indexMap.strides()), data(),
      Tuple<DIM,int>::filled(1) );
  }

  inline ForwardPosCursor forwardPosCursor() {
    return ForwardPosCursor(domain().size(), Tuple<DIM,int>(1,m_indexMap.strides()), getData(),
      Tuple<DIM,int>::filled(1) );
  }

  inline ConstForwardPosCursor constForwardPosCursor() const {
    return ConstForwardPosCursor(domain().size(), Tuple<DIM,int>(1,m_indexMap.strides()), data(),
      Tuple<DIM,int>::filled(1) );
  }

  template <typename PRED>
  inline void forEach(PRED pred) {
    for (ForwardCursor c(domain().size(), Tuple<DIM,int>(1,m_indexMap.strides()),
      getData()); c.advance();)
      pred(*c);
  }

  inline CMStrideRefArray& fill(const ENTRY& value) {
    for (ForwardCursor c(domain(), m_indexMap.strides(), getData()); c.advance();)
      *c = value;
    return *this;
  }

  inline CMStrideRefArray& fillFrom(const CMStrideRefArray& other) {
    return castFillFrom(other);
  }

  inline CMStrideRefArray& fillFromData(const ENTRY* data) {
    return castFillFromData(data);
  }

  template <typename ARRAY>
  CMStrideRefArray& castFillFrom(const ARRAY& other) {
    assert(other.domain() == domain());
    typename ARRAY::ConstForwardCursor c_other = other.constForwardCursor();
    for (ForwardCursor c_this = ForwardCursor(); c_other.advance(), c_this.advance(); )
      *c_this = *c_other;
    return *this;
  }

  template <typename OTHER_ENTRY>
  CMStrideRefArray& castFillFromData(const OTHER_ENTRY* data) {
    OTHER_ENTRY* p_other = data;
    for (ForwardCursor c_this = ForwardCursor(); c_this.advance(); ++p_other)
      *c_this = *p_other;
    return *this;
  }
};

} // namespace multi_array

#endif
