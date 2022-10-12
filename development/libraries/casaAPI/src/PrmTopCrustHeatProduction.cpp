//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmTopCrustHeatProduction.h
/// @brief This file keeps API implementation for handling Top Crust Heat Production Rate basement parameter.

// CASA API
#include "PrmTopCrustHeatProduction.h"

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

static const char * s_basementTblName      = "BasementIoTbl";
static const char * s_bottomBoundaryModel  = "BottomBoundaryModel";
static const char * s_topCrustHeatProd     = "TopCrustHeatProd";
static const char * s_topCrustHeatProdGrid = "TopCrustHeatProdGrid";


// Update given model with the parameter value
PrmTopCrustHeatProduction::PrmTopCrustHeatProduction(mbapi::Model& mdl, const std::vector<string>& /*parameters*/) :
  Parameter(),
  m_value(mdl.tableValueAsDouble( s_basementTblName, 0, s_topCrustHeatProd ))
{
}

PrmTopCrustHeatProduction::PrmTopCrustHeatProduction(const VarParameter* parent, const PrmTopCrustHeatProduction& param, const double val) :
  PrmTopCrustHeatProduction( param )
{
  setParent(parent);
  m_value = val;
}

ErrorHandler::ReturnCode PrmTopCrustHeatProduction::setInModel( mbapi::Model & cldModel, size_t /*caseID*/ )
{
  try
  {
    if ( ErrorHandler::NoError != cldModel.setTableValue( s_basementTblName, 0, s_topCrustHeatProd, m_value ) )
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

// Validate top crust heat production rate value if it is in positive range\n
// also it checks does the given model has the same value for this parameter.
std::string PrmTopCrustHeatProduction::validate( mbapi::Model & cldModel )
{
   std::ostringstream oss;

   if ( m_value < 0.0 )
   {
     oss << "Top crust heat production rate value can not be negative: " << m_value << std::endl;
   }

   if ( IsValueUndefined(m_value) )
   {
     oss << "Top crust heat production rate value can not be undefined" << std::endl;
   }

   const double valInModel = cldModel.tableValueAsDouble( s_basementTblName, 0, s_topCrustHeatProd );
   if ( ErrorHandler::NoError != cldModel.errorCode() )
   {
     oss << cldModel.errorMessage() << std::endl;
     return oss.str();
   }

   if ( !NumericFunctions::isEqual( valInModel, m_value, 1e-6 ) )
   {
      oss << "Top crust heat production rate parameter value in the model (" << valInModel;
      oss << ") is differ from a parameter value (" << m_value << ")" << std::endl;
   }

   return oss.str();
}

std::vector<double> PrmTopCrustHeatProduction::asDoubleArray() const
{
  return {m_value};
}


// Are two parameters equal?
bool PrmTopCrustHeatProduction::operator == ( const Parameter & prm ) const
{
   const PrmTopCrustHeatProduction * pp = dynamic_cast<const PrmTopCrustHeatProduction *>( &prm );
   if ( !pp ) return false;

   return NumericFunctions::isEqual( m_value, pp->m_value, 1.e-6 );
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmTopCrustHeatProduction::save( CasaSerializer & sz ) const
{
   bool ok = saveCommonPart(sz);
   ok = ok && sz.save( m_value, "heatProdRateValue" );
   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
PrmTopCrustHeatProduction::PrmTopCrustHeatProduction( CasaDeserializer & dz, unsigned int objVer )
  : Parameter(dz, objVer)
{
   bool ok = dz.load( m_value, "heatProdRateValue" );

   if (objVer == 1)
   {
     std::string tmp;
     ok = ok && dz.load(tmp, "mapName");
     ok = ok && dz.load(tmp, "minMapName");
     ok = ok && dz.load(tmp, "maxMapName");
   }

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "PrmTopCrustHeatProduction deserialization unknown error";
   }
}

void PrmTopCrustHeatProduction::loadVarPrm(CasaDeserializer& dz, const unsigned int objVer)
{
}

std::size_t PrmTopCrustHeatProduction::expectedParametersNumber()
{
  return 0;
}

std::size_t PrmTopCrustHeatProduction::optionalParametersNumber()
{
  return 0;
}

std::string PrmTopCrustHeatProduction::key()
{
  return "BasementIoTbl:TopCrustHeatProd";
}

std::string PrmTopCrustHeatProduction::varprmName()
{
  return "VarPrmTopCrustHeatProduction";
}

std::string PrmTopCrustHeatProduction::description()
{
  return "Surface radiogenic heat production rate of the basement [uW/m^3]";
}

std::string PrmTopCrustHeatProduction::fullDescription()
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

std::string PrmTopCrustHeatProduction::helpExample(const char* cmdName)
{
  std::ostringstream oss;
  oss << "    #                                      type               minVal  maxVal prmPDF\n";
  oss << "    " << cmdName << " \"Radiogenic heat rate\"  \"" << key() << "\"    0.1     4.9  \"Block\"\n";
  oss << "\n";
  return oss.str();
}

}
