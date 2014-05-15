//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file RunMangerImpl.h
/// @brief This file keeps API implementation declaration for creating and managing jobs on HPC cluster for DoE generated cases


#ifndef CASA_API_RUN_MANAGER_IMPL_H
#define CASA_API_RUN_MANAGER_IMPL_H

#include "RunManager.h"

namespace casa
{

   /// @brief Allows to run set of Cases on HPC cluster
   class RunManagerImpl : public RunManager
   {
   public:
      RunManagerImpl();
      virtual ~RunManagerImpl();

      /// @brief add Case to set
      /// @param newRun new Case to be scheduled for run
      /// @return ErrorHandler::NoError on success or error code otherwise
      virtual ErrorHandler::ReturnCode scheduleCase( RunCase & newRun );

      /// @brief Execute all scheduled cases
      /// @param asyncRun
      virtual ErrorHandler::ReturnCode runScheduledCases( bool asyncRun );

   protected:
   };
}

#endif // CASA_API_RUN_MANAGER_H
