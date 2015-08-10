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

class SoSeparator;
class SoSwitch; 
class SoGroup;

class SoScale;
class MoDrawStyle;
class MoMaterial;
class MoDataBinding;
class MoColorMapping;
class MoMesh;
class MoScalarSetIjk;
class MoMeshSkin;
class MoMeshSlab;

namespace DataAccess
{
  namespace Interface
  {
    class ProjectHandle;
    class Property;
    class Snapshot;
  }
}

struct SnapshotInfo
{
  struct FormationBounds
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

  SoSeparator* root;

  MoMesh* mesh;
  HexahedronMesh* meshData;

  MoScalarSetIjk* scalarSet;
  FormationIdProperty* formationIdProperty;

  SoSwitch* sliceSwitch[3];
  MoMeshSlab* slice[3];

  SoGroup* chunksGroup;
  SoGroup* slicesGroup;

  std::vector<FormationBounds> formations; 
  std::vector<Chunk> chunks;

  size_t formationVisibilityTimestamp;

  SnapshotInfo();
};

struct FormationInfo
{
  int id;
  bool visible;
  std::string name;
};

class VISUALIZATIONDLL_API SceneGraphManager
{
  const DataAccess::Interface::ProjectHandle* m_projectHandle;
  const DataAccess::Interface::Property* m_depthProperty;

  int m_numI;
  int m_numJ;
  int m_numIHiRes;
  int m_numJHiRes;

  std::map<std::string, int> m_formationIdMap;
  std::vector<FormationInfo> m_formations;
  std::vector<SnapshotInfo>  m_snapshots;

  size_t m_formationVisibilityTimestamp;
  size_t m_currentSnapshot;

  size_t m_slicePosition[3];
  bool   m_sliceEnabled[3];

  SoGroup*        m_root;
  SoScale*        m_scale;

  // Appearance group
  SoGroup*        m_appearanceNode;
  MoDrawStyle*    m_drawStyle;
  MoMaterial*     m_material;
  MoDataBinding*  m_dataBinding;
  MoColorMapping* m_colorMap;

  SoSwitch*       m_snapshotsSwitch;

  void updateSnapshot(size_t index);

  SnapshotInfo createSnapshotNode(const DataAccess::Interface::Snapshot* snapshot);
    
  void setupSnapshots();
  void setupSceneGraph();

public:

  SceneGraphManager::SceneGraphManager();

  SoGroup* getRoot() const;

  void setCurrentSnapshot(size_t index);

  size_t getSnapshotCount() const;

  int numI() const;

  int numJ() const;

  int numIHiRes() const;

  int numJHiRes() const;

  void setVerticalScale(float scale);

  void setFormationVisibility(const std::string& name, bool visible);

  void enableSlice(int slice, bool enabled);

  void setSlicePosition(int slice, int position);

  void setup(const DataAccess::Interface::ProjectHandle* handle);
};

#endif
