//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmTopCrustHeatProductionGrid.h
/// @brief This file keeps API declaration for Top Crust Heat Production Rate parameter.

#ifndef CASA_API_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_GRID_H
#define CASA_API_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_GRID_H

#include "Parameter.h"

// CMB API
#include <UndefinedValues.h>

#include <cassert>

/// @page CASA_TopCrustHeatProductionPage Top crust heat production rate parameter
///
/// This parameter defines the surface Radiogenic heat production of the basement.
/// The value is defined in units: @f$ [ \mu W/m^3] @f$
/// It is continuous parameter and his range can be from 0 to any other positive value

namespace casa
{
   /// @brief Surface Radiogenic heat production of the basement parameter
   class PrmTopCrustHeatProductionGrid : public Parameter
   {
   public:
      /// @brief Constructor. Create parameter by reading parameter value from the given model
      /// @param mdl get value of parameter from cauldron model
      PrmTopCrustHeatProductionGrid( mbapi::Model & mdl, const std::vector<std::string>& parameters );

      PrmTopCrustHeatProductionGrid( const VarParameter * parent,
                                     const PrmTopCrustHeatProductionGrid& param,
                                     double value);

      /// @brief Destructor
      virtual ~PrmTopCrustHeatProductionGrid();

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

      /// @brief Get value for the parameter as double
      /// @return parameter value
      double value() const { return m_value; }

      /// @brief Get map name
      /// @return map name
      std::string mapName() const { return m_mapName; }

      /// @brief Define min/max maps name to interpolate between them on setInModel request
      /// @param minMap low range map
      /// @param maxMap high range map
      void defineMapsRange( const std::string & minMap, const std::string & maxMap ) { m_minMapName = minMap; m_maxMapName = maxMap; }

      // The following methods are used for converting between CASA RunCase and SUMLib::Case objects

      /// @brief Get parameter value as an array of doubles
      /// @return parameter value represented as set of doubles
      virtual std::vector<double> asDoubleArray() const { return {m_value}; }

      /// @brief Get parameter value as integer
      /// @return parameter value represented as integer
      virtual int asInteger() const { assert(0); return Utilities::Numerical::NoDataIntValue; }

      /// @brief Are two parameters equal?
      /// @param prm Parameter object to compare with
      /// @return true if parameters are the same, false otherwise
      virtual bool operator == ( const Parameter & prm ) const;

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 1; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @return true if it succeeds, false if it fails.
      virtual bool save(CasaSerializer & sz) const;

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual std::string typeName() const { return "PrmTopCrustHeatProductionGrid"; }

      /// @brief Create a new parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      PrmTopCrustHeatProductionGrid( CasaDeserializer & dz, unsigned int objVer );
      /// @}

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
      double      m_value;
      std::string m_mapName;           ///< map name (if no interpolation is needed)
      std::string m_minMapName;        ///< low range map name if top crust heat production rate is defined by 2D map
      std::string m_maxMapName;        ///< high range map name if top crust heat production rate is defined by 2D map
   };
}

#endif // CASA_API_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_GRID_H
