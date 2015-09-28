//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmTopCrustHeatProduction.h
/// @brief This file keeps API implementation for handling variation of Top Crust Heat Production Rate parameter. 


#include "PrmTopCrustHeatProduction.h"
#include "VarPrmTopCrustHeatProduction.h"

#include <cassert>
#include <cstring>


namespace casa
{

VarPrmTopCrustHeatProduction::VarPrmTopCrustHeatProduction( double baseValue, double minValue, double maxValue, PDF pdfType, const char * name )
{
   m_pdf = pdfType;
   m_name = name && strlen( name ) > 0 ? std::string( name ) : std::string( "" );
  
   m_minValue.reset( new PrmTopCrustHeatProduction( this, minValue ) );
   m_maxValue.reset( new PrmTopCrustHeatProduction( this, maxValue ) );
   
   assert( minValue <= baseValue && maxValue >= baseValue );

   m_baseValue.reset( new PrmTopCrustHeatProduction( this, baseValue ) );
}

VarPrmTopCrustHeatProduction::~VarPrmTopCrustHeatProduction()
{
   ;
}

std::vector<std::string> VarPrmTopCrustHeatProduction::name() const
{
	std::vector<std::string> ret;

   if ( m_name.empty() ) { ret.push_back( "TopCrustHeatProdRate [\\mu W/m^3]" ); }
   else                  { ret.push_back( m_name ); }

	return ret;
}

SharedParameterPtr VarPrmTopCrustHeatProduction::newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const
{
   double minV = dynamic_cast<PrmTopCrustHeatProduction*>( m_minValue.get() )->value();
   double maxV = dynamic_cast<PrmTopCrustHeatProduction*>( m_maxValue.get() )->value();
   double valP = *vals++;

   if ( minV > valP || valP > maxV )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Variation of Top crust heat production rate parameter " << valP << 
                                                                        " falls out of range: [" << minV << ":" << maxV << "]";
   }
   
   SharedParameterPtr prm( new PrmTopCrustHeatProduction( this, valP ) );

   return prm;
}

// Save all object data to the given stream, that object could be later reconstructed from saved data
bool VarPrmTopCrustHeatProduction::save( CasaSerializer & sz, unsigned int version ) const
{
   bool ok = VarPrmContinuous::save( sz, version );
   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
VarPrmTopCrustHeatProduction::VarPrmTopCrustHeatProduction( CasaDeserializer & dz, unsigned int objVer )
{
   bool ok =  VarPrmContinuous::deserializeCommonPart( dz, objVer );

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "VarPrmTopCrustHeatProduction deserialization unknown error";
   }
}


}
