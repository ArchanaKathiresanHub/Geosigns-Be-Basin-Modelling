//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmLithologyProp.h
/// @brief This file keeps base class declaration for all lithology parameters

#ifndef CASA_API_PARAMETER_LITHOLOGY_PROP_H
#define CASA_API_PARAMETER_LITHOLOGY_PROP_H

#include "Parameter.h"

// CMB API
#include "UndefinedValues.h"

#include <cassert>

namespace mbapi
{
   class Model;
}

namespace casa
{
   class VarPrmLithologyProp;

   /// @brief Base class for lithology property parameter which can be represented as a scalar value
   class PrmLithologyProp : public Parameter
   {
   public:
      /// @brief Destructor
      virtual ~PrmLithologyProp() = default;

      /// @brief Get the level of influence to cauldron applications pipeline for this parametr
      /// @return number which indicates which solver influence this parameter
      virtual AppPipelineLevel appSolverDependencyLevel() const  { return PTSolver; }

      // The following methods are used for converting between CASA RunCase and SUMLib::Case objects

      /// @brief Get parameter value as an array of doubles
      /// @return parameter value represented as set of doubles
      virtual std::vector<double> asDoubleArray() const { return std::vector<double>( 1, m_val ); }

      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      ///        Note: This is the base class, therefore all the child classes have the same version as the base class.
      /// @return Actual version of serialized object representation
      unsigned int version() const { return 1; }

      /// @brief Get parameter value as integer
      /// @return parameter value represented as integer
      virtual int asInteger() const { assert(0); return Utilities::Numerical::NoDataIntValue; }

      /// @brief Are two parameters equal?
      /// @param prm Parameter object to compare with
      /// @return true if parameters are the same, false otherwise
      virtual bool operator == ( const Parameter & prm ) const;

      /// @brief Get value for the parameter as double
      /// @return parameter value
      double value() const { return m_val; }

      /// @{
      /// @brief Save common object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @return true if it succeeds, false if it fails.
      virtual bool serializeCommonPart( CasaSerializer & sz ) const;

      /// @brief Load common object data from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      /// @return true on success, false otherwise
      virtual bool deserializeCommonPart( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:
      /// @brief Default constructor, is being used in deserialization
      PrmLithologyProp();

      /// @brief Constructor
      /// @param parent influential parameter which created this one
      /// @param lithoName lithology name
      /// @param val lithology property value
      PrmLithologyProp( const VarParameter * parent, const std::vector<std::string> & lithoName, double val );

      std::vector<std::string>  m_lithosName; ///< lithology name
      std::string               m_propName;   ///< property name to be set in derived class
      double                    m_val;        ///< lithology property value
   };
}

#endif // CASA_API_PARAMETER_POROSITY_MODEL_H
