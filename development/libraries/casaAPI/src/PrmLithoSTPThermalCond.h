//
// Copyright (C) 2012-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmLithoSTPThermalCond.h
/// @brief This file keeps API declaration for lithology STP thermal conductivity coefficient parameter handling

#ifndef CASA_API_PARAMETER_LITHO_STP_THERMAL_COND_COEFF_H
#define CASA_API_PARAMETER_LITHO_STP_THERMAL_COND_COEFF_H

#include "Parameter.h"

// CMB API
#include <UndefinedValues.h>

#include <cassert>

namespace mbapi
{
   class Model;
}

/// @page CASA_LithoSTPThermalCondPage Lithology STP (Standard Temperature and Pressure) thermal conductivity coefficient [W/m/K]
///
/// This parameter defines STP thermal conductivity coefficient  [W/m/K] for the given lithology
//
/// To define lithology STP thermal conductivity coefficient user should specify
/// - Lithology name
/// - value for the coefficient
///

namespace casa
{

   /// @brief Parameter for thermal conductivity coefficient of a lithology
   class PrmLithoSTPThermalCond : public Parameter
   {
   public:

      /// @brief Constructor. Create parameter by reading parameter value from the given model
      /// @param mdl Cauldron model interface object to get parameter value for the given lithology.
      /// @param lithoName lithology name
      PrmLithoSTPThermalCond(mbapi::Model & mdl, const std::vector<std::string> parameters );

      /// @brief Constructor. Create parameter from variation of influential parameter
      /// @param parent pointer to a influential parameter which created this one
      /// @param lithoName lithology name
      /// @param val parameter value
      PrmLithoSTPThermalCond( const VarParameter * parent, const char * lithoName, double val );

      PrmLithoSTPThermalCond( const VarParameter * parent, const PrmLithoSTPThermalCond& param, double val);


      /// @brief Destructor
      virtual ~PrmLithoSTPThermalCond() = default;

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

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 0; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @return true if it succeeds, false if it fails.
      virtual bool save(CasaSerializer & sz) const;

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual std::string typeName() const { return "PrmLithoSTPThermalCond"; }

      std::vector<std::string> parameters() const override;

      /// @brief Create a new parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      PrmLithoSTPThermalCond( CasaDeserializer & dz, unsigned int objVer );

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

   protected:
      std::string          m_lithoName; ///< lithology name
      double               m_value;     ///< parameter value
   };
}


#endif // CASA_API_PARAMETER_LITHO_STP_THERMAL_COND_COEFF_H
