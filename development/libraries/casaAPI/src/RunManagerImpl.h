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
/// @brief This file keeps API implementation declaration for creating and managing jobs on HPC cluster for DoE generated cases using LSF

#ifndef CASA_API_RUN_MANAGER_LSF_IMPL_H
#define CASA_API_RUN_MANAGER_LSF_IMPL_H

#include "JobScheduler.h"
#include "RunManager.h"

#include <vector>
#include <string>

namespace casa
{

   class CauldronApp;
   class RunCaseImpl;

   /// @brief Allows to run set of Cases on HPC cluster
   class RunManagerImpl : public RunManager
   {
   public:

      RunManagerImpl( const std::string & clusterName = "" );
      virtual ~RunManagerImpl();

      /// @brief Add application to the list of simulators for pipeline calculation definitions
      /// @param app casa::CauldronApp object pointer
      /// @return ErrorHandler::NoError on success, error code otherwise
      virtual ErrorHandler::ReturnCode addApplication( CauldronApp * app );

      /// @brief Set up Cauldron version. Versions should be defined as "v2014.1007" (which is the default)
      ///        Version also could be defined by the environment variable CAULDRON_VERSION. This interface
      ///        has a priority over the environment variable.
      /// @param verString Cauldron version which will be used for submit jobs to HPC cluster.
      /// @return NoError on success, error code otherwise
      virtual ErrorHandler::ReturnCode setCauldronVersion( const char * verString ) { m_cldVersion = verString; return NoError; }

      /// @brief Add a new Case to the set
      /// @param newRun new Case to be scheduled for run
      /// @return ErrorHandler::NoError on success or error code otherwise
      virtual ErrorHandler::ReturnCode scheduleCase( RunCase & newRun );

      /// @brief Define how many jobs could be in Pending state. If there are to many jobs submitted,
      ///        cluster starts to reduce available slots for the user, in this case better do not 
      ///        submit all jobs in one time but keep feeding cluster with new jobs as previous one was finished
      /// @param pendJobsNum the number of pending jobs after which RunManager will stop to schedule the new jobs
      /// @return ErrorHandler::NoError on success or error code otherwise
      virtual ErrorHandler::ReturnCode setMaxNumberOfPendingJobs( size_t pendJobsNum );

      /// @brief Execute all scheduled cases
      /// @param asyncRun
      virtual ErrorHandler::ReturnCode runScheduledCases( bool asyncRun );

      /// @brief Set HPC cluster name
      /// @param clusterName name of the HPC cluster
      virtual ErrorHandler::ReturnCode setClusterName( const char * clusterName );

      /// @brief Return cluster name for which will be used for spawning jobs
      /// @return cluster name as a string
      virtual std::string clusterName();

      /// @brief Add case to the list of jobs and update case status by analysing completed stages into Case folder
      /// @param cs case object
      virtual void restoreCaseStatus( RunCase * cs );

      // Serialization / Deserialization
      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 1; }

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "RunManagerImpl"; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  version stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz, unsigned int version ) const;

      /// @brief Create a new DataDiggerImpl instance and deserialize it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      /// @return new observable instance on susccess, or throw and exception in case of any error
      RunManagerImpl( CasaDeserializer & inStream, const char * objName );
      /// @}


   protected:
      size_t                          m_maxPendingJobs; ///< define how much pending jobs should keep job scheduler
      std::string                     m_cldVersion;     ///< version of the cauldron to be used in calculations
      std::string                     m_ibsRoot;        ///< full path to cauldron installation

      std::vector< CauldronApp* >     m_appList;        ///< pipeline of cauldron applications to perform simulation
      std::auto_ptr<JobScheduler>     m_jobSched;       ///< OS dependent wrapper for the job scheduler

      std::vector< std::vector< JobScheduler::JobID > >   m_jobs;  ///< queue of jobs for each case
      std::vector< RunCaseImpl * >                        m_cases; ///< list of run cases
   
      void stopAllSubmittedJobs();                     ///< in case of scenario execution aborted - kill all submitted not finished jobs
   };
}

#endif // CASA_API_RUN_MANAGER_H
