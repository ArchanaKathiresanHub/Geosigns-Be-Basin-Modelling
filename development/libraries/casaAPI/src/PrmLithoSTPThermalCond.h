//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmLithoSTPThermalCond.h
/// @brief This file keeps API declaration for lithology STP thermal conductivity coefficient parameter handling

#ifndef CASA_API_PARAMETER_LITHO_STP_THERMAL_COND_COEFF_H
#define CASA_API_PARAMETER_LITHO_STP_THERMAL_COND_COEFF_H

#include "Parameter.h"

// CMB API
#include <UndefinedValues.h>

#include <cassert>

namespace mbapi
{
   class Model;
}

/// @page CASA_LithoSTPThermalCondPage Lithology STP (Standart Temperature and Pressure) thermal conductivity coefficient [W/m/K] 
/// 
/// This parameter defines STP thermal conductivity coefficient  [W/m/K] for the given lithology
//
/// To define lithology STP thermal conductivity coefficient user should specify 
/// - Lithology name
/// - value for the coefficient
///

namespace casa
{
   class VarPrmLithoSTPThermalCond;

   /// @brief Source rock initial organic content parameter
   class PrmLithoSTPThermalCond : public Parameter
   {
   public:
      /// @brief Constructor. Create parameter by reading parameter value from the given model
      /// @param mdl Cauldron model interface object to get parameter value for the given lithology. 
      /// @param lithoName lithology name
      PrmLithoSTPThermalCond( mbapi::Model & mdl, const char * lithoName );

      /// @brief Constructor. Create parameter from variation of influential parameter
      /// @param parent pointer to a influential parameter which created this one
      /// @param lithoName lithology name
      /// @param val parameter value
      PrmLithoSTPThermalCond( const VarPrmLithoSTPThermalCond * parent, const char * lithoName, double val );

      /// @brief Destructor
      virtual ~PrmLithoSTPThermalCond() { ; }
     
      /// @brief Get name of the parameter
      /// @return parameter name
      virtual const char * name() const { return m_name.c_str(); }

      /// @brief Get influential parameter which was used to create this parameter
      /// @return Pointer to the influential parameter
      virtual const VarParameter * parent() const { return m_parent; }

      /// @brief Set influential parameter which was used to create this parameter
      /// @param varPrm pointer to the influential parameter
      virtual void setParent( const VarParameter * varPrm )  { m_parent = varPrm; }

      /// @brief Get the level of influence to cauldron applications pipeline for this parametr
      /// @return number which indicates which solver influence this parameter
      virtual AppPipelineLevel appSolverDependencyLevel() const  { return PTSolver; }
                  
      /// @brief Set this parameter value in Cauldron model
      /// @param caldModel reference to Cauldron model
      /// @param caseID unique RunCase ID, in some parameters it is used in new map file name generation
      /// @return ErrorHandler::NoError in success, or error code otherwise     
      virtual ErrorHandler::ReturnCode setInModel( mbapi::Model & caldModel, size_t caseID );

      /// @brief Validate Porosity model parameter values
      /// @param caldModel reference to Cauldron model
      /// @return empty string on success or error message with current parameter value
      virtual std::string validate( mbapi::Model & caldModel );

      // The following methods are used for converting between CASA RunCase and SUMLib::Case objects
      
      /// @brief Get parameter value as an array of doubles
      /// @return parameter value represented as set of doubles
      virtual std::vector<double> asDoubleArray() const;

      /// @brief Get parameter value as integer
      /// @return parameter value represented as integer
      virtual int asInteger() const { assert( 0 ); return -1; }

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
      virtual const char * typeName() const { return "PrmLithoSTPThermalCond"; }

      /// @brief Create a new parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      PrmLithoSTPThermalCond( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:
      const VarParameter * m_parent;    ///< influential parameter which was used to create this one
      std::string          m_name;      ///< name of the parameter

      std::string          m_lithoName; ///< lithology name
      double               m_value;     ///< parameter value
   };
}


#endif // CASA_API_PARAMETER_LITHO_STP_THERMAL_COND_COEFF_H
