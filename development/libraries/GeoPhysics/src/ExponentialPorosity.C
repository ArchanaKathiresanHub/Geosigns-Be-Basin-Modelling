//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "ExponentialPorosity.h"

#include "GeoPhysicalConstants.h"
#include "NumericFunctions.h"
#include <cmath>


using namespace DataAccess;


namespace GeoPhysics
{

   ///Parameters from constructor
   ExponentialPorosity::ExponentialPorosity(const double depoPorosity,
                                            const double minimumMechanicalPorosity,
                                            const double compactionIncr,
                                            const double compactionDecr) :
	  Algorithm(depoPorosity,minimumMechanicalPorosity),
      m_compactionIncr(compactionIncr),
      m_compactionDecr(compactionDecr)
   {}

   ///Exponential porosity function
   double ExponentialPorosity::calculate( const double ves,
                                          const double maxVes,
                                          const bool includeChemicalCompaction,
                                          const double chemicalCompactionTerm ) const
   {
      double calculatedPorosity;

      const bool loadingPhase = (ves >= maxVes);
		const double minimumMechanicalPorosity = NumericFunctions::Maximum(m_minimumMechanicalPorosity, MinimumPorosity);
      
		if (loadingPhase)
		{
			calculatedPorosity = (m_depoPorosity - minimumMechanicalPorosity) * exp(-m_compactionIncr * ves) + minimumMechanicalPorosity;
		}
		else
		{
			calculatedPorosity = (m_depoPorosity - minimumMechanicalPorosity) * exp(m_compactionDecr * (maxVes - ves) - m_compactionIncr * maxVes) + minimumMechanicalPorosity;
		}

		if (includeChemicalCompaction)
		{
			calculatedPorosity += chemicalCompactionTerm;
			calculatedPorosity = NumericFunctions::Maximum(calculatedPorosity, MinimumPorosity);
		}

      return calculatedPorosity;
   }

   bool ExponentialPorosity::isIncompressible () const {
      return m_compactionIncr == 0.0;
   }


   ///PorosityModel
   Porosity::Model ExponentialPorosity::model() const
   {
      return DataAccess::Interface::EXPONENTIAL_POROSITY;
   }

   ///FullCompThickness
   double ExponentialPorosity::fullCompThickness(const double MaxVesValue,
                                                 const double thickness,
                                                 const double densitydiff,
                                                 const double vesScaleFactor,
                                                 const bool overpressuredCompaction) const
   {
      double c1;
      double c2;
      double Solid_Thickness;

      if (m_compactionIncr == 0) {
         Solid_Thickness = thickness * (1.0 - m_depoPorosity);
      }
      else {
         c1 = AccelerationDueToGravity * densitydiff * m_compactionIncr;

         c2 = (1.0 - exp(-c1 * thickness)) * m_depoPorosity
            * exp(-m_compactionIncr * MaxVesValue);

         Solid_Thickness = log(1.0 - c2) / c1 + thickness;
      }

      return Solid_Thickness;
   }

   ///CompactionCoefficent
   double ExponentialPorosity::compactionCoefficient() const {
      return m_compactionIncr;
   }

   ///PorosityDerivative
   double ExponentialPorosity::calculateDerivative( const double ves,
                                                    const double maxVes,
                                                    const bool includeChemicalCompaction,
                                                    const double chemicalCompactionTerm) const
   {
      //
      //  If there is NO chemical compaction
      //
      //  d Phi
      //  ----- = - cc * (Phi -PhiMin)
      //  d ves
      //
      //  else if Phi is greater than MinimumPorosity
      //
      //  d Phi
      //  ----- = - cc * ( Phi - PhiMin - chemicalCompactionTerm )
      //  d ves
      //
      //  otherwise the derivative is zero.
      //

      double porosityDerivative;
      const double porosityValue = calculate(ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm);
      const double minimumMechanicalPorosity = NumericFunctions::Maximum(m_minimumMechanicalPorosity, MinimumPorosity);
      double localChemicalCompactionTerm;

      // Chemical compaction equations also depends on VES, should we consider adding the chemical compaction derivative to 
      // these equations?
      
      if (porosityValue == MinimumPorosity)
      {
         porosityDerivative = 0.0;
      }
      else if (ves >= maxVes)
      {
         porosityDerivative = -m_compactionIncr * (porosityValue - minimumMechanicalPorosity - (includeChemicalCompaction?chemicalCompactionTerm:0.0) );
      }
      else
      {
         porosityDerivative = -m_compactionDecr * (porosityValue - minimumMechanicalPorosity - (includeChemicalCompaction?chemicalCompactionTerm:0.0));
      }
      
      return porosityDerivative;
   }

}
