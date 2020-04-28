//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmSurfacePorosity.h
/// @brief This file keeps API declaration for lithology porosity model surface porosity parameter handling.

#ifndef CASA_API_PARAMETER_SURFACE_POROSITY_H
#define CASA_API_PARAMETER_SURFACE_POROSITY_H

// CASA API
#include "PrmLithologyProp.h"

#include <cassert>

namespace mbapi
{
   class Model;
}

/// @page CASA_SurfacePorosityPage Surface porosity for lithology porosity model
///
/// This parameter defines the surface porosity for any porosity model for a lithology, a set of layers and a set of faults.
/// It defines an alternative way to vary porosity model parameter than casa::PrmPororsityModel class. They must not be defined
/// together for the same layer or lithology
///
/// To define surface porosity user should specify
/// - Lithology name
/// - surface porosity value
namespace casa
{
   class VarPrmSurfacePorosity;

   /// @brief Lithology surface porosity parameter
   class PrmSurfacePorosity : public PrmLithologyProp
   {
   public:
      /// @brief Constructor. Create parameter by reading parameter value from the given model
      /// @param mdl Cauldron model interface object to get Porosity model and it parameters value
      ///            for the given lithology.
      /// @param lithoName lithology name
      PrmSurfacePorosity( mbapi::Model & mdl, const std::string & lithoName );

      /// @brief Constructor. Create parameter object from the given parameter value
      /// @param parent pointer to a influential parameter which created this one
      /// @param lithosName lithology name
      /// @param surfPor value for surface porosity
      PrmSurfacePorosity( const VarPrmSurfacePorosity * parent, const std::vector<std::string> & lithosName, double surfPor );

      /// @brief Destructor
      virtual ~PrmSurfacePorosity() { ; }

      /// @brief Set this parameter value in Cauldron model
      /// @param caldModel reference to Cauldron model
      /// @param caseID unique RunCase ID, in some parameters it is used in new map file name generation
      /// @return ErrorHandler::NoError in success, or error code otherwise
      virtual ErrorHandler::ReturnCode setInModel( mbapi::Model & caldModel, size_t caseID );

      /// @brief Validate Porosity model parameter values
      /// @param caldModel reference to Cauldron model
      /// @return empty string on success or error message with current parameter value
      virtual std::string validate( mbapi::Model & caldModel );

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz ) const;

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual std::string typeName() const { return "PrmSurfacePorosity"; }

      /// @brief Create a new parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      PrmSurfacePorosity( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:
   };
}

#endif // CASA_API_PARAMETER_POROSITY_MODEL_H
