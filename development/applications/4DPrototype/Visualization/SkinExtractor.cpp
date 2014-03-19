#include "SkinExtractor.h"
#include "BpaMesh.h"

#include <MeshVizInterface/MxTimeStamp.h>
#include <MeshVizInterface/mesh/MiSurfaceMeshUnstructured.h>
#include <MeshVizInterface/mesh/topology/MiSurfaceTopologyExplicitI.h>
#include <MeshVizInterface/mesh/cell/MiSurfaceCell.h>
#include <MeshVizInterface/mesh/cell/MiCellFilterIjk.h>

#include <iostream>

/**
 *
 */
class QuadSurfaceCell : public MiSurfaceCell
{
  size_t m_indices[4];

public:

  QuadSurfaceCell()
  {
    for(int i=0; i < 4; ++i)
      m_indices[i] = 0;
  }

  QuadSurfaceCell(size_t i0, size_t i1, size_t i2, size_t i3)
  {
    m_indices[0] = i0;
    m_indices[1] = i1;
    m_indices[2] = i2;
    m_indices[3] = i3;
  }

  virtual size_t getNumNodes() const
  {
    return 4;
  }

  virtual size_t getNodeIndex(size_t node) const
  {
    return m_indices[node];
  }

  virtual size_t getSubTriangleNodesIndex(std::vector<size_t>& triangleNodeIds) const
  {
    int order[] = { 0, 1, 2, 0, 2, 3 };

    for(int i=0; i < 6; ++i)
      triangleNodeIds.push_back(m_indices[order[i]]);

    return 2;
  }
};

/**
 *
 */
class SurfaceTopology : public MiSurfaceTopologyExplicitI
{
  size_t m_timestamp;

  std::vector<QuadSurfaceCell> m_cells;

  size_t m_beginNodeId;
  size_t m_endNodeId;

public:

  SurfaceTopology()
    : m_timestamp(MxTimeStamp::getTimeStamp())
  {
  }

  void setupBeginAndEndNodeId()
  {
    if(m_cells.empty())
      return;

    m_beginNodeId = m_cells[0].getNodeIndex(0);
    m_endNodeId = m_beginNodeId;

    for(size_t i=0; i < m_cells.size(); ++i)
    {
      for(size_t j=0; j < 4; ++j)
      {
        size_t id = m_cells[i].getNodeIndex(j);
        m_beginNodeId = (id < m_beginNodeId) ? id : m_beginNodeId;
        m_endNodeId = (id > m_endNodeId) ? id : m_endNodeId;
      }
    }

    m_endNodeId += 1;
  }

  void addQuad(size_t i0, size_t i1, size_t i2, size_t i3)
  {
    m_cells.push_back(QuadSurfaceCell(i0, i1, i2, i3));
  }

  virtual size_t getBeginNodeId() const
  {
    return m_beginNodeId;;
  }

  virtual size_t getEndNodeId() const
  {
    return m_endNodeId;
  }

  virtual const MiSurfaceCell* getCell(size_t id) const
  {
    return &m_cells[id];
  }

  virtual std::string getCellName(size_t i) const
  {
    return "";
  }

  virtual std::string getNodeName(size_t i) const
  {
    return "";
  }

  virtual size_t getNumCells() const
  {
    return m_cells.size();
  }

  virtual size_t getTimeStamp() const
  {
    return m_timestamp;
  }

  virtual bool hasDeadCells() const
  {
    return false;
  }

  virtual bool isDead(size_t i) const
  {
    return false;
  }
};

/**
 *
 */
class SurfaceMeshUnstructured : public MiSurfaceMeshUnstructured
{
  std::shared_ptr<SurfaceTopology> m_topology;
  const MiGeometryI& m_geometry;

public:

  SurfaceMeshUnstructured(std::shared_ptr<SurfaceTopology> topology, const MiGeometryI& geometry)
    : m_topology(topology)
    , m_geometry(geometry)
  {
  }

  virtual const MiSurfaceTopologyExplicitI& getTopology() const
  {
    return *m_topology;
  }

  virtual const MiGeometryI& getGeometry() const
  {
    return m_geometry;
  }
};

SkinExtractor::SkinExtractor(const BpaMesh& mesh)
  : m_mesh(mesh)
{
}

  /*
  From the docs for MiHexahedronTopologyExplicitIjk:

                 J
               |
               |
              n3----------n2   facet 0 = 0123   
              /|          /|   facet 1 = 4765 
            /  |        /  |   facet 2 = 0374
          /    |      /    |   facet 3 = 1562    
        n7---------n6      |   facet 4 = 0451 
         |     |    |      |   facet 5 = 3267 
         |    n0----|-----n1    --- I               
         |    /     |     /                    
         |  /       |   /                      
         |/         | /                        
        n4---------n5
      /
    /
  K
 
 */

const MiSurfaceMeshUnstructured& SkinExtractor::extractSkin(const MiCellFilterIjk* cellFilter)
{
  std::cout << "Extracting skin..." << std::endl;

  //const BpaGeometry& geometry = (const BpaGeometry&)m_mesh.getGeometry();
  const MiHexahedronTopologyExplicitIjk& topology = m_mesh.getTopology();

  std::shared_ptr<SurfaceTopology> surfaceTopo(new SurfaceTopology);

  size_t ni = topology.getNumCellsI();
  size_t nj = topology.getNumCellsJ();
  size_t nk = topology.getNumCellsK();

  size_t n[8]; // cellNodeIndices

  for(size_t i=0; i < ni; ++i)
  {
    for(size_t j=0; j < nj; ++j)
    {
      for(size_t k=0; k < nk; ++k)
      {
        topology.getCellNodeIndices(i, j, k, n[0], n[1], n[2], n[3], n[4], n[5], n[6], n[7]);

        bool hasDeadCells = topology.hasDeadCells();

        if((hasDeadCells && topology.isDead(i, j, k)) || (cellFilter != 0 && !cellFilter->acceptCell(i, j, k)))
        {
          if((k < nk-1) && !topology.isDead(i, j, k+1) && (cellFilter == 0 || cellFilter->acceptCell(i, j, k+1)))
            surfaceTopo->addQuad(n[4], n[5], n[6], n[7]); // add facet 1 (reverse)

          if((j < nj-1) && !topology.isDead(i, j+1, k) && (cellFilter == 0 || cellFilter->acceptCell(i, j+1, k)))
            surfaceTopo->addQuad(n[3], n[7], n[6], n[2]); // add facet 5 (reverse)

          if((i < ni-1) && !topology.isDead(i+1, j, k) && (cellFilter == 0 || cellFilter->acceptCell(i+1, j, k)))
            surfaceTopo->addQuad(n[1], n[2], n[6], n[5]); // add facet 3 (reverse)
        }
        else
        {
          if(i == 0)
            surfaceTopo->addQuad(n[0], n[3], n[7], n[4]); // add facet 2
          if(j == 0)
            surfaceTopo->addQuad(n[0], n[4], n[5], n[1]); // add facet 4
          if(k == 0)
            surfaceTopo->addQuad(n[0], n[1], n[2], n[3]); // add facet 0

          if((k == nk-1) || topology.isDead(i, j, k+1) || (cellFilter != 0 && !cellFilter->acceptCell(i, j, k+1)))
            surfaceTopo->addQuad(n[4], n[7], n[6], n[5]); // add facet 1

          if((j == nj-1) || topology.isDead(i, j+1, k) || (cellFilter != 0 && !cellFilter->acceptCell(i, j+1, k)))
            surfaceTopo->addQuad(n[3], n[2], n[6], n[7]); // add facet 5

          if((i == ni-1) || topology.isDead(i+1, j, k) || (cellFilter != 0 && !cellFilter->acceptCell(i+1, j, k)))
            surfaceTopo->addQuad(n[1], n[5], n[6], n[2]); // add facet 3
        }
      }
    }
  }

  surfaceTopo->setupBeginAndEndNodeId();

  m_skin.reset(new SurfaceMeshUnstructured(surfaceTopo, m_mesh.getGeometry()));

  return *m_skin;
}
