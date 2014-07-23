//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file DataDiggerImpl.h
/// @brief This file keeps API implementation declaration for data digger. 

#ifndef CASA_API_DATA_DIGGER_IMPL_H
#define CASA_API_DATA_DIGGER_IMPL_H

#include "DataDigger.h"

namespace casa
{  
   // Data digger implementation 
   class DataDiggerImpl : public DataDigger
   {
   public:
      // Constructor / Destructor
      DataDiggerImpl();
      virtual ~DataDiggerImpl();

      /// @brief Insert into project file request for observables values using datadriller cauldron app and data mining interface.\n
      /// Observables must be requested after case mutation.
      /// @param obs casaObsSpace object which keeps list of observables. For each observable, data digger will request data using datadriller table
      /// @param rcs casa::RunCaseSet object which keeps list of observables
      /// @return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode requestObservables( ObsSpace & obs, RunCaseSet & rcs );

      // Collect observables value from simulation results for given case
      // obs - set of observables 
      // rcs - RunCaseSet object which keeps list of cases and reference to the cauldron model
      // return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode collectRunResults( ObsSpace & obs, RunCaseSet & rcs );

   protected:
   };
}

#endif // CASA_API_DATA_DIGGER_IMPL_H
