//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmSourceRockHI.h
/// @brief This file keeps API declaration for Source Rock HI parameter handling.

#ifndef CASA_API_PARAMETER_SOURCE_ROCK_HI_H
#define CASA_API_PARAMETER_SOURCE_ROCK_HI_H

#include "PrmSourceRockProp.h"

#include <cassert>

namespace mbapi
{
   class Model;
}

/// @page CASA_SourceRockHIPage Source rock Hydrogen Index (HI) initial ratio parameter
///
/// This parameter defines the hydrogen index initial ratio in source rock.
/// The value is defined in units: @f$ [ kg/tonne ] @f$
/// It is continuous parameter and his range is @f$ [0:1000] @f$

namespace casa
{
   class VarPrmSourceRockHI;

   /// @brief Source rock hydrogen index initial ratio parameter
   class PrmSourceRockHI : public PrmSourceRockProp
   {
   public:
      /// @brief Constructor. Create parameter by reading parameter value from the given model
      /// @param mdl Cauldron model interface object to get value for HI for given layer from.
      ///            If model has more than one source rock lithology for the same layer, the HI
      ///            value will be equal the first one
      /// @param layerName layer name
      /// @param srType    source rock type name
      /// @param mixID     source rock mixing ID for stratigraphy layer
      PrmSourceRockHI( mbapi::Model & mdl, const char * layerName, const char * srType = 0, int mixID = 1 );

      /// @brief Constructor. Create parameter from variation of influential parameter
      /// @param parent pointer to a influential parameter which created this one
      /// @param val value of the hydrogen index initial ratio in source rock @f$ [ kg/tonne ] @f$
      /// @param layerName layer name
      /// @param srType    source rock type name
      /// @param mixID     source rock mixing ID for stratigraphy layer
      PrmSourceRockHI( const VarPrmSourceRockHI * parent, double val, const char * layerName, const char * srType = 0, int mixID = 1 );

      /// @brief Destructor
      virtual ~PrmSourceRockHI() = default;

      /// @brief Set this parameter value in Cauldron model
      /// @param caldModel reference to Cauldron model
      /// @param caseID unique RunCase ID, in some parameters it is used in new map file name generation
      /// @return ErrorHandler::NoError in success, or error code otherwise
      virtual ErrorHandler::ReturnCode setInModel( mbapi::Model & caldModel, size_t caseID );

      /// @brief Validate HI value if it is in [0:100] range, also it check are any source rock
      ///        lithology in the model with the same layer name, does the parameter value is the
      ///        same as in source rock lithology.
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
      virtual const char * typeName() const { return "PrmSourceRockHI"; }

      /// @brief Create a new parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      PrmSourceRockHI( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:
   };

}

#endif // CASA_API_PARAMETER_SOURCE_ROCK_HI_H
