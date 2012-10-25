#ifndef _MULTI_ARRAY_RMCONTIGUOUSCONTIGUOUSREFARRAY_H_
#define _MULTI_ARRAY_RMCONTIGUOUSCONTIGUOUSREFARRAY_H_

#include "multi_array/RMStrideRefArrayBase.h"

#include "multi_array/ContiguousSteppedConstForwardCursor.h"
#include "multi_array/ContiguousSteppedForwardCursor.h"
#include "multi_array/ContiguousSteppedConstCursor.h"
#include "multi_array/ContiguousSteppedCursor.h"
#include "multi_array/RMContiguousArrayConstForwardPosCursor.h"
#include "multi_array/RMContiguousArrayForwardPosCursor.h"
#include "multi_array/RMContiguousArrayConstPosCursor.h"
#include "multi_array/RMContiguousArrayPosCursor.h"

#include "multi_array/RMStrideRefArray.h"

#include "multi_array/Tuple.h"

namespace multi_array {

template <int DIM, typename ENTRY>
class RMContiguousRefArray: public RMStrideRefArrayBase<DIM, ENTRY>
{
protected:

  inline RMContiguousRefArray()
  {}

  inline RMContiguousRefArray(const CloseOpenInterval<DIM,int>& dmn):
    RMStrideRefArrayBase<DIM,ENTRY>(dmn)
  {}

public:

  typedef ContiguousSteppedConstForwardCursor<ENTRY> ConstForwardCursor;
  typedef ContiguousSteppedForwardCursor<ENTRY> ForwardCursor;
  typedef ContiguousSteppedConstCursor<ENTRY> ConstCursor;
  typedef ContiguousSteppedCursor<ENTRY> Cursor;
  typedef RMContiguousArrayConstForwardPosCursor<DIM,ENTRY> ConstForwardPosCursor;
  typedef RMContiguousArrayForwardPosCursor<DIM,ENTRY> ForwardPosCursor;
  typedef RMContiguousArrayConstPosCursor<DIM,ENTRY> ConstPosCursor;
  typedef RMContiguousArrayPosCursor<DIM,ENTRY> PosCursor;

  inline RMContiguousRefArray(const RMContiguousRefArray& other):
    RMStrideRefArrayBase<DIM,ENTRY>(other)
  {}

  inline RMContiguousRefArray(const CloseOpenInterval<DIM,int>& dmn, ENTRY* dt):
    RMStrideRefArrayBase<DIM,ENTRY>(dmn, dt)
  {}

  inline Cursor cursor() {
    return Cursor(domain().volume(), 1, getData() );
  }

  inline ConstCursor constCursor() const {
    return ConstCursor(domain().volume(), 1, data() );
  }

  inline ForwardCursor forwardCursor() {
    return ForwardCursor(domain().volume(), 1, getData() );
  }

  inline ConstForwardCursor constForwardCursor() const {
    return ConstForwardCursor(domain().volume(), 1, data() );
  }

  inline PosCursor posCursor() {
    return PosCursor(domain(), m_strides.snoc(1), getData(),
      Tuple<DIM,int>::filled(1) );
  }

  inline ConstPosCursor constPosCursor() const {
    return ConstPosCursor(domain(), m_indexMap.strides().snoc(1), data(),
      Tuple<DIM,int>::filled(1) );
  }

  inline ForwardPosCursor forwardPosCursor() {
    return ForwardPosCursor(domain(), Tuple<DIM,int>(m_indexMap.strides(),1), getData() );
  }

  inline ConstForwardPosCursor constForwardPosCursor() const {
    return ConstForwardPosCursor(domain(), Tuple<DIM,int>(m_indexMap.strides(),1), data() );
  }

  inline RMContiguousRefArray& fill(const ENTRY& value) {
    for (ENTRY* p = getData(), *limit = getData() + domain().volume(); p < limit; ++p)
      *p = value;
  }

  template <typename ARRAY>
  inline RMContiguousRefArray& fillFrom(const ARRAY& other) {
    return castFillFrom(other);
  }

  inline RMContiguousRefArray& fillFromData(const ENTRY* data) {
    return castFillFromData(data);
  }

  template <typename ARRAY>
  inline RMContiguousRefArray& castFillFrom(const ARRAY& other) {
    assert(other.domain() == domain());
    ENTRY* p_this = getData(), *lim = getData() + domain().volume();
    typename ARRAY::ConstForwardCursor c_other(other.constForwardCursor());
    for (; p_this < lim && c_other.advance(); ++p_this)
      *p_this = *c_other;
    return *this;
  }

  template <typename OTHER_ENTRY>
  inline RMContiguousRefArray& castFillFromData(const OTHER_ENTRY* data) {
    ENTRY* p_this = getData(), *lim = getData() + domain().volume();
    for (OTHER_ENTRY *p_other = data; p_this < lim; ++p_this, ++p_other)
      *p_this = *p_other;
    return *this;
  }

  template <typename PRED>
  inline void forEach(PRED pred) {
    for (ENTRY* p = getData(), *limit = getData() + domain().volume(); p < limit; ++p)
      pred(*p);
  }
};

} // namespace multi_array

#endif



