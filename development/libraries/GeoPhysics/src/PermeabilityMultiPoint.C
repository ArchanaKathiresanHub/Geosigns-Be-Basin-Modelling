//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PermeabilityMultiPoint.h"
#include "GeoPhysicalConstants.h"

#include <cmath>

namespace GeoPhysics
{

   PermeabilityMultiPoint::PermeabilityMultiPoint(const double depoPorosity,
                                                  const std::vector<double> & porositySamples,
                                                  const std::vector<double> & permeabilitySamples)
      :m_porosityPermeabilityInterpolant(),
       m_depoPermeability( 0 )
   {
      assert( porositySamples.size() == permeabilitySamples.size());
      assert( porositySamples.size() > 0);

      const int n = static_cast<int>(porositySamples.size());
      m_porosityPermeabilityInterpolant.setInterpolation( n, porositySamples.data (), permeabilitySamples.data ());
      m_depoPermeability = m_porosityPermeabilityInterpolant.evaluate ( depoPorosity );
   }

   double PermeabilityMultiPoint::calculate( const double ves, const double maxVes, const double calculatedPorosity) const
   {
      double  val = exp ( Log10 * m_porosityPermeabilityInterpolant.evaluate ( calculatedPorosity ));
      return std::min( val, MaxPermeability );
   }

   void PermeabilityMultiPoint::calculateDerivative( const double ves,
                                                     const double maxVes,
                                                     const double calculatedPorosity,
                                                     const double porosityDerivativeWrtVes,
                                                     double & permeability,
                                                     double & derivative ) const
   {
      permeability = this->calculate( ves, maxVes, calculatedPorosity);
      if( permeability == MaxPermeability )
      {
         derivative = 0.0;
      }
      else
      {
         derivative = permeability * Log10 * porosityDerivativeWrtVes * m_porosityPermeabilityInterpolant.evaluateDerivative ( calculatedPorosity );
      }
   }

   double PermeabilityMultiPoint::depoPerm() const
   {
      return m_depoPermeability;
   }

   Permeability::Model PermeabilityMultiPoint::model() const
   {
      return DataAccess::Interface::MULTIPOINT_PERMEABILITY;
   }

}
