#ifndef TRAPS_H_INCLUDED
#define TRAPS_H_INCLUDED

class SoSeparator;
class SoGroup;
class SoMaterial;
class SoMultipleInstance;
class SoAlgebraicSphere;

namespace DataAccess
{
  namespace Interface
  {
    class Snapshot;
    class Reservoir;
  }
}

struct Traps
{
  SoSeparator* root;

  // Spill points
  SoGroup* spillpointsGroup;
  SoMaterial* spillpointsMaterial;
  SoMultipleInstance* spillpointsMultiInstance;

  // Leakage points
  SoGroup* leakagePointsGroup;
  SoMaterial* leakagePointsMaterial;
  SoMultipleInstance* leakagePointsMultiInstance;

  // The sphere used for both leakage and spill points
  SoAlgebraicSphere* sphere;

  float verticalScale;

  void clear();

  void setVerticalScale(float scale);

  Traps();

  static Traps create(
    const DataAccess::Interface::Snapshot* snapshot,
    const DataAccess::Interface::Reservoir* reservoir,
    float verticalScale);
};

#endif
