//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PaleowaterdepthCalculator.h"

PaleowaterdepthCalculator::PaleowaterdepthCalculator( const unsigned int firstI,
                                                      const unsigned int firstJ,
                                                      const unsigned int lastI,
                                                      const unsigned int lastJ,
                                                      const double theMantleDensity,
                                                      const double theWaterDensity,
                                                      AbstractInterfaceOutput& outputData,
                                                      AbstractValidator&       validator,
                                                      DerivedProperties::SurfacePropertyPtr presentDayPressureMantle,
                                                      DerivedProperties::SurfacePropertyPtr currentPressureMantle,
                                                      Interface::GridMap* presentDayPressureTTS,
                                                      Interface::GridMap* currentPressureTTS ) : m_firstI( firstI ),
                                                                                                 m_firstJ( firstJ ),
                                                                                                 m_lastI ( lastI  ),
                                                                                                 m_lastJ ( lastJ  ),
                                                                                                 m_mantleDensity( theMantleDensity ),
                                                                                                 m_waterDensity ( theWaterDensity ),
                                                                                                 m_outputData ( outputData ),
                                                                                                 m_validator  ( validator ),
                                                                                                 m_presentDayPressureMantle ( presentDayPressureMantle ),
                                                                                                 m_currentPressureMantle    ( currentPressureMantle ),
                                                                                                 m_presentDayPressureTTS    ( presentDayPressureTTS ),
                                                                                                 m_currentPressureTTS       ( currentPressureTTS )
{
   if (theMantleDensity == theWaterDensity){
      throw PWDException() << "The water density is equal to the mantle density in the Paleowaterdepth calculator but they should be different.";
   }
   if (m_presentDayPressureTTS){
      m_presentDayPressureTTS->retrieveData( true );
   }
   if (m_currentPressureTTS){
      m_currentPressureTTS->retrieveData( true );
   }
}

PaleowaterdepthCalculator::~PaleowaterdepthCalculator(){
   if (m_presentDayPressureTTS){
      m_presentDayPressureTTS->restoreData( true, true );
   }
   if (m_currentPressureTTS){
      m_currentPressureTTS->restoreData( true, true );
   }
}

void PaleowaterdepthCalculator::compute(){

   unsigned int i, j;
   double PWD;

   for ( i = m_firstI; i <= m_lastI; ++i ) {
      for ( j = m_firstJ; j <= m_lastJ; ++j ) {
         const double TTS = m_outputData.getMapValue( CrustalThicknessInterface::outputMaps::WLSMap, i, j );
         const double backstrip = m_outputData.getMapValue( CrustalThicknessInterface::outputMaps::cumSedimentBackstrip, i, j );
         if ( m_validator.isValid( i, j ) and
              TTS       != Interface::DefaultUndefinedMapValue and
              backstrip != Interface::DefaultUndefinedMapValue )
         {
            if (m_presentDayPressureMantle and m_presentDayPressureTTS and m_currentPressureMantle and m_currentPressureTTS){
               //Pressure data available to equilibrate TTS and Mantle pressure
               const double presentDayPressureMantle = m_presentDayPressureMantle->get( i, j );
               const double presentDayPressureTTS    = m_presentDayPressureTTS->getValue( i, j );
               const double currentPressureMantle    = m_currentPressureMantle->get( i, j );
               const double currentPressureTTS       = m_currentPressureTTS->getValue( i, j );
               if (presentDayPressureMantle != m_presentDayPressureMantle->getUndefinedValue() and
                   presentDayPressureTTS    != m_presentDayPressureTTS->getUndefinedValue()    and
                   currentPressureMantle    != m_currentPressureMantle->getUndefinedValue()    and
                   currentPressureTTS       != m_currentPressureTTS->getUndefinedValue() )
               {
                  PWD = calculatePWD( TTS,
                                      backstrip,
                                      presentDayPressureMantle,
                                      presentDayPressureTTS,
                                      currentPressureMantle,
                                      currentPressureTTS );
               }
               else{
                  PWD = calculatePWD( TTS, backstrip );
               }
               
            }
            else{
               PWD = calculatePWD( TTS, backstrip );
            }

         }
         else{
            PWD = Interface::DefaultUndefinedMapValue;
         }
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::isostaticBathymetry, i, j, PWD );
      }
   }
}

double PaleowaterdepthCalculator::calculatePWD( const double presentDayTTS,
                                                const double backstrip,
                                                const double presentDayPressureMantle,
                                                const double presentDayPressureTTS,
                                                const double currentPressureMantle,
                                                const double currentPressureTTS ) const {
   double PWD = presentDayTTS + backstrip;
   assert( m_mantleDensity != m_waterDensity );
   //Pressure data available to equilibrate TTS and Mantle pressure
   PWD -= ((presentDayPressureMantle - presentDayPressureTTS) - (currentPressureMantle - currentPressureTTS)) /
          (CrustalThicknessInterface::GRAVITY*(m_mantleDensity - m_waterDensity));
   return PWD;
}

double PaleowaterdepthCalculator::calculatePWD( const double presentDayTTS,
                                                const double backstrip ) const {
   double PWD = presentDayTTS + backstrip;
   return PWD;
}