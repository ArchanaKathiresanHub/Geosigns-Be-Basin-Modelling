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
class MiGeometryIjk;
template<class T>
class MiDataSetIjk;

class OutlineBuilder
{
  int32_t* m_indexGrid;

  std::vector<SbVec3f> m_vertices;
  std::vector<int32_t> m_indices;

  const MiGeometryIjk* m_geometry;
  const MiDataSetIjk<double>* m_data;

  unsigned int m_numI;
  unsigned int m_numJ;

  void addVertex(unsigned int i, unsigned int j);

  void addLine(unsigned int i0, unsigned int j0, unsigned int i1, unsigned int j1);

public:

  OutlineBuilder(unsigned int numI, unsigned int numJ);

  ~OutlineBuilder();

  OutlineBuilder(const OutlineBuilder&) = delete;
  OutlineBuilder& operator=(const OutlineBuilder&) = delete;

  SoIndexedLineSet* createOutline(
    const MiDataSetIjk<double>* data,
    const MiGeometryIjk* geometry);
};

#endif
