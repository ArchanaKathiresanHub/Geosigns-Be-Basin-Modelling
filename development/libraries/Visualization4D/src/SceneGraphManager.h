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
#include <MeshVizXLM/MbVec3.h>

class SnapshotGeometry;
class SnapshotTopology;
class FormationIdProperty;
class FlowDirectionProperty;

class FaultMesh;
class OutlineBuilder;

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

  SoGroup* chunksGroup;
  SoGroup* flowLinesGroup;
  SoGroup* flowVectorsGroup;
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

  int m_currentPropertyId;

  bool m_showGrid;
  bool m_showCompass;
  bool m_showText;
  bool m_showTraps;
  bool m_showTrapOutlines;
  bool m_showFlowVectors;

  DrainageAreaType m_drainageAreaType;
  int m_flowLinesStep;
  double m_flowLinesExpulsionThreshold;
  double m_flowLinesLeakageThreshold;

  float m_verticalScale;
  ProjectionType m_projectionType;

  size_t m_formationsTimeStamp;
  size_t m_surfacesTimeStamp;
  size_t m_reservoirsTimeStamp;
  size_t m_faultsTimeStamp;
  size_t m_flowLinesTimeStamp;

  size_t m_slicePosition[3];
  bool   m_sliceEnabled[3];

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
  MoColorMapping* m_colorMap;
  MoColorMapping* m_trapIdColorMap;
  MoColorMapping* m_fluidContactsColorMap;
  SoSwitch*       m_colorMapSwitch;

  SoAnnotation*   m_annotation;

  MoLegend*       m_legend;
  SoSwitch*       m_legendSwitch;

  SoText2*        m_text;
  SoSwitch*       m_textSwitch;
  SoSwitch*       m_compassSwitch;

  SoSwitch*       m_snapshotsSwitch;

  std::vector<bool> m_formationVisibility;
  std::vector<bool> m_surfaceVisibility;
  std::vector<bool> m_reservoirVisibility;
  std::vector<bool> m_faultVisibility;
  std::vector<bool> m_flowLinesVisibility;

  void updateCoordinateGrid();
  void updateSnapshotFormations();
  void updateSnapshotSurfaces();
  void updateSnapshotReservoirs();
  void updateSnapshotTraps();
  void updateSnapshotFaults();
  void updateSnapshotProperties();
  void updateSnapshotSlices();
  void updateSnapshotFlowLines();
  void updateColorMap();
  void updateText();
  void updateSnapshot();

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

  void setCurrentSnapshot(size_t index);

  SoCamera* getCamera() const;

  void setProjection(ProjectionType type);

  void setVerticalScale(float scale);

  void setTransparency(float transparency);

  void setRenderStyle(bool drawFaces, bool drawEdges);

  void setProperty(int propertyId);

  void showFlowVectors(bool enabled);

  void setFlowLinesStep(int step);

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

  void showCoordinateGrid(bool show);

  void showCompass(bool show);

  void showText(bool show);

  void showTraps(bool show);

  void showTrapOutlines(bool show);

  void showDrainageAreaOutlines(DrainageAreaType type);

  void setup(std::shared_ptr<Project> project);
};

#endif
