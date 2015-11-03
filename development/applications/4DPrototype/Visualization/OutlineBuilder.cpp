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

#include <Interface/GridMap.h>
#include <Interface/Property.h>
#include <Interface/PropertyValue.h>

#include <Inventor/nodes/SoIndexedLineSet.h>

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

namespace di = DataAccess::Interface;

void OutlineBuilder::addVertex(unsigned int i, unsigned int j)
{
  int32_t index = m_indexGrid[i + j * m_numI];

  if (index == -1)
  {
    index = (int)m_vertices.size();
    m_indexGrid[i + j * m_numI] = index;

    double x = i * m_deltaX;
    double y = j * m_deltaY;
    double z = -m_depth->getValue(i, j);

    m_vertices.emplace_back((float)x, (float)y, (float)z);
  }

  m_indices.push_back(index);
}

void OutlineBuilder::addLine(unsigned int i0, unsigned int j0, unsigned int i1, unsigned int j1)
{
  addVertex(i0, j0);
  addVertex(i1, j1);

  m_indices.push_back(-1);
}

OutlineBuilder::OutlineBuilder()
  : m_indexGrid(0)
  , m_values(0)
  , m_depth(0)
  , m_deltaX(.0f)
  , m_deltaY(.0f)
  , m_numI(0)
  , m_numJ(0)
{

}

SoIndexedLineSet* OutlineBuilder::createOutline(const di::GridMap* values, const di::GridMap* depth)
{
  // Initialization
  if (m_numI != values->numI() || m_numJ != values->numJ())
  {
    m_numI = values->numI();
    m_numJ = values->numJ();

    m_deltaX = values->deltaI();
    m_deltaY = values->deltaJ();

    delete[] m_indexGrid;
    m_indexGrid = new int32_t[m_numI * m_numJ];
  }

  m_vertices.clear();
  m_indices.clear();

  m_values = values;
  m_depth = depth;

  // Clear grid
  for (size_t i = 0; i < m_numI * m_numJ; ++i)
    m_indexGrid[i] = -1;

  // First row
  int id = (int)values->getValue(0u, 0u);
  if (isValid(id))
  {
    addLine(0, 1, 0, 0); // left
    addLine(0, 0, 1, 0); // bottom
  }

  for (unsigned int i = 1; i < m_numI - 1; ++i)
  {
    int prevId = id;
    id = (int)values->getValue(i, 0u);
    if (needLine(id, prevId))
      addLine(i, 1, i, 0); // left
    if (isValid(id))
      addLine(i, 0, i + 1, 0); // bottom
  }

  id = (int)values->getValue(m_numI - 2, 0u);
  if (isValid(id))
    addLine(m_numI - 1, 1, m_numI - 1, 0);// add right

  // Middle rows
  for (unsigned int j = 1; j < m_numJ - 1; ++j)
  {
    id = (int)values->getValue(0u, j);
    if (isValid(id))
      addLine(0, j + 1, 0, j); // add left
    int bottomId = (int)values->getValue(0u, j - 1);
    if (needLine(id, bottomId))
      addLine(0, j, 1, j); // add bottom

    for (unsigned int i = 1; i < m_numI - 1; ++i)
    {
      int prevId = id;
      id = (int)values->getValue(i, j);
      bottomId = (int)values->getValue(i, j - 1);
      if (needLine(prevId, id))
        addLine(i, j + 1, i, j); // add left
      if (needLine(bottomId, id))
        addLine(i, j, i + 1, j); // add bottom
    }

    id = (int)values->getValue(m_numI - 2, j);
    if (isValid(id))
      addLine(m_numI - 1, j + 1, m_numI - 1, j); //add right
  }

  // Last row
  for (unsigned int i = 0; i < m_numI - 1; ++i)
  {
    id = (int)values->getValue(i, m_numJ - 2);
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

SoIndexedLineSet* OutlineBuilder::createOutline(
  const DataAccess::Interface::Snapshot* snapshot,
  const DataAccess::Interface::Reservoir* reservoir,
  const DataAccess::Interface::Property* valuesProperty,
  const DataAccess::Interface::Property* depthProperty)
{
  std::unique_ptr<const di::PropertyValueList> values(
    valuesProperty->getPropertyValues(di::RESERVOIR, snapshot, reservoir, 0, 0));

  std::unique_ptr<const di::PropertyValueList> depth(
    depthProperty->getPropertyValues(di::RESERVOIR, snapshot, reservoir, 0, 0));

  if (!values || values->empty() || !depth || depth->empty())
    return nullptr;

  const di::GridMap* valuesGridMap = (*values)[0]->getGridMap();
  const di::GridMap* depthGridMap = (*depth)[0]->getGridMap();

  return createOutline(valuesGridMap, depthGridMap);
}
