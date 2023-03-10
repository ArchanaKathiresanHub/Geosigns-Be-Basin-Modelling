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
#include <cstring>

namespace casa
{

VarPrmSourceRockHC::VarPrmSourceRockHC( const char          * lrName
                                      , double                baseValue
                                      , double                minValue
                                      , double                maxValue
                                      , VarPrmContinuous::PDF pdfType
                                      , const char          * name
                                      , const char          * srTypeName
                                      , int                   mixID
                                      ) : VarPrmSourceRockProp( lrName, pdfType, name, srTypeName, mixID )
{
   m_propName = "H/C";

   m_minValue.reset(  new PrmSourceRockHC( this, minValue,  lrName, srTypeName, m_mixID ) );
   m_maxValue.reset(  new PrmSourceRockHC( this, maxValue,  lrName, srTypeName, m_mixID ) );
   m_baseValue.reset( new PrmSourceRockHC( this, baseValue, lrName, srTypeName, m_mixID ) );

   if ( !m_srTypeName.empty() ) { addSourceRockTypeRange( srTypeName, m_baseValue, m_minValue, m_maxValue, pdfType ); }
}

std::vector<std::string> VarPrmSourceRockHC::name() const
{
   std::vector<std::string> ret;
   if ( m_name.empty() ) { ret.push_back( m_layerName + " H/C [kg/tonne C]" ); }
   else                  { ret.push_back( m_name ); }
	return ret;
}

// Save all object data to the given stream, that object could be later reconstructed from saved data
bool VarPrmSourceRockHC::save( CasaSerializer & sz ) const
{
   bool ok = VarPrmSourceRockProp::serializeCommonPart( sz );
   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
VarPrmSourceRockHC::VarPrmSourceRockHC( CasaDeserializer & dz, unsigned int objVer )
{
   bool ok = VarPrmSourceRockProp::deserializeCommonPart( dz, objVer );

   if ( m_propName.empty() ) { m_propName = "HI"; }

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "VarPrmSourceRockHC deserialization unknown error";
   }
}

SharedParameterPtr VarPrmSourceRockHC::createNewPrm( double val, const std::string & srType ) const
{
   SharedParameterPtr prm( new PrmSourceRockHC( this, val, m_layerName.c_str(), srType.empty() ? 0 : srType.c_str(), m_mixID ) );
   return prm;
}

SharedParameterPtr VarPrmSourceRockHC::createNewPrmFromModel( mbapi::Model & mdl ) const
{
   SharedParameterPtr prm( new PrmSourceRockHC( mdl, m_layerName.c_str(), ( m_srTypeName.empty( ) ? 0 : m_srTypeName.c_str( ) ), m_mixID ) );
   return prm;
}

}
