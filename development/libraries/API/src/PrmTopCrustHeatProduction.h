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

/// @page CASA_TopCrustHeatProductionPage Top crust heat production rate parameter
/// 
/// This parameter defines the surface Radiogenic heat production of the basement.
/// The value is defined in units: @f$ [ \mu W/m^3] @f$
/// It is continuous parameter and his range can be from 0 to any other positive value

namespace casa
{
   /// @brief Surface Radiogenic heat production of the basement parameter
   class PrmTopCrustHeatProduction : public Parameter
   {
   public:
      /// @brief Constructor 
      /// @param val value of top crust heat production rate
      PrmTopCrustHeatProduction( double val );

      /// @brief Constructor from cauldron model
      /// @param mdl get value of parameter from cauldron model
      PrmTopCrustHeatProduction( mbapi::Model & mdl );

      /// @brief Destructor
      virtual ~PrmTopCrustHeatProduction();
     
      /// @brief Set this parameter value in Cauldron model
      /// @param caldModel reference to Cauldron model
      /// @return ErrorHandler::NoError in success, or error code otherwise     
      virtual ErrorHandler::ReturnCode setInModel( mbapi::Model & caldModel );

      /// @brief Get value for the parameter as double
      /// @return parameter value
      double value() const { return m_heatProdRateValue; }

      // The following methods are used for testing  
      virtual bool isDouble( ) { return true; }
      virtual double doubleValue( ) { return value(); }

   protected:
      double m_heatProdRateValue;
   };

}

#endif // CASA_API_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_H
