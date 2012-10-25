#ifndef _CBMGENERICS_FRACTUREPRESSURE_H_
#define _CBMGENERICS_FRACTUREPRESSURE_H_

#include "DllExport.h"

#include <vector>

using std::vector;

namespace CBMGenerics {

namespace fracturePressure {

enum CBMGENERICS_DLL_EXPORT FracturePressureFunctionType {
   None,
   FunctionOfDepthWrtSeaLevelSurface,
   FunctionOfDepthWrtSedimentSurface,
   FunctionOfLithostaticPressure
};

double hydraulicFracturingFrac(const vector<double>& lithHydraulicFracturingFracs, 
  const vector<double>& fracs);

double compute(FracturePressureFunctionType type, const vector<double>& fracturePressureFunctionParameters, 
   const vector<double>& lithHydraulicFracturingFracs, const vector<double>& lithFracs, 
   const double& depthWrtSeaLevel, const double& depthWrtSedimentSurface, 
   const double& lithostaticPressure, const double& hydrostaticPressure, 
   const double& hydrostaticPressureAtSedimentSurface);

double computeForFunctionOfDepthWrtSeaLevelSurface(const vector<double>& coefficients, 
   const double& depthWrtSeaLevel);

double computeForFunctionOfDepthWrtSedimentSurface(const vector<double>& coefficients, 
   const double& depthWrtSedimentSurface, const double& hydrostaticPressureAtSedimentSurface);

double computeForFunctionOfLithostaticPressure(const double& hydraulicFracture, 
   const double& lithostaticPressure, const double& hydrostaticPressure);

} // namespace fracturePressure
 
} //namespace CBMGenerics

#endif
