#ifndef _MULTI_ARRAY_CMCONTIGUOUSCONTIGUOUSREFARRAY_H_
#define _MULTI_ARRAY_CMCONTIGUOUSCONTIGUOUSREFARRAY_H_

#include "multi_array/CMStrideRefArrayBase.h"

#include "multi_array/ContiguousSteppedConstForwardCursor.h"
#include "multi_array/ContiguousSteppedForwardCursor.h"
#include "multi_array/ContiguousSteppedConstCursor.h"
#include "multi_array/ContiguousSteppedCursor.h"
#include "multi_array/CMContiguousArrayConstForwardPosCursor.h"
#include "multi_array/CMContiguousArrayForwardPosCursor.h"
#include "multi_array/CMContiguousArrayConstPosCursor.h"
#include "multi_array/CMContiguousArrayPosCursor.h"

#include "multi_array/CMStrideRefArray.h"

namespace multi_array {

template <int DIM, typename ENTRY>
class CMContiguousRefArray: public CMStrideRefArrayBase<DIM, ENTRY>
{
protected:

  inline CMContiguousRefArray()
  {}

  inline CMContiguousRefArray(const CloseOpenInterval<DIM,int>& dmn):
    CMStrideRefArrayBase<DIM,ENTRY>(dmn)
  {}

public:

  typedef ContiguousSteppedConstForwardCursor<ENTRY> ConstForwardCursor;
  typedef ContiguousSteppedForwardCursor<ENTRY> ForwardCursor;
  typedef ContiguousSteppedConstCursor<ENTRY> ConstCursor;
  typedef ContiguousSteppedCursor<ENTRY> Cursor;
  typedef CMContiguousArrayConstForwardPosCursor<DIM,ENTRY> ConstForwardPosCursor;
  typedef CMContiguousArrayForwardPosCursor<DIM,ENTRY> ForwardPosCursor;
  typedef CMContiguousArrayConstPosCursor<DIM,ENTRY> ConstPosCursor;
  typedef CMContiguousArrayPosCursor<DIM,ENTRY> PosCursor;

  inline CMContiguousRefArray(const CMContiguousRefArray& other):
    CMStrideRefArrayBase<DIM,ENTRY>(other)
  {}

  inline CMContiguousRefArray(const CloseOpenInterval<DIM,int>& dmn, ENTRY* dt):
    CMStrideRefArrayBase<DIM,ENTRY>(dmn, dt)
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
    return PosCursor(domain(), m_strides.cons(1), getData(),
      Tuple<DIM,int>::filled(1) );
  }

  inline ConstPosCursor constPosCursor() const {
    return ConstPosCursor(domain(), snoc(m_indexMap.strides(),1), data(),
      Tuple<DIM,int>::filled(1) );
  }

  inline ForwardPosCursor forwardPosCursor() {
    return ForwardPosCursor(domain(), Tuple<DIM,int>(1,m_indexMap.strides()), getData() );
  }

  inline ConstForwardPosCursor constForwardPosCursor() const {
    return ConstForwardPosCursor(domain(), Tuple<DIM,int>(1,m_indexMap.strides()), data() );
  }

  inline CMContiguousRefArray& fill(const ENTRY& value) {
    for (ENTRY* p = getData(), *limit = getData() + domain().volume(); p < limit; ++p)
      *p = value;
  }

  template <typename ARRAY>
  inline CMContiguousRefArray& fillFrom(const ARRAY& other) {
    return castFillFrom(other);
  }

  inline CMContiguousRefArray& fillFromData(const ENTRY* data) {
    return castFillFromData(data);
  }

  template <typename ARRAY>
  inline CMContiguousRefArray& castFillFrom(const ARRAY& other) {
    assert(other.domain() == domain());
    ENTRY* p_this = getData(), *lim = getData() + domain().volume();
    typename ARRAY::ConstForwardCursor c_other(other.constForwardCursor());
    for (; p_this < lim && c_other.advance(); ++p_this)
      *p_this = *c_other;
    return *this;
  }

  template <typename OTHER_ENTRY>
  inline CMContiguousRefArray& castFillFromData(const OTHER_ENTRY* data) {
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



