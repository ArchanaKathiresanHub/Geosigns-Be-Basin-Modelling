//
// Copyright (C) 2012-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmLithoSTPThermalCond.cpp
/// @brief This file keeps API implementation for STP thermal conductivity coefficient parameter

// CASA API
#include "PrmLithoSTPThermalCond.h"

// CMB API
#include "cmbAPI.h"

// Utilities lib
#include "NumericFunctions.h"

// STL/C lib
#include <cassert>
#include <cmath>
#include <sstream>
#include <vector>

namespace casa
{
    using namespace std;

PrmLithoSTPThermalCond::PrmLithoSTPThermalCond(mbapi::Model & mdl, const std::vector<std::string> parameters )
   : Parameter()
   , m_lithoName( parameters[0] )
   , m_value( Utilities::Numerical::IbsNoDataValue )

{
   try
   {
      mbapi::LithologyManager & mgr = mdl.lithologyManager();

      bool isFound = false;

      // go over all lithologies and look for the first lithology with the same name as given
      const std::vector<mbapi::LithologyManager::LithologyID> & lIDs = mgr.lithologiesIDs();
      for ( size_t i = 0; i < lIDs.size() && !isFound; ++i )
      {
         std::string lName = mgr.lithologyName( lIDs[i] );

         if ( lName != m_lithoName ) { continue; }

         m_value  = mgr.stpThermalConductivityCoeff( lIDs[i] );
         if ( ErrorHandler::NoError != mgr.errorCode() ) { throw ErrorHandler::Exception( mgr.errorCode() ) << mgr.errorMessage(); }
         isFound = true;
      }

      if ( !isFound )
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) <<  "Can't find lithology type with name " <<
            m_lithoName << " in lithologies type list";
      }
   }
   catch ( const ErrorHandler::Exception & e )
   {
      mdl.reportError( e.errorCode(), e.what() );
   }
}

// // Constructor
PrmLithoSTPThermalCond::PrmLithoSTPThermalCond( const VarParameter * parent, const char * lithoName, double val )
   : Parameter(     parent )
   , m_lithoName(   lithoName )
   , m_value(       val )
{
}

PrmLithoSTPThermalCond::PrmLithoSTPThermalCond(const VarParameter* parent, const PrmLithoSTPThermalCond& param, const double val) :
  PrmLithoSTPThermalCond(param)
{
  setParent(parent);
  m_value = val;
}

// Update given model with the parameter value
ErrorHandler::ReturnCode PrmLithoSTPThermalCond::setInModel( mbapi::Model & caldModel, size_t /* caseID */ )
{
   try
   {
      mbapi::LithologyManager & mgr = caldModel.lithologyManager();

      bool found = false;
      // go over all lithologies to find the lithology type with given name and set up porosity model
      const std::vector<mbapi::LithologyManager::LithologyID> & ids = mgr.lithologiesIDs();
      for ( size_t i = 0; i < ids.size() && !found; ++i )
      {
         if ( mgr.lithologyName( ids[i] ) == m_lithoName )
         {
            if ( ErrorHandler::NoError != mgr.setSTPThermalConductivityCoeff( ids[i], m_value ) )
            {
               throw ErrorHandler::Exception( mgr.errorCode() ) << mgr.errorMessage();
            }
            found = true;;
         }
      }
      if ( !found ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Can not find lithology with name: " << m_lithoName;
   }
   catch ( const ErrorHandler::Exception & e ) { return caldModel.reportError( e.errorCode(), e.what() ); }

   return ErrorHandler::NoError;
}

// Validate all porosity model parameters
std::string PrmLithoSTPThermalCond::validate( mbapi::Model & caldModel )
{
   std::ostringstream oss;

   if ( m_value < 0   ) oss << "STP thermal conductivity coeff. for lithology " << m_lithoName << " can not be negative: "     << m_value << std::endl;
   if ( m_value > 20  ) oss << "STP thermal conductivity coeff. for lithology " << m_lithoName << " can not be more than 20: " << m_value << std::endl;

   PrmLithoSTPThermalCond prm( caldModel, {m_lithoName} );

   if ( prm != *this )
   {
      oss << "STP thermal conductivity coeff. for lithology " << m_lithoName << " defined in project: " << prm.m_value <<
          " , is different from the parameter value: " << m_value << std::endl;
   }
   return oss.str();
}

// Get parameter value as an array of doubles
std::vector<double> PrmLithoSTPThermalCond::asDoubleArray() const
{
   std::vector<double> vals( 1, m_value );

   return vals;
}


// Are two parameters equal?
bool PrmLithoSTPThermalCond::operator == ( const Parameter & prm ) const
{
   const PrmLithoSTPThermalCond * pp = dynamic_cast<const PrmLithoSTPThermalCond *>( &prm );
   if ( !pp ) return false;

   const double eps = 1.e-6;

   if ( m_lithoName != pp->m_lithoName ) return false;
   if ( !NumericFunctions::isEqual( m_value, pp->m_value,  eps ) ) return false;

   return true;
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmLithoSTPThermalCond::save( CasaSerializer & sz ) const
{
   bool ok = saveCommonPart(sz);
   ok = ok && sz.save( m_lithoName, "LithologyName"          );
   ok = ok && sz.save( m_value,     "STPThermoCondCoefValue" );
   return ok;
}

std::vector<string> PrmLithoSTPThermalCond::parameters() const
{
  return {m_lithoName};
}

// Create a new var.parameter instance by deserializing it from the given stream
PrmLithoSTPThermalCond::PrmLithoSTPThermalCond( CasaDeserializer & dz, unsigned int  objVer ) :
  Parameter(dz, objVer)
{
   bool ok = dz.load( m_lithoName, "LithologyName");
   ok = ok && dz.load( m_value,     "STPThermoCondCoefValue");
   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "PrmLithoSTPThermalCond deserialization unknown error";
   }
}

void PrmLithoSTPThermalCond::loadVarPrm(CasaDeserializer& dz, const unsigned int objVer)
{
  bool ok = true;

  if (objVer < 1)
  {
    std::string tmp;
    ok = ok && dz.load( tmp, "lithoName");
  }

  if (!ok)
  {
    throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
       << "PrmLithoSTPThermalCond deserialization error";
  }
}

size_t PrmLithoSTPThermalCond::expectedParametersNumber()
{
  return 1;
}

size_t PrmLithoSTPThermalCond::optionalParametersNumber()
{
  return 0;
}

std::string PrmLithoSTPThermalCond::key()
{
  return "LithotypeIoTbl:StpThCond";
}

std::string PrmLithoSTPThermalCond::varprmName()
{
  return "VarPrmLithoSTPThermalCond";
}

std::string PrmLithoSTPThermalCond::description()
{
  return "a variation of STP (Standard P & T) thermal conductivity coefficient for the given lithology";
}

std::string PrmLithoSTPThermalCond::fullDescription()
{
  std::ostringstream oss;

  oss << "    \"" << key() << "\" <lithologyName> <minVal> <maxVal> <prmPDF>\n";
  oss << "    Where:\n";
  oss << "       lithologyName - lithology name\n";
  oss << "       minVal        - the parameter minimal range scale factor value\n";
  oss << "       maxVal        - the parameter maximal range scale factor value\n";
  oss << "       prmPDF        - the parameter probability density function type, the value could be one of the following:\n";
  oss << "                \"Block\"    - uniform probability between min and max values,\n";
  oss << "                \"Triangle\" - triangle shape probability function. The top triangle value is taken from the base case\n";
  oss << "                \"Normal\"   - normal (or Gaussian) probability function. The position of highest value is taken from the base case\n";

  return oss.str();
}

std::string PrmLithoSTPThermalCond::helpExample(const char* cmdName)
{
  std::ostringstream oss;
  oss << "    #       VarPrmName             LithName           min max  Parameter PDF\n";
  oss << "    " << cmdName << " \"" << varprmName() << "\"  \"SM.Mudstone40%Clay\" 2   4   \"Block\"\n";
  return oss.str();
}

} // namespace casa

