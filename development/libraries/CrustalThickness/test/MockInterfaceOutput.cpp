//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "MockInterfaceOutput.h"

// utilitites library
#include"../../utilities/src/LogHandler.h"

MockInterfaceOutput::MockInterfaceOutput() :
   m_outputMaps( boost::extents[CrustalThicknessInterface::numberOfOutputMaps][0][0] ),
   m_firstI( 0 ),
   m_firstJ( 0 ),
   m_lastI( 0 ),
   m_lastJ( 0 )
{}

MockInterfaceOutput::MockInterfaceOutput( unsigned int firstI,
                                          unsigned int firstJ,
                                          unsigned int lastI,
                                          unsigned int lastJ ) :
   m_outputMaps( boost::extents[CrustalThicknessInterface::numberOfOutputMaps][lastI - firstI + 1][lastJ - firstJ + 1] ),
   m_firstI( firstI ),
   m_firstJ( firstJ ),
   m_lastI( lastI ),
   m_lastJ( lastJ )
{}

void   MockInterfaceOutput::setMapValue( CrustalThicknessInterface::outputMaps mapIndex, unsigned int i, unsigned int j, const double value ){

   if (i >= m_firstI and j >= m_firstJ){
      m_outputMaps[mapIndex][i - m_firstI][j - m_firstJ] = value;
   }
   else{
      LogHandler( LogHandler::ERROR_SEVERITY ) << "Tring to set map value (" << i << "," << j << ") but it is out of range.";
   }
}

void MockInterfaceOutput::setMapValues( CrustalThicknessInterface::outputMaps mapIndex, const double value ){
   unsigned int i, j;
   for (i = m_firstI; i <= m_lastI; ++i) {
      for (j = m_firstJ; j <= m_lastJ; ++j) {
         setMapValue( mapIndex, i, j, value );
      }
   }
}

void MockInterfaceOutput::clear(){
   unsigned int i, j;
   for (i = m_firstI; i <= m_lastI; ++i) {
      for (j = m_firstJ; j <= m_lastJ; ++j) {
         for (size_t mapIndex = 0; mapIndex < m_outputMaps.shape()[0]; mapIndex++){
            m_outputMaps[mapIndex][i - m_firstI][j - m_firstJ] = 0.0;
         }
      }
   }
}