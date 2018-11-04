//                                                                      
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "Porosity.h"

#include <cmath>

#include "ExponentialPorosity.h"
#include "SoilMechanicsPorosity.h"
#include "DoubleExponentialPorosity.h"
#include "GeoPhysicalConstants.h"
#include "Interface/Interface.h"
#include "NumericFunctions.h"

// utilities library
#include "ConstantsPhysics.h"
using Utilities::Physics::AccelerationDueToGravity;

const double GeoPhysics::Porosity::SolidThicknessIterationTolerance = 0.00001;


namespace GeoPhysics
{
   Porosity
      ::Porosity(Algorithm * algorithm)
      :m_algorithm(algorithm)
   {}

   Porosity
      ::Porosity():m_algorithm()
   {}

   // Factory method: Intended to be used from SimpleLithology only

   Porosity
      Porosity::create(Model porosityModel,
      double depoPorosity,
      double minimumMechanicalPorosity,
      double compactionIncr,
      double compactionIncrA,
      double compactionIncrB,
      double compactionDecr,
      double compactionDecrA,
      double compactionDecrB,
      double compactionRatio,
      double soilMechanicsCompactionCoefficient,
      bool   isLegacy)
   {

     switch (porosityModel)
      {
      case DataAccess::Interface::EXPONENTIAL_POROSITY:
         return Porosity(new ExponentialPorosity(depoPorosity, minimumMechanicalPorosity, compactionIncr, compactionDecr, isLegacy));
      case DataAccess::Interface::SOIL_MECHANICS_POROSITY:
         return Porosity(new soilMechanicsPorosity(depoPorosity, minimumMechanicalPorosity, soilMechanicsCompactionCoefficient, depoPorosity/(1-depoPorosity)));
      case DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY:
         return Porosity(new DoubleExponentialPorosity(depoPorosity, minimumMechanicalPorosity, compactionIncrA, compactionIncrB, compactionDecrA, compactionDecrB, compactionRatio, isLegacy));
      default:
         assert(false);
      }
     
      return Porosity(0);
   }

   Porosity& Porosity::operator= (const Porosity& porosity){
      if (this != &porosity) {
         m_algorithm = porosity.m_algorithm;
      }
      return *this;
   }

   Porosity::Porosity( const Porosity& porosity )
      :m_algorithm( porosity.m_algorithm )
   {}
   
   Porosity::Algorithm::Algorithm(double depoPorosity, double minimumMechanicalPorosity):
      m_depoPorosity((depoPorosity < MinimumPorosityNonLegacy) ? 0.0 : depoPorosity),
      m_minimumMechanicalPorosity(NumericFunctions::Minimum(
         NumericFunctions::Maximum(minimumMechanicalPorosity,MinimumPorosityNonLegacy),m_depoPorosity))
   {
   }

   double Porosity::Algorithm::fullCompThickness(const double maxVesValue, const double thickness, const double densitydiff, const double vesScaleFactor, const bool overpressuredCompaction) const {

      double solidThickness;

      if ( isIncompressible ()) {
         solidThickness = thickness * (1.0 - surfacePorosity ());
      } else if (thickness == 0.0) {
         solidThickness = 0.0;
      } else {

         const bool includeChemicalCompaction = false;

         // If we are initialising the model for an Overpressure run
         // then we assume some overpressure. An amount that equates to VES = 0.5 * ( Pl - Ph )
         const double vesScaling = (overpressuredCompaction ? vesScaleFactor : 1.0);

         double vesTop = maxVesValue;
         double porosityTop = calculate(vesTop, vesTop, includeChemicalCompaction, 0.0);
         double vesBottom;
         double porosityBottom;
         double computedSolidThickness;
         double computedRealThickness;
         int iteration = 1;

         computedSolidThickness = thickness * (1.0 - porosityTop);

         do {
            vesBottom = maxVesValue + vesScaling * AccelerationDueToGravity * densitydiff * computedSolidThickness;
            porosityBottom = calculate( vesBottom, vesBottom, includeChemicalCompaction, 0.0 );
            computedRealThickness = 0.5 * computedSolidThickness * (1.0 / (1.0 - porosityTop) + 1.0 / (1.0 - porosityBottom));
            computedSolidThickness = computedSolidThickness * (thickness / computedRealThickness);
         } while ( std::abs ( thickness - computedRealThickness ) >= thickness * Porosity::SolidThicknessIterationTolerance && iteration++ <= 10);

         solidThickness = computedSolidThickness;
      }

      return solidThickness;
   }
}
