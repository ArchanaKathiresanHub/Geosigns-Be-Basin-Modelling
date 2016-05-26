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
#include <cstring>

namespace casa
{

VarPrmOneCrustThinningEvent::VarPrmOneCrustThinningEvent( double baseThickIni,    double minThickIni,    double maxThickIni,
                                                          double baseT0,          double minT0,          double maxT0,
                                                          double baseDeltaT,      double minDeltaT,      double maxDeltaT,
                                                          double baseThinningFct, double minThinningFct, double maxThinningFct,
                                                          PDF prmPDF, const char * name )
{
   m_pdf = prmPDF;
   m_name = name && strlen( name ) > 0 ? std::string( name ) : std::string( "" );

   ErrorHandler::Exception ex( ErrorHandler::OutOfRangeValue );
   if ( baseThickIni    < minThickIni    ) throw ex << "Base value of crust initial thickness less than minimal value";
   if ( baseThickIni    > maxThickIni    ) throw ex << "Base value of crust initial thickness greater than maximal value";
   if ( baseT0          < minT0          ) throw ex << "Base value of event start time less than minimal value";
   if ( baseT0          > maxT0          ) throw ex << "Base value of event start time greater than maximal value";
   if ( baseDeltaT      < minDeltaT      ) throw ex << "Base value of event duration less than minimal value";
   if ( baseDeltaT      > maxDeltaT      ) throw ex << "Base value of event duration greater than maximal value";
   if ( baseThinningFct < minThinningFct ) throw ex << "Base value of crust thinning factor less than minimal value";
   if ( baseThinningFct > maxThinningFct ) throw ex << "Base value of crust thinning factor greater than maximal value";

   m_minValue.reset( new PrmOneCrustThinningEvent( this, minThickIni, minT0, minDeltaT, minThinningFct ) );
   m_maxValue.reset( new PrmOneCrustThinningEvent( this, maxThickIni, maxT0, maxDeltaT, maxThinningFct ) );
   
   m_baseValue.reset( new PrmOneCrustThinningEvent( this, baseThickIni, baseT0, baseDeltaT, baseThinningFct ) );
}

VarPrmOneCrustThinningEvent::~VarPrmOneCrustThinningEvent()
{
}

std::vector<std::string> VarPrmOneCrustThinningEvent::name() const
{
   std::vector<std::string> ret;
   if ( m_name.empty() )
   {
      ret.push_back( "InitialCrustThickness [m]" );
      ret.push_back( "EventStartTime [Ma]" );
      ret.push_back( "EventDuration [Ma]" );
      ret.push_back( "CrustThinningFactor [m/m]" );
   }
   else
   {
      ret.push_back( m_name );
      ret.push_back( m_name + "_t0" );
      ret.push_back( m_name + "_dt" );
      ret.push_back( m_name + "_fct" );
   }
   return ret;
}

SharedParameterPtr VarPrmOneCrustThinningEvent::newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const
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

SharedParameterPtr VarPrmOneCrustThinningEvent::newParameterFromModel( mbapi::Model & mdl ) const
{
   SharedParameterPtr prm( new PrmOneCrustThinningEvent( mdl ) );
   return prm;
}

SharedParameterPtr VarPrmOneCrustThinningEvent::makeThreeDFromOneD( mbapi::Model & mdl, const std::vector<double>& xin, const std::vector<double>& yin, const std::vector<SharedParameterPtr>& prmVec ) const
{
   // Not yet implemented
   ErrorHandler::Exception ex( ErrorHandler::NotImplementedAPI );
   ex << "makeThreeDFromOneD method not yet implemented for VarPrmOneCrustThinningEvent";
   throw ex;
   return nullptr;
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool VarPrmOneCrustThinningEvent::save( CasaSerializer & sz, unsigned int version ) const
{
   bool ok = VarPrmContinuous::save( sz, version );

   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
VarPrmOneCrustThinningEvent::VarPrmOneCrustThinningEvent( CasaDeserializer & dz, unsigned int objVer ) 
{
   bool ok = VarPrmContinuous::deserializeCommonPart( dz, objVer );

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "VarPrmOneCrustThinningEvent deserialization unknown error";
   }
}


}
