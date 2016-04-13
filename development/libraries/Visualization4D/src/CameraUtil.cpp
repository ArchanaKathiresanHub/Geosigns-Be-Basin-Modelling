#include "CameraUtil.h"

#include <Inventor/SbVec.h>
#include <Inventor/SbRotation.h>
#include <Inventor/nodes/SoCamera.h>

SbVec3f getViewVector(const SbRotation& orientation)
{
  SbMatrix matrix;
  matrix.setRotate(orientation);

  return SbVec3f(
    -matrix[2][0],
    -matrix[2][1],
    -matrix[2][2]);
}

SbVec3f getFocalPoint(const SoCamera* cam)
{
  SbVec3f pos = cam->position.getValue();
  float focalDist = cam->focalDistance.getValue();
  SbRotation orientation = cam->orientation.getValue();
  SbVec3f viewVec = getViewVector(orientation);

  return pos + focalDist * viewVec;
}

void setViewPreset(SoCamera* camera, ViewPreset preset)
{
  const SbVec3f xAxis(1.f, 0.f, 0.f);
  const SbVec3f yAxis(0.f, 1.f, 0.f);
  const SbVec3f zAxis(0.f, 0.f, 1.f);
  const float rot90  = (float)M_PI_2;
  const float rot180 = (float)M_PI;

  SbRotation orientation; // default: top view
  switch (preset)
  {
  case ViewPreset_Top:
    // nothing to be done here
    break;
  case ViewPreset_Left:
    orientation = SbRotation(xAxis, rot90) * SbRotation(zAxis, -rot90);
    break;
  case ViewPreset_Front:
    orientation = SbRotation(xAxis, rot90);
    break;
  case ViewPreset_Bottom:
    orientation = SbRotation(xAxis, rot180);
    break;
  case ViewPreset_Right:
    orientation = SbRotation(xAxis, rot90) * SbRotation(zAxis, rot90);
    break;
  case ViewPreset_Back:
    orientation = SbRotation(xAxis, rot90) * SbRotation(zAxis, rot180);
    break;
  }

  SbVec3f focalPoint = getFocalPoint(camera);

  camera->orientation = orientation;
  camera->position = focalPoint - camera->focalDistance.getValue() * getViewVector(orientation);
}