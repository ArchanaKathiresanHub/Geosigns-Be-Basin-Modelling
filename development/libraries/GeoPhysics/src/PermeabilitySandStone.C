//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "PermeabilitySandStone.h"

#include "GeoPhysicalConstants.h"

#include <cmath>

namespace GeoPhysics
{

PermeabilitySandStone::PermeabilitySandStone( double depoPorosity, double depoPermeability, double permeabilityIncr)
   : m_depoPorosity( depoPorosity)
   , m_depoPermeability( depoPermeability)
   , m_permeabilityIncr(permeabilityIncr)
{}

double PermeabilitySandStone::calculate( const double ves, const double maxVes, const double calculatedPorosity) const
{
   double deltaphi = calculatedPorosity - m_depoPorosity;
   double m = 0.12 + 0.02 * m_permeabilityIncr;
   double val = m_depoPermeability * exp(Log10 * m * deltaphi * 100.0);

   if (val >= MaxPermeability) val = MaxPermeability;

   return val;
}

void PermeabilitySandStone::calculateDerivative( const double ves, const double maxVes,
                                                 const double calculatedPorosity,
		                                           const double porosityDerivativeWrtVes,
                                                 double & permeability, double & derivative ) const
{
   permeability = this->calculate( ves, maxVes, calculatedPorosity);

   double perm = permeability;
   double m = 0.12 + 0.02 * m_permeabilityIncr;

   if (perm >= MaxPermeability)
   {
      double deltaphi = calculatedPorosity - m_depoPorosity;

      perm =  m_depoPermeability * exp( Log10 * m * deltaphi * 100.0);
   }
  
   // Use chainrule and multiply with derivative of porosity with respect to stress
   derivative = Log10 * m * perm * porosityDerivativeWrtVes;
}

double PermeabilitySandStone::depoPerm() const
{
   return m_depoPermeability;
}

Permeability::Model PermeabilitySandStone::model() const
{
   return DataAccess::Interface::SANDSTONE_PERMEABILITY;
}

}
