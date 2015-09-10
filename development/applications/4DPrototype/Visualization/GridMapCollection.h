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

  double m_minValue;
  double m_maxValue;

  // Make this class noncopyable, because we're releasing the gridmaps in the destructor
  GridMapCollection(const GridMapCollection&) = delete;
  GridMapCollection& operator=(const GridMapCollection&) = delete;

public:

  GridMapCollection(const std::vector<const DataAccess::Interface::GridMap*>& gridMaps)
    : m_gridMaps(gridMaps)
    , m_minValue(std::numeric_limits<double>::max())
    , m_maxValue(-std::numeric_limits<double>::max())
  {
    assert(!gridMaps.empty());

    // Build a mapping from global k-indices to a gridmap index, and a local k
    for (size_t i = 0; i < gridMaps.size(); ++i)
    {
      double minval, maxval;
      gridMaps[i]->getMinMaxValue(minval, maxval);

      m_minValue = std::min(m_minValue, minval);
      m_maxValue = std::max(m_maxValue, maxval);

      // Skip last k for each gridmap to avoid duplication with the next layer
      for (unsigned int j = 0; j < gridMaps[i]->getDepth() - 1; ++j)
      {
        IndexPair p = { i, j };
        m_indexMap.push_back(p);
      }
    }

    const DataAccess::Interface::GridMap* last = gridMaps[gridMaps.size() - 1];

    // ... still need to include the last one though
    IndexPair p = { gridMaps.size() - 1, last->getDepth() - 1 };
    m_indexMap.push_back(p);

    m_numI = last->numI();
    m_numJ = last->numJ();
    m_numK = m_indexMap.size();
  }

  ~GridMapCollection()
  {
    for (auto gridMap : m_gridMaps)
      gridMap->release();
  }

  double getValue(size_t i, size_t j, size_t k) const
  {
    IndexPair p = m_indexMap[k];
    return m_gridMaps[p.gridMapIndex]->getValue((unsigned int)i, (unsigned int)j, p.kIndex);
  }

  size_t numI() const { return m_numI; }
  size_t numJ() const { return m_numJ; }
  size_t numK() const { return m_numK; }

  double minValue() const { return m_minValue; }
  double maxValue() const { return m_maxValue; }
};

#endif
