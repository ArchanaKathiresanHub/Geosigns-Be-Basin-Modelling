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

VarPrmTopCrustHeatProduction::VarPrmTopCrustHeatProduction( const std::vector<double>      & dblRng
                                                          , const std::vector<std::string> & mapRng
                                                          , PDF                              pdfType
                                                          , const char                     * name 
                                                          )
{
   m_pdf = pdfType;
   m_name = name && strlen( name ) > 0 ? std::string( name ) : std::string( "" );
  
   if ( dblRng.size() == 3 )
   {
      m_minValue.reset(  new PrmTopCrustHeatProduction( this, dblRng[0] ) );
      m_maxValue.reset(  new PrmTopCrustHeatProduction( this, dblRng[1] ) );
      m_baseValue.reset( new PrmTopCrustHeatProduction( this, dblRng[2] ) );
   }
   else if ( mapRng.size() == 3 )
   {
      m_minValue.reset(  new PrmTopCrustHeatProduction( this, -1, mapRng[0] ) );
      m_maxValue.reset(  new PrmTopCrustHeatProduction( this,  1, mapRng[1] ) );
      m_baseValue.reset( new PrmTopCrustHeatProduction( this,  0, mapRng[2] ) );
   }
   else
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong range is given for the TopCrustHeatProduction IP";
   }
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
   PrmTopCrustHeatProduction * minPrm = dynamic_cast<PrmTopCrustHeatProduction*>( m_minValue.get()  );
   PrmTopCrustHeatProduction * maxPrm = dynamic_cast<PrmTopCrustHeatProduction*>( m_maxValue.get()  );
   PrmTopCrustHeatProduction * basPrm = dynamic_cast<PrmTopCrustHeatProduction*>( m_baseValue.get() );

   double minV = minPrm->value();
   double maxV = maxPrm->value();
   double basV = basPrm->value();

   double valP = *vals++;

   if ( minV > valP || valP > maxV )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Variation of Top crust heat production rate parameter " << valP << 
                                                                        " falls out of range: [" << minV << ":" << maxV << "]";
   }
   
   if ( minPrm->mapName().empty() ) // no maps defined, simple range - just use given value
   {
      SharedParameterPtr prm( new PrmTopCrustHeatProduction( this, valP ) );
      return prm;
   }

   // maps range case. First check from low/middle/max case that we will not duplicate maps
   if ( valP == minV ) return m_minValue;
   if ( valP == maxV ) return m_maxValue;
   if ( valP == basV ) return m_baseValue;

   SharedParameterPtr newPrm( new PrmTopCrustHeatProduction( this, valP, basPrm->mapName() ) );

   PrmTopCrustHeatProduction * prm = dynamic_cast<PrmTopCrustHeatProduction*>( newPrm.get() );

   if ( valP < basV ) {  prm->defineMapsRange( minPrm->mapName(), basPrm->mapName() ); }
   else               {  prm->defineMapsRange( basPrm->mapName(), maxPrm->mapName() ); }

   return newPrm;
}

SharedParameterPtr VarPrmTopCrustHeatProduction::newParameterFromModel( mbapi::Model & mdl ) const
{
   return SharedParameterPtr( new PrmTopCrustHeatProduction( mdl ) );
}


SharedParameterPtr VarPrmTopCrustHeatProduction::makeThreeDFromOneD( mbapi::Model                          & mdl
                                                                   , const std::vector<double>             & xin
                                                                   , const std::vector<double>             & yin
                                                                   , const std::vector<SharedParameterPtr> & prmVeci
                                                                   ) const
{
   // Not yet implemented
   throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << "makeThreeDFromOneD method not yet implemented for VarPrmTopCrustHeatProduction";
   return nullptr;
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
