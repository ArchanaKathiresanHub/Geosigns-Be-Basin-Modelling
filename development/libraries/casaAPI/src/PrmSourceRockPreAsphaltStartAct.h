//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmSourceRockPreAsphaltStartAct.h
/// @brief This file keeps API declaration for Source Rock Preasphaltene Activation Energy parameter handling. 

#ifndef CASA_API_PARAMETER_SOURCE_ROCK_PRE_ASPH_ACT_ENERGY_H
#define CASA_API_PARAMETER_SOURCE_ROCK_PRE_ASPH_ACT_ENERGY_H

#include "Parameter.h"

// CMB API
#include <UndefinedValues.h>

#include <cassert>

namespace mbapi
{
   class Model;
}

/// @page CASA_SourceRockPreAsphaltStartActPage Source rock pre-asphaltene activation energy parameter
/// 
/// This parameter defines the activation energy limit for which the pre-asphalt cracking starts
/// The value is defined in units: @f$ [ kJ/mol ] @f$
/// It is a continuous parameter and it valid range usually is defined by the source rock type, but it
/// value can't be out of the range @f$ [200:220]\% @f$

namespace casa
{
   class VarPrmSourceRockPreAsphaltStartAct;

   /// @brief Source rock initial organic content parameter
   class PrmSourceRockPreAsphaltStartAct : public Parameter
   {
   public:
      /// @brief Constructor, creates parameter object by reading the parameter value from the given model
      /// @param mdl Cauldron model interface object to get the parameter value for the given layer from
      /// @param layerName layer name
      PrmSourceRockPreAsphaltStartAct( mbapi::Model & mdl, const char * layerName );

      /// @brief Constructor, creates the parameter object for the given parameter value and layer name
      /// @param parent pointer to a variable parameter which created this one
      /// @param val value of the parameter
      /// @param layerName layer name
      PrmSourceRockPreAsphaltStartAct( const VarPrmSourceRockPreAsphaltStartAct * parent, double val, const char * layerName );

      /// @brief Destructor
      virtual ~PrmSourceRockPreAsphaltStartAct();
     
      /// @brief Get name of the parameter
      /// @return parameter name
      virtual const char * name() const { return m_name.c_str(); }

      /// @brief Get variable parameter object which was used to create this parameter
      /// @return Pointer to the variable parameter
      virtual const VarParameter * parent() const { return m_parent; }

      /// @brief Set this parameter value in Cauldron model
      /// @param caldModel reference to Cauldron model
      /// @return ErrorHandler::NoError in success, or error code otherwise     
      virtual ErrorHandler::ReturnCode setInModel( mbapi::Model & caldModel );

      /// @brief Validate parameter value if it is inside of valid range
      /// @param caldModel reference to Cauldron model
      /// @return empty string on success or error message with current parameter value
      virtual std::string validate( mbapi::Model & caldModel );

      /// @brief Get value for the parameter as double
      /// @return parameter value
      double value() const { return m_value;  }

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
      virtual const char * typeName() const { return "PrmSourceRockPreAsphaltStartAct"; }

      /// @brief Create a new parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      PrmSourceRockPreAsphaltStartAct( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:
      const VarParameter * m_parent;    ///< variable parameter which was used to create this one

      std::string          m_name;      ///< name of the parameter
      
      std::string          m_layerName; ///< layer name with source rock
      double               m_value;     ///< pre-asphaltene activation energy value 
   };

}

#endif // CASA_API_PARAMETER_SOURCE_ROCK_PRE_ASPH_ACT_ENERGY_H