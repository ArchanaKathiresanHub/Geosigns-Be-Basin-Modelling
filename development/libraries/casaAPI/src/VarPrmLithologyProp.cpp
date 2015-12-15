//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmLithologyProp.h
/// @brief This file keeps base class methods definition for any lithology property implemented as variable parameter

// CASA
#include "VarPrmLithologyProp.h"

// CMB API
#include "cmbAPI.h"

// C lib
#include <cassert>
#include <cstring>
#include <cmath>

namespace casa
{

VarPrmLithologyProp::VarPrmLithologyProp( const std::vector<std::string> & lithosName
                                        , PDF                              prmPDF
                                        , const std::string              & name
                                        ) 
                                        : m_lithosName( lithosName.begin(), lithosName.end() )
{
   m_pdf = prmPDF;
   m_name = name;
}

std::vector<std::string> VarPrmLithologyProp::name() const
{
   return std::vector<std::string>( 1, (m_name.empty() ? m_propName : m_name) );
}

SharedParameterPtr VarPrmLithologyProp::newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const
{
   double minProp  = dynamic_cast<PrmLithologyProp*>( m_minValue.get( ) )->value();
   double maxProp  = dynamic_cast<PrmLithologyProp*>( m_maxValue.get( ) )->value();
   double baseProp = dynamic_cast<PrmLithologyProp*>( m_baseValue.get() )->value();

   double prmV = *vals++;

   if ( minProp > prmV || prmV > maxProp )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Variation of lithology property " << m_propName << 
         " for lithologies: " << m_lithosName.front() << ",... : " << prmV << " falls out of range: [" << minProp << ":" << maxProp << "]";
   }

   SharedParameterPtr prm( createNewPrm( prmV ) );

   return prm;
}

std::vector<double> VarPrmLithologyProp::asDoubleArray( const SharedParameterPtr prm ) const 
{
   const PrmLithologyProp * litPrm = dynamic_cast<const PrmLithologyProp*>( prm.get() );
   assert( litPrm );

   return std::vector<double>( 1, litPrm->value() );
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool VarPrmLithologyProp::serializeCommonPart( CasaSerializer & sz, unsigned int version ) const
{
   bool ok = VarPrmContinuous::save( sz, version );
   ok = ok ? sz.save( m_lithosName, "LithologiesList" ) : ok;
   ok = ok ? sz.save( m_propName,   "propName"        ) : ok;
   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
bool VarPrmLithologyProp::deserializeCommonPart( CasaDeserializer & dz, unsigned int objVer )
{
   bool ok = VarPrmContinuous::deserializeCommonPart( dz, objVer );
   ok = ok ? dz.load( m_lithosName, "LithologiesList" ) : ok;
   ok = ok ? dz.load( m_propName,   "propName"        ) : ok;
   return ok;
}


} // namespace casa
