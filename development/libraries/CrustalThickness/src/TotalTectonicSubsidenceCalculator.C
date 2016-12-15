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
using namespace  CrustalThickness;

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
      m_age                ( age                ),
      m_airCorrection      ( airCorrection      ),
      m_previousTTS        ( previousTTS        ),
      m_seaLevelAdjustment ( inputData.getDeltaSLMap( age ) ),
      m_surfaceDepthHistory( depthWaterBottom ),
      m_outputData         ( outputData       ),
      m_validator          ( validator        )
{}

//------------------------------------------------------------//
void TotalTectonicSubsidenceCalculator::compute(){

   double TTS = 0.0, incTS = 0.0, TTSadjusted = 0.0, incTSadjusted = 0.0;
   retrieveData();

   for ( unsigned int i = m_firstI; i <= m_lastI; ++i ) {
      for ( unsigned int j = m_firstJ; j <= m_lastJ; ++j ) {
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
            //else this is the first TTS
            else{
               incTS = TTS;
            }
            //3. Compute the see level adjusted TTS
            const double seaLevelAdjustment = m_seaLevelAdjustment.getValue( i, j );
            if (seaLevelAdjustment != Interface::DefaultUndefinedMapValue){
               TTSadjusted = calculateTSadjusted( TTS, seaLevelAdjustment );
               if (incTS != Interface::DefaultUndefinedMapValue){
                  incTSadjusted = calculateTSadjusted( incTS, seaLevelAdjustment );
               }
               else {
                  incTSadjusted = Interface::DefaultUndefinedMapValue;
               }
            }
            else{
               TTSadjusted   = Interface::DefaultUndefinedMapValue;
               incTSadjusted = Interface::DefaultUndefinedMapValue;
            }
         }
         else{
            TTS           = Interface::DefaultUndefinedMapValue;
            incTS         = Interface::DefaultUndefinedMapValue;
            TTSadjusted   = Interface::DefaultUndefinedMapValue;
            incTSadjusted = Interface::DefaultUndefinedMapValue;
         }

         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::WLSMap,                        i, j, TTS           );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::incTectonicSubsidence,         i, j, incTS         );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::WLSadjustedMap,                i, j, TTSadjusted   );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::incTectonicSubsidenceAdjusted, i, j, incTSadjusted );

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
double TotalTectonicSubsidenceCalculator::calculateTSadjusted( const double TS,
                                                               const double seaLevelAdjustment ) const{
   const double result = TS - seaLevelAdjustment;
   if (result < 0.0){
      return 0.0;
   }
   else{
      return result;
   }
}

//------------------------------------------------------------//
void TotalTectonicSubsidenceCalculator::retrieveData(){
   m_seaLevelAdjustment.retrieveData();
   if (m_previousTTS != nullptr) {
      m_previousTTS->retrieveData();
   }
}

//------------------------------------------------------------//
void TotalTectonicSubsidenceCalculator::restoreData(){
   m_seaLevelAdjustment.restoreData();
   if (m_previousTTS != nullptr) {
      m_previousTTS->restoreData();
   }
}

