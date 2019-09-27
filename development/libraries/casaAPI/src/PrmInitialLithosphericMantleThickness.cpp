//
// Copyright (C) 2012-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmInitialLithosphericMantleThickness.cpp
/// @brief Implementation for the initial lithospheric mantle thickness parameter

#include "PrmInitialLithosphericMantleThickness.h"

#include "cmbAPI.h"
#include "ConstantsNumerical.h"
#include "NumericFunctions.h"

namespace casa
{

namespace
{

const std::string project3dTableName = "BasementIoTbl";
const std::string project3dColumnName = "InitialLithosphericMantleThickness";

}

PrmInitialLithosphericMantleThickness::PrmInitialLithosphericMantleThickness(mbapi::Model& mdl, const std::vector<std::string>& /* parameters */ ) :
  Parameter(),
  m_value(mdl.tableValueAsDouble(project3dTableName, 0, project3dColumnName))
{
}

PrmInitialLithosphericMantleThickness::PrmInitialLithosphericMantleThickness(const VarParameter* parent, const PrmInitialLithosphericMantleThickness& param, const double val) :
  PrmInitialLithosphericMantleThickness( param )
{
  setParent(parent);
  m_value = val;
}

ErrorHandler::ReturnCode PrmInitialLithosphericMantleThickness::setInModel(mbapi::Model& caldModel, size_t caseID)
{
  return caldModel.setTableValue( project3dTableName, 0, project3dColumnName, m_value);
}

std::string PrmInitialLithosphericMantleThickness::validate(mbapi::Model& /*caldModel*/)
{
  if ( m_value < 0 )
  {
    return "The initial lithospheric mantle thickness should be larger than 0";
  }
  return "";
}

std::vector<double> PrmInitialLithosphericMantleThickness::asDoubleArray() const
{
  return {m_value};
}

bool PrmInitialLithosphericMantleThickness::operator ==(const Parameter& prm) const
{
  const PrmInitialLithosphericMantleThickness* pp = dynamic_cast<const PrmInitialLithosphericMantleThickness*>(&prm);
  if ( !pp )
  {
    return false;
  }

  return NumericFunctions::isEqual( m_value, pp->m_value, 1.e-6);
}

bool PrmInitialLithosphericMantleThickness::save(CasaSerializer& sz) const
{
  bool ok = saveCommonPart(sz);
  ok = ok && sz.save( m_value, "InitLithMantleThickness" );
  return ok;
}

PrmInitialLithosphericMantleThickness::PrmInitialLithosphericMantleThickness(CasaDeserializer& dz, unsigned int objVer) :
  Parameter(dz, objVer)
{
  bool ok = dz.load( m_value, "InitLithMantleThickness" );
  if ( !ok )
  {
     throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
        << "PrmInitialLithosphericMantleThickness deserialization unknown error";
  }
}

void PrmInitialLithosphericMantleThickness::loadVarPrm(CasaDeserializer& dz, const unsigned int objVer)
{
}

size_t PrmInitialLithosphericMantleThickness::expectedParametersNumber()
{
  return 0;
}

size_t PrmInitialLithosphericMantleThickness::optionalParametersNumber()
{
  return 0;
}

std::string PrmInitialLithosphericMantleThickness::key()
{
  return "BasementIoTbl:InitialLithosphericMantleThickness";
}

std::string PrmInitialLithosphericMantleThickness::varprmName()
{
  return "VarPrmInitialLithosphericMantleThickness";
}

std::string PrmInitialLithosphericMantleThickness::description()
{
  return "A variation of the initial lithospheric mantle thickness.";
}

std::string PrmInitialLithosphericMantleThickness::fullDescription()
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

std::string PrmInitialLithosphericMantleThickness::helpExample(const char* cmdName)
{
  std::ostringstream oss;
  oss << "    #                                     min max  Parameter PDF\n";
  oss << "    " << cmdName << " \"" << key() << "\" 100000   150000   \"Block\"\n";
  return oss.str();
}

}
