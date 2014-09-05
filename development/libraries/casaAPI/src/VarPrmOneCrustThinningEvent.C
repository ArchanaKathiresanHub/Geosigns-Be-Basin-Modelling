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

VarPrmOneCrustThinningEvent::VarPrmOneCrustThinningEvent( double baseThickIni, double minThickIni, double maxThickIni, 
                                                          double baseT0,       double minT0,       double maxT0,       
                                                          double baseDeltaT,   double minDeltaT,   double maxDeltaT,   
                                                          double baseThingFct, double minThingFct, double maxThingFct, 
                                                          PDF prmPDF )
{
   m_pdf = prmPDF;
   
   assert( minThickIni <= baseThickIni && baseThickIni <= maxThickIni );
   assert( minT0       <= baseT0       && baseT0       <= maxT0       );
   assert( minDeltaT   <= baseDeltaT   && baseDeltaT   <= maxDeltaT   );
   assert( minThingFct <= baseThingFct && baseThingFct <= maxThingFct );

   m_minValue.reset( new PrmOneCrustThinningEvent( minThickIni, minT0, minDeltaT, minThingFct ) );
   m_maxValue.reset( new PrmOneCrustThinningEvent( maxThickIni, maxT0, maxDeltaT, maxThingFct ) );
   
   m_baseValue.reset( new PrmOneCrustThinningEvent( baseThickIni, baseT0, baseDeltaT, baseThingFct ) );
}

VarPrmOneCrustThinningEvent::~VarPrmOneCrustThinningEvent()
{
}

Parameter * VarPrmOneCrustThinningEvent::createNewParameterFromDouble( const std::vector<double> & vals ) const
{
   const std::vector<double> & minV = dynamic_cast<PrmOneCrustThinningEvent*>( m_minValue.get() )->asDoubleArray();
   const std::vector<double> & maxV = dynamic_cast<PrmOneCrustThinningEvent*>( m_maxValue.get() )->asDoubleArray();

   assert( vals.size() == minV.size() );

   for ( size_t i = 0; i < vals.size(); ++i )
   {
      if ( vals[i] < minV[i] || vals[i] > maxV[i] ) return 0;
   }

   return new PrmOneCrustThinningEvent( vals[0], vals[1], vals[2], vals[3] );
}

}
