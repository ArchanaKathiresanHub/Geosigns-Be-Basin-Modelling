#ifndef SCENEGRAPH_H_INCLUDED
#define SCENEGRAPH_H_INCLUDED

#include "defines.h" 
#include "Extractor.h"

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
class VISUALIZATIONDLL_API SnapshotNode : public SoSeparator
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
    bool hires,
    Extractor& extractor);

  const DataAccess::Interface::Snapshot* getSnapShot() const;

  void setProperty(const DataAccess::Interface::Property* prop);

  MoScalarSetI* scalarSet() const;

  SoSFInt32 RenderMode;

  SoSFUInt32 SliceI;

  SoSFUInt32 SliceJ;

  SoSFPlane  Plane;
};

/**
 * Main scenegraph
 */
class VISUALIZATIONDLL_API SceneGraph : public SoSeparator
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

  int m_numI;
  int m_numJ;
  int m_numIHiRes;
  int m_numJHiRes;

  Extractor m_extractor;

  void createFilterNode();

  void createAppearanceNode();

  void createSnapshotsNodeHiRes(DataAccess::Interface::ProjectHandle* handle);

  void createSnapshotsNode(DataAccess::Interface::ProjectHandle* handle);

  void createRootNode();

  void initializeManip();

  void setProperty(const DataAccess::Interface::Property* prop, SoSwitch* snapshots);

public:

  static void initClass();

  SceneGraph();

  ~SceneGraph();
    
  void setup(DataAccess::Interface::ProjectHandle* handle);

  void setProperty(const DataAccess::Interface::Property* prop);

  int snapshotCount() const;

  const DataAccess::Interface::Snapshot* getSnapshot(int index) const;

  void setCurrentSnapshot(int index);

  void showPlaneManip(bool show);

  void enableROI(bool enable);

  void setROI(size_t minI, size_t minJ, size_t minK, size_t maxI, size_t maxJ, size_t maxK);

  void setVerticalScale(float scale);

  void setMeshMode(MeshMode mode);

  void setRenderStyle(bool drawFaces, bool drawEdges);

  int numI() const;

  int numJ() const;

  int numIHiRes() const;

  int numJHiRes() const;

  SoSFInt32 RenderMode;

  SoSFUInt32 SliceI;

  SoSFUInt32 SliceJ;

  SoSFPlane  Plane;
};

VISUALIZATIONDLL_API void BpaVizInit();

#endif
