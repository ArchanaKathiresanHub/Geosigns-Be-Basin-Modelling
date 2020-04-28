//
// Copyright (C) 2019 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmTopCrustHeatProductionGridScaling.h
/// @brief Influential parameter for scaling of grids

#ifndef CASA_API_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_SCALING_H
#define CASA_API_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_SCALING_H

#include "Parameter.h"

namespace casa
{
   /// @brief Scaling factor for the heat production of the basement
   class PrmTopCrustHeatProductionGridScaling : public Parameter
   {
   public:
      /// @brief Constructor. Create parameter by reading parameter value from the given model
      /// @param mdl get value of parameter from cauldron model
      PrmTopCrustHeatProductionGridScaling( mbapi::Model & mdl, const std::vector<std::string>& parameters );

      PrmTopCrustHeatProductionGridScaling( const VarParameter * parent, const PrmTopCrustHeatProductionGridScaling& param, const double val);

      /// @brief Destructor
      virtual ~PrmTopCrustHeatProductionGridScaling();

      /// @brief Get the level of influence to cauldron applications pipeline for this parametr
      /// @return number which indicates which solver influence this parameter
      virtual AppPipelineLevel appSolverDependencyLevel() const  { return PTSolver; }

      /// @brief Set this parameter value in Cauldron model
      /// @param caldModel reference to Cauldron model
      /// @param caseID unique RunCase ID, in some parameters it is used in new map file name generation
      /// @return ErrorHandler::NoError in success, or error code otherwise
      virtual ErrorHandler::ReturnCode setInModel( mbapi::Model & caldModel, size_t caseID );

      /// @brief Validate top crust heat production rate value if it is in positive range
      ///        also it checks does the given model has the same value for this parameter.
      /// @param caldModel reference to Cauldron model
      /// @return empty string on success or error message with current parameter value
      virtual std::string validate( mbapi::Model & caldModel );

      /// @brief Get parameter value as an array of doubles
      /// @return parameter value represented as set of doubles
      virtual std::vector<double> asDoubleArray() const { return std::vector<double>( 1, m_factor ); }

      /// @brief Get parameter value as integer
      /// @return parameter value represented as integer
      virtual int asInteger() const;

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
      virtual bool save( CasaSerializer & sz ) const;

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual std::string typeName() const { return "PrmTopCrustHeatProductionGridScaling"; }

      /// @brief Create a new parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      PrmTopCrustHeatProductionGridScaling( CasaDeserializer & dz, unsigned int objVer );

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
      double m_factor; // scaling factor for the grid
      std::string m_mapName;
   };

}

#endif //CASA_API_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_SCALING_H
