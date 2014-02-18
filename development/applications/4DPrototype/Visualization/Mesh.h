#ifndef MESH_H_INCLUDED
#define MESH_H_INCLUDED

class SoNode;
class MyGeometry;
class MyTopology;
class MiHexahedronTopologyExplicitIjk;
class MiGeometryI;

#include "defines.h"

#include <MeshVizInterface/mesh/MiVolumeMeshHexahedronIjk.h>

VISUALIZATIONDLL_API SoNode* createOIVTree();

/**
 *
 */
class MyMesh : public MiVolumeMeshHexahedronIjk
{
  std::shared_ptr<MyGeometry> m_geometry;
  std::shared_ptr<MyTopology> m_topology;

public:

  MyMesh(size_t numI, size_t numJ, size_t numK);

  virtual const MiHexahedronTopologyExplicitIjk& getTopology() const;

  virtual const MiGeometryI& getGeometry() const;
};

#endif
