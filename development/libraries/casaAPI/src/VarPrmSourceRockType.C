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

namespace casa
{

VarPrmSourceRockType::VarPrmSourceRockType( const char                                                * layerName 
                                          , const std::string                                         & baseVal 
                                          , const std::vector<std::string>                            & variation 
                                          , const std::vector<double>                                 & weights
                                          )
{ 
   assert( variation.size() == weights.size() );

   m_baseVal = variation.size() + 1;
   m_weights.assign( weights.begin(), weights.end() );

   for ( size_t i = 0; i < variation.size(); ++i )
   {
      SharedParameterPtr prm( new PrmSourceRockType( this, layerName, variation[i] ) );
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

VarPrmSourceRockType::~VarPrmSourceRockType()
{
   ;
}

std::vector<std::string> VarPrmSourceRockType::name() const
{
   std::vector<std::string> ret;

   if ( !m_variation.empty() )
   {
      const PrmSourceRockType * prm = dynamic_cast<const PrmSourceRockType*>( m_variation.front().get() );
      if ( prm )
      {
         std::ostringstream oss;
         oss << prm->layerName() + " Source rock type (" << m_variation.size() << ")";
	      ret.push_back( oss.str() );
      }
   }
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

// Save all object data to the given stream, that object could be later reconstructed from saved data
bool VarPrmSourceRockType::save( CasaSerializer & sz, unsigned int version ) const
{
   bool ok = VarPrmCategorical::save( sz, version );

   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
VarPrmSourceRockType::VarPrmSourceRockType( CasaDeserializer & dz, unsigned int objVer ) : VarPrmCategorical( dz, objVer )
{
   bool ok = true;

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "VarPrmSourceRockType deserialization unknown error";
   }
}

}
