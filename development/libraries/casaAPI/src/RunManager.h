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

// CMB
#include "ErrorHandler.h"

// CASA
#include "CasaSerializer.h"
#include "CauldronApp.h"

/// @page CASA_RunManagerPage Jobs execution manager
/// The casa::RunManager accepts the list of cases with project file names as input.
/// It spawns a cauldron simulation run cases on HPC cluster using LSF jobs scheduler by calling the corresponded methods of LSF API.
/// Then it waits and asks with some interval LSF about the status of each case. When the
/// case is completed, it updates the case run status in terms of succeeded/failed.
///
/// RunManager configuration could be changed by the following environment variables:
/// - @b SIEPRTS_LICENSE_FILE - Defines the list of license servers for cauldron applications. The default value is:
///      @code 3000@houic-s-9320.americas.shell.com:3000@cbj-s-8447.asia-pac.shell.com:3000@ams1-s-07489.europe.shell.com @endcode
/// - @b CAULDRON_MPIRUN_CMD  - Defines which mpirun command will be used and it parameters. The default value is:
///      @code
///      source /apps/3rdparty/intel/impi/4.1.1.036/intel64/bin/mpivars.sh
///      mpirun -outfile-pattern 'output-rank-%r.log' -env I_MPI_FABRICS shm:tcp -env I_MPI_DEBUG 5
///      @endcode
/// - @b CAULDRON_VERSION - Defines the version of Cauldron which will be used for submitting jobs to the cluster. The default value is:
///      @code v2014.0703 @endcode
/// - @b IBS_ROOT - Defines a path to IBS folder where the different versions of the Cauldron could be found. The default value is:
///      @code /apps/sssdev/ibs @endcode
/// - @b LSF_CAULDRON_PROJECT_NAME - allows to define a job project name (bsub -P ...)
/// - @b LSF_CAULDRON_PROJECT_QUEUE - allows to define a project queue (bsub -q .. )
/// - @b LSF_CAULDRON_PROJECT_GROUP - allows to define a project group (bsub -G ...)
/// - @b LSF_CAULDRON_PROJECT_SERVICE_CLASS_NAME - allows to define a project class service name (bsla)
/// RunManager API has a priority over these environment variables.
///
/// casa::RunCase keeps information about project file only, it has no any information which simulator should be run with this project. To create
/// a list of cauldron applications like fastcauldron, fastgenex, etc, user should add to casa::RunManager the set of casa::CauldronApp objects.
/// Each CauldronApp object represents one of the Cauldron simulator applications. The list of available now Cauldron applications is following:
///   - @b fastcauldron - To model deposition and erosion of sedimentary layers, pressure and temperature in these layers.
///   - @b fastgenex6 - To model Generation and expulsion of hydrocarbon from source rocks.
///   - @b fastmig - To model migration of hydrocarbon from source rock to reservoir layers.
///   - @b fastctc - To calculate crust thickness history.
///   - @b tracktraps - To search traps and generate trapID property
///   - @b track1d - To extract data along vertical well for given position
///   - @b general - Any other application defined by given script body.
///
/// casa::CauldronApp object shall be created using static function casa::RunManager::createApplication(). Then user could add one by one option
/// to the application through casa::CauldronApp::addOption( std::string ) method. And then add application to calculation pipeline using
/// RunManager::addApplication() method. Here is an example:
/// @code
///    CauldronApp * app = RunManager::createApplication( RunManager::fastcauldron );
///    app->addOption( "-itcoupled" );
///    runManager.addApplication( app );
///
///    app = RunManager::createApplication( RunManager::fastgenex6 );
///    runManager.addApplication( app );
/// @endcode
///
/// DoE algorithm variate the influential parameter values for all cauldron simulators included in pipeline. But some parameters
/// have influence on early and some for later stages of applications pipeline. For example, source rock TOC value has no influence
/// on pressure/temperature (P/T) calculation stage but has a big influence on genex and migration stages. Each DoE run case has
/// the same set of applications in pipeline that is why for some cases, the P/T simulation could be the same. To make it more
/// optimal, each influential parameter has its own @link casa::AppPipelineLevel level @endlink which shows on which cauldron
/// applications it has influence. For example @link casa::PrmSurfacePorosity surface porosity @endlink parameter has an influence
/// on P/T simulation and @link casa::PrmSourceRockHI source rock hydrogen index @endlink has an influence on genex simulation.
/// When run manager adds the new run case jobs to the job queue, it performs check over all previously added cases - are the influential
/// parameter values are equal to the new one, up to the current stage? If yes, it adds job to make a symbolic links of result files.
/// For example DoE has generated 4 run cases for 2 influential parameters:
///
/// - Case 1: surface porosity 60%, source rock HI 400
/// - Case 2: surface porosity 80%, source rock HI 800
/// - Case 3: surface porosity 80%, source rock HI 400
/// - Case 4: surface porosity 60%, source rock HI 800
///
/// Cases 1 and 4, cases 2 and 3 will have the same surface porosity value which will influence the P/T simulation stage. The HI value
/// will have influence on the later, genex, stage only. For cases 1 and 4, 2 and 3 the only one P/T simulation could be done per pair cases.
/// In this situation, the run manager will add two P/T runs for cases 1 and 2. For cases 4 and 3 it will add the script, which
/// will make symbolic links for the corresponded P/T simulation results, after the P/T simulations will be finished.


namespace casa
{
   class RunCase;
   class JobScheduler;

   /// @brief Allows to run set of Cases on HPC cluster
   class RunManager : public ErrorHandler, public CasaSerializable
   {
   public:
      /// @name Types definitions
      /// @{
      /// @brief Cauldron simulator applications
      enum ApplicationType
      {
         fastcauldron, ///< Conventional (Pressure/Temperature), unconventional (Darcy)
         fastctc,      ///< Crust thickness history calculation
         fastgenex6,   ///< Hydrocarbons expulsion from source rock
         fastmig,      ///< Hydrocarbons primary migration
         tracktraps,   ///< Traps search
         track1d,      ///< Extract data along vertical well for given position
         datadriller,  ///< Extract data from the simulation results using DataMiningIoTbl as a requests set
         generic,      ///< Allows to define any application not mentioned over
         casa          ///< Run set of 1D scenarios in calibration mode
      };
      /// @}

      /// @name Constants definitions
      /// @{
      static const char * s_scenarioExecStopFileName; /// To stop scenario execution user could create in current folder file with this name
      static const char * s_jobsIDListFileName;       /// File name to keep the plain list of submitted jobs
      ///@}

      /// @brief Destructor
      virtual ~RunManager() {}

      /// @brief Factory for creating application objects which can be used for creating calculation pipeline
      /// @return pointer to the application object which later should be passed to RunManager::addApplication() method
      static CauldronApp * createApplication( ApplicationType   appType           ///< type of application
                                            , int               cpus = 1          ///< (optional) run applicaton on this cpus number
                                            , size_t            runTimeLimit = 0  ///< (optional) hard time limit for application
                                            , const std::string cmdLine = ""      ///< (optional) if app is "general", contains the script body
                                            );

      /// @brief Set up Cauldron version. Version should be defined as "v2020.01nightly" (which is the default)
      ///        Version also could be defined by the environment influential CAULDRON_VERSION. This interface
      ///        has a priority over the environment influential.
      /// @param verString Cauldron version which will be used for submit jobs to HPC cluster.
      /// @return NoError on success, error code otherwise
      virtual ErrorHandler::ReturnCode setCauldronVersion( const std::string& verString ) = 0;

      /// @todo Add interface for casa::RunManager to set up IBS_ROOT

      /// @brief Add application to the list of simulators for pipeline calculation definitions
      /// @param app casa::CauldronApp object pointer
      /// @return ErrorHandler::NoError on success, error code otherwise
      virtual ErrorHandler::ReturnCode addApplication( CauldronApp * app ) = 0;

      /// @brief Add a new Case to the set
      /// @param newRun new Case to be scheduled for run
      /// @param scenarioID some unique name of the scenario to have reference in GUI
      /// @return ErrorHandler::NoError on success or error code otherwise
      virtual ErrorHandler::ReturnCode scheduleCase( RunCase & newRun, const std::string & scenarioID ) = 0;

      /// @brief Execute all scheduled cases
      /// @param updateStateTimeInterval how much to sleep before ask for the jobs states
      /// @return ErrorHandler::NoError on success or error code otherwise
      virtual ErrorHandler::ReturnCode runScheduledCases( int updateStateTimeInterval = -1 ) = 0;

      /// @brief In case of scenario execution aborted (any exceptions for example) - it kills all submitted but not finished jobs
      /// @return ErrorHandler::NoError on success or error code otherwise
      virtual ErrorHandler::ReturnCode stopAllSubmittedJobs() = 0;

      /// @brief Define how many jobs could be in Pending state. If there are to many jobs submitted,
      ///        cluster starts to reduce available slots for the user, in this case better do not
      ///        submit all jobs in one time but keep feeding cluster with new jobs as previous one was finished
      /// @param pendJobsNum the number of pending jobs after which RunManager will stop to schedule the new jobs
      /// @return ErrorHandler::NoError on success or error code otherwise
      virtual ErrorHandler::ReturnCode setMaxNumberOfPendingJobs( size_t pendJobsNum ) = 0;

      /// @brief Set HPC cluster name
      /// @param clusterName name of the HPC cluster
      virtual ErrorHandler::ReturnCode setClusterName( const char * clusterName ) = 0;

      /// @brief Return cluster name for which will be used for spawning jobs
      /// @return cluster name as a string
      virtual std::string clusterName() = 0;

      /// @brief Clean jobs and recreate job scheduler. Keep all RunManager settings
      /// @param cleanApps if set to true, function also cleans application pipeline
      /// @param cleanCases if set to true, function also cleans run cases
      virtual void resetState(bool cleanApps = true) = 0;

      /// @brief Add case to the list of jobs and update case status by analysing completed stages into Case folder
      /// @param cs case object
      virtual void restoreCaseStatus( RunCase * cs ) = 0;

      virtual void setJobScheduler (JobScheduler* scheduler) = 0;

   protected:
      RunManager() {}
   };
}

#endif // CASA_API_RUN_MANAGER_H
