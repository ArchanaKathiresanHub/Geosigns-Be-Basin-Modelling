//                                                                      
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmCrustThinning.h
/// @brief This file keeps API implementation for handling variation of Single thinning crust event parameter. 


#include "PrmCrustThinning.h"
#include "VarPrmCrustThinning.h"

#include "Path.h" // for to_string

#include <cassert>
#include <cstring>

namespace casa
{

VarPrmCrustThinning::VarPrmCrustThinning( const std::vector<double>      & baseValues
                                        , const std::vector<double>      & minValues
                                        , const std::vector<double>      & maxValues
                                        , const std::vector<std::string> & mapsName
                                        , PDF                              prmPDF
                                        , const char                     * name
                                        ) 
{
   m_pdf = prmPDF;
   m_mapsName.insert( m_mapsName.begin(), mapsName.begin(), mapsName.end() );
   m_eventsNumber = (baseValues.size() - 1) / 3;
   m_name = name && strlen( name ) > 0 ? std::string( name ) : std::string( "" );

   // check input values
   if ( baseValues.size() != minValues.size() ||
        baseValues.size() != maxValues.size() || // base/min/max arrays must have same dimensions
        m_eventsNumber == 0                   ||
        ((baseValues.size() - 1) % 3) != 0    || // initial crust thickness, (t0,deltaT,fact),(...),(...)
        mapsName.size() != m_eventsNumber        // maps name number must be the same as number of events
      )
   {
      throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << 
            "Wrong parameters number for CrustTinnning influential parameter";
   }

   for ( size_t i = 0, pos = 0; i <= m_eventsNumber; ++i )
   {
      for ( size_t j = 0; j < 3; ++j, ++pos )
      {
         if ( minValues[pos] > baseValues[pos] || baseValues[pos] > maxValues[pos] )
         {
            ErrorHandler::Exception ex( ErrorHandler::OutOfRangeValue );
            ex << "Crust thinning event: " << i + 1 << ", has ";

            if (      i == 0 ) { ex << "initial crust thickness"; }
            else if ( j == 0 ) { ex << "start time"; }
            else if ( j == 1 ) { ex << "event duration"; }
            else               { ex << "thinning factor"; }

            throw ex << " base case value: " << baseValues[pos] <<  " outside of the given [" << 
                        minValues[pos] << "," << maxValues[pos] << "] range";
         }
         if ( i == 0 ) { ++pos; break; }
      }
   }
   
   // create base/min/max parameters
   m_minValue.reset(  new PrmCrustThinning( this, minValues,  mapsName ) );
   m_maxValue.reset(  new PrmCrustThinning( this, maxValues,  mapsName ) );
   m_baseValue.reset( new PrmCrustThinning( this, baseValues, mapsName ) );
}

VarPrmCrustThinning::~VarPrmCrustThinning()
{
}

std::vector<std::string> VarPrmCrustThinning::name() const
{
   std::vector<std::string> ret;
   if ( m_name.empty() )
   {
      ret.push_back( "InitialCrustThickness [m]" );

      for ( size_t i = 0; i < m_eventsNumber; ++i )
      {  
         const std::string & evNum = ibs::to_string( m_eventsNumber );

         ret.push_back( std::string( "Event_" ) + evNum + "_StartTime [Ma]" );
         ret.push_back( std::string( "Event_" ) + evNum + "_Duration [Ma]" );
         ret.push_back( std::string( "Event_" ) + evNum + "_CrustThinningFactor [m/m]");
      }
   }
   else
   {
      ret.push_back( m_name );

      for ( size_t i = 0; i < m_eventsNumber; ++i )
      {  
         const std::string & evNum = ibs::to_string( m_eventsNumber );

         ret.push_back( m_name + std::string( "_" ) + evNum + "_t0" );
         ret.push_back( m_name + std::string( "_" ) + evNum + "_dt" );
         ret.push_back( m_name + std::string( "_" ) + evNum + "_fct" );
      }
   }
   return ret;
}


SharedParameterPtr VarPrmCrustThinning::newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const
{
   const std::vector<double> & minV = dynamic_cast<PrmCrustThinning*>( m_minValue.get() )->asDoubleArray();
   const std::vector<double> & maxV = dynamic_cast<PrmCrustThinning*>( m_maxValue.get() )->asDoubleArray();

   std::vector<double> prmV( minV.size() );

   for ( size_t i = 0; i < minV.size(); ++i )
   {
      prmV[i] = *vals++;

      if ( prmV[i] < minV[i] || prmV[i] > maxV[i] )
      {
         ErrorHandler::Exception  ex( ErrorHandler::OutOfRangeValue );
         ex << "Variation of CrustThinning sub-parameter ";

         if ( i == 0 )              ex << "initial crust thickness";
         else if ( (i - 1)%3 == 0 ) ex << "event start time";
         else if ( (i - 1)%3 == 1 ) ex << "event duration";
         else                       ex << "thinning factor";
         
         throw ex << " parameter " << prmV[i] << " falls out of range: [" << minV[i] << ":" << maxV[i] << "]";
      }
   }

   SharedParameterPtr prm( new PrmCrustThinning( this, prmV, m_mapsName ) );

   return prm;
}

SharedParameterPtr VarPrmCrustThinning::newParameterFromModel( mbapi::Model & mdl ) const
{
   SharedParameterPtr prm( new PrmCrustThinning( mdl ) );
   return prm;
}

SharedParameterPtr VarPrmCrustThinning::makeThreeDFromOneD( mbapi::Model                          & mdl
                                                          , const std::vector<double>             & xin
                                                          , const std::vector<double>             & yin
                                                          , const std::vector<SharedParameterPtr> & prmVec
                                                          ) const
{
   // Not yet implemented
   throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << "makeThreeDFromOneD method not yet implemented for VarPrmCrustThinning";
   
   return nullptr;
}

// Save all object data to the given stream, that object could be later reconstructed from saved data
bool VarPrmCrustThinning::save( CasaSerializer & sz, unsigned int version ) const
{
   bool ok = VarPrmContinuous::save( sz, version );

   ok = ok ? sz.save( m_eventsNumber, "eventsNum" ) : ok;
   ok = ok ? sz.save( m_mapsName,     "mapsList"  ) : ok;

   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
VarPrmCrustThinning::VarPrmCrustThinning( CasaDeserializer & dz, unsigned int objVer )
{
   bool ok = VarPrmContinuous::deserializeCommonPart( dz, objVer );

   ok = ok ? dz.load( m_eventsNumber, "eventsNum" ) : ok;
   ok = ok ? dz.load( m_mapsName,     "mapsList"  ) : ok;

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "VarPrmLithoSTPThermalCond deserialization unknown error";
   }
}

} // namespace casa

