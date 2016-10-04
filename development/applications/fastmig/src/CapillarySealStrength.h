//
// Copyright (C) 2010-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//


#ifndef _MIGRATION_CAPILLARYSEALSTRENGTH_H_
#define _MIGRATION_CAPILLARYSEALSTRENGTH_H_

#include "Composition.h"
#include "translateProps.h"

namespace migration {

   class CapillarySealStrength
   {
   private:

      const vector< vector<translateProps::CreateCapillaryLithoProp::output> > m_lithProps;
      const vector< vector<double> > m_lithFracs;
      const vector<CBMGenerics::capillarySealStrength::MixModel> m_mixModel;
      const vector<double> m_permeability;
      const double m_sealFluidDensity;
      const double m_lambdaPC;
      bool m_isLegacy;

   public:

      CapillarySealStrength( const vector< vector<translateProps::CreateCapillaryLithoProp::output> >& lithProps,
                             const vector< vector<double> >& lithFracs, 
                             const vector<CBMGenerics::capillarySealStrength::MixModel> mixModel,
                             const vector<double>& permeability, 
                             const double& sealFluidDensity, 
                             const double& lambdaPC, 
                             const bool isLegacy );
      
      double compute(const Composition& composition, const double& gorm, const double& T_K) const;
      double criticalTemperature(const Composition& composition, const double& gorm) const;
      double interfacialTension(const Composition& composition, const double& gorm, const double& T_K) const;
   };

} // namespace migration

#endif
