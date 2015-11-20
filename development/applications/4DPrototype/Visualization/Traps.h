#ifndef TRAPS_H_INCLUDED
#define TRAPS_H_INCLUDED

class SoNode; 
class SoGroup; 
class SoSeparator;
class SoMaterial;
class SoMultipleInstance;
class SoAlgebraicSphere;
class SoIndexedLineSet;

#include "Project.h"

#include <vector>

#include <Inventor/SbVec.h>

class Traps
{
  std::vector<Project::Trap> m_traps;

  SoSeparator* m_root;

  // Spill points
  SoGroup* m_spillpointsGroup;
  SoMaterial* m_spillpointsMaterial;
  SoMultipleInstance* m_spillpointsMultiInstance;

  // Leakage points
  SoGroup* m_leakagePointsGroup;
  SoMaterial* m_leakagePointsMaterial;
  SoMultipleInstance* m_leakagePointsMultiInstance;

  // The sphere used for both leakage and spill points
  SoAlgebraicSphere* m_sphere;

  SoIndexedLineSet* m_spillRoutes;

  float m_verticalScale;
  float m_radius;

  void initSpheres(const std::vector<SbVec3f>& spillPointPositions, const std::vector<SbVec3f>& trapPositions);

  void initLineSet(const std::vector<SbVec3f>& vertices);

  void init();

public:

  SoSeparator* root() const { return m_root; }

  float verticalScale() const { return m_verticalScale; }

  void setVerticalScale(float scale);

  Traps();

  Traps(const std::vector<Project::Trap>& traps, float radius, float verticalScale);
};

#endif
