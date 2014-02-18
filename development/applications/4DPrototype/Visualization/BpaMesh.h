#ifndef BPAMESH_H_INCLUDED
#define BPAMESH_H_INCLUDED

#include "defines.h"

#include <MeshVizInterface/mesh/MiVolumeMeshHexahedronIjk.h>

#include <memory>

class MiHexahedronTopologyExplicitIjk;
class MiGeometryI;
class SoNode;

class BpaTopology;
class BpaGeometry;

VISUALIZATIONDLL_API void foo(const char* filename);
VISUALIZATIONDLL_API SoNode* createOIVTree(const char* filename);

namespace DataAccess
{
  namespace Interface
  {
    class Snapshot;
  }
}

class BpaMesh : public MiVolumeMeshHexahedronIjk
{
  const DataAccess::Interface::Snapshot* m_snapshot;

  std::shared_ptr<BpaTopology> m_topology;
  std::shared_ptr<BpaGeometry> m_geometry;

public:

  explicit BpaMesh(const DataAccess::Interface::Snapshot* snapshot);

  virtual const MiHexahedronTopologyExplicitIjk& getTopology() const;

  virtual const MiGeometryI& getGeometry() const;
};

#endif
