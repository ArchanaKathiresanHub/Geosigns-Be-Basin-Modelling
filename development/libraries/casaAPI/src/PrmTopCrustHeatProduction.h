//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmTopCrustHeatProduction.h
/// @brief This file keeps API declaration for Top Crust Heat Production Rate parameter. 

#ifndef CASA_API_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_H
#define CASA_API_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_H

#include "Parameter.h"

// CMB API
#include <UndefinedValues.h>

#include <cassert>

/// @page CASA_TopCrustHeatProductionPage Top crust heat production rate parameter
/// 
/// This parameter defines the surface Radiogenic heat production of the basement.
/// The value is defined in units: @f$ [ \mu W/m^3] @f$
/// It is continuous parameter and his range can be from 0 to any other positive value

namespace casa
{
   class VarPrmTopCrustHeatProduction;

   /// @brief Surface Radiogenic heat production of the basement parameter
   class PrmTopCrustHeatProduction : public Parameter
   {
   public:
      /// @brief Constructor. Create parameter by reading parameter value from the given model
      /// @param mdl get value of parameter from cauldron model
      PrmTopCrustHeatProduction( mbapi::Model & mdl );

      /// @brief Constructor. Create parameter from variation of variable parameter
      /// @param parent pointer to a variable parameter which created this one
      /// @param val value of top crust heat production rate @f$ [ \mu W/m^3] @f$
      PrmTopCrustHeatProduction( const VarPrmTopCrustHeatProduction * parent, double val );

      /// @brief Destructor
      virtual ~PrmTopCrustHeatProduction();
     
      /// @brief Get name of the parameter
      /// @return parameter name
      virtual const char * name() const { return "TopCrustHeatProdRate"; }

      /// @brief Get variable parameter which was used to create this parameter
      /// @return Pointer to the variable parameter
      virtual const VarParameter * parent() const { return m_parent; }

      /// @brief Set this parameter value in Cauldron model
      /// @param caldModel reference to Cauldron model
      /// @return ErrorHandler::NoError in success, or error code otherwise     
      virtual ErrorHandler::ReturnCode setInModel( mbapi::Model & caldModel );

      /// @brief Validate top crust heat production rate value if it is in positive range
      ///        also it checks does the given model has the same value for this parameter.
      /// @param caldModel reference to Cauldron model
      /// @return empty string on success or error message with current parameter value
      virtual std::string validate( mbapi::Model & caldModel );

      /// @brief Get value for the parameter as double
      /// @return parameter value
      double value() const { return m_heatProdRateValue; }

      // The following methods are used for converting between CASA RunCase and SUMLib::Case objects
      
      /// @brief Get parameter value as an array of doubles
      /// @return parameter value represented as set of doubles
      virtual std::vector<double> asDoubleArray() const { return std::vector<double>( 1, value() ); }
   
      /// @brief Get parameter value as integer
      /// @return parameter value represented as integer
      virtual int asInteger() const { assert(0); return UndefinedIntegerValue; }


   protected:
      const VarParameter  * m_parent;            ///< variable parameter which was used to create this one
      double                m_heatProdRateValue; ///< top crust heat production rate value
   };

}

#endif // CASA_API_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_H
