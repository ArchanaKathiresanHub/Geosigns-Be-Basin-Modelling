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
class ScalarProperty;
class HexahedronMesh;
class SurfaceMesh;

class SoSeparator;
class SoSwitch; 
class SoGroup;
class SoNode;
class SoShapeHints;

class MoLegend;
class SoScale;
class MoDrawStyle;
class MoMaterial;
class MoDataBinding;
class MoColorMapping;
class MoMesh;
class MoScalarSetIjk;
class MoMeshSkin;
class MoMeshSlab;
class MoMeshSurface;
template<class T>
class MiDataSetIjk;

namespace DataAccess
{
  namespace Interface
  {
    class ProjectHandle;
    class Formation;
    class Surface;
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
    MoMeshSurface* surfaceMesh;

    Surface()
      : id(0)
      , root(0)
      , mesh(0)
      , meshData(0)
      , surfaceMesh(0)
    {
    }
  };

  const DataAccess::Interface::Snapshot* snapshot;
  const DataAccess::Interface::Property* currentProperty;

  SoSeparator* root;

  MoMesh* mesh;
  HexahedronMesh* meshData;

  MoScalarSetIjk* scalarSet;
  std::shared_ptr<MiDataSetIjk<double> > scalarDataSet;
  std::shared_ptr<FormationIdProperty> formationIdDataSet;

  SoSwitch* sliceSwitch[3];
  MoMeshSlab* slice[3];

  SoGroup* chunksGroup;
  SoGroup* surfacesGroup;
  SoGroup* slicesGroup;

  std::vector<Formation> formations; 
  std::vector<Chunk> chunks;
  std::vector<Surface> surfaces;

  size_t formationsTimeStamp;
  size_t surfacesTimeStamp;

  SnapshotInfo();
};

struct FormationInfo
{
  const DataAccess::Interface::Formation* formation;

  int id;
  bool visible;
};

struct SurfaceInfo
{
  const DataAccess::Interface::Surface* surface;

  int id;
  bool visible;
};

class VISUALIZATIONDLL_API SceneGraphManager
{
  const DataAccess::Interface::ProjectHandle* m_projectHandle;
  const DataAccess::Interface::Property* m_depthProperty;
  const DataAccess::Interface::Property* m_currentProperty;

  int m_numI;
  int m_numJ;
  int m_numIHiRes;
  int m_numJHiRes;

  std::map<std::string, int> m_formationIdMap;
  std::map<std::string, int> m_surfaceIdMap;
  std::vector<FormationInfo> m_formations;
  std::vector<SurfaceInfo>   m_surfaces;
  std::vector<SnapshotInfo>  m_snapshots;

  size_t m_formationsTimeStamp;
  size_t m_surfacesTimeStamp;
  size_t m_currentSnapshot;

  size_t m_slicePosition[3];
  bool   m_sliceEnabled[3];

  SoGroup*        m_root;
  SoShapeHints*   m_formationShapeHints;
  SoShapeHints*   m_surfaceShapeHints;
  SoScale*        m_scale;

  // Appearance group
  SoGroup*        m_appearanceNode;
  MoDrawStyle*    m_drawStyle;
  MoMaterial*     m_material;
  MoDataBinding*  m_dataBinding;
  MoColorMapping* m_colorMap;

  MoLegend*       m_legend;
  SoSwitch*       m_legendSwitch;

  SoSwitch*       m_snapshotsSwitch;

  void updateSnapshotFormations(size_t index);
  void updateSnapshotSurfaces(size_t index);
  void updateSnapshotProperties(size_t index);
  void updateSnapshotSlices(size_t index);
  void updateSnapshot(size_t index);

  SnapshotInfo createSnapshotNode(const DataAccess::Interface::Snapshot* snapshot);
    
  void setupSnapshots();
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

  void setVerticalScale(float scale);

  void setRenderStyle(bool drawFaces, bool drawEdges);

  void setProperty(const std::string& name);

  void enableFormation(const std::string& name, bool enabled);

  void enableSurface(const std::string& name, bool enabled);

  void enableSlice(int slice, bool enabled);

  void setSlicePosition(int slice, int position);

  void setup(const DataAccess::Interface::ProjectHandle* handle);
};

#endif
