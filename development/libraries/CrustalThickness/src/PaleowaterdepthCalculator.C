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

// utilities
#include "PhysicalConstants.h"

//------------------------------------------------------------//
PaleowaterdepthCalculator::PaleowaterdepthCalculator(
   const InterfaceInput& inputData,
   AbstractInterfaceOutput& outputData,
   const AbstractValidator&       validator,
   const Interface::GridMap*      presentDayTTS,
   const Interface::GridMap*      presentDayPressureTTS,
   const Interface::GridMap*      currentPressureTTS ) : 
      m_firstI                  ( inputData.firstI() ),
      m_firstJ                  ( inputData.firstJ() ),
      m_lastI                   ( inputData.lastI()  ),
      m_lastJ                   ( inputData.lastJ()  ),
      m_mantleDensity           ( inputData.getBackstrippingMantleDensity() ),
      m_waterDensity            ( inputData.getWaterDensity()               ),
      m_presentDayPressureMantle( inputData.getPressureMantleAtPresentDay() ),
      m_currentPressureMantle   ( inputData.getPressureMantle()             ),
      m_presentDayTTS           ( presentDayTTS         ),
      m_presentDayPressureTTS   ( presentDayPressureTTS ),
      m_currentPressureTTS      ( currentPressureTTS    ),
      m_outputData              ( outputData ),
      m_validator               ( validator  )
{
   if (m_mantleDensity == m_waterDensity){
      throw PWDException() << "The water density is equal to the mantle density in the Paleowaterdepth calculator but they should be different";
   }
   else if (m_presentDayTTS == nullptr) {
      throw PWDException() << "The present day total tectonic subsidence is a null pointer and is required by the Paleowaterdepth calculator";
   }
   else if (m_presentDayPressureMantle == nullptr){
      throw PWDException() << "The present day bottom mantle pressure is a null pointer and is required by the Paleowaterdepth calculator";
   }
   else if (m_currentPressureMantle == nullptr){
      throw PWDException() << "The current bottom mantle pressure is a null pointer and is required by the Paleowaterdepth calculator";
   }
}

//------------------------------------------------------------//
PaleowaterdepthCalculator::~PaleowaterdepthCalculator(){}

//------------------------------------------------------------//
void PaleowaterdepthCalculator::compute(){

   unsigned int i, j;
   double PWD;
   retrieveData();

   for ( i = m_firstI; i <= m_lastI; ++i ) {
      for ( j = m_firstJ; j <= m_lastJ; ++j ) {
         const double TTS = m_presentDayTTS->getValue( i, j );
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

   restoreData();
}

//------------------------------------------------------------//
double PaleowaterdepthCalculator::calculatePWD( const double presentDayTTS,
                                                const double backstrip,
                                                const double presentDayPressureBotMantle,
                                                const double presentDayPressureTTS,
                                                const double currentPressureBotMantle,
                                                const double currentPressureTTS ) const {
   double PWD = presentDayTTS - backstrip;
   assert( m_mantleDensity != m_waterDensity );
   //Pressure data available to equilibrate TTS and Mantle pressure
   PWD -= ((presentDayPressureBotMantle - presentDayPressureTTS) - (currentPressureBotMantle - currentPressureTTS)) /
      (PhysicalConstants::AccelerationDueToGravity*(m_mantleDensity - m_waterDensity));
   return PWD;
}

//------------------------------------------------------------//
double PaleowaterdepthCalculator::calculatePWD( const double presentDayTTS,
                                                const double backstrip ) const {
   return presentDayTTS - backstrip;
}

//------------------------------------------------------------//
void PaleowaterdepthCalculator::retrieveData() {
   if (m_presentDayPressureTTS){
      m_presentDayPressureTTS->retrieveData();
   }
   if (m_currentPressureTTS){
      m_currentPressureTTS->retrieveData();
   }
   m_presentDayTTS           ->retrieveData();
   m_presentDayPressureMantle->retrieveData();
   m_currentPressureMantle   ->retrieveData();

}

//------------------------------------------------------------//
void PaleowaterdepthCalculator::restoreData() {
   if (m_presentDayPressureTTS){
      m_presentDayPressureTTS->restoreData();
   }
   if (m_currentPressureTTS){
      m_currentPressureTTS->restoreData();
   }
   m_presentDayTTS           ->restoreData();
   m_presentDayPressureMantle->restoreData();
   m_currentPressureMantle   ->restoreData();

}