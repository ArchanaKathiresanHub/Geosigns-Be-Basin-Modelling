#include "Traps.h"

#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoAlgebraicSphere.h>
#include <Inventor/nodes/SoMultipleInstance.h>
#include <Inventor/nodes/SoInstanceParameter.h>
#include <Inventor/nodes/SoVertexProperty.h>
#include <Inventor/nodes/SoIndexedLineSet.h>
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

void Traps::setVerticalScale(float scale)
{
  setScaleParameter(m_spillpointsMultiInstance, scale);
  setScaleParameter(m_leakagePointsMultiInstance, scale);

  m_verticalScale = scale;
}

bool Traps::getTrap(const SbVec3f& position, Project::Trap& pickedTrap) const
{
  float maxDistance = 1.5f * m_radius * m_radius;
  SbVec3f p(position[0], position[1], position[2] / m_verticalScale);
  for (auto const& trap : m_traps)
  {
    if (
      (trap.leakagePoint - p).lengthSquared() < maxDistance ||
      (trap.spillPoint - p).lengthSquared() < maxDistance)
    {
      pickedTrap = trap;
      return true;
    }
  }

  return false;
}

Traps::Traps()
  : m_root(0)
  , m_spillpointsGroup(0)
  , m_spillpointsMaterial(0)
  , m_spillpointsMultiInstance(0)
  , m_leakagePointsGroup(0)
  , m_leakagePointsMaterial(0)
  , m_leakagePointsMultiInstance(0)
  , m_sphere(0)
  , m_spillRoutes(0)
  , m_verticalScale(1.f)
  , m_radius(1.f)
{

}

Traps::Traps(const std::vector<Project::Trap>& traps, float radius, float verticalScale)
  : m_traps(traps)
  , m_root(0)
  , m_spillpointsGroup(0)
  , m_spillpointsMaterial(0)
  , m_spillpointsMultiInstance(0)
  , m_leakagePointsGroup(0)
  , m_leakagePointsMaterial(0)
  , m_leakagePointsMultiInstance(0)
  , m_sphere(0)
  , m_spillRoutes(0)
  , m_verticalScale(verticalScale)
  , m_radius(radius)
{
  init();
}

void Traps::initSpheres(const std::vector<SbVec3f>& spillPointPositions, const std::vector<SbVec3f>& trapPositions)
{
  // Geometry to be instanced, shared between spillpoints and leakage points
  m_sphere = new SoAlgebraicSphere;
  m_sphere->radius = m_radius;

  // Spill points
  m_spillpointsMaterial = new SoMaterial;
  m_spillpointsMaterial->diffuseColor = SbColor(1.f, 1.f, 0.f);

  m_spillpointsMultiInstance = createSpheresInstanced(spillPointPositions, m_verticalScale);
  m_spillpointsMultiInstance->addChild(m_sphere);

  m_spillpointsGroup = new SoGroup;
  m_spillpointsGroup->setName("spillpoints");
  m_spillpointsGroup->addChild(m_spillpointsMaterial);
  m_spillpointsGroup->addChild(m_spillpointsMultiInstance);

  // Leakage points
  m_leakagePointsMaterial = new SoMaterial;
  m_leakagePointsMaterial->diffuseColor = SbColor(0.f, 1.f, 0.f);

  m_leakagePointsMultiInstance= createSpheresInstanced(trapPositions, m_verticalScale);
  m_leakagePointsMultiInstance->addChild(m_sphere);

  m_leakagePointsGroup = new SoGroup;
  m_leakagePointsGroup->setName("leakagepoints");
  m_leakagePointsGroup->addChild(m_leakagePointsMaterial);
  m_leakagePointsGroup->addChild(m_leakagePointsMultiInstance);

  // Put both groups under a separator so they don't affect other nodes
  m_root->addChild(m_spillpointsGroup);
  m_root->addChild(m_leakagePointsGroup);
}

void Traps::initLineSet(const std::vector<SbVec3f>& vertices)
{
  SoVertexProperty* vertexProp = new SoVertexProperty;
  SoIndexedLineSet* lineSet = new SoIndexedLineSet;

  SbVec3f p[5];
  for (int i = 0; i < vertices.size() / 2; ++i)
  {
    p[0] = vertices[2 * i];
    p[3] = vertices[2 * i + 1];

    float len = 2 * m_radius;
    float width = .5f * m_radius;

    SbVec3f v = p[3] - p[0];
    v.normalize();

    // Move head back so the arrow doesn't penetrate into the sphere
    p[3] = p[3] - m_radius * v;
    p[1] = p[3] - len * v;
    SbVec3f vt(-v[1], v[0], 0.f);
    p[2] = p[1] - width * vt;
    p[4] = p[1] + width * vt;

    vertexProp->vertex.setValues(5 * i, 5, p);

    int32_t indices[7] = { 5 * i, 5 * i + 1, 5 * i + 2, 5 * i + 3, 5 * i + 4, 5 * i + 1, -1 };
    lineSet->coordIndex.setValues(7 * i, 7, indices);
  }

  lineSet->vertexProperty = vertexProp;

  m_spillRoutes = lineSet;
  m_root->addChild(lineSet);
}

void Traps::init()
{
  std::vector<SbVec3f> spillPointPositions;
  std::vector<SbVec3f> trapPositions;
  std::vector<SbVec3f> vertices;

  m_root = new SoSeparator;
  m_root->setName("traps");

  for (auto const& trap : m_traps)
  {
    spillPointPositions.push_back(trap.spillPoint);
    trapPositions.push_back(trap.leakagePoint);
  }

  initSpheres(spillPointPositions, trapPositions);

  for (auto const& trap : m_traps)
  {
    if (trap.downStreamId == -1)
      continue;

    auto iter = std::lower_bound(
      m_traps.begin(),
      m_traps.end(),
      trap.downStreamId,
      [](const Project::Trap& trap, int id)
        {
          return trap.id < id;
        });

    if (iter->id == trap.downStreamId)
    {
      vertices.push_back(trap.spillPoint);
      vertices.push_back(iter->leakagePoint);
    }
  }

  if (!vertices.empty())
    initLineSet(vertices);
}
