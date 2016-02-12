//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "DoubleExponentialPorosity.h"

#include "NumericFunctions.h"
#include "GeoPhysicalConstants.h"
#include <cmath>



namespace GeoPhysics
{
   ///Constructor
   DoubleExponentialPorosity::DoubleExponentialPorosity(double depoPorosity, double minimumMechanicalPorosity, double compactionIncrA, double compactionIncrB, double compactionDecrA, double compactionDecrB)
      : Algorithm(depoPorosity,minimumMechanicalPorosity),
        m_compactionIncrA(compactionIncrA),
        m_compactionIncrB(compactionIncrB),
        m_compactionDecrA(compactionDecrA),
        m_compactionDecrB(compactionDecrB)
   {}

   ///Double Exponential porosity function
   double DoubleExponentialPorosity::calculate(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const
   {

      double calculatedPorosity;
      bool   loadingPhase = (ves >= maxVes);

      if (loadingPhase) {
    	  calculatedPorosity  = (m_depoPorosity - m_minimumMechanicalPorosity)/2 * exp(-m_compactionIncrA * maxVes);
          calculatedPorosity += (m_depoPorosity - m_minimumMechanicalPorosity)/2 * exp(-m_compactionIncrB * maxVes) + m_minimumMechanicalPorosity;
          
      }
      else {
          calculatedPorosity  = (m_depoPorosity - m_minimumMechanicalPorosity)/2 * exp( m_compactionDecrA * (maxVes - ves) - m_compactionIncrA * maxVes);
          calculatedPorosity += (m_depoPorosity - m_minimumMechanicalPorosity)/2 * exp( m_compactionDecrB * (maxVes - ves) - m_compactionIncrB * maxVes) + m_minimumMechanicalPorosity;
      }
         
      if ( includeChemicalCompaction ) {
         calculatedPorosity += chemicalCompactionTerm;
         calculatedPorosity = NumericFunctions::Maximum(calculatedPorosity, MinimumPorosity);
      }
      return calculatedPorosity;
   }

   bool DoubleExponentialPorosity::isIncompressible () const {
      return m_compactionIncrA == 0.0 and m_compactionIncrB == 0.0;
   }

   Porosity::Model DoubleExponentialPorosity::model() const
   {
      return DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY;
   }

   ///CompactionCoefficientA
   double DoubleExponentialPorosity::compactionCoefficientA() const {
      return m_compactionIncrA;
   }

   ///CompactionCoefficientB
   double DoubleExponentialPorosity::compactionCoefficientB() const {
      return m_compactionIncrB;
   }

   ///PorosityDerivative
   double DoubleExponentialPorosity::calculateDerivative(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const {

      //
      //
      //   d Phi   d Phi   d ves       d Phi
      //   ----- = ----- x -----  =  - -----  = phiA * kA * exp(-kA*ves) + phiB * kB * exp(-kB*ves)
      //     dp    d ves     dp        d ves
      //
      //   NB: ves = lithostatic pressure - pressure 
      //
      double porosityDerivative;
      bool loadingPhase = ves >= maxVes; 

      if (loadingPhase) {
         porosityDerivative  = (m_depoPorosity - m_minimumMechanicalPorosity)/2 * m_compactionIncrA * exp(-m_compactionIncrA * maxVes);
         porosityDerivative += (m_depoPorosity - m_minimumMechanicalPorosity)/2 * m_compactionIncrB * exp(-m_compactionIncrB * maxVes);
      }
      else {
         porosityDerivative  = m_compactionDecrA * (m_depoPorosity - m_minimumMechanicalPorosity)/2 * exp( m_compactionDecrA * (maxVes - ves) - m_compactionIncrA * maxVes );
         porosityDerivative += m_compactionDecrB * (m_depoPorosity - m_minimumMechanicalPorosity)/2 * exp( m_compactionDecrB * (maxVes - ves) - m_compactionIncrB * maxVes );
      }

       return porosityDerivative;
   }


} //end of namespace GeoPhysics
