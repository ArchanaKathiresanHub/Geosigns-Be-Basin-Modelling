//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file RunManager.h
/// @brief This file keeps API declaration for creating and managing jobs on HPC cluster for DoE generated cases

#ifndef CASA_API_RUN_MANAGER_H
#define CASA_API_RUN_MANAGER_H

#include "ErrorHandler.h"

/// @page CASA_RunManagerPage Jobs execution manager
/// The casa::RunManager accepts the list of cases with project file names as input. 
/// It will spawn cases using Cauldron simulator on HPC cluster through LSF scheduler. 
/// Then it waits and asks with some interval LSF about the status of each case. When the 
/// last case is completed, it returns the run status for each case in terms of succeeded/failed.
/// @todo Work in 2015 could add an extra analysis of simulation failures, input to output matching, possible input duplications\n
/// (e.g. using one P/T run to feed many G/M runs). 

namespace casa
{
   class RunCase;

   /// @brief Allows to run set of Cases on HPC cluster
   class RunManager : public ErrorHandler
   {
   public:
      /// @brief Destructor
      virtual ~RunManager() {;}

      /// @brief add Case to set
      /// @param newRun new Case to be scheduled for run
      /// @return ErrorHandler::NoError on success or error code otherwise
      virtual ErrorHandler::ReturnCode scheduleCase( RunCase & newRun ) = 0;

      /// @brief Execute all scheduled cases
      /// @param asyncRun
      virtual ErrorHandler::ReturnCode runScheduledCases( bool asyncRun ) = 0;

   protected:
      RunManager() {;}
   };
}

#endif // CASA_API_RUN_MANAGER_H
