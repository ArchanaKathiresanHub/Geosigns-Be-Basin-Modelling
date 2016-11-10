//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "DensityCalculator.h"

// std library
#include <math.h>

// utilitites
#include "LogHandler.h"
#include "ConstantsPhysics.h"
using Utilities::Physics::AccelerationDueToGravity;
#include "ConstantsMathematics.h"
using Utilities::Maths::MegaPaToPa;

//------------------------------------------------------------//
DensityCalculator::DensityCalculator( 
   const InterfaceInput&    inputData,
   AbstractInterfaceOutput& outputData,
   const AbstractValidator& validator ) : 
      m_firstI             ( inputData.firstI() ),
      m_firstJ             ( inputData.firstJ() ),
      m_lastI              ( inputData.lastI()  ),
      m_lastJ              ( inputData.lastJ()  ),
      m_mantleDensity      ( inputData.getBackstrippingMantleDensity() ),
      m_waterDensity       ( inputData.getWaterDensity()               ),
      m_pressureBasement   ( inputData.getPressureBasement()           ),
      m_pressureWaterBottom( inputData.getPressureWaterBottom()        ),
      m_depthBasement      ( inputData.getDepthBasement()              ),
      m_depthWaterBottom   ( inputData.getDepthWaterBottom()           ),
      m_sedimentDensity  (0.0),
      m_sedimentThickness(0.0),
      m_backstrip        (0.0),
      m_compensation     (0.0),
      m_outputData         ( outputData ),
      m_validator          ( validator  )
{
   // Check the inputs
   // In normal behavior these exceptions will be handled before the creation of the McKenzieCalculator by the inputData object itself
   if (m_depthBasement == nullptr){
      throw std::invalid_argument( "Basement depth provided by the interface input is a null pointer" );
   }
   if (m_depthWaterBottom == nullptr){
      throw std::invalid_argument( "Water bottom depth provided by the interface input is a null pointer" );
   }
   if (m_pressureBasement == nullptr){
      throw std::invalid_argument( "Basement pressure provided by the interface input is a null pointer" );
   }
   if (m_pressureWaterBottom == nullptr){
      throw std::invalid_argument( "Water bottom pressure provided by the interface input is a null pointer" );
   }
   if (m_waterDensity == m_mantleDensity){
      throw std::invalid_argument( "Mantle density is equal to the water density (both provided by the interface input), this will lead to divisions by 0" );
   }
   // Set densities
   setDensities();
}

//------------------------------------------------------------//
void DensityCalculator::retrieveData() {

   m_depthBasement      ->retrieveData ();
   m_depthWaterBottom   ->retrieveData ();
   m_pressureBasement   ->retrieveData ();
   m_pressureWaterBottom->retrieveData ();
}

//------------------------------------------------------------//
void DensityCalculator::restoreData() {

   m_depthBasement      ->restoreData ();
   m_depthWaterBottom   ->restoreData ();
   m_pressureBasement   ->restoreData ();
   m_pressureWaterBottom->restoreData ();
}

//------------------------------------------------------------//
void DensityCalculator::compute() {

   unsigned int i, j;
   retrieveData();

   for (i = m_firstI; i <= m_lastI; ++i) {
      for (j = m_firstJ; j <= m_lastJ; ++j) {
         double topBasementDepthValue    = m_depthBasement->get( i, j );
         double waterBottomDepthValue    = m_depthWaterBottom->get( i, j );
         double pressureTopBasementValue = m_pressureBasement->get( i, j );
         double pressureWaterBottomValue = m_pressureWaterBottom->get( i, j );
         if (m_validator.isValid( i, j ) and
             topBasementDepthValue    != m_depthBasement->getUndefinedValue()    and
             waterBottomDepthValue    != m_depthWaterBottom->getUndefinedValue() and
             pressureTopBasementValue != m_pressureBasement->getUndefinedValue() and
             pressureWaterBottomValue != m_pressureWaterBottom->getUndefinedValue() )
         {
            calculate( topBasementDepthValue,
                       waterBottomDepthValue,
                       pressureTopBasementValue,
                       pressureWaterBottomValue );
         }
         else {
            m_backstrip         = Interface::DefaultUndefinedMapValue;
            m_sedimentThickness = Interface::DefaultUndefinedMapValue;
            m_compensation      = Interface::DefaultUndefinedMapValue;
            m_sedimentDensity   = Interface::DefaultUndefinedMapValue;
         }
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::cumSedimentBackstrip,    i, j, m_backstrip         );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::cumSedimentThickness,    i, j, m_sedimentThickness );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::cumBasementCompensation, i, j, m_compensation      );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::sedimentDensityMap,      i, j, m_sedimentDensity   );
      }
   }

   restoreData();
}

//------------------------------------------------------------//
void DensityCalculator::calculate( const double topBasementDepthValue,
                                   const double waterBottomDepthValue,
                                   const double pressureTopBasementValue,
                                   const double pressureWaterBottomValue ) {

  
   m_sedimentThickness = topBasementDepthValue - waterBottomDepthValue;

   // Integrated sediment density calculated across grid using pressure at WaterBottom and TopBasement surfaces
   if( m_sedimentThickness > 0.0 ) {
      m_sedimentDensity = ((pressureTopBasementValue - pressureWaterBottomValue) * MegaPaToPa) / (AccelerationDueToGravity * m_sedimentThickness);
      m_backstrip = m_sedimentThickness *  m_mantleDensity * m_densityTerm -
         (((pressureTopBasementValue - pressureWaterBottomValue) * MegaPaToPa) / AccelerationDueToGravity) * m_densityTerm;
      m_compensation = m_sedimentThickness - m_backstrip;
   }
   else if (m_sedimentThickness == 0.0) {
      m_sedimentDensity = Interface::DefaultUndefinedMapValue;
      m_backstrip       = 0;
      m_compensation    = 0;
   }
   else {
      m_sedimentDensity = Interface::DefaultUndefinedMapValue;
      m_backstrip       = Interface::DefaultUndefinedMapValue;
      m_compensation    = Interface::DefaultUndefinedMapValue;
   }

}

//------------------------------------------------------------//
void DensityCalculator::setDensities() {

   m_densityTerm = 1.0 / (m_mantleDensity - m_waterDensity);
   m_airCorrection = m_mantleDensity * m_densityTerm;
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "Densities are set to:";
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #mantle=" << m_mantleDensity;
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #water="  << m_waterDensity;
}
