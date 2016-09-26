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
                                          , PDF          pdfType
                                          , const char * name
                                          , const char * srTypeName
                                          , int          mixID
                                          )
                                          : m_layerName( layerName )
                                          , m_mixID( mixID )
                                          , m_srTypeName( srTypeName ? srTypeName : "" )
{
   m_pdf  = pdfType;
   m_name = name && strlen( name ) > 0 ? std::string( name ) : std::string( "" );
}


SharedParameterPtr VarPrmSourceRockProp::newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const
{
   double minProp  = dynamic_cast<PrmSourceRockProp*>( m_minValue.get( ) )->value();
   double maxProp  = dynamic_cast<PrmSourceRockProp*>( m_maxValue.get( ) )->value();
   double baseProp = dynamic_cast<PrmSourceRockProp*>( m_baseValue.get() )->value();

   double prmV = *vals++; // in [-1:0:1] range

   // scale to range [-1:0:1] if variable parameter has more than 1 range
   if ( m_name2range.size() > 1 ) 
   {  
      // scale prmV from [-1:1] to the current Prop range - [minProp:maxProp]
      // 0.0 is the base value for [-1:1] range
      prmV = prmV < 0.0 ? minProp  + (prmV + 1.0) * (baseProp - minProp) : // [minV, baseV] range
                          baseProp + prmV * ( maxProp  - baseProp);  // [baseV, maxV] range
   }

   if ( minProp > prmV || prmV > maxProp )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Variation of source rock " << m_propName << " parameter for layer " <<
         m_layerName << ": " << prmV << " falls out of range: [" << minProp << ":" << maxProp << "]";
   }

   SharedParameterPtr prm( createNewPrm( prmV, m_srTypeName ) );

   return prm;
}

SharedParameterPtr VarPrmSourceRockProp::newParameterFromModel( mbapi::Model & mdl, const std::vector<double> & /* vin */ ) const
{
   SharedParameterPtr prm( createNewPrmFromModel( mdl ) );
   prm->setParent( const_cast<VarPrmSourceRockProp *>( this ) );
   return prm;
}

SharedParameterPtr VarPrmSourceRockProp::makeThreeDFromOneD( mbapi::Model                          & /* mdl    */
                                                           , const std::vector<double>             & /* xin    */
                                                           , const std::vector<double>             & /* yin    */
                                                           , const std::vector<SharedParameterPtr> & /* prmVec */ ) const
{
   // Not yet implemented
   throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << "makeThreeDFromOneD method not yet implemented for VarPrmSourceRockProp";
   return nullptr;
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
      const std::vector<SharedParameterPtr> & rng = m_name2range.find( tocPrm->sourceRockTypeName() )->second;
      minV = dynamic_cast<PrmSourceRockProp*>( rng[0].get() )->value();
      maxV = dynamic_cast<PrmSourceRockProp*>( rng[1].get() )->value();
      basV = dynamic_cast<PrmSourceRockProp*>( rng[2].get() )->value();
   }

   std::vector<double> ret;
   if ( m_name2range.size() > 1 )
   {  // scale to range [-1:0:1] if variable parameter has more than 1 range
      ret.push_back( tocVal < basV ? -1.0 + (tocVal - minV)/(basV-minV) * (0.0 - (-1.0)) :
                                      0.0 + (tocVal - basV)/(maxV-basV) * (1.0 - 0.0 ) );
   }
   else { ret.push_back( tocVal ); } // if just one simple range without source rock type parameter - do not scale it

   return ret;
}

void VarPrmSourceRockProp::addSourceRockTypeRange( const char         * srTypeName
                                                 , SharedParameterPtr   baseVal   
                                                 , SharedParameterPtr   minVal    
                                                 , SharedParameterPtr   maxVal    
                                                 , PDF                  pdfType   
                                                 )
{  
   ErrorHandler::Exception ex( ErrorHandler::OutOfRangeValue );

   if ( !srTypeName ) { throw ex << "Empty source rock type name for new " << m_propName << " range for: " << name(); }

   if ( m_name2range.count( std::string( srTypeName ) ) )
   {
      throw ex << "Duplicate source rock type name: " << srTypeName << " for new " << m_propName << " range for: " << name();
   }

   if ( pdfType != m_pdf )
   {
      throw ex << "Different PDF for source rock type name: " << srTypeName << " for new " << m_propName << " range for: " << name();
   }

   std::vector<SharedParameterPtr> vec;

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
   for ( auto it = m_name2range.begin(); it != m_name2range.end(); ++it )
   {
      const std::string & key = it->first;
      const std::vector<SharedParameterPtr> & vec = it->second;

      ok = ok ? sz.save( key, "srTypeNameKey"    ) : ok;
      if ( key != m_srTypeName ) // this set of min/max/base values alrady saved in VarPrmContinuous serialization
      {
         ok = ok ? sz.save( *(vec[0].get()), "minValPropRange" ) : ok;
         ok = ok ? sz.save( *(vec[1].get()), "maxValPropRange" ) : ok;
         ok = ok ? sz.save( *(vec[2].get()), "basValPropRange" ) : ok;
      }
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

      ok = ok ? dz.load( key, "srTypeNameKey" ) : ok;
      
      std::vector<SharedParameterPtr > vec;   // load min, max & base range values

      if ( objVer > 0 ) // load ranges as min/max/base parameters objects
      {
         if ( key != m_srTypeName )
         {
            for ( size_t j = 0; j < 3 && ok; ++j )
            {
               switch( j ) 
               {
                  case 0: vec.push_back( SharedParameterPtr( Parameter::load( dz, "minValPropRange" ) ) ); break;
                  case 1: vec.push_back( SharedParameterPtr( Parameter::load( dz, "maxValPropRange" ) ) ); break;
                  case 2: vec.push_back( SharedParameterPtr( Parameter::load( dz, "basValPropRange" ) ) ); break;
               }
               ok = vec.back() ? true : false;
            }
         }
         else
         {
            vec.push_back( m_minValue );
            vec.push_back( m_maxValue );
            vec.push_back( m_baseValue );
         }
      }
      else // load old style min/max/base range as double values
      {
         std::vector<double> dvec;   // load min, max & base range values
         ok = ok ? dz.load( dvec, "valPropRange"  ) : ok;

         if ( ok )
         {
            if ( key != m_srTypeName )
            {
               for ( auto v : dvec ) { vec.push_back( SharedParameterPtr( createNewPrm( v, key ) ) ); }
            }
            else
            {
               vec.push_back( m_minValue );
               vec.push_back( m_maxValue );
               vec.push_back( m_baseValue );
            }
         }
      }
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
   const std::vector<SharedParameterPtr> & vec = m_name2range[srName];

   // Switch to the new range
   m_srTypeName = srName;

   m_minValue  = vec[0];
   m_maxValue  = vec[1];
   m_baseValue = vec[2];
}


} // namespace casa
