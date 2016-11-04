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
   const AbstractValidator& validator,
   const Interface::GridMap*presentDayTTS ) : 
      m_firstI                    ( inputData.firstI() ),
      m_firstJ                    ( inputData.firstJ() ),
      m_lastI                     ( inputData.lastI()  ),
      m_lastJ                     ( inputData.lastJ()  ),
      m_mantleDensity             ( inputData.getBackstrippingMantleDensity()   ),
      m_waterDensity              ( inputData.getWaterDensity()                 ),
      m_presentDayPressureMantle  ( inputData.getPressureMantleAtPresentDay()   ),
      m_currentPressureMantle     ( inputData.getPressureMantle()               ),
      m_presentDayPressureBasement( inputData.getPressureBasementAtPresentDay() ),
      m_currentPressureBasement   ( inputData.getPressureBasement()             ),
      m_presentDayTTS             ( presentDayTTS ),
      m_outputData                ( outputData ),
      m_validator                 ( validator  )
{
   if (m_mantleDensity == m_waterDensity){
      throw PWDException() << "The water density is equal to the mantle density in the Paleowaterdepth calculator but they should be different";
   }
   else if (m_presentDayTTS == nullptr) {
      throw PWDException() << "The present day total tectonic subsidence is a null pointer and is required by the Paleowaterdepth calculator";
   }
   else if (m_presentDayPressureMantle == nullptr){
      LogHandler( LogHandler::INFO_SEVERITY ) << "      the Paleowaterdepth will not be thermally corrected as there is no present day bottom mantle pressure available";
   }
   else if (m_currentPressureMantle == nullptr){
      LogHandler( LogHandler::INFO_SEVERITY ) << "      the Paleowaterdepth will not be thermally corrected as there is no transient bottom mantle pressure available";
   }
   else if (m_presentDayPressureBasement == nullptr){
      LogHandler( LogHandler::INFO_SEVERITY ) << "      the Paleowaterdepth will not be thermally corrected as there is no present day basement pressure available";
   }
   else if (m_currentPressureBasement == nullptr){
      LogHandler( LogHandler::INFO_SEVERITY ) << "      the Paleowaterdepth will not be thermally corrected as there is no transient basement pressure available";
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
            if (m_presentDayPressureMantle and m_presentDayPressureBasement and m_currentPressureMantle and m_currentPressureBasement){
               //Pressure data available to equilibrate TTS and Mantle pressure
               const double presentDayPressureMantle   = m_presentDayPressureMantle  ->get( i, j );
               const double presentDayPressureBasement = m_presentDayPressureBasement->get( i, j );
               const double currentPressureMantle      = m_currentPressureMantle     ->get( i, j );
               const double currentPressureBasement    = m_currentPressureBasement   ->get( i, j );
               if (presentDayPressureMantle   != m_presentDayPressureMantle  ->getUndefinedValue() and
                   presentDayPressureBasement != m_presentDayPressureBasement->getUndefinedValue() and
                   currentPressureMantle      != m_currentPressureMantle     ->getUndefinedValue() and
                   currentPressureBasement    != m_currentPressureBasement   ->getUndefinedValue() )
               {
                  PWD = calculatePWD( TTS,
                                      backstrip,
                                      presentDayPressureMantle,
                                      presentDayPressureBasement,
                                      currentPressureMantle,
                                      currentPressureBasement );
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
                                                const double presentDayPressureBasement,
                                                const double currentPressureBotMantle,
                                                const double currentPressureBasement ) const {
   double PWD = presentDayTTS - backstrip;
   assert( m_mantleDensity != m_waterDensity );
   //Pressure data available to equilibrate Basement and Bottom Mantle pressure
   PWD -= ((presentDayPressureBotMantle - presentDayPressureBasement) - (currentPressureBotMantle - currentPressureBasement)) /
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
   if (m_presentDayPressureBasement!=nullptr){
      m_presentDayPressureBasement->retrieveData();
   }
   if (m_currentPressureBasement != nullptr){
      m_currentPressureBasement->retrieveData();
   }
   if (m_presentDayPressureMantle != nullptr){
      m_presentDayPressureMantle->retrieveData();
   }
   if (m_currentPressureMantle != nullptr){
      m_currentPressureMantle->retrieveData();
   }
   m_presentDayTTS->retrieveData();
}

//------------------------------------------------------------//
void PaleowaterdepthCalculator::restoreData() {
   if (m_presentDayPressureBasement != nullptr){
      m_presentDayPressureBasement->restoreData();
   }
   if (m_currentPressureBasement != nullptr){
      m_currentPressureBasement->restoreData();
   }
   if (m_presentDayPressureMantle != nullptr){
      m_presentDayPressureMantle->restoreData();
   }
   if (m_currentPressureMantle != nullptr){
      m_currentPressureMantle->restoreData();
   }
   m_presentDayTTS->restoreData();
}