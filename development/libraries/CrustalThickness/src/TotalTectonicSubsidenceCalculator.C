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

TotalTectonicSubsidenceCalculator::TotalTectonicSubsidenceCalculator( const unsigned int firstI,
   const unsigned int firstJ,
   const unsigned int lastI,
   const unsigned int lastJ,
   const double airCorrection,
   DerivedProperties::SurfacePropertyPtr depthWaterBottom,
   AbstractInterfaceOutput& outputData,
   AbstractValidator& validator ) : m_firstI( firstI ),
                                    m_firstJ( firstJ ),
                                    m_lastI ( lastI ),
                                    m_lastJ ( lastJ ),
                                    m_airCorrection( airCorrection ),
                                    m_depthWaterBottom( depthWaterBottom ),
                                    m_outputData( outputData ),
                                    m_validator ( validator ){}

void TotalTectonicSubsidenceCalculator::compute(){

   unsigned int i, j;
   double TTS;

   for ( i = m_firstI; i <= m_lastI; ++i ) {
      for ( j = m_firstJ; j <= m_lastJ; ++j ) {
         const double depthWaterBottom = m_depthWaterBottom->get( i, j );
         const double backstrip        = m_outputData.getMapValue( CrustalThicknessInterface::outputMaps::cumSedimentBackstrip, i, j );
         if ( m_validator.isValid( i, j ) and
              depthWaterBottom != m_depthWaterBottom->getUndefinedValue() and
              backstrip        != Interface::DefaultUndefinedMapValue )
         {
            TTS = calculateTTS( depthWaterBottom, backstrip );
         }
         else{
            TTS = Interface::DefaultUndefinedMapValue;
         }
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::WLSMap, i, j, TTS );
      }
   }
}

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
