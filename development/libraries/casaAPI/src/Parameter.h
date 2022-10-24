//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file Parameter.h
/// @brief This file keeps declaration of top level class for handling Cauldron model parameters

#ifndef CASA_API_PARAMETER_H
#define CASA_API_PARAMETER_H

// CASA
#include "ErrorHandler.h"
#include "CasaSerializer.h"
#include "CasaDeserializer.h"
#include "CauldronApp.h"

// STL
#include <memory>
#include <vector>

#include "ConstantsNumerical.h"

/// @page CASA_ParameterPage Cauldron project parameter
/// Parameter is some value (or possibly set of values) in project file which has an influence on simulation.
/// Any change of the parameter value, will change in some way the simulation results.
///

namespace mbapi
{
   class Model;
}

namespace casa
{
   class Parameter;
}

typedef std::shared_ptr<casa::Parameter> SharedParameterPtr;

namespace casa
{
   class VarParameter;

   struct TableInfo
   {
      std::string tableName;
      std::string variableGridName;
      int tableRow = Utilities::Numerical::IbsNoDataValueInt;
   };

   /// @brief Base class for all types of Cauldron model parameters used in CASA
   class Parameter : public CasaSerializable
   {
   public:
      /// @brief Parameter destructor
      virtual ~Parameter() = default;

      /// @brief Get influential parameter which was used to create this parameter
      /// @return Pointer to the influential parameter
      virtual const VarParameter * parent() const;

      /// @brief Set influential parameter which was used to create this parameter
      /// @param varPrm Pointer to the influential parameter
      void  setParent( const VarParameter * varPrm );

      /// @brief Get the level of influence to cauldron applications pipeline for this parametr
      /// @return number which indicates which solver influence this parameter
      virtual AppPipelineLevel appSolverDependencyLevel() const  = 0;

      /// @brief Set this parameter value in Cauldron model
      /// @param caldModel reference to Cauldron model
      /// @param caseID unique RunCase ID, in some parameters it is used in new map file name generation
      /// @return ErrorHandler::NoError in success, or error code otherwise
      virtual ErrorHandler::ReturnCode setInModel( mbapi::Model & caldModel, size_t caseID ) = 0;

      /// @brief Validate parameter value if it is in range of allowed values
      /// @param caldModel reference to Cauldron model
      /// @return empty string on success, or error message if validation fail
      virtual std::string validate( mbapi::Model & caldModel ) = 0;

      // The following methods are used for converting between CASA RunCase and SUMLib::Case objects

      /// @brief Get parameter value as an array of doubles
      /// @return parameter value represented as set of doubles
      ///
      /// @pre parent()->variationType() must returns VarParameter::Continuous
      /// @post return set of doubles which represent value of this parameter if parameter is continuous
      ///       or empty array otherwise
      virtual std::vector<double> asDoubleArray() const = 0;

      /// @brief Get parameter value as integer
      /// @return parameter value represented as integer
      ///
      /// @pre parent()->variationType() must returns VarParameter::Categorical
      /// @post return one non negative integer number which represents a value of this parameter if
      ///       parameter is categorical, or -1 otherwise
      virtual int asInteger() const = 0;

      /// @brief Get the parameters/arguments for this parameter
      /// @return list of parameters/arguments
      virtual std::vector<std::string> parameters() const;

      /// @brief Are two parameters equal?
      /// @param prm Parameter object to compare with
      /// @return true if parameters are the same, false otherwise
      virtual bool operator == ( const Parameter & prm ) const = 0;

      /// @brief Are two parameters not equal?
      /// @param prm Parameter object to compare with
      /// @return false if parameters are the same, true otherwise
      virtual bool operator != ( const Parameter & prm ) const { return ! (*this == prm); }

      /// @brief Create a new parameter instance and deserialize it from the given stream
      /// @param dz input stream
      /// @param objName expected object name
      /// @return new observable instance on susccess, or throw and exception in case of any error
      static Parameter * load( CasaDeserializer & dz, const char * objName );

      virtual TableInfo tableInfo() const { throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "TableInfo is not available for this parameter"; }

      virtual SharedParameterPtr createNewPrmFromModel(mbapi::Model& mdl) { throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "the function produceParameter is not available for this parameter"; }

   protected:
      explicit Parameter(const VarParameter * parent = 0);
      Parameter(CasaDeserializer& dz, unsigned int version);

      bool saveCommonPart(CasaSerializer& sz) const;

   private:
      Parameter & operator = ( const Parameter & );

      const VarParameter * m_parent; ///< influential parameter which was used to create this parameter
   };
}


#endif // CASA_API_PARAMETER_H
