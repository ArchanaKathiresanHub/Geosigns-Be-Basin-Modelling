//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "TotalTectonicSubsidenceCalculator.h"

// DataAccess library
#include "Interface/Interface.h"

//------------------------------------------------------------//
TotalTectonicSubsidenceCalculator::TotalTectonicSubsidenceCalculator( 
   InterfaceInput&           inputData,
   AbstractInterfaceOutput&  outputData,
   AbstractValidator&        validator,
   const double              age,
   const double              airCorrection,
   const Interface::GridMap* previousTTS,
   const PolyFunction2DArray& depthWaterBottom ) :
      m_firstI             ( inputData.firstI() ),
      m_firstJ             ( inputData.firstJ() ),
      m_lastI              ( inputData.lastI()  ),
      m_lastJ              ( inputData.lastJ()  ),
      m_seeLevelAdjustment ( inputData.getDeltaSLMap() ),
      m_outputData         ( outputData       ),
      m_validator          ( validator        ),
      m_age                ( age              ),
      m_airCorrection      ( airCorrection    ),
      m_previousTTS        ( previousTTS      ),
      m_surfaceDepthHistory( depthWaterBottom )
{}

//------------------------------------------------------------//
void TotalTectonicSubsidenceCalculator::compute(){

   unsigned int i, j;
   double TTS, incTS, TTSadjusted;
   retrieveData();

   for ( i = m_firstI; i <= m_lastI; ++i ) {
      for ( j = m_firstJ; j <= m_lastJ; ++j ) {
         const double depthWaterBottom = m_surfaceDepthHistory( i, j ).F( m_age );
         const double backstrip        = m_outputData.getMapValue( CrustalThicknessInterface::outputMaps::cumSedimentBackstrip, i, j );
         if ( m_validator.isValid( i, j ) and
              depthWaterBottom != Interface::DefaultUndefinedMapValue and
              backstrip        != Interface::DefaultUndefinedMapValue )
         {
            //1. Compute the TTS
            TTS = calculateTTS( depthWaterBottom, backstrip );
            //2. Compute the incremental TTS
            if ( m_previousTTS != nullptr ) {
               const double previousTTS = m_previousTTS->getValue( i, j );
               if ( previousTTS != Interface::DefaultUndefinedMapValue ){
                  incTS = calculateIncrementalTTS( TTS, previousTTS );
               }
               else{
                  incTS = Interface::DefaultUndefinedMapValue;
               }
            }
            //else this is the first TTS, there is nothing to compute
            else{
               incTS = 0;
            }
            //3. Compute the see level adjusted TTS
            const double seeLevelAdjustment = m_seeLevelAdjustment.getValue( i, j );
            if ( seeLevelAdjustment != Interface::DefaultUndefinedMapValue ){
               TTSadjusted = calculateTTSadjusted( TTS, seeLevelAdjustment );
            }
            else{
               TTSadjusted = Interface::DefaultUndefinedMapValue;
            }
         }
         else{
            TTS         = Interface::DefaultUndefinedMapValue;
            incTS       = Interface::DefaultUndefinedMapValue;
            TTSadjusted = Interface::DefaultUndefinedMapValue;
         }

         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::WLSMap,                i, j, TTS         );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::incTectonicSubsidence, i, j, incTS       );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::WLSadjustedMap,        i, j, TTSadjusted );

      }
   }

   restoreData();

}

//------------------------------------------------------------//
double TotalTectonicSubsidenceCalculator::calculateTTS( const double waterBottom,
                                                        const double backstrip ) const {
   double TTS = Interface::DefaultUndefinedMapValue;
   if (waterBottom >= 0.0) {
      TTS = waterBottom + backstrip;
   }
   else {
      TTS = waterBottom * m_airCorrection + backstrip;
   }
   if (TTS < 0) {
      TTS = 0.0;
   }
   return TTS;
}

//------------------------------------------------------------//
double TotalTectonicSubsidenceCalculator::calculateIncrementalTTS( const double TTS,
                                                                   const double previousTTS ) const {
   return TTS - previousTTS;
}

//------------------------------------------------------------//
double TotalTectonicSubsidenceCalculator::calculateTTSadjusted( const double TTS,
                                                                const double seeLevelAdjustment ) const{
   return TTS - seeLevelAdjustment;
}

//------------------------------------------------------------//
void TotalTectonicSubsidenceCalculator::retrieveData(){
   m_seeLevelAdjustment.retrieveData();
   if (m_previousTTS != nullptr) {
      m_previousTTS->retrieveData();
   }
}

//------------------------------------------------------------//
void TotalTectonicSubsidenceCalculator::restoreData(){
   m_seeLevelAdjustment.restoreData();
   if (m_previousTTS != nullptr) {
      m_previousTTS->restoreData();
   }
}

