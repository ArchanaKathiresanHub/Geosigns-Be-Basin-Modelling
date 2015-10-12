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

Traps::Traps()
  : m_snapshot(0)
  , m_reservoir(0)
  , m_topValues(0)
  , m_minI(0.0)
  , m_minJ(0.0)
  , m_deltaI(0.0)
  , m_deltaJ(0.0)
  , m_root(0)
  , m_spillpointsGroup(0)
  , m_spillpointsMaterial(0)
  , m_spillpointsMultiInstance(0)
  , m_leakagePointsGroup(0)
  , m_leakagePointsMaterial(0)
  , m_leakagePointsMultiInstance(0)
  , m_sphere(0)
  , m_spillRoutes(0)
  , m_verticalScale(1.f)
{

}

Traps::Traps(const di::Snapshot* snapshot, const di::Reservoir* reservoir, float verticalScale)
  : m_snapshot(snapshot)
  , m_reservoir(reservoir)
  , m_topValues(0)
  , m_minI(0.0)
  , m_minJ(0.0)
  , m_deltaI(0.0)
  , m_deltaJ(0.0)
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
{
  const di::ProjectHandle* handle = snapshot->getProjectHandle();

  const di::Grid* grid = handle->getHighResolutionOutputGrid();
  m_deltaI = grid->deltaI();
  m_deltaJ = grid->deltaJ();
  m_minI = grid->minI();
  m_minJ = grid->minJ();

  const di::Property* top = handle->findProperty("ResRockTop");
  std::unique_ptr<di::PropertyValueList> topValues(top->getPropertyValues(di::RESERVOIR, snapshot, reservoir, 0, 0));
  if (topValues || !topValues->empty())
  {
    m_topValues = (*topValues)[0]->getGridMap();

    init();
  }
}

SbVec3f Traps::getPosition(PositionType type, const DataAccess::Interface::Trapper* trapper) const
{
  double x, y;
  if (type == SpillPointPosition)
    trapper->getSpillPointPosition(x, y);
  else
    trapper->getPosition(x, y);

  x -= m_minI;
  y -= m_minJ;

  double i = x / m_deltaI;
  double j = y / m_deltaJ;
  double z = -m_topValues->getValue(i, j, 0.0);

  return SbVec3f((float)x, (float)y, (float)z);
}

void Traps::initSpheres(
  const std::vector<SbVec3f>& spillPointPositions,
  const std::vector<SbVec3f>& trapPositions)
{
  // Geometry to be instanced, shared between spillpoints and leakage points
  m_sphere = new SoAlgebraicSphere;
  m_sphere->radius = (float)std::min(m_deltaI, m_deltaJ);

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
  //vertexProp->vertex.setValues(0, (int)vertices.size(), &vertices[0]);

  SoIndexedLineSet* lineSet = new SoIndexedLineSet;

  SbVec3f p[4];
  for (int i = 0; i < vertices.size() / 2; ++i)
  {
    p[0] = vertices[2 * i];
    p[1] = vertices[2 * i + 1];

    float radius = (float)std::min(m_deltaI, m_deltaJ);
    float len = 2 * radius;
    float width = .5f * radius;

    SbVec3f v = p[1] - p[0];
    v.normalize();

    // Move p1 back so the arrow doesn't penetrate into the sphere
    p[1] = p[1] - radius * v;
    SbVec3f q = p[1] - len * v;
    SbVec3f vt(-v[1], v[0], 0.f);
    p[2] = q - width * vt;
    p[3] = q + width * vt;

    vertexProp->vertex.setValues(4 * i, 4, p);

    int32_t indices[6] = { 4 * i, 4 * i + 1, 4 * i + 2, 4 * i + 3, 4 * i + 1, -1 };
    lineSet->coordIndex.setValues(6 * i, 6, indices);
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

  const di::ProjectHandle* handle = m_snapshot->getProjectHandle();

  std::unique_ptr<di::TrapperList> trapperList(handle->getTrappers(m_reservoir, m_snapshot, 0, 0));
  for (auto trapper : *trapperList)
  {
    SbVec3f spillPoint = getPosition(SpillPointPosition, trapper);
    SbVec3f leakagePoint = getPosition(LeakagePointPosition, trapper);

    spillPointPositions.push_back(spillPoint);
    trapPositions.push_back(leakagePoint);

    const di::Trapper* dsTrapper = trapper->getDownstreamTrapper();
    if (dsTrapper != 0)
    {
      SbVec3f downStreamPoint = getPosition(LeakagePointPosition, dsTrapper);

      vertices.push_back(downStreamPoint);
      vertices.push_back(leakagePoint);
    }
  }

  m_root = new SoSeparator;
  m_root->setName("traps");

  initSpheres(spillPointPositions, trapPositions);
  initLineSet(vertices);
}
