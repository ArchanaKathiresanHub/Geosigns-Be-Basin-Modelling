#ifndef TRAPS_H_INCLUDED
#define TRAPS_H_INCLUDED

class SoNode; 
class SoGroup; 
class SoSeparator;
class SoMaterial;
class SoMultipleInstance;
class SoAlgebraicSphere;
class SoIndexedLineSet;

namespace DataAccess
{
  namespace Interface
  {
    class Snapshot;
    class Reservoir;
    class GridMap;
    class Trapper;
  }
}

#include <vector>

#include <Inventor/SbVec.h>

class Traps
{
  const DataAccess::Interface::Snapshot*  m_snapshot;
  const DataAccess::Interface::Reservoir* m_reservoir;
  const DataAccess::Interface::GridMap*   m_topValues;

  double m_minI;
  double m_minJ;
  double m_deltaI;
  double m_deltaJ;

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

  enum PositionType
  {
    SpillPointPosition,
    LeakagePointPosition
  };

  SbVec3f getPosition(PositionType type, const DataAccess::Interface::Trapper* trapper) const;

  void initSpheres(
    const std::vector<SbVec3f>& spillPointPositions,
    const std::vector<SbVec3f>& leakagePointPositions);

  void initLineSet(const std::vector<SbVec3f>& vertices);

  void init();

public:

  SoSeparator* root() const { return m_root; }

  float verticalScale() const { return m_verticalScale; }

  void setVerticalScale(float scale);

  Traps();

  Traps(
    const DataAccess::Interface::Snapshot* snapshot,
    const DataAccess::Interface::Reservoir* reservoir,
    float verticalScale);
};

#endif
