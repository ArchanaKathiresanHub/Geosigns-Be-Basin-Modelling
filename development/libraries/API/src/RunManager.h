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
#include "CauldronApp.h"

/// @page CASA_RunManagerPage Jobs execution manager
/// The casa::RunManager accepts the list of cases with project file names as input. 
/// It spawns cases using Cauldron simulator on HPC cluster through LSF scheduler. 
/// Then it waits and asks with some interval LSF about the status of each case. When the 
/// last case is completed, it returns the run status for each case in terms of succeeded/failed.
///
/// RunManager configuration could be changed by the following environment variables:
/// - \b SIEPRTS_LICENSE_FILE - the default value is: 3000@houic-s-9320.americas.shell.com:3000@cbj-s-8447.asia-pac.shell.com:3000@ams1-s-07489.europe.shell.com
/// - \b CAULDRON_MPIRUN_CMD  - Defines which mpirun command will be used and it parameters. The default value is:\n
///    "source /apps/3rdparty/intel/impi/4.1.1.036/intel64/bin/mpivars.sh \n mpirun -outfile-pattern 'output-rank-%r.log' -env I_MPI_FABRICS shm:tcp -env I_MPI_DEBUG 5 ";
/// - \b CAULDRON_VERSION     - Version of Cauldron which will be used for submitting jobs to the cluster. The default value is: \b v2014.0703 
/// - \b IBS_ROOT             - Path to IBS folder where the different versions of the Cauldron could be found. The default value is: /apps/sssdev/ibs
/// RunManager API has a priority over these environment variables.
///
/// CASA::RunCase keeps information about project file only, it has no any information which simulator should be run with this project. To create
/// the list of cauldron applications like fastcauldron, fastgenex, etc, user should add to casa::RunManager the set of casa::CauldronApp objects.
/// Each CauldronApp object represents one of the Cauldron simulator applications. The current list of available now cauldron applications is following:
///   - fastcauldron (To model deposition and erosion of sedimentary layers, pressure and temperature in these layers).
///   - fastgenex6   (To model Generation and expulsion of hydrocarbon from source rocks).
///   - fastmig      (To model migration of hydrocarbon from source rock to reservoir layers).
///   - fastctc      (To calculate crust thickness history. Not implemented yet in casa) @TODO: implement as CauldronApp
///   - fasttouch7   (To calculate reservoir quality using Geocosm ResQ library. Not implemented yet in casa) @TODO: implement as CauldronApp
///   - general      (Any other application)
///
/// casa::CauldronApp object should be created using static function casa::RunManager::createApplication(). User could add one by one options,
/// to the application through casa::CauldronApp::addOption( std::string ). And then add to calculation pipeline using 
/// RunManager::addApplication() method.

///
/// @todo Work in 2015 could add an extra analysis of simulation failures, input to output matching, possible input duplications\n
/// (e.g. using one P/T run to feed many G/M runs). 

namespace casa
{
   class RunCase;

   /// @brief Allows to run set of Cases on HPC cluster
   class RunManager : public ErrorHandler
   {
   public:
      /// @name Types definitions
      /// @{
      /// @brief Cauldron simulator applications
      typedef enum
      {
         fastcauldron, ///< Conventional (Pressure/Temperature), unconventional (Darcy)
         fastctc,      ///< TODO document here
         fastgenex6,   ///< hydrocarbons expulsion from source rock
         fasttouch7,   ///< ResQ reservoir quality
         fastmig,      ///< hydrocarbons primary migration
         track1d,      ///< extract data along vertical well for given position
         generic       ///< allows to define any application not mentioned over
      } ApplicationType;
      /// @}
      
      /// @brief Destructor
      virtual ~RunManager() {;}

      /// @brief Factory for creating application objects which can be used for creating calculation pipeline
      /// @param appType type of application
      /// @param cpus optional parameters, it defines the number of cpus to use in this application. The default value is 1
      /// @param cmdLine optional parameter for general application. This string should contains the script itself
      /// @return pointer to the application object which later should be passed to RunManager::addApplication() method
      static CauldronApp * createApplication( ApplicationType appType, int cpus = 1, const std::string cmdLine = "" );

      /// @brief Set up Cauldron version. Version should be defined as "v2014.1007" (which is the default)\n
      ///        Version also could be defined by the environment variable CAULDRON_VERSION. This interface\n
      ///        has a priority over the environment variable.
      /// @param verString Cauldron version which will be used for submit jobs to HPC cluster.
      /// @return NoError on success, error code otherwise
      virtual ErrorHandler::ReturnCode setCauldronVersion( const char * verString ) = 0;

      // TODO add methods to set up IBS_ROOT

      /// @brief Add application to the list of simulators for pipeline calculation definitions
      /// @param app casa::CauldronApp object pointer
      /// @return ErrorHandler::NoError on success, error code otherwise
      virtual ErrorHandler::ReturnCode addApplication( CauldronApp * app ) = 0;

      /// @brief add Case to set
      /// @param newRun new Case to be scheduled for run
      /// @return ErrorHandler::NoError on success or error code otherwise
      virtual ErrorHandler::ReturnCode scheduleCase( const RunCase & newRun ) = 0;

      /// @brief Execute all scheduled cases
      /// @param asyncRun
      virtual ErrorHandler::ReturnCode runScheduledCases( bool asyncRun ) = 0;

      /// @brief Return cluster name for which will be used for spawning jobs
      /// @return cluster name as a string
      virtual std::string clusterName() = 0;

   protected:
      RunManager() {;}
   };
}

#endif // CASA_API_RUN_MANAGER_H
