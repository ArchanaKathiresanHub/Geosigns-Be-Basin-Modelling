//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "OutlineBuilder.h"

#include <Inventor/nodes/SoIndexedLineSet.h>
#include <MeshVizXLM/mesh/geometry/MiGeometryIjk.h>
#include <MeshVizXLM/mesh/data/MiDataSetIjk.h>

namespace
{
  bool isValid(int id)
  {
    return id >= 0 && id < 99999;
  }

  bool needLine(int id0, int id1)
  {
    return id0 != id1 && (isValid(id0) || isValid(id1));
  }
}

void OutlineBuilder::addVertex(unsigned int i, unsigned int j)
{
  int32_t index = m_indexGrid[i + j * m_numI];

  if (index == -1)
  {
    index = (int)m_vertices.size();
    m_indexGrid[i + j * m_numI] = index;

    MbVec3d p = m_geometry->getCoord(i, j, 0);

    m_vertices.emplace_back((float)p[0], (float)p[1], (float)p[2]);
  }

  m_indices.push_back(index);
}

void OutlineBuilder::addLine(unsigned int i0, unsigned int j0, unsigned int i1, unsigned int j1)
{
  addVertex(i0, j0);
  addVertex(i1, j1);

  m_indices.push_back(-1);
}

OutlineBuilder::OutlineBuilder(unsigned int numI, unsigned int numJ)
  : m_indexGrid(0)
  , m_geometry(0)
  , m_data(0)
  , m_numI(numI)
  , m_numJ(numJ)
{
  m_indexGrid = new int32_t[m_numI * m_numJ];
}

OutlineBuilder::~OutlineBuilder()
{
  delete[] m_indexGrid;
}

SoIndexedLineSet* OutlineBuilder::createOutline(const MiDataSetIjk<double>* data, const MiGeometryIjk* geometry)
{
  m_vertices.clear();
  m_indices.clear();

  m_data = data;
  m_geometry = geometry;

  // Clear grid
  for (size_t i = 0; i < m_numI * m_numJ; ++i)
    m_indexGrid[i] = -1;

  // First row
  int id = (int)data->get(0u, 0u, 0u);
  if (isValid(id))
  {
    addLine(0, 1, 0, 0); // left
    addLine(0, 0, 1, 0); // bottom
  }

  for (unsigned int i = 1; i < m_numI - 1; ++i)
  {
    int prevId = id;
    id = (int)data->get(i, 0u, 0u);
    if (needLine(id, prevId))
      addLine(i, 1, i, 0); // left
    if (isValid(id))
      addLine(i, 0, i + 1, 0); // bottom
  }

  id = (int)data->get(m_numI - 2, 0u, 0u);
  if (isValid(id))
    addLine(m_numI - 1, 1, m_numI - 1, 0);// add right

  // Middle rows
  for (unsigned int j = 1; j < m_numJ - 1; ++j)
  {
    id = (int)data->get(0u, j, 0u);
    if (isValid(id))
      addLine(0, j + 1, 0, j); // add left
    int bottomId = (int)data->get(0u, j - 1, 0u);
    if (needLine(id, bottomId))
      addLine(0, j, 1, j); // add bottom

    for (unsigned int i = 1; i < m_numI - 1; ++i)
    {
      int prevId = id;
      id = (int)data->get(i, j, 0u);
      bottomId = (int)data->get(i, j - 1, 0u);
      if (needLine(prevId, id))
        addLine(i, j + 1, i, j); // add left
      if (needLine(bottomId, id))
        addLine(i, j, i + 1, j); // add bottom
    }

    id = (int)data->get(m_numI - 2, j, 0u);
    if (isValid(id))
      addLine(m_numI - 1, j + 1, m_numI - 1, j); //add right
  }

  // Last row
  for (unsigned int i = 0; i < m_numI - 1; ++i)
  {
    id = (int)data->get(i, m_numJ - 2, 0u);
    if (isValid(id))
      addLine(i, m_numJ - 1, i + 1, m_numJ - 1); //add top
  }

  SoVertexProperty* vertexProperty = new SoVertexProperty;
  vertexProperty->vertex.setValues(0, (int)m_vertices.size(), &m_vertices[0]);

  SoIndexedLineSet* lineSet = new SoIndexedLineSet;
  lineSet->coordIndex.setValues(0, (int)m_indices.size(), &m_indices[0]);
  lineSet->vertexProperty = vertexProperty;

  return lineSet;
}

