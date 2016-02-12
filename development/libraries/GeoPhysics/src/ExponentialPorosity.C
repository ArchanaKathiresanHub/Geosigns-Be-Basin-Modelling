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
   ExponentialPorosity::ExponentialPorosity(double depoPorosity,
                                            double minimumMechanicalPorosity,
                                            double compactionIncr,
                                            double compactionDecr) :
	  Algorithm(depoPorosity,minimumMechanicalPorosity),
      m_compactionIncr(compactionIncr),
      m_compactionDecr(compactionDecr)
   {}

   ///Exponential porosity function
   double ExponentialPorosity::calculate( const double ves, const double maxVes,
                                          const bool includeChemicalCompaction,
                                          const double chemicalCompactionTerm ) const {

      double calculatedPorosity;

      bool   loadingPhase = (ves >= maxVes);

      if (includeChemicalCompaction) {

         if (loadingPhase) {
            calculatedPorosity = (m_depoPorosity - m_minimumMechanicalPorosity) * exp(-m_compactionIncr * maxVes) + m_minimumMechanicalPorosity;
         }
         else {
            calculatedPorosity = (m_depoPorosity - m_minimumMechanicalPorosity) * exp(m_compactionDecr * (maxVes - ves) - m_compactionIncr * maxVes) + m_minimumMechanicalPorosity;
         }

      }
      else {

         if (loadingPhase) {
            calculatedPorosity = m_depoPorosity * exp(-m_compactionIncr * maxVes);
         }
         else {
            calculatedPorosity = m_depoPorosity * exp(m_compactionDecr * (maxVes - ves) - m_compactionIncr * maxVes);
         }
      }

      if (includeChemicalCompaction) {
         calculatedPorosity = calculatedPorosity + chemicalCompactionTerm;
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
                                                 const bool overpressuredCompaction) const {

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
   double ExponentialPorosity::calculateDerivative(const double ves, const double maxVes,
                                                  const bool includeChemicalCompaction,
                                                  const double chemicalCompactionTerm) const {

      //
      //
      //   d Phi   d Phi   d ves       d Phi
      //   ----- = ----- x -----  =  - -----  = cc * Phi
      //     dp    d ves     dp        d ves
      //
      //
      //
      double porosityDerivative;
      double porosityValue;

      porosityValue = calculate( ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm );

      if (ves >= maxVes) {
         porosityDerivative = m_compactionIncr * porosityValue;
      }
      else {
         porosityDerivative = m_compactionDecr * porosityValue;
      }

      return porosityDerivative;
   }


}
