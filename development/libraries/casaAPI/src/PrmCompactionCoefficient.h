//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmCompactioncoefficient.h
/// @brief This file keeps API declaration for compaction coefficient parameter handling. 

#ifndef CASA_API_PARAMETER_COMPACTION_COEFFICIENT_H
#define CASA_API_PARAMETER_COMPACTION_COEFFICIENT_H

// CASA API
#include "PrmLithologyProp.h"

// CMB API
#include <UndefinedValues.h>

#include <cassert>

namespace mbapi
{
   class Model;
}

/// @page CASA_CompactionCoefficientPage Lithology porosity model
///   
/// This parameter defines the compaction coefficient for any porosity model for a lithology, a set of layers and a set of faults.
/// It defines an alternative way to vary compaction coefficient parameter than casa::PrmPororsityModel class. They must not be defined
/// together for the same layer or lithology
///
/// To define the compaction coefficient user should specify 
/// - Lithology name
/// - Compaction coefficient value
namespace casa
{
   class VarPrmCompactionCoefficient;
   
   /// @brief Lithology compaction coefficient parameter
   class PrmCompactionCoefficient : public PrmLithologyProp
   {
   public:
      /// @brief Constructor. Create parameter by reading parameter value from the given model
      /// @param mdl Cauldron model interface object to get Porosity model and its parameters value 
      ///            for the given lithology. 
      /// @param lithoName lithology name
      PrmCompactionCoefficient( mbapi::Model & mdl, const std::string & lithoName );

      /// @brief Constructor. Create parameter object from the given parameter value
      /// @param parent pointer to a variable parameter which created this one
      /// @param lithoName lithology name
      /// @param compCoef values for the compaction coefficient
      PrmCompactionCoefficient( const VarPrmCompactionCoefficient * parent, const std::vector<std::string> & lithosName, double compCoef );

      /// @brief Destructor
      virtual ~PrmCompactionCoefficient( ) { ; }
     
      /// @brief Set this parameter value in Cauldron model
      /// @param caldModel reference to Cauldron model
      /// @param caseID unique RunCase ID, in some parameters it is used in new map file name generation
      /// @return ErrorHandler::NoError in success, or error code otherwise     
      virtual ErrorHandler::ReturnCode setInModel( mbapi::Model & caldModel, size_t caseID );

      /// @brief Validate compaction coefficient value
      /// @param caldModel reference to Cauldron model
      /// @return empty string on success or error message with current parameter value
      virtual std::string validate( mbapi::Model & caldModel );

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 0; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  version stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz, unsigned int version ) const;

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "PrmCompactionCoefficient"; }

      /// @brief Create a new parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      PrmCompactionCoefficient( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:
   };
}

#endif // CASA_API_PARAMETER_COMPACTION_COEFFICIENT_H
