#include "Traps.h"

#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoAlgebraicSphere.h>
#include <Inventor/nodes/SoMultipleInstance.h>
#include <Inventor/nodes/SoInstanceParameter.h>
#include <Inventor/devices/SoCpuBufferObject.h>

#include <Interface/ProjectHandle.h>
#include <Interface/PropertyValue.h>
#include <Interface/Property.h>
#include <Interface/Snapshot.h>
#include <Interface/Trapper.h>
#include <Interface/GridMap.h>
#include <Interface/Grid.h>

namespace di = DataAccess::Interface;

namespace
{
  SoCpuBufferObject* createPositionsBuffer(const std::vector<SbVec3f>& positions)
  {
    size_t size = positions.size() * sizeof(SbVec3f);
    SoCpuBufferObject* positionBuffer = new SoCpuBufferObject;
    positionBuffer->setSize(size);
    SbVec3f* p = (SbVec3f*)positionBuffer->map(SoBufferObject::SET);
    memcpy(p, &positions[0], size);
    positionBuffer->unmap();

    return positionBuffer;
  }

  SoCpuBufferObject* createScaleBuffer(float scale)
  {
    SoCpuBufferObject* scaleBuffer = new SoCpuBufferObject;
    scaleBuffer->setSize(sizeof(SbVec3f));
    SbVec3f* p = (SbVec3f*)scaleBuffer->map(SoBufferObject::SET);
    *p = SbVec3f(1.f, 1.f, 1.f / scale);
    scaleBuffer->unmap();

    return scaleBuffer;
  }

  void setScaleParameter(SoMultipleInstance* multiInstance, float scale)
  {
    SoInstanceParameter* param = multiInstance->parameters.findParameterByName(
      SoInstanceParameter::getPredefinedParameterName(SoInstanceParameter::SCALE));

    if (param == 0)
      return;

    param->value = createScaleBuffer(scale);
  }

  SoMultipleInstance* createSpheresInstanced(const std::vector<SbVec3f>& positions, float verticalScale)
  {
    // Position data for multiple instances.
    SoCpuBufferObject* positionBuffer = createPositionsBuffer(positions);

    // Add a scale instance parameter that compensates for the vertical scale of the scene, so the 
    // spheres will stay on the mesh under vertical scaling without turning into ellipsoids.
    SoCpuBufferObject* scaleBuffer = createScaleBuffer(verticalScale);

    // Parameter object for multiple instance positions.
    SoInstanceParameter* positionParam =
      SoInstanceParameter::createPredefinedParameter(SoInstanceParameter::TRANSLATION, positionBuffer);
    SoInstanceParameter* scaleParam =
      SoInstanceParameter::createPredefinedParameter(SoInstanceParameter::SCALE, scaleBuffer, (uint32_t)positions.size());

    // Multiple instance node.
    SoMultipleInstance* multiInstance = new SoMultipleInstance();
    multiInstance->numInstances = (uint32_t)positions.size();
    multiInstance->parameters.set1Value(0, positionParam);
    multiInstance->parameters.set1Value(1, scaleParam);

    return multiInstance;
  }
}

Traps createSpillPointsGroup(
  const std::vector<SbVec3f>& spillPointPositions, 
  const std::vector<SbVec3f>& trapPositions, 
  float radius,
  float verticalScale)
{
  // Geometry to be instanced, shared between spillpoints and leakage points
  SoAlgebraicSphere* geometry = new SoAlgebraicSphere;
  geometry->radius = radius;

  // Spill points
  SoMaterial* spillPointMaterial = new SoMaterial;
  spillPointMaterial->diffuseColor = SbColor(1.f, 1.f, 0.f);

  SoMultipleInstance* spillPointsInstance = createSpheresInstanced(spillPointPositions, verticalScale);
  spillPointsInstance->addChild(geometry);

  SoGroup* spillPointsGroup = new SoGroup;
  spillPointsGroup->setName("spillpoints");
  spillPointsGroup->addChild(spillPointMaterial);
  spillPointsGroup->addChild(spillPointsInstance);

  // Leakage points
  SoMaterial* trapsMaterial = new SoMaterial;
  trapsMaterial->diffuseColor = SbColor(0.f, 1.f, 0.f);

  SoMultipleInstance* trapsInstance = createSpheresInstanced(trapPositions, verticalScale);
  trapsInstance->addChild(geometry);

  SoGroup* trapsGroup = new SoGroup;
  trapsGroup->setName("leakagepoints");
  trapsGroup->addChild(trapsMaterial);
  trapsGroup->addChild(trapsInstance);

  // Put both groups under a separator so they don't affect other nodes
  SoSeparator* root = new SoSeparator;
  root->setName("traps");
  root->addChild(spillPointsGroup);
  root->addChild(trapsGroup);

  Traps traps;
  traps.root = root;
  traps.spillpointsGroup = spillPointsGroup;
  traps.spillpointsMaterial = spillPointMaterial;
  traps.spillpointsMultiInstance = spillPointsInstance;
  traps.leakagePointsGroup = trapsGroup;
  traps.leakagePointsMaterial = trapsMaterial;
  traps.leakagePointsMultiInstance = trapsInstance;
  traps.sphere = geometry;
  traps.verticalScale = verticalScale;

  return traps;
}

void Traps::clear()
{
  root = 0;
  spillpointsGroup = 0;
  spillpointsMaterial = 0;
  spillpointsMultiInstance = 0;
  leakagePointsGroup = 0;
  leakagePointsMaterial = 0;
  leakagePointsMultiInstance = 0;
  sphere = 0;
  verticalScale = 1.f;
}

void Traps::setVerticalScale(float scale)
{
  setScaleParameter(spillpointsMultiInstance, scale);
  setScaleParameter(leakagePointsMultiInstance, scale);

  verticalScale = scale;
}

Traps::Traps()
{
  clear();
}

Traps Traps::create(const di::Snapshot* snapshot, const di::Reservoir* reservoir, float verticalScale)
{
  const di::ProjectHandle* handle = snapshot->getProjectHandle();
  const di::Property* top = handle->findProperty("ResRockTop");
  std::unique_ptr<di::PropertyValueList> topValues(top->getPropertyValues(di::RESERVOIR, snapshot, reservoir, 0, 0));
  if (!topValues || topValues->empty())
    return Traps();

  const di::GridMap* topGridMap = (*topValues)[0]->getGridMap();

  const di::Grid* grid = handle->getHighResolutionOutputGrid();
  double deltaI = grid->deltaI();
  double deltaJ = grid->deltaJ();
  double minI = grid->minI();
  double minJ = grid->minJ();

  std::vector<SbVec3f> spillPointPositions;
  std::vector<SbVec3f> trapPositions;

  std::unique_ptr<di::TrapperList> trapperList(handle->getTrappers(reservoir, snapshot, 0, 0));
  for (auto trapper : *trapperList)
  {
    // Spill points
    double x, y;
    trapper->getSpillPointPosition(x, y);
    double i = (x - minI) / deltaI;
    double j = (y - minJ) / deltaJ;
    double z = -topGridMap->getValue(i, j, 0.0);
    spillPointPositions.emplace_back((float)(x - minI), (float)(y - minJ), (float)z);

    // Leakage points
    trapper->getPosition(x, y);
    i = (x - minI) / deltaI;
    j = (y - minJ) / deltaJ;
    z = -topGridMap->getValue(i, j, 0.0);
    trapPositions.emplace_back((float)(x - minI), (float)(y - minJ), (float)z);
  }

  float radius = (float)std::min(deltaI, deltaJ);
  return createSpillPointsGroup(spillPointPositions, trapPositions, radius, verticalScale);
}
