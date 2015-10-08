//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef ROICELLFILTER_H_INCLUDED
#define ROICELLFILTER_H_INCLUDED

#include <MeshVizXLM/mesh/cell/MiCellFilterIjk.h>

/**
 * Defines a 'region of interest' on the mesh, in order to show a grid-aligned subset
 * of the data. Note that with OIV9.3, a similar (but probably more efficient) effect
 * can be achieved by using the minCellRanges and maxCellRanges fields of the MoMeshSkin
 * class.
 */
class ROICellFilter : public MiCellFilterIjk
{
  size_t m_minI;
  size_t m_maxI;
  size_t m_minJ;
  size_t m_maxJ;
  size_t m_minK;
  size_t m_maxK;

  size_t m_timestamp;

public:

  ROICellFilter();

  void setROI(size_t minI, size_t minJ, size_t minK, size_t maxI, size_t maxJ, size_t maxK);

  bool acceptCell(size_t i, size_t j, size_t k) const;

  size_t getTimeStamp() const;
};

#endif
