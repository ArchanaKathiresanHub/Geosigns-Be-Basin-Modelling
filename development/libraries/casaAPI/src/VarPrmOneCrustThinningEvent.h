//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmOneCrustTinningEvent.h
/// @brief This file keeps API declaration for handling variation of casa::PrmOneCrustThinningEvent parameter

#ifndef CASA_API_VAR_PARAMETER_ONE_CRUST_THINNING_EVENT_H
#define CASA_API_VAR_PARAMETER_ONE_CRUST_THINNING_EVENT_H

#include "VarPrmContinuous.h"

namespace casa
{
   /// @brief Variation for casa::PrmOneCrustThinningEvent parameter
   class VarPrmOneCrustThinningEvent: public VarPrmContinuous
   {
   public:
      VarPrmOneCrustThinningEvent( double baseThickIni, double minThickIni, double maxThickIni, 
                                   double baseT0,       double minT0,       double maxT0,       
                                   double baseDeltaT,   double minDeltaT,   double maxDeltaT,   
                                   double baseThingFct, double minThingFct, double maxThingFct, 
                                   PDF prmPDF );

      virtual ~VarPrmOneCrustThinningEvent();

      virtual Parameter * createNewParameterFromDouble( const std::vector<double> & val ) const;

   protected:
   };
}

#endif // CASA_API_VAR_PARAMETER_ONE_CRUST_THINNING_EVENT_H

