#ifndef _MULTI_ARRAY_RMSTRIDEREFARRAY_H_
#define _MULTI_ARRAY_RMSTRIDEREFARRAY_H_

#include "multi_array/RMStrideRefArrayBase.h"

#include "multi_array/RMStridedSteppedConstForwardCursor.h"
#include "multi_array/RMStridedSteppedForwardCursor.h"
#include "multi_array/RMStridedSteppedConstCursor.h"
#include "multi_array/RMStridedSteppedCursor.h"
#include "multi_array/RMStrideArrayConstForwardPosCursor.h"
#include "multi_array/RMStrideArrayForwardPosCursor.h"
#include "multi_array/RMStrideArrayConstPosCursor.h"
#include "multi_array/RMStrideArrayPosCursor.h"

namespace multi_array {

template <int DIM, typename ENTRY>
class RMStrideRefArray: public RMStrideRefArrayBase<DIM, ENTRY>
{
protected:

  inline RMStrideRefArray()
  {}

  inline RMStrideRefArray(const CloseOpenInterval<DIM,int>& domain):
    RMStrideRefArrayBase<DIM,ENTRY>(domain)
  {}

public:

  typedef RMStridedSteppedConstForwardCursor<DIM,ENTRY> ConstForwardCursor;
  typedef RMStridedSteppedForwardCursor<DIM,ENTRY> ForwardCursor;
  typedef RMStridedSteppedConstCursor<DIM,ENTRY> ConstCursor;
  typedef RMStridedSteppedCursor<DIM,ENTRY> Cursor;
  typedef RMStrideArrayConstForwardPosCursor<DIM,ENTRY> ConstForwardPosCursor;
  typedef RMStrideArrayForwardPosCursor<DIM,ENTRY> ForwardPosCursor;
  typedef RMStrideArrayConstPosCursor<DIM,ENTRY> ConstPosCursor;
  typedef RMStrideArrayPosCursor<DIM,ENTRY> PosCursor;

  inline RMStrideRefArray(const RMStrideRefArray& other):
    RMStrideRefArrayBase<DIM,ENTRY>(other)
  {}

  inline RMStrideRefArray(const CloseOpenInterval<DIM,int>& dmn,
    const RMStrided2IndexMap<DIM>& indxMp, ENTRY* dt):
    RMStrideRefArrayBase<DIM,ENTRY>(dmn, indxMp, dt)
  {}

  inline Cursor cursor() {
    return Cursor(domain().size(), Tuple<DIM,int>(m_indexMap.strides(),1), getData() );
  }

  inline ConstCursor constCursor() const {
    return ConstCursor(domain().size(), Tuple<DIM,int>(m_indexMap.strides(),1), data() );
  }

  inline ForwardCursor forwardCursor() {
    return ForwardCursor(domain().size(), Tuple<DIM,int>(m_indexMap.strides(),1), getData() );
  }

  inline ConstForwardCursor constForwardCursor() const {
    return ConstForwardCursor(domain().size(), Tuple<DIM,int>(m_indexMap.strides(),1), data() );
  }

  inline PosCursor posCursor() {
    return PosCursor(domain(), Tuple<DIM,int>(m_indexMap.strides(),1), getData(),
      Tuple<DIM,int>::filled(1) );
  }

  inline ConstPosCursor constPosCursor() const {
    return ConstPosCursor(domain(), Tuple<DIM,int>(m_indexMap.strides(),1), data(),
      Tuple<DIM,int>::filled(1) );
  }

  inline ForwardPosCursor forwardPosCursor() {
    return ForwardPosCursor(domain(), Tuple<DIM,int>(m_indexMap.strides(),1), getData(),
      Tuple<DIM,int>::filled(1) );
  }

  inline ConstForwardPosCursor constForwardPosCursor() const {
    return ConstForwardPosCursor(domain(), Tuple<DIM,int>(m_indexMap.strides(),1), data(),
      Tuple<DIM,int>::filled(1) );
  }

  inline RMStrideRefArray& fill(const ENTRY& value) {
    for (ForwardCursor c(domain(), m_indexMap.strides(), getData()); c.advance();)
      *c = value;
    return *this;
  }

  template <typename ARRAY>
  inline RMStrideRefArray& fillFrom(const ARRAY& other) {
    return castFillFrom(other);
  }

  inline RMStrideRefArray& fillFromData(const ENTRY* data) {
    return castFillFromData(data);
  }

  template <typename ARRAY>
  RMStrideRefArray& castFillFrom(const ARRAY& other) {
    assert(other.domain() == domain());
    typename ARRAY::ConstForwardCursor c_other = other.constForwardCursor();
    for (ForwardCursor c_this = ForwardCursor(); c_other.advance(), c_this.advance(); )
      *c_this = *c_other;
    return *this;
  }

  template <typename OTHER_ENTRY>
  RMStrideRefArray& castFillFromData(const OTHER_ENTRY* data) {
    OTHER_ENTRY* p_other = data;
    for (ForwardCursor c_this = ForwardCursor(); c_this.advance(); ++p_other)
      *c_this = *p_other;
    return *this;
  }

  template <typename PRED>
  inline void forEach(PRED pred) {
    for (ForwardCursor c(domain().size(), Tuple<DIM,int>(m_indexMap.strides(),1),
      getData()); c.advance();)
      pred(*c);
  }
};

} // namespace multi_array

#endif
