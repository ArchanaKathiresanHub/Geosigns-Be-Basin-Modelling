//
// Copyright (C) 2019 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmSurfaceTemperature.h
/// @brief Class to modify the surface temperature at a specific age

#ifndef CASA_API_PARAMETER_SURFACE_TEMPERATURE_H
#define CASA_API_PARAMETER_SURFACE_TEMPERATURE_H

#include "Parameter.h"

namespace casa
{

class PrmSurfaceTemperature : public Parameter
{
public:
  PrmSurfaceTemperature( mbapi::Model& mdl, const std::vector<std::string>& parameters);

  PrmSurfaceTemperature( const VarParameter * parent, const PrmSurfaceTemperature& param, const double val);

  virtual ~PrmSurfaceTemperature() = default;

  /// @brief Get the level of influence to cauldron applications pipeline for this parametr
  /// @return number which indicates which solver influence this parameter
  virtual AppPipelineLevel appSolverDependencyLevel() const  { return PTSolver; }

  /// @brief Set this parameter value in Cauldron model
  /// @param caldModel reference to Cauldron model
  /// @param caseID unique RunCase ID, in some parameters it is used in new map file name generation
  /// @return ErrorHandler::NoError in success, or error code otherwise
  virtual ErrorHandler::ReturnCode setInModel( mbapi::Model & caldModel, size_t caseID );

  /// @brief Validate Porosity model parameter values
  /// @param caldModel reference to Cauldron model
  /// @return empty string on success or error message with current parameter value
  virtual std::string validate( mbapi::Model & caldModel );

  /// @brief Get parameter value as an array of doubles
  /// @return parameter value represented as set of doubles
  virtual std::vector<double> asDoubleArray() const;

  /// @brief Get parameter value as integer
  /// @return parameter value represented as integer
  virtual int asInteger() const { assert( false ); return -1; }

  /// @brief Are two parameters equal?
  /// @param prm Parameter object to compare with
  /// @return true if parameters are the same, false otherwise
  virtual bool operator == ( const Parameter & prm ) const;

  /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
  /// @param sz Serializer stream
  /// @param  version stream version
  /// @return true if it succeeds, false if it fails.
  virtual bool save( CasaSerializer & sz ) const;

  /// @brief Create a new parameter instance by deserializing it from the given stream
  /// @param dz input stream
  /// @param objVer version of object representation in stream
  PrmSurfaceTemperature( CasaDeserializer & dz, unsigned int objVer );

  /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
  /// @return object class name
  virtual const char * typeName() const { return "PrmSurfaceTemperature"; }

  /// @brief Read obsolete old varparameter specific information
  static void loadVarPrm( CasaDeserializer& dz, const unsigned int objVer );

  /// @brief The number of required input parameters for this influential parameter
  static size_t expectedParametersNumber();

  /// @brief The number of required optional parameters
  static size_t optionalParametersNumber();

  /// @brief Key for this influential parameter, used for selecting it in casa files
  static std::string key();

  /// @brief The name of the assiociated VarParameter, as used for storing in data files
  static std::string varprmName();

  /// @brief Short description used in the help instructions
  static std::string description();

  /// @brief Long description displayed in the help instructions
  static std::string fullDescription();

  /// @brief Example on how to use the parameter
  static std::string helpExample( const char * cmdName );

private:
  bool ageIsSnapshot(mbapi::Model& mdl, const double age) const;

  double m_value;     ///< parameter value
  double m_age;       ///< age for the parameter
  int m_tableRow;     ///< row in the surface temperature table
};

}

#endif // CASA_API_PARAMETER_SURFACE_TEMPERATURE_H
