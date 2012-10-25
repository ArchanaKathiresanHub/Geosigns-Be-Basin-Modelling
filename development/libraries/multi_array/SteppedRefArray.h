#ifndef _MULTI_ARRAY_STEPPEDREFARRAY_H_
#define _MULTI_ARRAY_STEPPEDREFARRAY_H_

#include "multi_array/SteppedRefArrayBase.h"

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
class SteppedRefArray: public SteppedRefArrayBase<DIM, ENTRY>
{
protected:

  inline SteppedRefArray()
  {}

  inline SteppedRefArray(const CloseOpenInterval<DIM,int>& dmn):
    SteppedRefArrayBase<DIM,ENTRY>(dmn)
  {}

  inline SteppedRefArray(const CloseOpenInterval<DIM,int>& dmn, 
    const Step2IndexMap<DIM>& indxMp):
    SteppedRefArrayBase<DIM,ENTRY>(dmn, indxMp)
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

  inline SteppedRefArray(const SteppedRefArray& other):
    SteppedRefArrayBase<DIM,ENTRY>(other)
  {}

  inline SteppedRefArray(const CloseOpenInterval<DIM,int>& dmn, 
    const Step2IndexMap<DIM>& indxMp, ENTRY* dt):
    SteppedRefArrayBase<DIM,ENTRY>(dmn, indxMp, dt)
  {}

  inline Cursor cursor() {
    return Cursor(domain().size(), m_strides, getData() );
  }

  inline ConstCursor constCursor() const {
    return ConstCursor(domain().size(), m_indexMap.strides(), data() );
  }    

  inline ForwardCursor forwardCursor() {
    return ForwardCursor(domain().size(), m_indexMap.strides(), getData() );
  }

  inline ConstForwardCursor constForwardCursor() const {
    return ConstForwardCursor(domain().size(), m_indexMap.strides(), data() );
  }    

  inline PosCursor posCursor() {
    return PosCursor(domain(), m_indexMap.strides(), getData(), 
      Tuple<DIM,int>::filled(1) );
  }

  inline ConstPosCursor constPosCursor() const {
    return ConstPosCursor(domain(), m_indexMap.strides(), data(), 
      Tuple<DIM,int>::filled(1) );
  }    

  inline ForwardPosCursor forwardPosCursor() {
    return ForwardPosCursor(domain(), m_indexMap.strides(), getData(), 
      Tuple<DIM,int>::filled(1) );
  }

  inline ConstForwardPosCursor constForwardPosCursor() const {
    return ConstForwardPosCursor(domain(), m_indexMap.strides(), data(), 
      Tuple<DIM,int>::filled(1) );
  }

  inline SteppedRefArray& fill(const ENTRY& value) {
    for (ForwardCursor c(domain(), m_indexMap.strides(), getData()); c.advance();)
      *c = value;
    return *this;
  }

  template <typename ARRAY>
  inline SteppedRefArray& fillFrom(const ARRAY& other) {
    return castFillFrom(other);
  }

  inline SteppedRefArray& fillFromData(const ENTRY* data) {
    return castFillFromData(data);
  }

  template <typename ARRAY>
  SteppedRefArray& castFillFrom(const ARRAY& other) {
    assert(other.domain() == domain());
    typename ARRAY::ConstForwardCursor c_other(other.constForwardCursor());
    for (ForwardCursor c_this(forwardCursor()); c_other.advance(), c_this.advance(); )
      *c_this = *c_other;
    return *this;
  }

  template <typename OTHER_ENTRY>
  SteppedRefArray& castFillFromData(const OTHER_ENTRY* data) {
    OTHER_ENTRY* p_other = data;
    for (ForwardCursor c_this(forwardCursor()); c_this.advance(); ++p_other)
      *c_this = *p_other;
    return *this;
  }

  template <typename PRED>
  inline void forEach(PRED pred) {
    for (ForwardCursor c(domain(), m_indexMap.strides(), 
      getData()); c.advance();)
      pred(*c);
  }
};

} // namespace multi_array

#endif
