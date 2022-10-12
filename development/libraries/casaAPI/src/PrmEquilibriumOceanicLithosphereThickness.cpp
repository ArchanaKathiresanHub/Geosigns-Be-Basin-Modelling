//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmEquilibriumOceanicLithosphereThickness.cpp
/// @brief This file keeps API implementation for handling Equilibrium oceanic lithosphere thickness basement parameter.

// CASA API
#include "PrmEquilibriumOceanicLithosphereThickness.h"

// CMB API
#include "cmbAPI.h"
#include "ErrorHandler.h"
#include "UndefinedValues.h"

// utilities lib
#include <NumericFunctions.h>

// STL/C lib
#include <cassert>
#include <cmath>
#include <sstream>

namespace casa
{
    using namespace std;

static const char* s_basementTblName      = "BasementIoTbl";
static const char* s_eqOceanLitThick      = "EquilibriumOceanicLithosphereThickness";

namespace
{

const double valueTolerance = 1.e-6;

}  //namespace

// Update given model with the parameter value
PrmEquilibriumOceanicLithosphereThickness::PrmEquilibriumOceanicLithosphereThickness(mbapi::Model& mdl,
                                                                                     const std::vector<string>& /*parameters*/) :
  Parameter(),
  m_value(mdl.tableValueAsDouble( s_basementTblName, 0, s_eqOceanLitThick ))
{
}

PrmEquilibriumOceanicLithosphereThickness::PrmEquilibriumOceanicLithosphereThickness(const VarParameter* parent,
                                                                                     const PrmEquilibriumOceanicLithosphereThickness& param,
                                                                                     const double val) :
  PrmEquilibriumOceanicLithosphereThickness( param )
{
  setParent(parent);
  m_value = val;
}

ErrorHandler::ReturnCode PrmEquilibriumOceanicLithosphereThickness::setInModel( mbapi::Model & cldModel, size_t /*caseID*/ )
{
  try
  {
    if ( ErrorHandler::NoError != cldModel.setTableValue( s_basementTblName, 0, s_eqOceanLitThick, m_value ) )
    {
      throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage();
    }
  }
  catch ( const ErrorHandler::Exception & e )
  {
    return cldModel.reportError( e.errorCode(), e.what() );
  }

  return ErrorHandler::NoError;
}

// Validate equilibrium oceanic lithosphere thickness value if it is in positive range\n
// also it checks does the given model has the same value for this parameter.
std::string PrmEquilibriumOceanicLithosphereThickness::validate( mbapi::Model & cldModel )
{
   std::ostringstream oss;

   if ( m_value < 0.0 )
   {
     oss << "Equilibrium oceanic lithosphere thickness value can not be negative: " << m_value << std::endl;
   }

   if ( IsValueUndefined(m_value) )
   {
     oss << "Equilibrium oceanic lithosphere thickness value can not be undefined" << std::endl;
   }

   const double valInModel = cldModel.tableValueAsDouble( s_basementTblName, 0, s_eqOceanLitThick );
   if ( ErrorHandler::NoError != cldModel.errorCode() )
   {
     oss << cldModel.errorMessage() << std::endl;
     return oss.str();
   }

   if ( !NumericFunctions::isEqual( valInModel, m_value, valueTolerance ) )
   {
      oss << "Equilibrium oceanic lithosphere thickness parameter value in the model (" << valInModel;
      oss << ") is different from a parameter value (" << m_value << ")" << std::endl;
   }

   return oss.str();
}

std::vector<double> PrmEquilibriumOceanicLithosphereThickness::asDoubleArray() const
{
  return {m_value};
}


// Are two parameters equal?
bool PrmEquilibriumOceanicLithosphereThickness::operator == ( const Parameter & prm ) const
{
   const PrmEquilibriumOceanicLithosphereThickness * pp = dynamic_cast<const PrmEquilibriumOceanicLithosphereThickness *>( &prm );
   if ( !pp ) return false;

   return NumericFunctions::isEqual( m_value, pp->m_value, valueTolerance );
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmEquilibriumOceanicLithosphereThickness::save( CasaSerializer & sz ) const
{
   bool ok = saveCommonPart(sz);
   ok = ok && sz.save( m_value, "equilibriumOceanicLithosphereThicknessValue" );
   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
PrmEquilibriumOceanicLithosphereThickness::PrmEquilibriumOceanicLithosphereThickness( CasaDeserializer & dz, unsigned int objVer )
  : Parameter(dz, objVer)
{
   bool ok = dz.load( m_value, "equilibriumOceanicLithosphereThicknessValue" );

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "PrmEquilibriumOceanicLithosphereThicknessValue deserialization unknown error";
   }
}

void PrmEquilibriumOceanicLithosphereThickness::loadVarPrm(CasaDeserializer& dz, const unsigned int objVer)
{
}

std::size_t PrmEquilibriumOceanicLithosphereThickness::expectedParametersNumber()
{
  return 0;
}

std::size_t PrmEquilibriumOceanicLithosphereThickness::optionalParametersNumber()
{
  return 0;
}

std::string PrmEquilibriumOceanicLithosphereThickness::key()
{
  return "BasementIoTbl:EquilibriumOceanicLithosphereThickness";
}

std::string PrmEquilibriumOceanicLithosphereThickness::varprmName()
{
  return "VarPrmEquilibriumOceanicLithosphereThickness";
}

std::string PrmEquilibriumOceanicLithosphereThickness::description()
{
  return "Equilibrium oceanic lithosphere thickness [m]";
}

std::string PrmEquilibriumOceanicLithosphereThickness::fullDescription()
{
  std::ostringstream oss;

  oss << "    \"" << key() << "\" <minVal> <maxVal> <prmPDF>\n";
  oss << "    Where:\n";
  oss << "       minVal     - the parameter minimal range scale factor value\n";
  oss << "       maxVal     - the parameter maximal range scale factor value\n";
  oss << "       prmPDF     - the parameter probability density function type, the value could be one of the following:\n";
  oss << "                \"Block\"    - uniform probability between min and max values,\n";
  oss << "                \"Triangle\" - triangle shape probability function. The top triangle value is taken from the base case\n";
  oss << "                \"Normal\"   - normal (or Gaussian) probability function. The position of highest value is taken from the base case\n";

  return oss.str();
}

std::string PrmEquilibriumOceanicLithosphereThickness::helpExample(const char* cmdName)
{
  std::ostringstream oss;
  oss << "    #                                      type               minVal  maxVal prmPDF\n";
  oss << "    " << cmdName << " \"" << key() << "\"    88000     97000  \"Block\"\n";
  oss << "\n";
  return oss.str();
}

}
