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

#include <map>
#include <vector>
#include <string>
#include <memory>

#include <Inventor/SbColor.h>

class SnapshotGeometry;
class SnapshotTopology;
class FormationIdProperty;
class HexahedronMesh;
class SurfaceMesh;
class ReservoirMesh;
class FaultMesh;

class SoSeparator;
class SoSwitch; 
class SoGroup;
class SoNode;
class SoShapeHints;
class SoCamera;
class SoPerspectiveCamera;
class SoOrthographicCamera;
class PoAutoCubeAxis;
class MoLegend;
class SoScale;
class MoDrawStyle;
class MoMaterial;
class MoDataBinding;
class MoColorMapping;
class MoMesh;
class MoScalarSet;
class MoScalarSetIjk;
class MoMeshSkin;
class MoMeshSlab;
class MoMeshSurface;
class MoMeshFenceSlice;
template<class T>
class MiDataSetIjk;
template<class T>
class MiDataSetI;

namespace DataAccess
{
  namespace Interface
  {
    class ProjectHandle;
    class Formation;
    class Surface;
    class Reservoir;
    class Fault;
    class Property;
    class Snapshot;
  }
}

struct SnapshotInfo
{
  struct Formation
  {
    int id;
    int minK;
    int maxK;
  };

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
    SurfaceMesh* meshData;
    MoScalarSet* scalarSet;
    MiDataSetI<double>* propertyData;
    MoMeshSurface* surfaceMesh;

    Surface()
      : id(0)
      , root(0)
      , mesh(0)
      , meshData(0)
      , scalarSet(0)
      , propertyData(0)
      , surfaceMesh(0)
    {
    }
  };

  struct Reservoir
  {
    int id;

    SoSeparator* root;
    MoMesh* mesh;
    ReservoirMesh* meshData;
    MoScalarSet* scalarSet;
    MiDataSetIjk<double>* propertyData;
    MoMeshSkin* skin;

    Reservoir()
      : id(0)
      , root(0)
      , mesh(0)
      , meshData(0)
      , scalarSet(0)
      , propertyData(0)
      , skin(0)
    {
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

  double minZ; // = max depth (negative)
  double maxZ;

  const DataAccess::Interface::Snapshot* snapshot;
  const DataAccess::Interface::Property* currentProperty;

  std::shared_ptr<SnapshotGeometry> geometry;
  std::shared_ptr<SnapshotTopology> topology;

  SoSeparator* root;
  SoSeparator* formationsRoot;

  MoMesh* mesh;
  HexahedronMesh* meshData;

  MoScalarSetIjk* scalarSet;
  std::shared_ptr<MiDataSetIjk<double> > scalarDataSet;

  SoSwitch* sliceSwitch[3];
  MoMeshSlab* slice[3];

  SoGroup* chunksGroup;
  SoGroup* surfacesGroup;
  SoGroup* reservoirsGroup;
  SoGroup* faultsGroup;
  SoGroup* slicesGroup;

  std::vector<Formation> formations; 
  std::vector<Chunk> chunks;
  std::vector<Surface> surfaces;
  std::vector<Reservoir> reservoirs;
  std::vector<Fault> faults;

  size_t formationsTimeStamp;
  size_t surfacesTimeStamp;
  size_t reservoirsTimeStamp;
  size_t faultsTimeStamp;

  SnapshotInfo();
};

class VISUALIZATIONDLL_API SceneGraphManager
{
public:

  enum ProjectionType
  {
    PerspectiveProjection,
    OrthographicProjection
  };

private:

  template<class T>
  struct ObjectInfo
  {
    const T* object;

    int id;
    bool visible;
  };

  typedef ObjectInfo<DataAccess::Interface::Formation> FormationInfo;
  typedef ObjectInfo<DataAccess::Interface::Surface> SurfaceInfo;
  typedef ObjectInfo<DataAccess::Interface::Reservoir> ReservoirInfo;
  typedef ObjectInfo<DataAccess::Interface::Fault> FaultInfo;

  const DataAccess::Interface::ProjectHandle* m_projectHandle;
  const DataAccess::Interface::Property* m_depthProperty;
  const DataAccess::Interface::Property* m_resRockTopProperty;
  const DataAccess::Interface::Property* m_resRockBottomProperty;
  const DataAccess::Interface::Property* m_resRockTrapIdProperty;
  const DataAccess::Interface::Property* m_currentProperty;

  int m_numI;
  int m_numJ;
  int m_numIHiRes;
  int m_numJHiRes;

  double m_minX;
  double m_minY;
  double m_maxX;
  double m_maxY;

  unsigned int m_maxPersistentTrapId;

  std::map<std::string, int> m_formationIdMap;
  std::map<std::string, int> m_surfaceIdMap;
  std::map<std::string, int> m_reservoirIdMap;
  std::map<std::tuple<std::string, std::string>, int> m_faultIdMap;

  std::vector<FormationInfo> m_formations;
  std::vector<SurfaceInfo>   m_surfaces;
  std::vector<ReservoirInfo> m_reservoirs;
  std::vector<FaultInfo>     m_faults;
  std::vector<SnapshotInfo>  m_snapshots;

  bool m_showGrid;
  ProjectionType m_projectionType;

  size_t m_formationsTimeStamp;
  size_t m_surfacesTimeStamp;
  size_t m_reservoirsTimeStamp;
  size_t m_faultsTimeStamp;
  size_t m_currentSnapshot;

  size_t m_slicePosition[3];
  bool   m_sliceEnabled[3];

  SoPerspectiveCamera*  m_perspectiveCamera;
  SoOrthographicCamera* m_orthographicCamera;
  SoSwitch*             m_cameraSwitch;

  SoGroup*        m_root;
  SoShapeHints*   m_formationShapeHints;
  SoShapeHints*   m_surfaceShapeHints;

  PoAutoCubeAxis* m_coordinateGrid;
  SoSwitch*       m_coordinateGridSwitch;

  SoScale*        m_scale;

  // Appearance group
  SoGroup*        m_appearanceNode;
  MoDrawStyle*    m_drawStyle;
  MoMaterial*     m_material;
  MoDataBinding*  m_dataBinding;
  MoColorMapping* m_colorMap;
  MoColorMapping* m_trapIdColorMap;
  SoSwitch*       m_colorMapSwitch;

  MoLegend*       m_legend;
  SoSwitch*       m_legendSwitch;

  SoSwitch*       m_snapshotsSwitch;

  MiDataSetIjk<double>* createFormationProperty(const DataAccess::Interface::Property* prop, const SnapshotInfo& snapshot);

  void updateCoordinateGrid();
  void updateSnapshotFormations();
  void updateSnapshotSurfaces();
  void updateSnapshotReservoirs();
  void updateSnapshotFaults();
  void updateSnapshotProperties();
  void updateSnapshotSlices();
  void updateColorMap();
  void updateSnapshot();

  SnapshotInfo createSnapshotNode(const DataAccess::Interface::Snapshot* snapshot);
    
  void setupSnapshots();
  void setupCoordinateGrid();
  void setupSceneGraph();

public:

  SceneGraphManager();

  SoNode* getRoot() const;

  void setCurrentSnapshot(size_t index);

  size_t getSnapshotCount() const;

  int numI() const;

  int numJ() const;

  int numIHiRes() const;

  int numJHiRes() const;

  SoCamera* getCamera() const;

  void setProjection(ProjectionType type);

  void setVerticalScale(float scale);

  void setRenderStyle(bool drawFaces, bool drawEdges);

  void setProperty(const std::string& name);

  void enableFormation(const std::string& name, bool enabled);

  void enableSurface(const std::string& name, bool enabled);

  void enableReservoir(const std::string& name, bool enabled);

  void enableFault(const std::string& collectionName, const std::string& name, bool enabled);

  void enableSlice(int slice, bool enabled);

  void setSlicePosition(int slice, int position);

  void showCoordinateGrid(bool show);

  void setup(const DataAccess::Interface::ProjectHandle* handle);
};

#endif