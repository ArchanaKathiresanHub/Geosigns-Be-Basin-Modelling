//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ROICellFilter.h"

#include <MeshVizXLM/MxTimeStamp.h>

ROICellFilter::ROICellFilter()
  : m_minI(0)
  , m_maxI(10)
  , m_minJ(0)
  , m_maxJ(10)
  , m_minK(0)
  , m_maxK(10)
  , m_timestamp(MxTimeStamp::getTimeStamp())
{
}

void ROICellFilter::setROI(size_t minI, size_t minJ, size_t minK, size_t maxI, size_t maxJ, size_t maxK)
{
  if(
    m_minI != minI ||
    m_minJ != minJ ||
    m_minK != minK ||
    m_maxI != maxI ||
    m_maxJ != maxJ ||
    m_maxK != maxK)
  {
    m_minI = minI;
    m_maxI = maxI;
    m_minJ = minJ;
    m_maxJ = maxJ;
    m_minK = minK;
    m_maxK = maxK;

    m_timestamp = MxTimeStamp::getTimeStamp();
  }
}

bool ROICellFilter::acceptCell(size_t i, size_t j, size_t k) const
{
  return 
    (i >= m_minI && i < m_maxI) &&
    (j >= m_minJ && j < m_maxJ) &&
    (k >= m_minK && k < m_maxK);
}

size_t ROICellFilter::getTimeStamp() const
{
  return m_timestamp;
}

