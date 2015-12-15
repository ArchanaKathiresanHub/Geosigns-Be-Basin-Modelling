//                                                                      
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmSourceRockType.h
/// @brief This file keeps API declaration for Source Rock Type parameter handling.

#ifndef CASA_API_PARAMETER_SOURCE_ROCK_TYPE_H
#define CASA_API_PARAMETER_SOURCE_ROCK_TYPE_H

#include "Parameter.h"
#include "VarPrmSourceRockType.h"

// CMB API
#include <UndefinedValues.h>

#include <cassert>

namespace mbapi
{
   class Model;
}

/// @page CASA_SourceRockTypePage Source rock type parameter
/// 
/// This parameter defines a source rock type for the stratigraphy layer as a source rock type name 
/// from the source rock lithology table. This table must has a record for the combination of 
/// the given stratigraphy layer and the given source rock type name
///

namespace casa
{

   /// @brief Layer source rock type parameter
   class PrmSourceRockType : public Parameter
   {
   public:
      /// @brief Constructor. Create parameter by reading parameter value from the given model
      /// @param mdl Cauldron model interface object to get from it the value of source rock type name for the 
      ///            given layer.
      /// @param layerName layer name
      /// @param mixID     source rock mixing ID, must be 1 or 2
      PrmSourceRockType( mbapi::Model & mdl, const std::string & layerName, int mixID = 1 );

      /// @brief Constructor. Creates the parameter to variate source rock types for the given layer
      PrmSourceRockType( const VarPrmSourceRockType            * parent     ///< pointer to a variable parameter which created this one
                       , const std::string                     & layerName  ///< stratigraphy layer name
                       , const std::string                     & srTypeName ///< name of the source rock type
                       , int                                     mixID      ///< 1 or 2 - source rock mixing ID
                      );

      /// @brief Destructor
      virtual ~PrmSourceRockType() { ; }
     
      /// @brief Get name of the parameter
      /// @return parameter name
      virtual const char * name() const { return m_name.c_str(); }

      /// @brief Get variable parameter which was used to create this parameter
      /// @return Pointer to the variable parameter
      virtual const VarParameter * parent() const { return m_parent; }

      /// @brief Get the level of influence to cauldron applications pipeline for this parametr
      /// @return number which indicates which solver influence this parameter
      virtual AppPipelineLevel appSolverDependencyLevel() const  { return Genex; }
                  
      /// @brief Set this parameter value in Cauldron model
      /// @param caldModel reference to Cauldron model
      /// @param caseID unique RunCase ID, in some parameters it is used in new map file name generation
      /// @return ErrorHandler::NoError in success, or error code otherwise     
      virtual ErrorHandler::ReturnCode setInModel( mbapi::Model & caldModel, size_t caseID );

      /// @brief Validate parameter value if it exist in source rock lithology table and if 
      ///         stratigraphy layer has this source rock type 
      /// @param caldModel reference to Cauldron model
      /// @return empty string on success or error message with current parameter value
      virtual std::string validate( mbapi::Model & caldModel );

      /// @brief Get source rock type name
      /// @return source rock type name
      std::string sourceRockTypeName() const { return m_srtName; }

      /// @brief Get layer name
      /// @return layer name
      std::string layerName() const { return m_layerName; }

      // The following methods are used for converting between CASA RunCase and SUMLib::Case objects
      
      /// @brief Get parameter value as an array of doubles
      /// @return parameter value represented as set of doubles
      virtual std::vector<double> asDoubleArray() const { assert( 0 ); return std::vector<double>(); }

      /// @brief Get parameter value as integer
      /// @return parameter value represented as integer
      virtual int asInteger() const;

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
      virtual const char * typeName() const { return "PrmSourceRockType"; }

      /// @brief Create a new parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      PrmSourceRockType( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:
      const VarPrmSourceRockType           * m_parent;    ///< variable parameter which was used to create this one
      std::string                            m_name;      ///< name of the parameter
      
      std::string                            m_layerName; ///< layer name with source rock
      std::string                            m_srtName;   ///< source rock type name
      int                                    m_mixID;     ///< which source rock in the mix should be affected
   };
}

#endif // CASA_API_PARAMETER_SOURCE_ROCK_TYPE_H
