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

VarPrmSourceRockTOC::VarPrmSourceRockTOC( const char                     * lrName
                                        , const std::vector<double>      & dblRng
                                        , const std::vector<std::string> & mapRng
                                        , VarPrmContinuous::PDF            pdfType
                                        , const char                     * name
                                        , const char                     * srTypeName
                                        , int                              mixID
                                        ) : VarPrmSourceRockProp( lrName, pdfType, name, srTypeName, mixID )
{
   m_propName = "TOC";

   if ( dblRng.size() == 3 )
   {
      m_minValue.reset(  new PrmSourceRockTOC( this, dblRng[0], lrName, std::string(), srTypeName, m_mixID ) );
      m_maxValue.reset(  new PrmSourceRockTOC( this, dblRng[1], lrName, std::string(), srTypeName, m_mixID ) );
      m_baseValue.reset( new PrmSourceRockTOC( this, dblRng[2], lrName, std::string(), srTypeName, m_mixID ) );
   }
   else if ( mapRng.size() == 3 )
   {
      m_minValue.reset(  new PrmSourceRockTOC( this, -1.0, lrName, mapRng[0], srTypeName, m_mixID ) );
      m_maxValue.reset(  new PrmSourceRockTOC( this,  1.0, lrName, mapRng[1], srTypeName, m_mixID ) );
      m_baseValue.reset( new PrmSourceRockTOC( this,  0.0, lrName, mapRng[2], srTypeName, m_mixID ) );
   }
   else
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong range is given for the TOC IP";
   }

   // add given range to map
   if ( !m_srTypeName.empty() ) { addSourceRockTypeRange( srTypeName, m_baseValue, m_minValue, m_maxValue, pdfType ); }
}

std::vector<std::string> VarPrmSourceRockTOC::name() const
{
  std::vector<std::string> ret;

   if ( m_name.empty() ) { ret.push_back( m_layerName + " TOC [%]" ); }
   else                  { ret.push_back( m_name ); }

  return ret;
}
// Save all object data to the given stream, that object could be later reconstructed from saved data
bool VarPrmSourceRockTOC::save( CasaSerializer & sz ) const
{
   bool ok = VarPrmSourceRockProp::serializeCommonPart( sz );
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

SharedParameterPtr VarPrmSourceRockTOC::createNewPrm( double val, const std::string & srType ) const
{
   // special case when we deserializing the old state file and we needing to create simple case parameter
   if ( m_srTypeName != srType )
   {
      SharedParameterPtr prm( new PrmSourceRockTOC( this, val, m_layerName.c_str(), std::string(), srType.empty() ? 0 : srType.c_str(), m_mixID ) );
      return prm;
   }

   PrmSourceRockTOC * minPrm = dynamic_cast<PrmSourceRockTOC*>( m_minValue.get()  );
   PrmSourceRockTOC * maxPrm = dynamic_cast<PrmSourceRockTOC*>( m_maxValue.get()  );
   PrmSourceRockTOC * basPrm = dynamic_cast<PrmSourceRockTOC*>( m_baseValue.get() );

   double minV = minPrm->value();
   double maxV = maxPrm->value();
   double basV = basPrm->value();

   if ( minV > val || val > maxV )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Variation of initial TOC parameter " << val <<
                                                                        " falls out of range: [" << minV << ":" << maxV << "]";
   }

   if ( minPrm->mapName().empty() ) // no maps defined, simple range - just use given value
   {
      SharedParameterPtr prm( new PrmSourceRockTOC( this, val, m_layerName.c_str(), std::string(),
                                                    (m_srTypeName.empty() ? 0 : m_srTypeName.c_str()), m_mixID ) );
      return prm;
   }

   // maps range case. First check from low/middle/max case that we will not duplicate maps
   if ( val == minV ) return m_minValue;
   if ( val == maxV ) return m_maxValue;
   if ( val == basV ) return m_baseValue;

   PrmSourceRockTOC * prm = new PrmSourceRockTOC( this, val, m_layerName.c_str(), std::string(),
                                                  (m_srTypeName.empty() ? 0 : m_srTypeName.c_str()), m_mixID );

   if ( val < basV ) {  prm->defineMapsRange( minPrm->mapName(), basPrm->mapName() ); }
   else              {  prm->defineMapsRange( basPrm->mapName(), maxPrm->mapName() ); }

   return SharedParameterPtr( prm );
}

SharedParameterPtr VarPrmSourceRockTOC::createNewPrmFromModel( mbapi::Model & mdl ) const
{
   SharedParameterPtr prm( new PrmSourceRockTOC( mdl, m_layerName.c_str(), ( m_srTypeName.empty() ? 0 : m_srTypeName.c_str() ), m_mixID ) );
   return prm;
}


} // namespace casa
