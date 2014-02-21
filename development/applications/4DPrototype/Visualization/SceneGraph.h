#ifndef SCENEGRAPH_H_INCLUDED
#define SCENEGRAPH_H_INCLUDED

#include "defines.h" 

#include "Interface/Interface.h"

#include <Inventor/nodes/SoSeparator.h>

class MoMesh;
class MoMeshSkin;
class MoScalarSetI;
class MoPredefinedColorMapping;

class SoSwitch;
class SoGroup;

/**
 * Scenegraph for a mesh representing a single snapshot
 */
class SnapshotNode : public SoSeparator
{
  const DataAccess::Interface::Snapshot* m_snapshot;

  MoMesh*       m_mesh;
  MoScalarSetI* m_scalarSet;
  MoMeshSkin*   m_skin;

public:

  SnapshotNode(
    const DataAccess::Interface::Snapshot* snapshot, 
    std::shared_ptr<DataAccess::Interface::PropertyValueList> depthValues);

  VISUALIZATIONDLL_API void setProperty(const DataAccess::Interface::Property* prop);

  VISUALIZATIONDLL_API MoScalarSetI* scalarSet() const;
};

/**
 * Main scenegraph
 */
class SceneGraph
{
  SoGroup*  m_root;
  SoGroup*  m_appearance;
  SoSwitch* m_snapshots;

  MoPredefinedColorMapping* m_colorMap;

  void createAppearanceNode();

  void createSnapshotsNode(DataAccess::Interface::ProjectHandle* handle);

  void createRootNode();

public:

  VISUALIZATIONDLL_API explicit SceneGraph(DataAccess::Interface::ProjectHandle* handle);

  VISUALIZATIONDLL_API SoGroup* root() const;

  VISUALIZATIONDLL_API void setProperty(const DataAccess::Interface::Property* prop);

  VISUALIZATIONDLL_API int snapshotCount() const;

  VISUALIZATIONDLL_API void setCurrentSnapshot(int index);
};

#endif
