//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef SCENEGRAPHMANAGER_H_INCLUDED
#define SCENEGRAPHMANAGER_H_INCLUDED

#include "defines.h"
#include "Project.h"
#include "Traps.h"

#include <map>
#include <list>
#include <vector>
#include <string>
#include <memory>

#include <Inventor/SbColor.h>
#include <Inventor/misc/SoRef.h>
#include <MeshVizXLM/MbVec3.h>

class SnapshotGeometry;
class SnapshotTopology;
class FormationIdProperty;
class FlowDirectionProperty;
class ColorMap;
class PropertyValueCellFilter;
class FaultMesh;
class OutlineBuilder;
class SeismicScene;

class SbViewportRegion;
class SoPickedPoint;
class SoSeparator;
class SoSwitch; 
class SoGroup;
class SoNode;
class SoText2;
class SoBaseColor;
class SoLineSet;
class SoShapeHints;
class SoAnnotation;
class SoCamera;
class SoPerspectiveCamera;
class SoOrthographicCamera;
class SoMultipleInstance;
class SoAlgebraicSphere;
class SoMaterial;
class PoAutoCubeAxis;
class MoLegend;
class SoScale;
class SoTransparencyType;
class SoEventCallback;
class MoDrawStyle;
class MoMaterial;
class MoDataBinding;
class MoColorMapping;
class MoMesh;
class MoScalarSet;
class MoVec3SetIjk;
class MoMeshSkin;
class MoMeshSlab;
class MoMeshSurface;
class MoMeshFenceSlice;
class MiSkinExtractIjk;
class MexSurfaceMeshUnstructured;

template<class T>
class MiDataSetIjk;
template<class T>
class MiDataSetIj;
template<class T>
class MiDataSetI;

struct SnapshotInfo
{
  struct Chunk
  {
    int minK = 0;
    int maxK = 0;

    std::shared_ptr<MiSkinExtractIjk> extractor;

    SoSeparator* root = nullptr;
    MoMesh* mesh = nullptr;
    MoScalarSet* scalarSet = nullptr;
    MoMeshSurface* skin = nullptr;
  };

  struct Surface
  {
    int id;

    SoSeparator* root;
    MoMesh* mesh;
    MoScalarSet* scalarSet;
    MoMeshSurface* surfaceMesh;
    std::shared_ptr<MiSurfaceMeshCurvilinear> meshData;
    std::shared_ptr<MiDataSetIj<double> > propertyData;

    Surface()
      : id(0)
      , root(0)
      , mesh(0)
      , scalarSet(0)
      , surfaceMesh(0)
    {
    }
  };

  struct Reservoir
  {
    int id;

    std::shared_ptr<MiSkinExtractIjk> extractor;

    SoSeparator* root = nullptr;
    MoMesh* mesh = nullptr;
    MoScalarSet* scalarSet = nullptr;
    MoMeshSurface* skin = nullptr;

    SoIndexedLineSet* trapOutlines;
    SoIndexedLineSet* drainageAreaOutlinesFluid;
    SoIndexedLineSet* drainageAreaOutlinesGas;

    std::shared_ptr<MiVolumeMeshCurvilinear> meshData;
    std::shared_ptr<MiDataSetIjk<double> > propertyData;
    std::weak_ptr<MiDataSetIjk<double> > trapIdPropertyData;

    Traps traps;

    void clear()
    {
      root = 0;
      mesh = 0;
      scalarSet = 0;
      skin = 0;
      trapOutlines = 0;
      drainageAreaOutlinesFluid = 0;
      drainageAreaOutlinesGas = 0;

      meshData.reset();
      propertyData.reset();

      traps = Traps();
    }

    Reservoir()
      : id(0)
    {
      clear();
    }
  };

  struct Fault
  {
    int id;
    int minK;
    int maxK;

    SoSeparator* root;
    MoMesh* mesh;
    MoMeshSurface* surfaceMesh;

    std::shared_ptr<FaultMesh> meshData;

    Fault()
      : id(0)
      , root(0)
      , mesh(0)
      , surfaceMesh(0)
    {
    }
  };

  struct FlowLines
  {
    int id = 0;
    int startK = 0;

    SoSeparator* root  = nullptr;
    SoBaseColor* color = nullptr;
    SoLineSet*   lines = nullptr;

    std::shared_ptr<MiDataSetIj<double> > expulsionData;

    void clear()
    {
      root = 0;
      color = 0;
      lines = 0;

      expulsionData.reset();
    }
  };

  struct Slice
  {
    int axis = 0;
    int position = 0;

    std::shared_ptr<MiSkinExtractIjk> extractor;

    SoSeparator* root = nullptr;
    MoMesh* mesh = nullptr;
    MoScalarSet* scalarSet = nullptr;
    MoMeshSurface* skin = nullptr;

    void clear()
    {
      extractor.reset();

      root = nullptr;
      mesh = nullptr;
      scalarSet = nullptr;
      skin = nullptr;
    }
  };

  struct Fence
  {
	int id = 0;
	size_t timestamp = 0;

	SoSwitch* root = nullptr;
	MoMeshFenceSlice* slice = nullptr;
  };

  size_t index; // index in snapshot list
  double time;

  int currentPropertyId;

  double minZ; // = max depth (negative)
  double maxZ;

  SoRef<SoGroup> root;
  SoGroup* formationsRoot;

  MoMesh* mesh;
  std::shared_ptr<MiVolumeMeshCurvilinear> meshData;

  MoScalarSet* scalarSet;
  std::shared_ptr<MiDataSetIjk<double>> scalarDataSet;
  std::shared_ptr<PropertyValueCellFilter> propertyValueCellFilter;

  MoVec3SetIjk* flowDirSet;
  std::shared_ptr<MiDataSetIjk<double>> flowDirScalarSet;
  std::shared_ptr<MiDataSetIjk<MbVec3<double>>> flowDirVectorSet;

  SoGroup* chunksGroup;
  SoGroup* flowLinesGroup;
  SoGroup* surfacesGroup;
  SoGroup* reservoirsGroup;
  SoGroup* faultsGroup;
  SoGroup* slicesGroup;
  SoGroup* fencesGroup;

  std::vector<Project::SnapshotFormation> formations; 

  std::vector<Chunk> chunks;
  std::vector<Surface> surfaces;
  std::vector<Reservoir> reservoirs;
  std::vector<Fault> faults;
  std::vector<FlowLines> flowlines;
  std::vector<Fence> fences;
  Slice slices[2];

  size_t formationsTimeStamp;
  size_t surfacesTimeStamp;
  size_t reservoirsTimeStamp;
  size_t faultsTimeStamp;
  size_t flowLinesTimeStamp;
  size_t fencesTimeStamp;
  size_t cellFilterTimeStamp;
  size_t seismicPlaneSliceTimeStamp;

  SnapshotInfo();
};

class SceneGraphManager
{
public:

  enum ProjectionType
  {
    PerspectiveProjection,
    OrthographicProjection
  };

  enum DrainageAreaType
  {
    DrainageAreaNone,
    DrainageAreaFluid,
    DrainageAreaGas
  };

  enum FlowLinesType
  {
    FlowLinesLeakage,
    FlowLinesExpulsion
  };

  struct PickResult
  {
    enum Type
    {
      Formation,
      Surface,
      Reservoir,
      Slice,
      Fence,
      Trap,
      Unknown
    } type = Unknown;

    SbVec3f position = SbVec3f(0.f, 0.f, 0.f);

    // Only valid in case type is not Trap
    MbVec3ui cellIndex;
    std::string name;
    double propertyValue = 99999.0;

    // Only valid in case type is Trap
    int trapID = -1;
    int persistentTrapID = -1;
  };

  struct ColorScaleParams
  {
    enum Mapping
    {
      Linear,
      Logarithmic
    } mapping = Linear;

    enum Range
    {
      Automatic,
      Manual
    } range = Automatic;

    double minValue = 0.0;
    double maxValue = 1.0;
  };

  struct FenceParams
  {
	int id;
	bool visible;
	size_t timestamp;
	std::vector<SbVec3f> points;
  };

  struct ViewState
  {
	int currentSnapshotIndex = 0;
    int currentPropertyId = FormationIdPropertyId;

    bool showFaces = true;
    bool showEdges = true;
    bool showGrid = false;
    bool showCompass = true;
    bool showText = true;
    bool showTraps = false;
    bool showTrapOutlines = false;

    DrainageAreaType drainageAreaType = DrainageAreaNone;
    int flowLinesExpulsionStep = 1;
    int flowLinesLeakageStep = 1;
    double flowLinesExpulsionThreshold = 0.0;
    double flowLinesLeakageThreshold = 0.0;

    float verticalScale = 1.f;
    float transparency = 0.f;

    std::vector<bool> formationVisibility;
    std::vector<bool> surfaceVisibility;
    std::vector<bool> reservoirVisibility;
    std::vector<bool> faultVisibility;
    std::vector<bool> flowLinesVisibility;
	std::vector<FenceParams> fences;

    size_t slicePosition[3];
    bool   sliceEnabled[3];

    ColorScaleParams colorScaleParams;

    bool cellFilterEnabled = false;
    double cellFilterMinValue = 0.0;
    double cellFilterMaxValue = 1.0;
  };

  // Derived property ids. These properties are built at runtime
  // based on one or more base properties from the data set.
  static const int DerivedPropertyBaseId      = 0x10000;
  static const int FormationIdPropertyId      = DerivedPropertyBaseId;
  static const int PersistentTrapIdPropertyId = DerivedPropertyBaseId + 1;
  static const int FluidContactsPropertyId    = DerivedPropertyBaseId + 2;

private:

  std::shared_ptr<Project> m_project;
  Project::ProjectInfo m_projectInfo;

  std::list<SnapshotInfo> m_snapshotInfoCache;
  size_t m_maxCacheItems;

  std::shared_ptr<OutlineBuilder> m_outlineBuilder;

  ViewState m_viewState;

  size_t m_formationsTimeStamp;
  size_t m_surfacesTimeStamp;
  size_t m_reservoirsTimeStamp;
  size_t m_faultsTimeStamp;
  size_t m_flowLinesTimeStamp;
  size_t m_fencesTimeStamp;
  size_t m_cellFilterTimeStamp;

  SoGroup*        m_root;
  SoShapeHints*   m_formationShapeHints;
  SoShapeHints*   m_surfaceShapeHints;
  SoShapeHints*   m_decorationShapeHints;

  PoAutoCubeAxis* m_coordinateGrid;
  SoSwitch*       m_coordinateGridSwitch;

  SoScale*        m_scale;

  // Appearance group
  SoGroup*        m_appearanceNode;
  SoTransparencyType* m_transparencyType;
  MoDrawStyle*    m_drawStyle;
  MoMaterial*     m_material;
  MoDataBinding*  m_dataBinding;

  std::shared_ptr<ColorMap> m_colors;
  MoColorMapping* m_colorMap;
  MoColorMapping* m_trapIdColorMap;
  MoColorMapping* m_fluidContactsColorMap;
  SoSwitch*       m_colorMapSwitch;

  SoAnnotation*   m_annotation;

  MoLegend*       m_legend;
  SoSwitch*       m_legendSwitch;

  SoText2*        m_text;
  SoSwitch*       m_textSwitch;
  SoText2*        m_pickText;
  SoSwitch*       m_pickTextSwitch;
  SoSwitch*       m_compassSwitch;

  SoSwitch*       m_seismicSwitch;
  SoSwitch*       m_snapshotsSwitch;

  std::shared_ptr<SeismicScene> m_seismicScene;

  static void mousePressedCallback(void* userData, SoEventCallback* node);
  static void mouseMovedCallback(void* userData, SoEventCallback* node);

  void onMousePressed(SoEventCallback* node);
  void onMouseMoved(SoEventCallback* node);

  int getSurfaceId(MoMeshSurface* surface) const;
  int getFormationId(/*MoMeshSkin* skin, */size_t k) const;
  int getReservoirId(MoMeshSurface* skin) const;

  void updateCoordinateGrid();
  void updateSnapshotMesh();
  void updateSnapshotFormations();
  void updateSnapshotSurfaces();
  void updateSnapshotReservoirs();
  void updateSnapshotTraps();
  void updateSnapshotFaults();
  void updateSnapshotProperties();
  void updateSnapshotSlices();
  void updateSnapshotFlowLines();
  void updateSnapshotCellFilter();
  void updateSnapshotFences();
  void updateColorMap();
  void updateText();
  void updateSnapshot();

  void showPickResult(const PickResult& pickResult);

  SnapshotInfo createSnapshotNode(size_t index);

  void setupCoordinateGrid();
  void setupSceneGraph();

  std::shared_ptr<FaultMesh> generateFaultMesh(
    const std::vector<SbVec2d>& points,
    const MiVolumeMeshCurvilinear& mesh,
    int k0,
    int k1);

  std::shared_ptr<MiDataSetIjk<double> > createFormationProperty(
    const SnapshotInfo& snapshot,
    int propertyId);

  std::shared_ptr<MiDataSetIjk<double> > createReservoirProperty(
    const SnapshotInfo& snapshot, 
    const SnapshotInfo::Reservoir& res, 
    int propertyId);

public:

  SceneGraphManager();

  SoNode* getRoot() const;

  PickResult processPickedPoint(const SoPickedPoint* point);

  const ViewState& getViewState() const;

  void setCurrentSnapshot(size_t index);

  void setVerticalScale(float scale);

  void setTransparency(float transparency);

  void setRenderStyle(bool drawFaces, bool drawEdges);

  void setProperty(int propertyId);

  void setFlowLinesStep(FlowLinesType type, int step);

  void setFlowLinesThreshold(FlowLinesType type, double threshold);

  void enableFormation(int formationId, bool enabled);

  void enableAllFormations(bool enabled);

  void enableSurface(int surfaceId, bool enabled);

  void enableAllSurfaces(bool enabled);

  void enableReservoir(int reservoirId, bool enabled);

  void enableAllReservoirs(bool enabled);

  void enableFault(int faultId, bool enabled);

  void enableAllFaults(bool enabled);

  void enableFlowLines(int flowLinesId, bool enabled);

  void enableAllFlowLines(bool enabled);

  void enableSlice(int slice, bool enabled);

  void setSlicePosition(int slice, int position);

  int  addFence(const std::vector<SbVec3f>& polyline);

  void updateFence(int id, const std::vector<SbVec3f>& polyline);

  void removeFence(int id);

  void enableFence(int id, bool enabled);

  void setColorScaleParams(const ColorScaleParams& params);

  void showCoordinateGrid(bool show);

  void showCompass(bool show);

  void showText(bool show);

  void showTraps(bool show);

  void showTrapOutlines(bool show);

  void showDrainageAreaOutlines(DrainageAreaType type);

  void enableCellFilter(bool enable);

  void setCellFilterRange(double minValue, double maxValue);

  void addSeismicScene(std::shared_ptr<SeismicScene> seismicScene);

  void setup(std::shared_ptr<Project> project);
};

#endif
