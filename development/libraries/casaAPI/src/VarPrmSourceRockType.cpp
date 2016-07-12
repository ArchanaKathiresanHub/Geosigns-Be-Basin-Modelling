//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmSourceRockType.h
/// @brief This file keeps API implementation for handling variation of source rock type for stratigraphy layer. 

#include "PrmSourceRockType.h"
#include "VarPrmSourceRockType.h"

// STL/C lib
#include <sstream>
#include <cassert>
#include <cstring>

namespace casa
{

VarPrmSourceRockType::VarPrmSourceRockType( const char                     * layerName 
                                          , const std::string              & baseVal 
                                          , int                              mixID
                                          , const std::vector<std::string> & variation 
                                          , const std::vector<double>      & weights
                                          , const char                     * name
                                          )
                                          : m_mixID ( mixID )
                                          , m_layerName( layerName )
{
   assert( variation.size() == weights.size() );

   m_baseVal = variation.size() + 1;
   m_weights.assign( weights.begin(), weights.end() );
   m_name = name && strlen( name ) > 0 ? std::string( name ) : std::string( "" );

   for ( size_t i = 0; i < variation.size(); ++i )
   {
      SharedParameterPtr prm( new PrmSourceRockType( this, layerName, variation[i], mixID ) );
      m_variation.push_back( prm );
      if ( variation[i] == baseVal )
      {
         m_baseVal = i;
      }
   }
   if ( m_baseVal > variation.size() )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << 
         "Variation of source rock parameter: Base value is not in the list";
   }
}

std::vector<std::string> VarPrmSourceRockType::name() const
{
   std::vector<std::string> ret;

   if ( m_name.empty() ) 
   {
      if ( !m_variation.empty() )
      {
         const PrmSourceRockType * prm = dynamic_cast<const PrmSourceRockType*>( m_variation.front().get() );
         if ( prm )
         {
            std::ostringstream oss;
            oss << prm->layerName() + " Source rock type [" << m_mixID << "](" << m_variation.size() << ")";
	         ret.push_back( oss.str() );
         }
      }
   }
   else { ret.push_back( m_name ); }

	return ret;
}

int VarPrmSourceRockType::index( const PrmSourceRockType * prm ) const
{
   for ( size_t i = 0; i < m_variation.size(); ++i )
   {
      if ( *prm == *(m_variation[i].get()) ) return static_cast<int>( i );
   }
   return -1;
}

// Create parameter by reading the values stored in the project file
SharedParameterPtr VarPrmSourceRockType::newParameterFromModel( mbapi::Model & mdl, const std::vector<double> & vin ) const
{
   std::unique_ptr<PrmSourceRockType> prm( new PrmSourceRockType( mdl, m_layerName, m_mixID ) );
   prm->setParent( this );

   for ( size_t i = 0; i < m_variation.size(); ++i )
   {
      if ( (*(m_variation[i].get())) == (*(prm.get()) )) { return m_variation[i]; }
   }
   throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Categorical value of source rock type: " << prm->sourceRockTypeName() 
                                                                  << "found in the model for the layer "  << m_layerName << ", mixing id: " 
                                                                  << m_mixID << ", does not exist in VarParameter variation";
}
 
// Save all object data to the given stream, that object could be later reconstructed from saved data
bool VarPrmSourceRockType::save( CasaSerializer & sz, unsigned int version ) const
{
   bool ok = VarPrmCategorical::save( sz, version );
   ok = ok ? sz.save( m_mixID, "mixingID" ) : ok;

   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
VarPrmSourceRockType::VarPrmSourceRockType( CasaDeserializer & dz, unsigned int objVer ) 
{
   bool ok = VarPrmCategorical::deserializeCommonPart( dz, objVer );

   ok = ok ? dz.load( m_mixID, "mixingID" ) : ok;

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "VarPrmSourceRockType deserialization unknown error";
   }
}

}
