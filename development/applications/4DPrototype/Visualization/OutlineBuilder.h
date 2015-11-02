//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef OUTLINEBUILDER_H_INCLUDED
#define OUTLINEBUILDER_H_INCLUDED

#include <vector>

#include <Inventor/SbVec.h>

class SoIndexedLineSet;

namespace DataAccess
{
  namespace Interface
  {
    class GridMap;
  }
}

class OutlineBuilder
{
  int32_t* m_indexGrid;

  std::vector<SbVec3f> m_vertices;
  std::vector<int32_t> m_indices;

  const DataAccess::Interface::GridMap* m_values;
  const DataAccess::Interface::GridMap* m_depth;

  double m_deltaX;
  double m_deltaY;

  unsigned int m_numI;
  unsigned int m_numJ;

  void addVertex(unsigned int i, unsigned int j);

  void addLine(unsigned int i0, unsigned int j0, unsigned int i1, unsigned int j1);

public:

  OutlineBuilder();

  SoIndexedLineSet* createOutline(
    const DataAccess::Interface::GridMap* values,
    const DataAccess::Interface::GridMap* depth);
};

#endif
