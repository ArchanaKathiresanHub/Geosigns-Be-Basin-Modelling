//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmSourceRockHC.h
/// @brief This file keeps API declaration for Source Rock H/C parameter handling.

#ifndef CASA_API_PARAMETER_SOURCE_ROCK_HC_H
#define CASA_API_PARAMETER_SOURCE_ROCK_HC_H

#include "PrmSourceRockProp.h"

// CMB API
#include <UndefinedValues.h>

#include <cassert>

namespace mbapi
{
   class Model;
}

/// @page CASA_SourceRockHCPage Source rock H/C initial ratio parameter
///
/// This parameter defines the H/C initial ratio in source rock.
/// The value is defined in units: @f$ [ kg/tonne C] @f$
/// It is continuous parameter and his range is @f$ [0:2] @f$

namespace casa
{
   class VarPrmSourceRockHC;

   /// @brief Source rock hydrogen index initial ratio parameter
   class PrmSourceRockHC : public PrmSourceRockProp
   {
   public:
      /// @brief Constructor. Create parameter by reading parameter value from the given model
      /// @param mdl Cauldron model interface object to get value for H/C for given layer from.
      ///            If model has more than one source rock lithology for the same layer, the H/C
      ///            value will be equal the first one
      /// @param layerName layer name
      /// @param srType    source rock type name
      /// @param mixID     source rock mixing ID for stratigraphy layer
      PrmSourceRockHC( mbapi::Model & mdl, const char * layerName, const char * srType = 0, int mixID = 1 );

      /// @brief Constructor. Create parameter from variation of influential parameter
      /// @param parent pointer to a influential parameter which created this one
      /// @param val value of the hydrogen index initial ratio in source rock @f$ [ kg/tonne C] @f$
      /// @param layerName layer name
      /// @param srType    source rock type name
      /// @param mixID     source rock mixing ID for stratigraphy layer
      PrmSourceRockHC( const VarPrmSourceRockHC * parent, double val, const char * layerName, const char * srType = 0, int mixID = 1 );

      /// @brief Destructor
      virtual ~PrmSourceRockHC() = default;

      /// @brief Set this parameter value in Cauldron model
      /// @param caldModel reference to Cauldron model
      /// @param caseID unique RunCase ID, in some parameters it is used in new map file name generation
      /// @return ErrorHandler::NoError in success, or error code otherwise
      virtual ErrorHandler::ReturnCode setInModel( mbapi::Model & caldModel, size_t caseID );

      /// @brief Validate H/C value if it is in [0:2] range
      /// @param caldModel reference to Cauldron model
      /// @return empty string on success or error message with current parameter value
      virtual std::string validate( mbapi::Model & caldModel );

      /// @brief Are two parameters equal?
      /// @param prm Parameter object to compare with
      /// @return true if parameters are the same, false otherwise
      virtual bool operator == ( const Parameter & prm ) const;

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz ) const;

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "PrmSourceRockHC"; }

      /// @brief Create a new parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      PrmSourceRockHC( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:
   };

}

#endif // CASA_API_PARAMETER_SOURCE_ROCK_HC_H
