//
// Copyright (C) 2012-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmTopCrustHeatProduction.h
/// @brief This file keeps API declaration for Top Crust Heat Production Rate parameter.

#ifndef CASA_API_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_H
#define CASA_API_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_H

#include "Parameter.h"

// CMB API
#include <UndefinedValues.h>
#include "PrmTopCrustHeatProductionGrid.h"

#include <cassert>

/// @page CASA_TopCrustHeatProductionPage Top crust heat production rate parameter
///
/// This parameter defines the surface Radiogenic heat production of the basement.
/// The value is defined in units: @f$ [ \mu W/m^3] @f$
/// It is continuous parameter and his range can be from 0 to any other positive value

namespace casa
{
   /// @brief Surface Radiogenic heat production of the basement parameter
   class PrmTopCrustHeatProduction : public Parameter
   {
   public:
     PrmTopCrustHeatProduction( mbapi::Model& mdl, const std::vector<std::string>& parameters = {});

     PrmTopCrustHeatProduction( const VarParameter * parent, const PrmTopCrustHeatProduction& param, const double val);

     virtual ~PrmTopCrustHeatProduction() {}

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
     PrmTopCrustHeatProduction( CasaDeserializer & dz, unsigned int objVer );

     /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
     /// @return object class name
     virtual std::string typeName() const { return "PrmTopCrustHeatProduction"; }

     /// @brief Defines version of serialized object representation. Must be updated on each change in save()
     /// @return Actual version of serialized object representation
     virtual unsigned int version() const { return 2; }

     TableInfo tableInfo() const override;

     SharedParameterPtr createNewGridVersionOfParameterFromModel(mbapi::Model& mdl) override {return SharedParameterPtr(new PrmTopCrustHeatProductionGrid( mdl,{}));}

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
      double m_value;             ///< top crust heat production rate value
   };
}

#endif // CASA_API_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_H
