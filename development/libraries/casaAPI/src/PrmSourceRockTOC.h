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
      PrmSourceRockTOC( const VarPrmSourceRockTOC * parent       /// pointer to a influential parameter which created this one
                      , double                      val          /// value of the parameter
                      , const char                * layerName    /// layer name
                      , const std::string         & mapName = "" /// name of the TOC map
                      , const char                * srType = 0   /// source rock type name
                      , int                         mixID  = 1   /// source rock mixing ID for stratigraphy layer
                      );

      /// @brief Destructor
      virtual ~PrmSourceRockTOC();

      /// @brief Get map name
      /// @return map name
      std::string mapName() const { return m_mapName; }

      /// @brief Define min/max maps name to interpolate between them on setInModel request
      /// @param minMap low range map
      /// @param maxMap high range map
      void defineMapsRange( const std::string & minMap, const std::string & maxMap ) { m_minMapName = minMap; m_maxMapName = maxMap; }

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

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz ) const;

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "PrmSourceRockTOC"; }

      /// @brief Create a new parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      PrmSourceRockTOC( CasaDeserializer & dz, unsigned int objVer );

   protected:
      std::string           m_mapName;           ///< map name (if no interpolation is needed)
      std::string           m_minMapName;        ///< low range map name if TOC is defined by 2D map
      std::string           m_maxMapName;        ///< high range map name if TOS is defined by 2D map
 };

}

#endif // CASA_API_PARAMETER_SOURCE_ROCK_TOC_H
