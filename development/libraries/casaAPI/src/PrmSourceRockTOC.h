//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmSourceRockTOC.h
/// @brief This file keeps API declaration for Source Rock TOC parameter handling. 

#ifndef CASA_API_PARAMETER_SOURCE_ROCK_TOC_H
#define CASA_API_PARAMETER_SOURCE_ROCK_TOC_H

#include "PrmSourceRockProp.h"

// CMB API
#include <UndefinedValues.h>

#include <cassert>

namespace mbapi
{
   class Model;
}

/// @page CASA_SourceRockTOCPage Source rock initial Total Organic Contents (TOC) parameter
/// 
/// This parameter defines the initial total organic content in source rock.
/// The value is defined in units: @f$ [ weight \% ] @f$
/// It is continuous parameter and his range is @f$ [0:100]\% @f$

namespace casa
{
   class VarPrmSourceRockTOC;

   /// @brief Source rock initial organic content parameter
   class PrmSourceRockTOC : public PrmSourceRockProp
   {
   public:
      /// @brief Constructor, creates parameter object by reading the parameter value from the given model
      /// @param mdl Cauldron model interface object to get the parameter value for the given layer from
      /// @param layerName layer name
      /// @param srType    source rock type name
      /// @param mixID     source rock mixing ID for stratigraphy layer
      PrmSourceRockTOC( mbapi::Model & mdl, const char * layerName, const char * srType = 0, int mixID = 1 );

      /// @brief Constructor, creates the parameter object for the given parameter value and layer name
      /// @param parent pointer to a variable parameter which created this one
      /// @param val value of the parameter
      /// @param layerName layer name
      /// @param srType    source rock type name
      /// @param mixID     source rock mixing ID for stratigraphy layer
      PrmSourceRockTOC( const VarPrmSourceRockTOC * parent, double val, const char * layerName, const char * srType = 0, int mixID = 1 );

      /// @brief Destructor
      virtual ~PrmSourceRockTOC();

      /// @brief Are two parameters equal?
      /// @param prm Parameter object to compare with
      /// @return true if parameters are the same, false otherwise
      virtual bool operator == ( const Parameter & prm ) const;

      /// @brief Set this parameter value in Cauldron model
      /// @param caldModel reference to Cauldron model
      /// @param caseID unique RunCase ID, in some parameters it is used in new map file name generation
      /// @return ErrorHandler::NoError in success, or error code otherwise     
      virtual ErrorHandler::ReturnCode setInModel( mbapi::Model & caldModel, size_t caseID );

      /// @brief Validate parameter value if it is inside of valid range
      /// @param caldModel reference to Cauldron model
      /// @return empty string on success or error message with current parameter value
      virtual std::string validate( mbapi::Model & caldModel );

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      //  Version 1: - added mixing ID and source rock type name
      virtual unsigned int version() const { return 1; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  version stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz, unsigned int version ) const;

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "PrmSourceRockTOC"; }

      /// @brief Create a new parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      PrmSourceRockTOC( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:
   };

}

#endif // CASA_API_PARAMETER_SOURCE_ROCK_TOC_H
