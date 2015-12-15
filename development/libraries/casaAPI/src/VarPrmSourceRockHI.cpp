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
#include <cstring>

namespace casa
{

VarPrmSourceRockHI::VarPrmSourceRockHI( const char          * lrName
                                      , double                baseValue
                                      , double                minValue
                                      , double                maxValue
                                      , VarPrmContinuous::PDF pdfType
                                      , const char          * name
                                      , const char          * srTypeName
                                      , int                   mixID
                                      ) : VarPrmSourceRockProp( lrName, baseValue, minValue, maxValue, pdfType, name, srTypeName, mixID )
{
   m_propName = "HI";
   m_minValue.reset(  new PrmSourceRockHI( this, minValue,  lrName, srTypeName, m_mixID ) );
   m_maxValue.reset(  new PrmSourceRockHI( this, maxValue,  lrName, srTypeName, m_mixID ) );
   m_baseValue.reset( new PrmSourceRockHI( this, baseValue, lrName, srTypeName, m_mixID ) );
}

std::vector<std::string> VarPrmSourceRockHI::name() const
{
	std::vector<std::string> ret;

   if ( m_name.empty() ) { ret.push_back( m_layerName + " HI [kg/tonne]" ); }
   else                  { ret.push_back( m_name ); }
 
	return ret;
}

// Save all object data to the given stream, that object could be later reconstructed from saved data
bool VarPrmSourceRockHI::save( CasaSerializer & sz, unsigned int version ) const
{
   bool ok = VarPrmSourceRockProp::serializeCommonPart( sz, version );
   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
VarPrmSourceRockHI::VarPrmSourceRockHI( CasaDeserializer & dz, unsigned int objVer )
{
   bool ok = VarPrmSourceRockProp::deserializeCommonPart( dz, objVer );

   if ( m_propName.empty() ) { m_propName = "HI"; }

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "VarPrmSourceRockHC deserialization unknown error";
   }
}

PrmSourceRockProp * VarPrmSourceRockHI::createNewPrm( double val ) const
{
   return new PrmSourceRockHI( this, val, m_layerName.c_str(), (m_srTypeName.empty() ? 0 : m_srTypeName.c_str()), m_mixID ); 
}


}
