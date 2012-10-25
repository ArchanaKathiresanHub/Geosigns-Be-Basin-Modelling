#ifndef _MULTI_ARRAY_MULTIINDEXREFARRAYBASE_H_
#define _MULTI_ARRAY_MULTIINDEXREFARRAYBASE_H_

#include "multi_array/Tuple.h"
#include "multi_array/RefArrayBase.h"
#include "multi_array/MultiArrayBase.h"

namespace multi_array {

template <int DIM, typename ENTRY, typename INDEXMAP>
class MultiIndexRefArrayBase:
  public RefArrayBase<ENTRY>,
  public MultiArrayBase<DIM, typename INDEXMAP::type>
{
protected:

  INDEXMAP m_indexMap;

  inline MultiIndexRefArrayBase()
  {}

  inline MultiIndexRefArrayBase(const CloseOpenInterval<DIM,int>& dmn,
    const INDEXMAP& indxMp):
    MultiArrayBase<DIM,int>(dmn),
    m_indexMap(indxMp)
  {}

  inline ENTRY* realAddress(const Tuple<DIM,int>& crd) {
    return m_data + m_indexMap.index(crd);
  }

  inline const ENTRY* realAddress(const Tuple<DIM,int>& crd) const {
    return m_data + m_indexMap.index(crd);
  }

public:

  inline MultiIndexRefArrayBase(const MultiIndexRefArrayBase& other):
    RefArrayBase<ENTRY>(other),
    MultiArrayBase<DIM,int>(other),
    m_indexMap(other.m_indexMap)
  {}

  inline MultiIndexRefArrayBase(const CloseOpenInterval<DIM,int>& dmn,
    const INDEXMAP& indxMp, ENTRY* dt):
    RefArrayBase<ENTRY>(dt),
    MultiArrayBase<DIM,int>(dmn),
    m_indexMap(indxMp)
  {}

  /// Get the one dimensional index in the storage.
  ///
  /// @param[in] crd: the domain coordinate for which we must find the
  /// storage index.
  /// @return: the storage index.
  inline int index(const Tuple<DIM,int>& crd) const {
    return m_indexMap.index(crd - m_domain.low());
  }

  inline const INDEXMAP& indexMap() const {
    return m_indexMap;
  }

  inline ENTRY& operator[](const Tuple<DIM,int>& crd) {
    assert(m_domain.contains(crd));
    return *realAddress(crd);
  }

  inline const ENTRY& operator[](const Tuple<DIM,int>& crd) const {
    assert(m_domain.contains(crd));
    return *realAddress(crd);
  }

  inline ENTRY* at(const Tuple<DIM,int>& crd) {
    return m_domain.contains(crd) ? realAddress(crd) : NULL;
  }

  inline const ENTRY* at(const Tuple<DIM,int>& crd) const {
    return m_domain.contains(crd) ? realAddress(crd) : NULL;
  }
};

} // namespace multi_array

#endif
