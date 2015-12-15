#ifndef CAMERAUTIL_H_INCLUDED
#define CAMERAUTIL_H_INCLUDED

#include "defines.h"

class SbVec3f;
class SbRotation;
class SoCamera;

/**
 * Used to choose a predefined camera orientation
 */
enum ViewPreset
{
  ViewPreset_Top,
  ViewPreset_Left,
  ViewPreset_Front,
  ViewPreset_Bottom,
  ViewPreset_Right,
  ViewPreset_Back
};

/**
 * Computes the view vector (the direction in which the camera is pointing) from
 * the camera orientation.
 */
SbVec3f VISUALIZATIONDLL_API getViewVector(const SbRotation& orientation);

/**
 * Computes the focal point of the camera.
 */
SbVec3f VISUALIZATIONDLL_API getFocalPoint(const SoCamera* camera);

/**
 * Sets up the camera according to one of the predefined views. Only the camera
 * orientation and position are changed, other parameters (such as focal distance)
 * remain the same.
 */
void VISUALIZATIONDLL_API setViewPreset(SoCamera* camera, ViewPreset preset);

#endif
