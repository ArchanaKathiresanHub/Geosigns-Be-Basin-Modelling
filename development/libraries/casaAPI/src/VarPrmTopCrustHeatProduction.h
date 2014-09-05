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
/// @brief This file keeps API declaration for handling variation of casa::PrmTopCrustHeatProduction parameter

#ifndef CASA_API_VAR_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_H
#define CASA_API_VAR_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_H

#include "VarPrmContinuous.h"

namespace casa
{
   /// @brief Variation for casa::PrmTopCrustHeatProduction parameter
   class VarPrmTopCrustHeatProduction : public VarPrmContinuous
   {
   public:
      VarPrmTopCrustHeatProduction( double baseValue, double minValue, double maxValue, PDF pdfType );
      virtual ~VarPrmTopCrustHeatProduction();

      virtual Parameter * createNewParameterFromDouble( const std::vector<double> & vals ) const;

   protected:
   };

}

#endif // CASA_API_VAR_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_H
