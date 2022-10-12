//
// Copyright (C) 2010-2018 Shell International Exploration & Production.
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
#include <vector>

namespace migration {

   class MigrationCapillarySealStrength
   {
   private:

      const std::vector< std::vector<translateProps::CreateCapillaryLithoProp::output> > m_lithProps;
      const std::vector< std::vector<double> > m_lithFracs;
      const std::vector<CBMGenerics::capillarySealStrength::MixModel> m_mixModel;
      const std::vector<double> m_permeability;
      const double m_sealFluidDensity;
      const double m_lambdaPC;

   public:

      MigrationCapillarySealStrength( const std::vector< std::vector<translateProps::CreateCapillaryLithoProp::output> >& lithProps,
                             const std::vector< std::vector<double> >& lithFracs, 
                             const std::vector<CBMGenerics::capillarySealStrength::MixModel>& mixModel,
                             const std::vector<double>& permeability, 
                             const double sealFluidDensity, 
                             const double lambdaPC );
      
      void compute(const std::vector<Composition> & composition, const double gorm, const double T_K,
                   const double brinePressure, double & capSealStrength_H2O_Gas, double & capSealStrength_H2O_Oil, const bool performAdvancedMigration) const;
      double criticalTemperature(const Composition& composition, const double gorm) const;
      double interfacialTension(const Composition& composition, const double gorm, const double T_K) const;
   };

} // namespace migration

#endif
