//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmOneCrustThinningEvent.h
/// @brief This file keeps API implementation for handling variation of Single thinning crust event parameter. 


#include "PrmOneCrustThinningEvent.h"
#include "VarPrmOneCrustThinningEvent.h"

#include <cassert>

namespace casa
{

VarPrmOneCrustThinningEvent::VarPrmOneCrustThinningEvent( double baseThickIni,    double minThickIni,    double maxThickIni, 
                                                          double baseT0,          double minT0,          double maxT0,       
                                                          double baseDeltaT,      double minDeltaT,      double maxDeltaT,   
                                                          double baseThinningFct, double minThinningFct, double maxThinningFct, 
                                                          PDF prmPDF )
{
   m_pdf = prmPDF;
   
   assert( minThickIni <= baseThickIni && baseThickIni <= maxThickIni );
   assert( minT0       <= baseT0       && baseT0       <= maxT0       );
   assert( minDeltaT   <= baseDeltaT   && baseDeltaT   <= maxDeltaT   );
   assert( minThinningFct <= baseThinningFct && baseThinningFct <= maxThinningFct );

   m_minValue.reset( new PrmOneCrustThinningEvent( this, minThickIni, minT0, minDeltaT, minThinningFct ) );
   m_maxValue.reset( new PrmOneCrustThinningEvent( this, maxThickIni, maxT0, maxDeltaT, maxThinningFct ) );
   
   m_baseValue.reset( new PrmOneCrustThinningEvent( this, baseThickIni, baseT0, baseDeltaT, baseThinningFct ) );
}

VarPrmOneCrustThinningEvent::~VarPrmOneCrustThinningEvent()
{
}

std::vector<std::string> VarPrmOneCrustThinningEvent::name()
{
	std::vector<std::string> ret;
	ret.push_back("InitialCrustThickness");
	ret.push_back("EventStartTime");
	ret.push_back("EventDuration");
	ret.push_back("CrustThinningFactor");
	return ret;
}

SharedParameterPtr VarPrmOneCrustThinningEvent::createNewParameterFromDouble( std::vector<double>::const_iterator & vals ) const
{
   const std::vector<double> & minV = dynamic_cast<PrmOneCrustThinningEvent*>( m_minValue.get() )->asDoubleArray();
   const std::vector<double> & maxV = dynamic_cast<PrmOneCrustThinningEvent*>( m_maxValue.get() )->asDoubleArray();
   double valsP[4];

   for ( size_t i = 0; i < minV.size(); ++i )
   {
      valsP[i] = *vals++;

      if ( valsP[i] < minV[i] || valsP[i] > maxV[i] )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Variation of OneCrustThinningEvent parameter " << valsP[i] << 
               " falls out of range: [" << minV[i] << ":" << maxV[i] << "]";
      }
   }

   SharedParameterPtr prm( new PrmOneCrustThinningEvent( this, valsP[0], valsP[1], valsP[2], valsP[3] ) );

   return prm;
}

}
