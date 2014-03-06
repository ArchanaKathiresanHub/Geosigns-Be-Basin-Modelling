#ifndef SCENEGRAPH_H_INCLUDED
#define SCENEGRAPH_H_INCLUDED

#include "defines.h" 

#include "Interface/Interface.h"

#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/fields/SoSFUInt32.h>
#include <Inventor/fields/SoSFPlane.h>

class MoMesh;
class MoMeshSkin;
class MoMeshLogicalSlice;
class MoMeshPlaneSlice;
class MoMeshOutline;
class MoScalarSetI;
class MoPredefinedColorMapping;

class SoSwitch;
class SoGroup;
class SoClipPlaneManip;

/**
 * Scenegraph for a mesh representing a single snapshot
 */
class SnapshotNode : public SoSeparator
{
  SO_NODE_HEADER(SnapshotNode);

  const DataAccess::Interface::Snapshot* m_snapshot;

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

public:

  enum RenderMode
  {
    RenderMode_Skin,
    RenderMode_Slices,
    RenderMode_CrossSection
  };

  static void initClass();

  SnapshotNode();

  void setup(
    const DataAccess::Interface::Snapshot* snapshot, 
    std::shared_ptr<DataAccess::Interface::PropertyValueList> depthValues);

  VISUALIZATIONDLL_API void setProperty(const DataAccess::Interface::Property* prop);

  VISUALIZATIONDLL_API MoScalarSetI* scalarSet() const;

  SoSFInt32 RenderMode;

  SoSFUInt32 SliceI;

  SoSFUInt32 SliceJ;

  SoSFPlane  Plane;
};

/**
 * Main scenegraph
 */
class SceneGraph : public SoGroup
{
  SO_NODE_HEADER(SceneGraph);

  SoGroup*          m_appearance;
  SoSwitch*         m_snapshots;

  MoPredefinedColorMapping* m_colorMap;
  
  bool              m_planeManipInitialized;
  SoSwitch*         m_planeManipSwitch;
  SoClipPlaneManip* m_planeManip;

  void createAppearanceNode();

  void createSnapshotsNode(DataAccess::Interface::ProjectHandle* handle);

  void createRootNode();

  void initializeManip();

public:

  static void initClass();

  VISUALIZATIONDLL_API SceneGraph();
    
  VISUALIZATIONDLL_API void setup(DataAccess::Interface::ProjectHandle* handle);

  VISUALIZATIONDLL_API void setProperty(const DataAccess::Interface::Property* prop);

  VISUALIZATIONDLL_API int snapshotCount() const;

  VISUALIZATIONDLL_API void setCurrentSnapshot(int index);

  VISUALIZATIONDLL_API void showPlaneManip(bool show);

  SoSFInt32 RenderMode;

  SoSFUInt32 SliceI;

  SoSFUInt32 SliceJ;

  SoSFPlane  Plane;
};

VISUALIZATIONDLL_API void BpaVizInit();

#endif
