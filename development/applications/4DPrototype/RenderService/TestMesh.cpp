#include <MeshVizInterface/mapping/nodes/MoMesh.h>
#include <MeshVizInterface/mapping/nodes/MoMeshSkin.h>
#include <MeshVizInterface/mapping/nodes/MoDrawStyle.h>
#include <MeshVizInterface/mapping/nodes/MoMaterial.h>

#include <MeshVizInterface/implement/mesh/volumes/MbVolumeMeshRegular.h>

/**
 *
 */
SoNode* buildTestSceneGraph()
{
  MoMesh* mesh = new MoMesh;
  mesh->setMesh(new MbVolumeMeshRegular<float, float, MbVec3f>(10, 10, 10, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f));

  MoDrawStyle* drawStyle = new MoDrawStyle;
  drawStyle->displayFaces = true;
  drawStyle->displayEdges = true;
  drawStyle->displayPoints = false;

  MoMaterial* material = new MoMaterial;
  material->faceColoring = MoMaterial::COLOR;
  material->faceColor = SbColor(.2f, .2f, 1.0f);
  material->lineColoring = MoMaterial::COLOR;
  material->lineColor = SbColor(.0f, .0f, .0f);

  MoMeshSkin* skin = new MoMeshSkin;

  SoGroup* root = new SoGroup;
  root->addChild(mesh);
  root->addChild(material);
  root->addChild(drawStyle);
  root->addChild(skin);

  return root;
}


