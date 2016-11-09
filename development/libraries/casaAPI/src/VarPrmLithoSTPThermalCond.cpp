//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmLithoSTPThermalCond.h
/// @brief This file keeps API implementation for handling variation of STP thermal conductivity coefficient for the given lithology 

#include "PrmLithoSTPThermalCond.h"
#include "VarPrmLithoSTPThermalCond.h"

#include <cassert>
#include <cstring>


namespace casa
{

VarPrmLithoSTPThermalCond::VarPrmLithoSTPThermalCond( const char * lithoName
                                                    , double       baseValue
                                                    , double       minValue
                                                    , double       maxValue
                                                    , PDF          pdfType
                                                    , const char * name
                                                    ) : m_lithoName( lithoName )
{
   m_pdf  = pdfType;
   m_name = name && strlen( name ) > 0 ? std::string( name ) : std::string( "" );

   assert( minValue <= baseValue && maxValue >= baseValue );

   m_minValue.reset( new PrmLithoSTPThermalCond( this, lithoName, minValue ) );
   m_maxValue.reset( new PrmLithoSTPThermalCond( this, lithoName, maxValue ) );
   
   m_baseValue.reset( new PrmLithoSTPThermalCond( this, lithoName, baseValue ) );
}

VarPrmLithoSTPThermalCond::~VarPrmLithoSTPThermalCond()
{
   ;
}

std::vector<std::string> VarPrmLithoSTPThermalCond::name() const
{
	std::vector<std::string> ret;
	ret.push_back( m_lithoName + " STP Thermal Cond. Coeff [W/m/K]" );
	return ret;
}

SharedParameterPtr VarPrmLithoSTPThermalCond::newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const
{
   double minV = dynamic_cast<PrmLithoSTPThermalCond*>( m_minValue.get() )->asDoubleArray()[0];
   double maxV = dynamic_cast<PrmLithoSTPThermalCond*>( m_maxValue.get() )->asDoubleArray()[0];

   double prmV = *vals++;

   if ( minV > prmV || prmV > maxV )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Variation of STP thermal cond. coeff. parameter for lithology " <<
         m_lithoName << ": " << prmV << " falls out of range: [" << minV << ":" << maxV << "]";
   }
   SharedParameterPtr prm( new PrmLithoSTPThermalCond( this, m_lithoName.c_str(), prmV ) );

   return prm;
}

SharedParameterPtr VarPrmLithoSTPThermalCond::newParameterFromModel( mbapi::Model & mdl, const std::vector<double> & /*vin*/ ) const
{
   SharedParameterPtr prm( new PrmLithoSTPThermalCond( mdl, m_lithoName.c_str( ) ) );
   prm->setParent( const_cast<VarPrmLithoSTPThermalCond *>( this ) );
   return prm;
}

SharedParameterPtr VarPrmLithoSTPThermalCond::makeThreeDFromOneD( mbapi::Model                          & // mdl
                                                                , const std::vector<double>             & // xin
                                                                , const std::vector<double>             & // yin
                                                                , const std::vector<SharedParameterPtr> & // prmVec 
                                                                ) const
{
   // Not yet implemented
   throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << "makeThreeDFromOneD method not yet implemented for VarPrmLithoSTPThermalCond";
   return nullptr;
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool VarPrmLithoSTPThermalCond::save( CasaSerializer & sz, unsigned int version ) const
{
   bool ok = VarPrmContinuous::save( sz, version );
   ok = ok ? sz.save( m_lithoName, "lithoName" ) : ok;

   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
VarPrmLithoSTPThermalCond::VarPrmLithoSTPThermalCond( CasaDeserializer & dz, unsigned int objVer ) 
{
   bool ok = VarPrmContinuous::deserializeCommonPart( dz, objVer );

   ok = ok ? dz.load( m_lithoName, "lithoName" ) : ok;

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "VarPrmLithoSTPThermalCond deserialization unknown error";
   }
}

}
