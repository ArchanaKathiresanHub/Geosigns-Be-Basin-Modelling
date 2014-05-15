//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmTopCrustHeatProduction.h
/// @brief This file keeps API declaration for handling continuous parameters. 

#ifndef CASA_API_VAR_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_H
#define CASA_API_VAR_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_H

#include "ContinuousParameter.h"

namespace casa
{
   /// @brief Variation for casa::PrmTopCrustHeatProduction parameter
   class VarPrmTopCrustHeatProduction : public ContinuousParameter
   {
   public:
      VarPrmTopCrustHeatProduction( double baseValue, double minValue, double maxValue, PDF pdfType );
      virtual ~VarPrmTopCrustHeatProduction();

      /// @brief Get base value for the variable parameter as double
      /// @return base value
      virtual double baseValueAsDouble();

      virtual Parameter * createNewParameterFromDouble( double val );

   protected:
   };

}

#endif // CASA_API_VAR_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_H
