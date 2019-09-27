//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmLithofraction.h
/// @brief This file keeps API declaration for lithofraction parameter handling.

#ifndef CASA_API_LITHO_FRACTION_H
#define CASA_API_LITHO_FRACTION_H

// CASA API
#include "Parameter.h"

// CMB API
#include <UndefinedValues.h>

#include <cassert>

namespace mbapi
{
   class Model;
}

/// @page CASA_LithoFractionPage Lithologies mixing proportions
///
/// This parameter defines the lithology fractions in a mix for a particular layer.
/// From the one or two lithology fractions, three percentages are calculated and set in the project file.
/// If only one lithofraction is specified the remaining part is divided among the remaining lithofractions using the same ratio of the base case.
///
/// To define the parameter the user should define:
/// - the layer name
/// - one or two lithofractions indexes
/// - one or two lithofractions

namespace casa
{
   class  VarPrmLithoFraction;

   /// @brief Lithofraction parameter
   class PrmLithoFraction : public Parameter
   {
   public:

      /// @brief Calculate the percentages of each lithology from given lithofractions and lithofractions indexes
      /// @return three percentages
      static std::vector<double> createLithoPercentages( const std::vector< double> & lithoFractions, const std::vector<int> & lithoFractionsInds );

      /// @brief Calculate the lithofractions from given percentages and lithofractions indexes
      /// @return one or two lithofractions
      static std::vector<double> createLithoFractions( const std::vector<double> & lithoPercentages, const std::vector<int> & lithoFractionsInds );

      /// @brief Constructor. Create parameter by reading parameter value from the given model
      /// @param mdl Cauldron model interface object to get the percentages of each lithology
      /// @param layerName the layer name
      /// @param lithoFractionsInds the lithofractions indexes to calculate
      PrmLithoFraction( mbapi::Model & mdl, const std::string & layerName, const std::vector<int> & lithoFractionsInds );

      /// @brief Constructor. Create parameter by reading parameter value from the given model and at specific i and j locations
      PrmLithoFraction( mbapi::Model              & mdl                 ///< Cauldron model interface object to get the percentages of each lithology
                      , const std::string         & layerName           ///< the layer name
                      , const std::vector<int>    & lithoFractionsInds  ///< the lithofractions indexes to calculate
                      , const std::vector<size_t> & coordinates         ///< the i, j positions in the model grid
                      );

      /// @brief Constructor. Create parameter object from the given parameter value
      PrmLithoFraction( const VarPrmLithoFraction * parent,                             ///< parent influential parameter which created this one
                        const std::string         & name,                               ///< parameter name
                        const std::string         & layerName,                          ///< the layer name
                        const std::vector<int>    & lithoFractionsInds,                 ///< the indexes of the lithofractions
                        const std::vector<double> & lithoFractions,                     ///< the lithofractions
                        const std::string         & m_mapNameFirstLithoPercentage = "", ///< the name of the first lithofraction map
                        const std::string         & m_mapNameSecondLithoPercentage = "" ///< the name of the second lithofraction map
                      );

      /// @brief Destructor
      virtual ~PrmLithoFraction() { ; }

      /// @brief Set this parameter value in Cauldron model
      /// @param caldModel reference to Cauldron model
      /// @param caseID unique RunCase ID, in some parameters it is used in new map file name generation
      /// @return ErrorHandler::NoError in success, or error code otherwise
      virtual ErrorHandler::ReturnCode setInModel( mbapi::Model & caldModel, size_t caseID );

      /// @brief Validate lithofraction parameter values
      /// @param caldModel reference to Cauldron model
      /// @return empty string on success or error message with current parameter value
      virtual std::string validate( mbapi::Model & caldModel );

      // The following methods are used for converting between CASA RunCase and SUMLib::Case objects

      /// @brief Get parameter value as an array of doubles
      /// @return parameter value represented as set of doubles
      virtual std::vector<double> asDoubleArray() const;

      /// @brief Get parameter value as integer
      /// @return parameter value represented as integer
      virtual int asInteger() const { assert( false ); return Utilities::Numerical::NoDataIntValue; }

      /// @brief Are two parameters equal?
      /// @param prm Parameter object to compare with
      /// @return true if parameters are the same, false otherwise
      virtual bool operator == ( const Parameter & prm ) const;

      /// @brief Get the level of influence to cauldron applications pipeline for this parametr
      /// @return number which indicates which solver influence this parameter
      virtual AppPipelineLevel appSolverDependencyLevel() const  { return PTSolver; }

      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 2; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @return true if it succeeds, false if it fails.
      virtual bool save(CasaSerializer & sz) const;

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "PrmLithoFraction"; }

      /// @brief Get the layer name
      /// @return the layer name
      virtual std::string layerName( ) const { return m_layerName; }
      /// @}

      /// @brief Create a new parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      PrmLithoFraction( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:
      std::string                              m_layerName;                          ///< stratigraphic layer name
      std::vector<int>                         m_lithoFractionsInds;                 ///< indexes of the lithofractions
      std::vector<double>                      m_lithoFractions;                     ///< lithofractions values

      std::string                              m_mapNameFirstLithoPercentage;        ///< name of the first lithopercentage
      std::string                              m_mapNameSecondLithoPercentage;       ///< name of the second lithopercentage
   };
}

#endif // CASA_API_LITHO_FRACTION_H
