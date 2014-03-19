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
class MoCellFilter;
class MoDrawStyle;
class ROICellFilter;

class SoSwitch;
class SoGroup;
class SoScale;
class SoClipPlaneManip;

class SkinExtractor;

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

  SkinExtractor*      m_skinExtractor;

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
    std::shared_ptr<DataAccess::Interface::PropertyValueList> depthValues,
    bool hires);

  VISUALIZATIONDLL_API const DataAccess::Interface::Snapshot* getSnapShot() const;

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
public:

  enum MeshMode
  {
    MeshMode_All,
    MeshMode_Reservoirs
  };

private:

  SO_NODE_HEADER(SceneGraph);

  SoScale*          m_verticalScale;
  SoSwitch*         m_cellFilterSwitch;
  MoCellFilter*     m_cellFilter;
  ROICellFilter*    m_roiFilter;

  SoGroup*          m_appearance;
  MoDrawStyle*      m_drawStyle;
  SoSwitch*         m_snapshots;
  SoSwitch*         m_snapshotsHiRes;
  SoSwitch*         m_resolutionSwitch;

  MoPredefinedColorMapping* m_colorMap;
  
  bool              m_planeManipInitialized;
  SoSwitch*         m_planeManipSwitch;
  SoClipPlaneManip* m_planeManip;

  void createFilterNode();

  void createAppearanceNode();

  void createSnapshotsNodeHiRes(DataAccess::Interface::ProjectHandle* handle);

  void createSnapshotsNode(DataAccess::Interface::ProjectHandle* handle);

  void createRootNode();

  void initializeManip();

  void setProperty(const DataAccess::Interface::Property* prop, SoSwitch* snapshots);

public:

  static void initClass();

  VISUALIZATIONDLL_API SceneGraph();
    
  VISUALIZATIONDLL_API void setup(DataAccess::Interface::ProjectHandle* handle);

  VISUALIZATIONDLL_API void setProperty(const DataAccess::Interface::Property* prop);

  VISUALIZATIONDLL_API int snapshotCount() const;

  VISUALIZATIONDLL_API const DataAccess::Interface::Snapshot* getSnapshot(int index) const;

  VISUALIZATIONDLL_API void setCurrentSnapshot(int index);

  VISUALIZATIONDLL_API void showPlaneManip(bool show);

  VISUALIZATIONDLL_API void enableROI(bool enable);

  VISUALIZATIONDLL_API void setROI(size_t minI, size_t minJ, size_t minK, size_t maxI, size_t maxJ, size_t maxK);

  VISUALIZATIONDLL_API void setVerticalScale(float scale);

  VISUALIZATIONDLL_API void setMeshMode(MeshMode mode);

  VISUALIZATIONDLL_API void setRenderStyle(bool drawFaces, bool drawEdges);

  SoSFInt32 RenderMode;

  SoSFUInt32 SliceI;

  SoSFUInt32 SliceJ;

  SoSFPlane  Plane;
};

VISUALIZATIONDLL_API void BpaVizInit();

#endif
