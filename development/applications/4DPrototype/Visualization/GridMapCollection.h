//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef GRIDMAPCOLLECTION_H_INCLUDED
#define GRIDMAPCOLLECTION_H_INCLUDED

#include <vector>
#include <limits>

#include <Interface/GridMap.h>

/**
* Treat a collection of GridMaps as a single 3D array
*/
class GridMapCollection
{
public:

  enum Mapping
  {
    Complete,
    EliminateBoundaries,
    SkipFirstK,
    SkipLastK
  };

private:

  /**
   * Maps a single k-index to an index in the list of gridmaps, and
   * a local k-index in that particular gridmap
   */
  struct IndexPair
  {
    size_t gridMapIndex;
    unsigned int kIndex;
  };

  std::vector<const DataAccess::Interface::GridMap*> m_gridMaps;
  std::vector<IndexPair> m_indexMap;

  size_t m_numI;
  size_t m_numJ;
  size_t m_numK;

  mutable double m_minValue;
  mutable double m_maxValue;
  mutable bool m_minMaxValid;

  void initMinMax() const
  {
    for (size_t i = 0; i < m_gridMaps.size(); ++i)
    {
      if (m_gridMaps[i])
      {
        double minval, maxval;
        m_gridMaps[i]->getMinMaxValue(minval, maxval);

        m_minValue = std::min(m_minValue, minval);
        m_maxValue = std::max(m_maxValue, maxval);
      }
    }

    m_minMaxValid = true;
  }

public:

  /**
   * Build the collection from a vector of gridmaps. The gridMaps vector may contain
   * nullptrs, which represent a layer of undefined values. This is done to accomodate 
   * properties that are not available for every formation
   */
  GridMapCollection(const std::vector<const DataAccess::Interface::GridMap*>& gridMaps, Mapping mapping)
    : m_gridMaps(gridMaps)
    , m_numI(0)
    , m_numJ(0)
    , m_numK(0)
    , m_minValue(std::numeric_limits<double>::max())
    , m_maxValue(-std::numeric_limits<double>::max())
    , m_minMaxValid(false)
  {
    assert(!gridMaps.empty());

    // Build a mapping from global k-indices to a gridmap index, and a local k
    for (size_t i = 0; i < gridMaps.size(); ++i)
    {
      unsigned int n = 1;
      if (gridMaps[i])
      {
        n = gridMaps[i]->getDepth();
        if (mapping != Complete)
          n -= 1;

        m_numI = gridMaps[i]->numI();
        m_numJ = gridMaps[i]->numJ();
      }

      unsigned int first = (mapping == SkipFirstK) ? 1 : 0;
      for (unsigned int j = 0; j < n; ++j)
      {
        IndexPair p = { i, first + j };
        m_indexMap.push_back(p);
      }
    }

    if (mapping == EliminateBoundaries)
    {
      // ... still need to include the last one though
      const DataAccess::Interface::GridMap* last = gridMaps[gridMaps.size() - 1];
      unsigned int n = (last == nullptr) ? 1 : last->getDepth() - 1;
      IndexPair p = { gridMaps.size() - 1, n };
      m_indexMap.push_back(p);
    }

    m_numK = m_indexMap.size();
  }

  ~GridMapCollection()
  {
    for (auto gridMap : m_gridMaps)
    {
      if (gridMap)
        gridMap->release();
    }
  }

  // Make this class noncopyable, because we're releasing the gridmaps in the destructor
  GridMapCollection(const GridMapCollection&) = delete;
  GridMapCollection& operator=(const GridMapCollection&) = delete;

  double getValue(size_t i, size_t j, size_t k) const
  {
    IndexPair p = m_indexMap[k];
    auto gridMap = m_gridMaps[p.gridMapIndex];
    return (gridMap != nullptr)
      ? gridMap->getValue((unsigned int)i, (unsigned int)j, p.kIndex)
      : 99999.0;
  }

  size_t numI() const { return m_numI; }
  size_t numJ() const { return m_numJ; }
  size_t numK() const { return m_numK; }

  double minValue() const 
  { 
    if (!m_minMaxValid)
      initMinMax();

    return m_minValue; 
  }

  double maxValue() const 
  { 
    if (!m_minMaxValid)
      initMinMax();

    return m_maxValue; 
  }
};

#endif
