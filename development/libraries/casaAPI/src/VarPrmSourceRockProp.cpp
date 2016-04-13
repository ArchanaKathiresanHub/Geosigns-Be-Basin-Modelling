//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmSourceRockProp.h
/// @brief This file keeps API implementation for handling variation of initial source rock Prop parameter. 

#include "PrmSourceRockType.h"
#include "PrmSourceRockProp.h"
#include "VarPrmSourceRockProp.h"

#include <cassert>
#include <cstring>

namespace casa
{

VarPrmSourceRockProp::VarPrmSourceRockProp()
{
   m_mixID = 1;
   m_pdf = Block;
}

VarPrmSourceRockProp::VarPrmSourceRockProp( const char * layerName
                                          , double       baseValue
                                          , double       minValue
                                          , double       maxValue
                                          , PDF          pdfType
                                          , const char * name
                                          , const char * srTypeName
                                          , int          mixID
                                          )
                                          : m_layerName( layerName )
                                          , m_mixID( mixID )
                                          , m_srTypeName( srTypeName ? srTypeName : "" )
{
   m_pdf = pdfType;
   m_name = name && strlen( name ) > 0 ? std::string( name ) : std::string( "" );

   assert( minValue <= baseValue && maxValue >= baseValue );

   // if this parameter is intendent to be dependend on source rock type, store range in map
   if ( !m_srTypeName.empty() )
   {
      std::vector<double> vec;
      vec.push_back( minValue );
      vec.push_back( maxValue );
      vec.push_back( baseValue );
      m_name2range[ m_srTypeName ] = vec;
   }
}


SharedParameterPtr VarPrmSourceRockProp::newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const
{
   double minProp  = dynamic_cast<PrmSourceRockProp*>( m_minValue.get( ) )->value();
   double maxProp  = dynamic_cast<PrmSourceRockProp*>( m_maxValue.get( ) )->value();
   double baseProp = dynamic_cast<PrmSourceRockProp*>( m_baseValue.get() )->value();

   double prmV = *vals++; // in [-1:0:1] range

   // 0.0 is the base value for [-1:1] range
   prmV = prmV < 0.0 ? minProp  + (prmV + 1.0) * (baseProp - minProp) : // [minV, baseV] range
                       baseProp + prmV * ( maxProp  - baseProp);  // [baseV, maxV] range

   // scale prmV from [-1:1] to the current Prop range - [minProp:maxProp]

   if ( minProp > prmV || prmV > maxProp )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Variation of source rock " << m_propName << " parameter for layer " <<
         m_layerName << ": " << prmV << " falls out of range: [" << minProp << ":" << maxProp << "]";
   }

   SharedParameterPtr prm( createNewPrm( prmV ) );

   return prm;
}

std::vector<double> VarPrmSourceRockProp::asDoubleArray( const SharedParameterPtr prm ) const 
{
   const PrmSourceRockProp * tocPrm = dynamic_cast<const PrmSourceRockProp*>( prm.get() );
   assert( tocPrm );

   double tocVal = tocPrm->value();

   double minV = dynamic_cast<PrmSourceRockProp*>( m_minValue.get( ) )->value();
   double maxV = dynamic_cast<PrmSourceRockProp*>( m_maxValue.get( ) )->value();
   double basV = dynamic_cast<PrmSourceRockProp*>( m_baseValue.get() )->value();

   if ( !m_name.empty() && !tocPrm->sourceRockTypeName().empty() && m_name2range.count( tocPrm->sourceRockTypeName() ) > 0 )
   {  
      const std::vector<double> & rng = m_name2range.find( tocPrm->sourceRockTypeName() )->second;
      minV = rng[0];
      maxV = rng[1];
      basV = rng[2];
   }

   std::vector<double> ret;
   // scale to range [-1:0:1]
   ret.push_back( tocVal < basV ? -1.0 + (tocVal - minV)/(basV-minV) * (0.0 - (-1.0)) :
                                   0.0 + (tocVal - basV)/(maxV-basV) * (1.0 - 0.0 ) );
   return ret;
}

void VarPrmSourceRockProp::addSourceRockTypeRange( const char * srTypeName, double baseVal, double minVal, double maxVal, PDF pdfType )
{
   if ( !srTypeName )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Empty source rock type name for new " << m_propName << 
         " range for: " << name();
   }

   if ( m_name2range.empty() )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << m_propName << " parameter was defined before without dependency on " <<
         "source rock type, you can't mix dependent and independent Prop" << name();
   }

   if ( m_name2range.count( std::string( srTypeName ) ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Duplicate source rock type name: " << srTypeName << 
         " for new " << m_propName << " range for: " << name();
   }

   if ( pdfType != m_pdf )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Different PDF for source rock type name: " << srTypeName << 
         " for new " << m_propName << " range for: " << name();
   }

   assert( minVal <= baseVal && maxVal >= baseVal );
   std::vector<double> vec;

   vec.push_back( minVal );
   vec.push_back( maxVal );
   vec.push_back( baseVal );
   
   m_name2range[std::string( srTypeName )] = vec;
}

// Save all object data to the given stream, that object could be later reconstructed from saved data
bool VarPrmSourceRockProp::serializeCommonPart( CasaSerializer & sz, unsigned int version ) const
{
   bool ok = VarPrmContinuous::save( sz, version );
   ok = ok ? sz.save( m_layerName, "layerName" ) : ok;

   ok = ok ? sz.save( m_mixID,      "mixingID"   ) : ok;
   ok = ok ? sz.save( m_srTypeName, "srTypeName" ) : ok;

   size_t numRanges = m_name2range.size();
   ok = ok ? sz.save( numRanges, "numPropRanges" ) : ok;
   for ( std::map<std::string, std::vector<double> >::const_iterator it = m_name2range.begin(); it != m_name2range.end(); ++it )
   {
      const std::string & key = it->first;
      const std::vector<double> & vec = it->second;
      ok = ok ? sz.save( key, "srTypeNameKey"    ) : ok;
      ok = ok ? sz.save( vec, "valPropRange"     ) : ok;
   } 
   
   ok = ok ? sz.save( m_propName, "propName"     ) : ok;
   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
bool VarPrmSourceRockProp::deserializeCommonPart( CasaDeserializer & dz, unsigned int objVer )
{
   bool ok = VarPrmContinuous::deserializeCommonPart( dz, objVer );
   ok = ok ? dz.load( m_layerName, "layerName" ) : ok;

   ok = ok ? dz.load( m_mixID,        "mixingID" ) : ok;
   ok = ok ? dz.load( m_srTypeName, "srTypeName" ) : ok;

   size_t numRanges = 0;
   ok = ok ? dz.load( numRanges, "numPropRanges" ) : ok;

   for ( size_t i = 0; ok && i < numRanges; ++i )
   {
      std::string         key;
      std::vector<double> vec;   // load min, max & base range values

      ok = ok ? dz.load( key, "srTypeNameKey" ) : ok;
      ok = ok ? dz.load( vec, "valPropRange"  ) : ok;

      if ( ok ) { m_name2range[key] = vec; } // add property range
   }
   ok = ok ? dz.load( m_propName, "propName"    ) : ok;
   return ok;
}

void VarPrmSourceRockProp::onCategoryChosen( const Parameter * prm )
{
   const PrmSourceRockType * srtPrm = dynamic_cast<const PrmSourceRockType *>( prm );
   if ( !srtPrm ) return;

   const std::string & srName = srtPrm->sourceRockTypeName();
   const std::string & lrName = srtPrm->layerName();
   
   if ( srName == m_srTypeName ) return;

   if ( lrName != m_layerName )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << 
         "Mismatch layer name for the source rock type parameter (" << lrName << ") and " <<
         m_propName << " (" << m_layerName << ") parameter";
   }

   if ( !m_name2range.count( srName ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Unknown source rock type in dependent " << m_propName << 
         " parameter: " <<  srName << " for the layer: " << lrName;
   }
   const std::vector<double> & vec = m_name2range[srName];

   // Switch to the new range
   m_srTypeName = srName;

   dynamic_cast<PrmSourceRockProp*>( m_minValue.get( ) )->update( vec[0], srName );
   dynamic_cast<PrmSourceRockProp*>( m_maxValue.get( ) )->update( vec[1], srName );
   dynamic_cast<PrmSourceRockProp*>( m_baseValue.get() )->update( vec[2], srName );
}


} // namespace casa
