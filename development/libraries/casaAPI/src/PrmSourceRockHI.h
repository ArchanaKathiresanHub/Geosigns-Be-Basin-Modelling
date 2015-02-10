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

#include "Parameter.h"

// CMB API
#include <UndefinedValues.h>

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
   class PrmSourceRockHI : public Parameter
   {
   public:
      /// @brief Constructor. Create parameter by reading parameter value from the given model
      /// @param mdl Cauldron model interface object to get value for HI for given layer from.
      ///            If model has more than one source rock lithology for the same layer, the HI
      ///            value will be equal the first one
      /// @param layerName layer name
      PrmSourceRockHI( mbapi::Model & mdl, const char * layerName );

      /// @brief Constructor. Create parameter from variation of variable parameter
      /// @param parent pointer to a variable parameter which created this one
      /// @param val value of the hydrogen index initial ratio in source rock @f$ [ kg/tonne ] @f$
      /// @param layerName layer name
      PrmSourceRockHI( const VarPrmSourceRockHI * parent, double val, const char * layerName );

      /// @brief Destructor
      virtual ~PrmSourceRockHI();
     
      /// @brief Get name of the parameter
      /// @return parameter name
      virtual const char * name() const { return m_name.c_str(); }

      /// @brief Get variable parameter which was used to create this parameter
      /// @return Pointer to the variable parameter
      virtual const VarParameter * parent() const { return m_parent; }

      /// @brief Set this parameter value in Cauldron model
      /// @param caldModel reference to Cauldron model
      /// @return ErrorHandler::NoError in success, or error code otherwise     
      virtual ErrorHandler::ReturnCode setInModel( mbapi::Model & caldModel );

      /// @brief Validate HI value if it is in [0:100] range, also it check are any source rock
      ///        lithology in the model with the same layer name, does the parameter value is the
      ///        same as in source rock lithology.
      /// @param caldModel reference to Cauldron model
      /// @return empty string on success or error message with current parameter value
      virtual std::string validate( mbapi::Model & caldModel );

      /// @brief Get value for the parameter as double
      /// @return parameter value
      double value() const { return m_hi;  }

      // The following methods are used for converting between CASA RunCase and SUMLib::Case objects
      
      /// @brief Get parameter value as an array of doubles
      /// @return parameter value represented as set of doubles
      virtual std::vector<double> asDoubleArray() const { return std::vector<double>( 1, value() ); }

      /// @brief Get parameter value as integer
      /// @return parameter value represented as integer
      virtual int asInteger() const { assert( 0 ); return UndefinedIntegerValue; }

      /// @brief Are two parameters equal?
      /// @param prm Parameter object to compare with
      /// @return true if parameters are the same, false otherwise
      virtual bool operator == ( const Parameter & prm ) const;

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
      virtual const char * typeName() const { return "PrmSourceRockHI"; }

      /// @brief Create a new parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      PrmSourceRockHI( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:
      const VarParameter * m_parent;    ///< variable parameter which was used to create this one

      std::string          m_name;      ///< name of the parameter
      
      std::string          m_layerName; ///< layer name with source rock
      double               m_hi;        ///< HI value
   };

}

#endif // CASA_API_PARAMETER_SOURCE_ROCK_HI_H
