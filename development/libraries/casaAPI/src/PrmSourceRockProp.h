//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmSourceRockProp.h
/// @brief This file keeps API declaration for base class for all source rock lithology parameters handling. 

#ifndef CASA_API_PARAMETER_SOURCE_ROCK_PROP_H
#define CASA_API_PARAMETER_SOURCE_ROCK_PROP_H

// CASA API
#include "Parameter.h"

// CMB API
#include <UndefinedValues.h>

#include <cassert>

namespace mbapi
{
   class Model;
}

namespace casa
{
   class VarPrmSourceRockProp;

   /// @brief Base class for all source rock lithology parameters 
   class PrmSourceRockProp : public Parameter
   {
   public:
      /// @brief Destructor
      virtual ~PrmSourceRockProp() {;}
     
      /// @brief Get name of the parameter
      /// @return parameter name
      virtual const char * name() const { return m_name.c_str(); }

      /// @brief Get variable parameter object which was used to create this parameter
      /// @return Pointer to the variable parameter
      virtual const VarParameter * parent() const { return m_parent; }

      /// @brief Set variable parameter which was used to create this parameter
      /// @param Pointer to the variable parameter
      virtual void setParent( const VarParameter * varPrm )  { m_parent = varPrm; }

      /// @brief Get the level of influence to cauldron applications pipeline for this parametr
      /// @return number which indicates which solver influence this parameter
      virtual AppPipelineLevel appSolverDependencyLevel() const  { return Genex; }
                  
      // The following methods are used for converting between CASA RunCase and SUMLib::Case objects
      
      /// @brief Get parameter value as an array of doubles
      /// @return parameter value represented as set of doubles
      virtual std::vector<double> asDoubleArray() const { return std::vector<double>( 1, value() ); }

      /// @brief Get parameter value as integer
      /// @return parameter value represented as integer
      virtual int asInteger() const { assert( 0 ); return UndefinedIntegerValue; }

      /// @{ SourceRockProp specific methods
      /// @brief Get mixing ID
      /// @return mixing ID
      int mixID() const { return m_mixID; }

      /// @brief Get layer name
      /// @return layer name
      std::string layerName() const { return m_layerName; }

      /// @brief Get source rock type name
      /// @return source rock type name
      std::string sourceRockTypeName() const { return m_srTypeName; }

      /// @brief Get value for the parameter as double
      /// @return parameter value
      double value() const { return m_val; }

      /// @brief Update parameter value according new source rock type
      /// @param val new parameter value
      /// @param srTypeName - new source rock type name
      void update( double val, const std::string & srTypeName ) { m_val = val; m_srTypeName = srTypeName; }
      /// @}

      /// @{
      /// @brief Save common object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  version stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool serializeCommonPart( CasaSerializer & sz, unsigned int version ) const;

      /// @brief Load common object data from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      /// @return true on success, false otherwise
      virtual bool deserializeCommonPart( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:
      /// @brief Default constructor, is being used in deserialization
      PrmSourceRockProp();

      /// @brief Constructor, creates parameter object by reading the parameter value from the given model
      /// @param mdl Cauldron model interface object to get the parameter value for the given layer from
      /// @param layerName layer name
      /// @param srType    source rock type name
      /// @param mixID     source rock mixing ID for stratigraphy layer
      PrmSourceRockProp( mbapi::Model & mdl, const char * layerName, const char * srType, int mixID );

      /// @brief Constructor, creates the parameter object for the given parameter value and layer name
      /// @param parent pointer to a variable parameter which created this one
      /// @param val value of the parameter
      /// @param layerName layer name
      /// @param srType    source rock type name
      /// @param mixID     source rock mixing ID for stratigraphy layer
      PrmSourceRockProp( const VarPrmSourceRockProp * parent, double val, const char * layerName, const char * srType, int mixID );

      const VarParameter * m_parent;     ///< variable parameter which was used to create this one

      std::string          m_propName;   ///< Defines which source rock lithology property this parameter defines
      std::string          m_name;       ///< name of the parameter
      
      std::string          m_layerName;  ///< layer name with source rock
      std::string          m_srTypeName; ///< name for source rock type
      int                  m_mixID;      ///< mixing ID for stratigraphy layer source rock mix
      double               m_val;        ///< property value
   };

}

#endif // CASA_API_PARAMETER_SOURCE_ROCK_Prop_H
