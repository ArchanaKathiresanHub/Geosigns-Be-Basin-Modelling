//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "LinearFunction.h"

// utilities library
#include "LogHandler.h"

//------------------------------------------------------------//
LinearFunction::LinearFunction() {
   m_TTS_crit                  = 0.0;
   m_TTS_onset                 = 0.0;
   m_m1                        = 0.0;
   m_m2                        = 0.0;
   m_c2                        = 0.0;
   m_maxBasalticCrustThickness = 0.0;
   m_magmaThicknessCoeff       = 0.0;
} 
//------------------------------------------------------------//
double LinearFunction::getCrustTF( const double incrementalTTS ) const {

   double TF = 1.0;
   
   if( m_maxBasalticCrustThickness == 0.0 ) {
      TF = m_m1 * incrementalTTS;
   } else {
      if (incrementalTTS < m_TTS_onset) {
         TF = m_m1 * incrementalTTS;
      }
      else if (incrementalTTS > m_TTS_crit) {
         TF = 1.0;
      } else {
         TF = m_m2 * incrementalTTS + m_c2;
      }
   }
   return TF;
}
//------------------------------------------------------------//
double LinearFunction::getBasaltThickness( const double incrementalTTS ) const {

   double thickness = 0.0;

   if( m_maxBasalticCrustThickness != 0.0 ) {
      if (incrementalTTS < m_TTS_onset)  {
         thickness = 0.0;
      }
      else if (incrementalTTS >= m_TTS_crit){
         thickness = m_maxBasalticCrustThickness - (incrementalTTS - m_TTS_crit) * m_magmaThicknessCoeff;
      } else {
         thickness = m_maxBasalticCrustThickness * ((incrementalTTS - m_TTS_onset) / (m_TTS_crit - m_TTS_onset));
      }
      
      if( thickness < 0.0 ) {
         thickness = 0.0;
      }
   }
   return thickness;
}

//------------------------------------------------------------//
void LinearFunction::printCoeffs() const {

   LogHandler( LogHandler::INFO_SEVERITY ) << "Linear function is defined by:";
   LogHandler( LogHandler::INFO_SEVERITY ) << "   #m1 = " << m_m1;
   LogHandler( LogHandler::INFO_SEVERITY ) << "   #m2 = " << m_m2;
   LogHandler( LogHandler::INFO_SEVERITY ) << "   #c2 = " << m_c2;
}

