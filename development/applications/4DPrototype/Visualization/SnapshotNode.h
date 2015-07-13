#ifndef SNAPSHOTNODE_H_INCLUDED
#define SNAPSHOTNODE_H_INCLUDED

#include "defines.h"

#include "Interface/Interface.h"

#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/fields/SoSFUInt32.h>
#include <Inventor/fields/SoSFPlane.h>

class SoGroup;
class SoSwitch;
class MoMesh;
class MoMeshSkin;
class MoMeshLogicalSlice;
class MoMeshPlaneSlice;
class MoMeshOutline;
class MoScalarSetI;

class SkinExtractor;
class Extractor;

/**
* Scenegraph for a mesh representing a single snapshot
*/
class VISUALIZATIONDLL_API SnapshotNode : public SoSeparator
{
  SO_NODE_HEADER(SnapshotNode);

  const DataAccess::Interface::Snapshot* m_snapshot;

  size_t m_subdivision;

  MoMesh*             m_mesh;
  MoScalarSetI*       m_scalarSet;
  MoMeshSkin*         m_skin;
  MoMeshLogicalSlice* m_sliceI;
  MoMeshLogicalSlice* m_sliceJ;
  SoGroup*            m_sliceGroup;
  MoMeshPlaneSlice*   m_planeSlice;
  MoMeshOutline*      m_outline;
  SoGroup*            m_planeGroup;
  SoSwitch*           m_renderSwitch;
  SkinExtractor*      m_skinExtractor;

public:

  enum RenderMode
  {
    RenderMode_Skin,
    RenderMode_Slices,
    RenderMode_CrossSection
  };

  static void initClass();
  static void exitClass();

  SnapshotNode();

  void setup(
    const DataAccess::Interface::Snapshot* snapshot,
    std::shared_ptr<DataAccess::Interface::PropertyValueList> depthValues,
    bool hires,
    Extractor& extractor,
    size_t subdivision = 1);

  const DataAccess::Interface::Snapshot* getSnapShot() const;

  void setProperty(const DataAccess::Interface::Property* prop);

  MoScalarSetI* scalarSet() const;

  SoSFInt32 RenderMode;

  SoSFUInt32 SliceI;

  SoSFUInt32 SliceJ;

  SoSFPlane  Plane;
};

#endif
