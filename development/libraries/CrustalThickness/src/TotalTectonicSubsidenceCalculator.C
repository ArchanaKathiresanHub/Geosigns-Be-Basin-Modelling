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
TotalTectonicSubsidenceCalculator::TotalTectonicSubsidenceCalculator( const unsigned int firstI,
   const unsigned int firstJ,
   const unsigned int lastI,
   const unsigned int lastJ,
   const double age,
   const double airCorrection,
   const Interface::GridMap* previousTTS,
   const PolyFunction2DArray& depthWaterBottom,
   AbstractInterfaceOutput& outputData,
   AbstractValidator& validator ) : m_firstI( firstI ),
                                    m_firstJ( firstJ ),
                                    m_lastI ( lastI ),
                                    m_lastJ ( lastJ ),
                                    m_age( age ),
                                    m_airCorrection( airCorrection ),
                                    m_previousTTS( previousTTS),
                                    m_surfaceDepthHistory( depthWaterBottom ),
                                    m_outputData( outputData ),
                                    m_validator ( validator ){}

//------------------------------------------------------------//
void TotalTectonicSubsidenceCalculator::compute(){

   unsigned int i, j;
   double TTS, incTS;

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
            if (m_previousTTS != nullptr) {
               const double previousTTS = m_previousTTS->getValue( i, j );
               incTS = calculateIncrementalTTS( TTS, previousTTS );
            }
            //else this is the first TTS, there is nothing to compute
            else{
               incTS = 0;
            }
         }
         else{
            TTS    = Interface::DefaultUndefinedMapValue;
            incTS = Interface::DefaultUndefinedMapValue;
         }

         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::WLSMap,                i, j, TTS    );
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::incTectonicSubsidence, i, j, incTS );

      }
   }

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
   double incTS = TTS - previousTTS;
   return incTS;
}