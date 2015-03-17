//                                                                      
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmSourceRockHC.h
/// @brief This file keeps API implementation for handling variation of source rock H/C initial ratio parameter. 

#include "PrmSourceRockHC.h"
#include "VarPrmSourceRockHC.h"

#include <cassert>

namespace casa
{

VarPrmSourceRockHC::VarPrmSourceRockHC( const char * layerName, double baseValue, double minValue, double maxValue, PDF pdfType ) : 
   m_layerName( layerName )
{
   m_pdf = pdfType;

   assert( minValue <= baseValue && maxValue >= baseValue );

   m_minValue.reset( new PrmSourceRockHC( this, minValue, layerName ) );
   m_maxValue.reset( new PrmSourceRockHC( this, maxValue, layerName ) );
   
   m_baseValue.reset( new PrmSourceRockHC( this, baseValue, layerName ) );
}

VarPrmSourceRockHC::~VarPrmSourceRockHC()
{
   ;
}

std::vector<std::string> VarPrmSourceRockHC::name() const
{
	std::vector<std::string> ret;
	ret.push_back( m_layerName + " H/C [kg/tonne C]" );
	return ret;
}

SharedParameterPtr VarPrmSourceRockHC::newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const
{
   double minV = dynamic_cast<PrmSourceRockHC*>( m_minValue.get() )->value();
   double maxV = dynamic_cast<PrmSourceRockHC*>( m_maxValue.get() )->value();
   double prmV = *vals++;

   if ( minV > prmV || prmV > maxV )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Variation of source rock H/C parameter for layer " << m_layerName << ": " << prmV << 
                                                                        " falls out of range: [" << minV << ":" << maxV << "]";
   }
   SharedParameterPtr prm( new PrmSourceRockHC( this, prmV, m_layerName.c_str() ) );

   return prm;
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool VarPrmSourceRockHC::save( CasaSerializer & sz, unsigned int version ) const
{
   bool ok = VarPrmContinuous::save( sz, version );
   ok = ok ? sz.save( m_layerName, "layerName" ) : ok;

   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
VarPrmSourceRockHC::VarPrmSourceRockHC( CasaDeserializer & dz, unsigned int objVer ) : VarPrmContinuous( dz, objVer )
{
   bool ok = dz.load( m_layerName, "layerName" );

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "VarPrmSourceRockHC deserialization unknown error";
   }
}

}
