//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _CBMGENERICS_CAPILLARYSEALSTRENGTH_H_
#define _CBMGENERICS_CAPILLARYSEALSTRENGTH_H_

#include <vector>

using std::vector;

namespace CBMGenerics { 

namespace capillarySealStrength {

enum MixModel {
   Homogeneous,
   Layered
};

struct LithoProp {

   LithoProp(const double& permeability_, const double& capC1_, const double& capC2_):
      permeability(permeability_), capC1(capC1_), capC2(capC2_)
   {}
   double permeability;
   double capC1;
   double capC2;
};

void capParameters(const vector<LithoProp>& liths, const vector<double>& fracs, 
   MixModel mixModel, double& capC1, double& capC2);

double capSealStrength_Air_Hg(const double& capC1, const double& capC2, const double& permeability);

// Does this function return values in units mN/M?
double capTension_H2O_HC ( const double& density_H2O,
                           const double& density_HC,
                           const double& T_K,
                           const double& T_c_HC_K );

double capTension_H2O_HC_FR(const double& density_H2O, const double& density_HC, const double& T_K,
   const double& T_c_HC_K);

double capSealStrength_H2O_HC(const double& capSealStrength_Air_Hg, const double& capTension_H2O_HC);

double capSealStrength_H2O_HC(const double& capC1, const double& capC2, const double& permeability, 
                              const double& density_H2O, const double& density_HC, 
                              const double& T_K, const double& T_c_HC_K);

double capSealStrength_H2O_HC(const vector<LithoProp>& lithProps, const vector<double>& lithFracs, 
   MixModel mixModel, const double& permeability, const double& density_H2O, 
   const double& density_HC, const double& T_K, const double& T_c_HC_K);


double capPressure( const unsigned int phaseId, const double& density_H2O, 
                    const double& density_HC, const double& T_K, const double& T_c_HC_K,
                    const double& specificSurfArea, const double& geometricVariance,
                    const double& wettingSaturation, const double& porosity,  const double& solidRockDensity );
double capPressure( const unsigned int phaseId, const double& capTens_H20_HC,
                    const double& specificSurfArea, const double& geometricVariance,
                    const double& wettingSaturation, const double& porosity,  const double& solidRockDensity );
double computeCapillaryPressureData(const double& specificSurfArea, const double& solidRockDensity,
                                    const double& geometricVariance, const double& porosity, 
                                    const double& wettingSaturation);

} // namespace capillarySealStrength

} // namespace CBMGenerics

#endif
