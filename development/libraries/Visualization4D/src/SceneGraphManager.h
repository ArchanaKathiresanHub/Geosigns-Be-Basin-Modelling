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
class MoScalarSetIj;
class MoScalarSetIjk;
class MoVec3SetIjk;
class MoMeshSkin;
class MoMeshSlab;
class MoMeshSurface;
class MoMeshFenceSlice;
class MoCellFilter;

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
    int minK;
    int maxK;

    MoMeshSkin* skin;

    Chunk(int kmin, int kmax, MoMeshSkin* meshSkin = 0)
      : minK(kmin)
      , maxK(kmax)
      , skin(meshSkin)
    {
    }
  };

  struct Surface
  {
    int id;

    SoSeparator* root;
    MoMesh* mesh;
    MoScalarSetIj* scalarSet;
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

    SoSeparator* root;
    MoMesh* mesh;
    MoScalarSetIjk* scalarSet;
    MoMeshSkin* skin;

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
    int id;
    int startK;

    SoSeparator* root;
    SoBaseColor* color;
    SoLineSet*   lines;

    std::shared_ptr<MiDataSetIj<double> > expulsionData;

    FlowLines()
      : id(0)
      , startK(0)
      , root(0)
      , color(0)
      , lines(0)
    {
    }

    void clear()
    {
      root = 0;
      color = 0;
      lines = 0;

      expulsionData.reset();
    }
  };

  size_t index; // index in snapshot list
  double time;

  int currentPropertyId;

  double minZ; // = max depth (negative)
  double maxZ;

  SoSeparator* root;
  SoSeparator* formationsRoot;

  MoMesh* mesh;
  std::shared_ptr<MiVolumeMeshCurvilinear> meshData;

  MoScalarSetIjk* scalarSet;
  std::shared_ptr<MiDataSetIjk<double> > scalarDataSet;

  MoVec3SetIjk* flowDirSet;
  std::shared_ptr<MiDataSetIjk<double> > flowDirScalarSet;
  std::shared_ptr<MiDataSetIjk<MbVec3<double> > > flowDirVectorSet;

  SoSwitch* sliceSwitch[3];
  MoMeshSlab* slice[3];

  SoSwitch* cellFilterSwitch;
  MoCellFilter* cellFilter;
  std::shared_ptr<PropertyValueCellFilter> propertyValueCellFilter;

  SoGroup* chunksGroup;
  SoGroup* flowLinesGroup;
  SoGroup* surfacesGroup;
  SoGroup* reservoirsGroup;
  SoGroup* faultsGroup;
  SoGroup* slicesGroup;

  std::vector<Project::SnapshotFormation> formations; 

  std::vector<Chunk> chunks;
  std::vector<Surface> surfaces;
  std::vector<Reservoir> reservoirs;
  std::vector<Fault> faults;
  std::vector<FlowLines> flowlines;

  size_t formationsTimeStamp;
  size_t surfacesTimeStamp;
  size_t reservoirsTimeStamp;
  size_t faultsTimeStamp;
  size_t flowLinesTimeStamp;

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
      Trap,
      Unknown
    } type = Unknown;

    SbVec3f position = SbVec3f(0.f, 0.f, 0.f);

    // Only valid in case type is not Trap
    size_t i=0, j=0, k=0;
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

  // ----------------------------------------
  // View state
  int m_currentPropertyId;

  bool m_showGrid;
  bool m_showCompass;
  bool m_showText;
  bool m_showTraps;
  bool m_showTrapOutlines;

  DrainageAreaType m_drainageAreaType;
  int m_flowLinesExpulsionStep;
  int m_flowLinesLeakageStep;
  double m_flowLinesExpulsionThreshold;
  double m_flowLinesLeakageThreshold;

  float m_verticalScale;
  ProjectionType m_projectionType;

  std::vector<bool> m_formationVisibility;
  std::vector<bool> m_surfaceVisibility;
  std::vector<bool> m_reservoirVisibility;
  std::vector<bool> m_faultVisibility;
  std::vector<bool> m_flowLinesVisibility;

  size_t m_slicePosition[3];
  bool   m_sliceEnabled[3];

  ColorScaleParams m_colorScaleParams;

  bool m_cellFilterEnabled;
  double m_cellFilterMinValue;
  double m_cellFilterMaxValue;
  // ----------------------------------------

  size_t m_formationsTimeStamp;
  size_t m_surfacesTimeStamp;
  size_t m_reservoirsTimeStamp;
  size_t m_faultsTimeStamp;
  size_t m_flowLinesTimeStamp;

  SoPerspectiveCamera*  m_perspectiveCamera;
  SoOrthographicCamera* m_orthographicCamera;
  SoSwitch*             m_cameraSwitch;

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

  SoSwitch*       m_snapshotsSwitch;

  struct FenceSlice
  {
    int id;
    bool visible;
    std::vector<SbVec3f> points;

    SoSwitch* fenceSwitch;
    MoMeshFenceSlice* fence;
  };

  std::vector<FenceSlice> m_fences;
  SoRef<SoGroup>  m_fencesGroup;

  static void mousePressedCallback(void* userData, SoEventCallback* node);
  static void mouseMovedCallback(void* userData, SoEventCallback* node);

  void onMousePressed(SoEventCallback* node);
  void onMouseMoved(SoEventCallback* node);

  int getSurfaceId(MoMeshSurface* surface) const;
  int getFormationId(/*MoMeshSkin* skin, */size_t k) const;
  int getReservoirId(MoMeshSkin* skin) const;

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

  void setCurrentSnapshot(size_t index);

  SoCamera* getCamera() const;

  void setProjection(ProjectionType type);

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

  void setup(std::shared_ptr<Project> project);
};

#endif
