//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FracturePressureCalculator.h"

#include "Interface.h"

#include "GeoPhysicalConstants.h"
#include "GeoPhysicalFunctions.h"
#include "NumericFunctions.h"
#include "LogHandler.h"

// std library
#include <vector>

// utilities library
#include "ConstantsPhysics.h"
using Utilities::Physics::AtmosphericPressureMpa;

GeoPhysics::FracturePressureCalculator::FracturePressureCalculator (DataAccess::Interface::ProjectHandle& projectHandle ) {

   m_fracturePressureFunctionParameters = projectHandle.getFracturePressureFunctionParameters ();
   m_selectedFunction = DataAccess::Interface::FracturePressureFunctionType::None;

   if(m_fracturePressureFunctionParameters)
      m_selectedFunction = m_fracturePressureFunctionParameters->type ();

   if ( m_selectedFunction == DataAccess::Interface::FracturePressureFunctionType::FunctionOfDepthWrtSeaLevelSurface or
        m_selectedFunction == DataAccess::Interface::FracturePressureFunctionType::FunctionOfDepthWrtSedimentSurface ) {
      m_a = m_fracturePressureFunctionParameters->coefficients ()[ 0 ];
      m_b = m_fracturePressureFunctionParameters->coefficients ()[ 1 ];
      m_c = m_fracturePressureFunctionParameters->coefficients ()[ 2 ];
      m_d = m_fracturePressureFunctionParameters->coefficients ()[ 3 ];
   } else {
      m_a = DataAccess::Interface::DefaultUndefinedScalarValue;
      m_b = DataAccess::Interface::DefaultUndefinedScalarValue;
      m_c = DataAccess::Interface::DefaultUndefinedScalarValue;
      m_d = DataAccess::Interface::DefaultUndefinedScalarValue;
      if ( m_selectedFunction == DataAccess::Interface::FracturePressureFunctionType::None )
          LogHandler(LogHandler::INFO_SEVERITY) << "No fracture type is selected , hence fracture pressure calculations will be avoided";
   }

}

GeoPhysics::FracturePressureCalculator::~FracturePressureCalculator () {
}

const DataAccess::Interface::FracturePressureFunctionParameters* GeoPhysics::FracturePressureCalculator::getFracturePressureFunctionParameters () const {
   return m_fracturePressureFunctionParameters;
}

double GeoPhysics::FracturePressureCalculator::fracturePressure  ( const CompoundLithology* lithology,
                                                                   const double             depth,
                                                                   const double             seaBottomDepth,
                                                                   const double             hydrostaticPressure,
                                                                   const double             lithostaticPressure,
                                                                   const double             pressureAtSeaBottom ) const {

   // The value of the fracture-pressure.
   double calculatedFracturePressure;

   // The depth at which the fracture-pressure function is to be evaluated.
   // Valid only for FunctionOfDepthWrtSeaLevelSurface.
   double sedimentDepth;

   // The constant term of the fracture-pressure polynomial.
   // Valid only for FunctionOfDepthWrtSeaLevelSurface.
   double constantTerm;


   switch ( m_selectedFunction ) {

      case DataAccess::Interface::None :
         calculatedFracturePressure = NoFracturePressureValue;
         break;

      case DataAccess::Interface::FunctionOfDepthWrtSeaLevelSurface :
         calculatedFracturePressure = (( m_d * depth + m_c ) * depth + m_b ) * depth + m_a;
         break;

      case DataAccess::Interface::FunctionOfDepthWrtSedimentSurface :

         if ( depth > 0.0 ) {
            // Subtract the surface-pressure since it has already been included in
            // the hydrostatic pressure and the constant 'a' term.
            constantTerm = m_a + pressureAtSeaBottom - AtmosphericPressureMpa;
         } else {
            constantTerm = m_a;
         }

         sedimentDepth = depth - seaBottomDepth;
         calculatedFracturePressure = (( m_d * sedimentDepth + m_c ) * sedimentDepth + m_b ) * sedimentDepth + constantTerm;
         break;

      case DataAccess::Interface::FunctionOfLithostaticPressure :
         calculatedFracturePressure = lithology->fracturePressure ( hydrostaticPressure, lithostaticPressure );
         break;

      default:

         calculatedFracturePressure = DataAccess::Interface::DefaultUndefinedScalarValue;
  }

  if ( calculatedFracturePressure != DataAccess::Interface::DefaultUndefinedScalarValue and calculatedFracturePressure != NoFracturePressureValue ) {
      // bound calculated fracture pressure value within a resonable range of lithostaticPressure & hydrostaticPressure.
      calculatedFracturePressure = NumericFunctions::clipValueToRange(calculatedFracturePressure, hydrostaticPressure, lithostaticPressure);
  }

  return calculatedFracturePressure;
}

double GeoPhysics::FracturePressureCalculator::fracturePressure ( const CompoundLithology* lithology,
                                                                  const FluidType*  currentFluid,
                                                                  const double      seaTemperature,
                                                                  const double      surfaceDepth,
                                                                  const double      depth,
                                                                  const double      hydrostaticPressure,
                                                                  const double      lithostaticPressure ) const {

   // The value of the fracture-pressure.
   double calculatedFracturePressure;

   // The depth at which the fracture-pressure function is to be evaluated.
   // Valid only for FunctionOfDepthWrtSeaLevelSurface.
   double sedimentDepth;

   // The constant term of the fracture-pressure polynomial.
   // Valid only for FunctionOfDepthWrtSeaLevelSurface.
   double constantTerm;

   // The pressure at the bottom of the sea.
   // Valid only for FunctionOfDepthWrtSeaLevelSurface.
   double sedimentSurfaceHydrostaticPressure;

   switch ( m_selectedFunction ) {

      case DataAccess::Interface::None :
         calculatedFracturePressure = NoFracturePressureValue;
         break;

      case DataAccess::Interface::FunctionOfDepthWrtSeaLevelSurface :
         calculatedFracturePressure = (( m_d * depth + m_c ) * depth + m_b ) * depth + m_a;
         break;

      case DataAccess::Interface::FunctionOfDepthWrtSedimentSurface :

         if ( depth > 0.0 ) {
            computeHydrostaticPressure ( currentFluid,
                                         seaTemperature,
                                         surfaceDepth,
                                         sedimentSurfaceHydrostaticPressure );
            // Subtract the surface-pressure since it has already been included in
            // the hydrostatic pressure and the constant 'a' term.
            constantTerm = m_a + sedimentSurfaceHydrostaticPressure - AtmosphericPressureMpa;
         } else {
            constantTerm = m_a;
         }

         sedimentDepth = depth - surfaceDepth;
         calculatedFracturePressure = (( m_d * sedimentDepth + m_c ) * sedimentDepth + m_b ) * sedimentDepth + constantTerm;
         break;

      case DataAccess::Interface::FunctionOfLithostaticPressure :
         calculatedFracturePressure =  lithology->fracturePressure ( hydrostaticPressure, lithostaticPressure );
         break;

      default:

         calculatedFracturePressure = DataAccess::Interface::DefaultUndefinedScalarValue;

   }

   if ( calculatedFracturePressure != DataAccess::Interface::DefaultUndefinedScalarValue and calculatedFracturePressure != NoFracturePressureValue ) {
       // bound calculated fracture pressure value within a resonable range of lithostaticPressure & hydrostaticPressure.
       calculatedFracturePressure = NumericFunctions::clipValueToRange(calculatedFracturePressure, hydrostaticPressure, lithostaticPressure );
   }

   return calculatedFracturePressure;
}


bool GeoPhysics::FracturePressureCalculator::hasFractured ( const CompoundLithology* lithology,
                                                            const double             depth,
                                                            const double             seaBottomDepth,
                                                            const double             hydrostaticPressure,
                                                            const double             lithostaticPressure,
                                                            const double             pressureAtSeaBottom,
                                                            const double             porePressure ) const {

   double calculatedFracturePressure = fracturePressure ( lithology, depth, seaBottomDepth, hydrostaticPressure, lithostaticPressure, pressureAtSeaBottom );

   if ( calculatedFracturePressure == DataAccess::Interface::DefaultUndefinedScalarValue or calculatedFracturePressure == NoFracturePressureValue ) {
      // Should we check the lithosattic pressure? I.e. porePressure > lithostaticPressure.
      return false;
   } else {
      return porePressure > calculatedFracturePressure;
   }

}

bool GeoPhysics::FracturePressureCalculator::hasFractured ( const CompoundLithology* lithology,
                                                            const FluidType*  currentFluid,
                                                            const double      seaTemperature,
                                                            const double      seaBottomDepth,
                                                            const double      depth,
                                                            const double      hydrostaticPressure,
                                                            const double      lithostaticPressure,
                                                            const double      porePressure ) const {

   double calculatedFracturePressure = fracturePressure ( lithology, currentFluid, seaTemperature, seaBottomDepth, depth, hydrostaticPressure, lithostaticPressure );

   if ( calculatedFracturePressure == DataAccess::Interface::DefaultUndefinedScalarValue or calculatedFracturePressure == NoFracturePressureValue ) {
      // Should we check the lithosattic pressure? I.e. porePressure > lithostaticPressure.
      return false;
   } else {
      return porePressure > calculatedFracturePressure;
   }

}
