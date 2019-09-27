//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file VarPrmSourceRockType.h
/// @brief This file keeps API declaration for handling variation of source rock type parameter of stratigraphy layer.

#ifndef CASA_API_VAR_PARAMETER_SOURCE_ROCK_TYPE_H
#define CASA_API_VAR_PARAMETER_SOURCE_ROCK_TYPE_H

#include "VarPrmCategorical.h"

namespace casa
{
   class PrmSourceRockType;

   /// @brief Variation for casa::PrmSourceRockType parameter
   class VarPrmSourceRockType : public VarPrmCategorical
   {
   public:
      /// @brief Create a new initial source rock HI influential parameter
      VarPrmSourceRockType( const char                        * layerName ///< name of the layer for source rock type variation
                           , const std::string                & baseVal   ///< base value of parameter
                           , int                                mixID     ///< 1 or 2 - mixing ID
                           , const std::vector<std::string>   & variation ///< set of possible values for this parameter
                           , const std::vector<double>        & weights   ///< parameter values weights
                           , const char                       * name      ///< user defined parameter name
                           );

      /// @brief Destructor
      virtual ~VarPrmSourceRockType() {;}

		 /// @brief Get name of influential parameter in short form
		 /// @return array of names for each sub-parameter
		 virtual std::vector<std::string> name() const;

      /// @brief Get number of subparameters if it is more than one
      /// @return dimension of influential parameter
      virtual size_t dimension() const { return 1; }

      /// @brief Find parameter position in enumeration
      /// @return found parameter position on success or negative value on error
      int index( const PrmSourceRockType * prm ) const;

      /// @{ SourceRockType specific methods
      /// @brief Get mixing ID
      /// @return mixing ID
      int mixID() const { return m_mixID; }

      /// @brief Get layer name
      /// @return layer name
      std::string layerName() const { return m_layerName; }

      /// @brief Create parameter by reading the values stored in the project file
      /// @param [in] mdl the model where the parameters values should be read
      /// @param[in] vin an input vector with parameter specific values. Not used in this parameter
      /// @return the new parameter read from the model
      virtual SharedParameterPtr newParameterFromModel( mbapi::Model & mdl, const std::vector<double> & vin ) const;
      /// @}

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return VarPrmCategorical::version() + 0; }

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "VarPrmSourceRockType"; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz ) const;

      /// @brief Create a new var.parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      VarPrmSourceRockType( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:
      int         m_mixID;
      std::string m_layerName;
   };

}

#endif // CASA_API_VAR_PARAMETER_SOURCE_ROCK_HC_H
