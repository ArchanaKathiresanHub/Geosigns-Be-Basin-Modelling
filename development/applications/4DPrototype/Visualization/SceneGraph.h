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
class SnapshotNode;

/**
 * Main scenegraph
 */
class VISUALIZATIONDLL_API SceneGraph : public SoSeparator
{
  SO_NODE_HEADER(SceneGraph);

  SoScale*          m_verticalScale;
  SoSwitch*         m_cellFilterSwitch;
  MoCellFilter*     m_cellFilter;
  ROICellFilter*    m_roiFilter;

  SoGroup*          m_appearance;
  MoDrawStyle*      m_drawStyle;
  SoSwitch*         m_snapshots;

  MoPredefinedColorMapping* m_colorMap;
  
  bool              m_planeManipInitialized;
  SoSwitch*         m_planeManipSwitch;
  SoClipPlaneManip* m_planeManip;

  int m_numI;
  int m_numJ;
  int m_numIHiRes;
  int m_numJHiRes;

  std::map<std::string, int> m_formationIdMap;

  Extractor m_extractor;

  void createFilterNode();

  void createAppearanceNode();

  SoGroup* createSnapshotNode(
    DataAccess::Interface::ProjectHandle* handle, 
    const DataAccess::Interface::Snapshot* snapshot);

  void createSnapshotsNode(DataAccess::Interface::ProjectHandle* handle);

  void createRootNode();

  void initializeManip();

public:

  static void initClass();

  static void exitClass();

  SceneGraph();

  ~SceneGraph();
    
  void setup(DataAccess::Interface::ProjectHandle* handle);

  void setProperty(const DataAccess::Interface::Property* prop);

  void setVectorProperty(const DataAccess::Interface::Property* prop[3]);

  int snapshotCount() const;

  const DataAccess::Interface::Snapshot* getSnapshot(int index) const;

  void setCurrentSnapshot(int index);

  void showPlaneManip(bool show);

  void enableROI(bool enable);

  void setROI(size_t minI, size_t minJ, size_t minK, size_t maxI, size_t maxJ, size_t maxK);

  void setVerticalScale(float scale);

  void setRenderStyle(bool drawFaces, bool drawEdges);

  void getRenderStyle(bool& drawFaces, bool& drawEdges);

  void setFormationVisibility(const std::string& name, bool visible);

  void setSurfaceVisibility(const std::string& name, bool visible);

  void setReservoirVisibility(const std::string& name, bool visible);

  int numI() const;

  int numJ() const;

  int numIHiRes() const;

  int numJHiRes() const;

  SoSFUInt32 SliceI;

  SoSFUInt32 SliceJ;

  SoSFPlane  Plane;
};

VISUALIZATIONDLL_API void BpaVizInit();
VISUALIZATIONDLL_API void BpaVizFinish();

#endif
