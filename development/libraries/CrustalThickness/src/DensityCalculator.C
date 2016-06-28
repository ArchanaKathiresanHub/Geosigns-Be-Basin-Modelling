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

//------------------------------------------------------------//
DensityCalculator::DensityCalculator( const unsigned int firstI,
   const unsigned int firstJ,
   const unsigned int lastI,
   const unsigned int lastJ,
   const double theMantleDensity,
   const double theWaterDensity,
   DerivedProperties::SurfacePropertyPtr pressureBasement,
   DerivedProperties::SurfacePropertyPtr pressureWaterBottom,
   DerivedProperties::SurfacePropertyPtr depthBasement,
   DerivedProperties::SurfacePropertyPtr depthWaterBottom,
   AbstractInterfaceOutput& outputData,
   AbstractValidator&       validator ) : m_firstI( firstI ),
                                          m_firstJ( firstJ ),
                                          m_lastI( lastI ),
                                          m_lastJ( lastJ ),
                                          m_mantleDensity( theMantleDensity ),
                                          m_waterDensity( theWaterDensity ),
                                          m_pressureBasement( pressureBasement ),
                                          m_pressureWaterBottom( pressureWaterBottom ),
                                          m_depthBasement( depthBasement ),
                                          m_depthWaterBottom( depthWaterBottom ),
                                          m_outputData( outputData ),
                                          m_validator( validator ) {

   m_sedimentThickness = 0.0;
   m_sedimentDensity   = 0.0;
   m_backstrip         = 0.0;
   m_compensation      = 0.0;
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
         double topBasementDepthValue = m_depthBasement->get( i, j );
         double waterBottomDepthValue = m_depthWaterBottom->get( i, j );
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
      m_sedimentDensity = ((pressureTopBasementValue - pressureWaterBottomValue) * 1e6) / (CrustalThicknessInterface::GRAVITY * m_sedimentThickness);
      m_backstrip = m_sedimentThickness *  m_mantleDensity * m_densityTerm -
         (((pressureTopBasementValue - pressureWaterBottomValue) * 1e6) / CrustalThicknessInterface::GRAVITY) * m_densityTerm;
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

   if ((m_waterDensity - m_mantleDensity) != 0.0) {
      m_densityTerm = 1.0 / (m_mantleDensity - m_waterDensity);
      m_airCorrection = m_mantleDensity * m_densityTerm;
   }
   else{
      throw DensityException() << "BackstrippingMantleDensity = WaterDensity. Check the constants in the configuration file.";
   }
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "Densities are set to:";
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #mantle=" << m_mantleDensity;
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #water="  << m_waterDensity;
}