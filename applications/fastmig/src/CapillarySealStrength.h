#ifndef _MIGRATION_CAPILLARYSEALSTRENGTH_H_
#define _MIGRATION_CAPILLARYSEALSTRENGTH_H_

#include "Composition.h"
#include "translateProps.h"

namespace migration {

class CapillarySealStrength
{
private:

   vector<translateProps::CreateCapillaryLithoProp::output> m_lithProps;
   vector<double> m_lithFracs;
   CBMGenerics::capillarySealStrength::MixModel m_mixModel;
   double m_permeability;
   double m_sealFluidDensity;

public:

   CapillarySealStrength(const vector<translateProps::CreateCapillaryLithoProp::output>& lithProps,
      const vector<double>& lithFracs, CBMGenerics::capillarySealStrength::MixModel mixModel,
      const double& permeability, const double& sealFluidDensity);

   double compute(const Composition& composition, const double& gorm, const double& T_K) const;
   double criticalTemperature(const Composition& composition, const double& gorm) const;
   double interfacialTension(const Composition& composition, const double& gorm, const double& T_K) const;
};

} // namespace migration

#endif
