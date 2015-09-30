//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmSourceRockTOC.h
/// @brief This file keeps API implementation for handling variation of initial source rock TOC parameter. 

#include "PrmSourceRockTOC.h"
#include "VarPrmSourceRockTOC.h"

#include <cassert>
#include <cstring>

namespace casa
{

VarPrmSourceRockTOC::VarPrmSourceRockTOC( const char           * lrName
                                        , double                 baseValue
                                        , double                 minValue
                                        , double                 maxValue
                                        , VarPrmContinuous::PDF  pdfType
                                        , const char           * name
                                        , const char           * srTypeName
                                        , int                    mixID
                                        ) : VarPrmSourceRockProp( lrName, baseValue, minValue, maxValue, pdfType, name, srTypeName, mixID )
{
   m_propName = "TOC";
   m_minValue.reset(  new PrmSourceRockTOC( this, minValue,  lrName, srTypeName, m_mixID ) );
   m_maxValue.reset(  new PrmSourceRockTOC( this, maxValue,  lrName, srTypeName, m_mixID ) );
   m_baseValue.reset( new PrmSourceRockTOC( this, baseValue, lrName, srTypeName, m_mixID ) );
}

std::vector<std::string> VarPrmSourceRockTOC::name() const
{
	std::vector<std::string> ret;
   
   if ( m_name.empty() ) { ret.push_back( m_layerName + " TOC [%]" ); }
   else                  { ret.push_back( m_name ); }

 	return ret;
}
// Save all object data to the given stream, that object could be later reconstructed from saved data
bool VarPrmSourceRockTOC::save( CasaSerializer & sz, unsigned int version ) const
{
   bool ok = VarPrmSourceRockProp::serializeCommonPart( sz, version );
   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
VarPrmSourceRockTOC::VarPrmSourceRockTOC( CasaDeserializer & dz, unsigned int objVer )
{
   bool ok = VarPrmSourceRockProp::deserializeCommonPart( dz, objVer );
 
   if ( m_propName.empty() ) { m_propName = "TOC"; }

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "VarPrmSourceRockTOC deserialization unknown error";
   }
}

PrmSourceRockProp * VarPrmSourceRockTOC::createNewPrm( double val ) const
{
   return new PrmSourceRockTOC( this, val, m_layerName.c_str(), (m_srTypeName.empty() ? 0 : m_srTypeName.c_str()), m_mixID ); 
}

} // namespace casa
