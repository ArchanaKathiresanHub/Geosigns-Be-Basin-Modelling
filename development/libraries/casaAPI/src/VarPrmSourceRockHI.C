//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmSourceRockHI.h
/// @brief This file keeps API implementation for handling variation of source rock HI initial ratio parameter. 

#include "PrmSourceRockHI.h"
#include "VarPrmSourceRockHI.h"

#include <cassert>

namespace casa
{

VarPrmSourceRockHI::VarPrmSourceRockHI( const char * layerName, double baseValue, double minValue, double maxValue, PDF pdfType ) : 
   m_layerName( layerName )
{
   m_pdf = pdfType;

   assert( minValue <= baseValue && maxValue >= baseValue );

   m_minValue.reset( new PrmSourceRockHI( this, minValue, layerName ) );
   m_maxValue.reset( new PrmSourceRockHI( this, maxValue, layerName ) );
   
   m_baseValue.reset( new PrmSourceRockHI( this, baseValue, layerName ) );
}

VarPrmSourceRockHI::~VarPrmSourceRockHI()
{
   ;
}

std::vector<std::string> VarPrmSourceRockHI::name() const
{
	std::vector<std::string> ret;
	ret.push_back( m_layerName + " HI [kg/tonne]" );
	return ret;
}

SharedParameterPtr VarPrmSourceRockHI::newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const
{
   double minV = dynamic_cast<PrmSourceRockHI*>( m_minValue.get() )->value();
   double maxV = dynamic_cast<PrmSourceRockHI*>( m_maxValue.get() )->value();
   double prmV = *vals++;

   if ( minV > prmV || prmV > maxV )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Variation of source rock HI parameter for layer " << m_layerName << ": " << prmV << 
                                                                        " falls out of range: [" << minV << ":" << maxV << "]";
   }
   SharedParameterPtr prm( new PrmSourceRockHI( this, prmV, m_layerName.c_str() ) );

   return prm;
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool VarPrmSourceRockHI::save( CasaSerializer & sz, unsigned int version ) const
{
   bool ok = VarPrmContinuous::save( sz, version );
   ok = ok ? sz.save( m_layerName, "layerName" ) : ok;

   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
VarPrmSourceRockHI::VarPrmSourceRockHI( CasaDeserializer & dz, unsigned int objVer ) : VarPrmContinuous( dz, objVer )
{
   bool ok = dz.load( m_layerName, "layerName" );

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "VarPrmSourceRockHI deserialization unknown error";
   }
}

}
