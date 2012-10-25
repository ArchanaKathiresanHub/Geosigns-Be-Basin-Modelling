#include <iostream.h>
#include <vector>
#include <algorithm>
#include <limits>

#include "DiffusionLeak.h"
#include "DiffusionCoefficient.h"

using std::vector;
using std::cout;

using migration::DiffusionLeak;
using migration::DiffusionCoefficient;

int main() {
  
  vector<DiffusionLeak::OverburdenProp> overburdenProps;

  // Include the following overburder formations, with the following thickness, topPorosity,
  // basePorosity, topTemperature and baseTemperature:
  overburdenProps.push_back(DiffusionLeak::OverburdenProp(100.0, 1e-6, 2e-6, 400.0, 380.0));
  overburdenProps.push_back(DiffusionLeak::OverburdenProp(100.0, 1e-6, 8e-6, 380.0, 350.0));
  overburdenProps.push_back(DiffusionLeak::OverburdenProp(100.0, 1e-7, 2e-7, 350.0, 300.0));

  vector<double> diffusionConsts;
  diffusionConsts.push_back(3.0e-06);
  diffusionConsts.push_back(2.5e-06);
  diffusionConsts.push_back(2.25e-06);
  diffusionConsts.push_back(2.0e-06);
  diffusionConsts.push_back(1.75e-06);

  double activationTemperature = 3.5e-07;

  DiffusionCoefficient coefficient(2.0e-06, activationTemperature);

  double sealFluidDensity = 1000.0;
  double penetrationDistance = 0.0;
  double maxPenetrationDistance = 250.0;

  DiffusionLeak leak(overburdenProps, sealFluidDensity, penetrationDistance, maxPenetrationDistance,
     coefficient, 50, 0.05);

  double timeInterval = 175;
  double componentWeight = 100.0;
  double molarFraction = 0.001;
  double solubility = 0.001;
  double surfaceArea = 1.0;
  double lost = 0.0;

  for (int i = 0; i < 100; ++i) {
     leak.compute(timeInterval, componentWeight, molarFraction, solubility, surfaceArea, lost);
     cout << "time step " << i << " penetration distance " << leak.penetrationDistance() << " lost " << lost << "." << endl;
  }

  return 0;
}
