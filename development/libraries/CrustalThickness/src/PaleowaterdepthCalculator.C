//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PaleowaterdepthCalculator.h"

// std library
#include <exception>

// CrustalThickness library
#include "AbstractInterfaceOutput.h"
#include "InterfaceInput.h"

// DataModel library
#include "AbstractValidator.h"

//DataAccess library
#include "Interface/GridMap.h"

// utilities library
#include "ConstantsPhysics.h"
using Utilities::Physics::AccelerationDueToGravity;
#include "ConstantsMathematics.h"
using Utilities::Maths::MegaPaToPa;

using namespace CrustalThickness;

//------------------------------------------------------------//
PaleowaterdepthCalculator::PaleowaterdepthCalculator(
   const InterfaceInput& inputData,
   AbstractInterfaceOutput& outputData,
   const DataModel::AbstractValidator& validator,
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
      throw std::invalid_argument( "The water density is equal to the mantle density in the Paleowaterdepth calculator but they should be different" );
   }
   else if (m_presentDayTTS == nullptr) {
      throw std::invalid_argument( "The present day total tectonic subsidence is a null pointer and is required by the Paleowaterdepth calculator" );
   }
   else if (m_presentDayPressureMantle == nullptr){
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS ) << "the Paleowaterdepth will not be thermally corrected as there is no present day bottom mantle pressure available";
   }
   else if (m_currentPressureMantle == nullptr){
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS ) << "the Paleowaterdepth will not be thermally corrected as there is no transient bottom mantle pressure available";
   }
   else if (m_presentDayPressureBasement == nullptr){
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS ) << "the Paleowaterdepth will not be thermally corrected as there is no present day basement pressure available";
   }
   else if (m_currentPressureBasement == nullptr){
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS ) << "the Paleowaterdepth will not be thermally corrected as there is no transient basement pressure available";
   }
}

//------------------------------------------------------------//
PaleowaterdepthCalculator::~PaleowaterdepthCalculator(){
   // Empty constructor (comment removes SonarQube issue)
}

//------------------------------------------------------------//
void PaleowaterdepthCalculator::compute(){

   unsigned int i, j;
   double PWD = 0, responseFactor = 0;
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
                  responseFactor = calculateResponseFactor( presentDayPressureMantle,
                                                            presentDayPressureBasement,
                                                            currentPressureMantle,
                                                            currentPressureBasement );
                  PWD = calculateThermallyCorrectedPWD( TTS,
                                                        backstrip,
                                                        responseFactor );
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
            responseFactor = Interface::DefaultUndefinedMapValue;
            PWD            = Interface::DefaultUndefinedMapValue;
         }
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::isostaticBathymetry, i, j, PWD            );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::ResponseFactor     , i, j, responseFactor );
      }
   }

   restoreData();
}

//------------------------------------------------------------//
double PaleowaterdepthCalculator::calculateResponseFactor( const double presentDayPressureBotMantle,
                                                           const double presentDayPressureBasement,
                                                           const double currentPressureBotMantle,
                                                           const double currentPressureBasement ) const {
   assert( m_mantleDensity != m_waterDensity );
   //Pressure data available to equilibrate Basement and Bottom Mantle pressure
   double responseFactor = ((presentDayPressureBotMantle - presentDayPressureBasement) - (currentPressureBotMantle - currentPressureBasement)) * MegaPaToPa /
      (AccelerationDueToGravity*(m_mantleDensity - m_waterDensity));
   return responseFactor;
}

//------------------------------------------------------------//
double PaleowaterdepthCalculator::calculateThermallyCorrectedPWD( const double presentDayTTS,
                                                                  const double backstrip,
                                                                  const double responseFactor ) const {
   return presentDayTTS - backstrip - responseFactor;
}

//------------------------------------------------------------//
double PaleowaterdepthCalculator::calculatePWD( const double presentDayTTS,
                                                const double backstrip ) const {
   return presentDayTTS - backstrip;
}

//------------------------------------------------------------//
void PaleowaterdepthCalculator::retrieveData() {
   if (m_presentDayPressureBasement != nullptr){
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