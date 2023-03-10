//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file JobSchedulerLocal.h
/// @brief This file keeps declaration of the class for job scheduler which submit jobs to the local computer\n
///        It is used by casa::RunManager and is not intended be exposed through CASA API

#ifndef CASA_CASE_SCHEDULER_LOCAL_H
#define CASA_CASE_SCHEDULER_LOCAL_H

#include "JobScheduler.h"

#include <vector>

namespace casa
{

   // This class provides access for casa::RunManager to the local job scheduler.
   class JobSchedulerLocal : public JobScheduler
   {
   public:
      // Constructor/Destructor
      JobSchedulerLocal( );
      virtual ~JobSchedulerLocal( );

      // get cluster name
      virtual void setClusterName( const char * clusterName );

      // Add job to the list
      virtual JobID addJob( const std::string & cwd
                          , const std::string & scriptName
                          , const std::string & jobName
                          , int                 cpus
                          , size_t              runTimeLim
                          , const std::string & scenarioID );

      // run job
      virtual JobState runJob( JobID job );

      // stop submitted job
      virtual JobState stopJob( JobID job );

      // get job state
      virtual JobState jobState( JobID job );

      // should mpirun command contains -np CPUS or number of cpus will be requested by the job scheduler
      virtual bool cpusNumberByScheduler() { return false; }

      // Wait a bit (~10 sec) before asking about job state again
      // for the LOCAL cluster - do nothing
      // secs -1 - use default value for scheduler, 0 or positive number seconds to sleep
      virtual void sleep( int secs = -1 );

      // version of serialized object representation
      virtual unsigned int version() const { return 0; }

      bool hasRunsLeft(JobID /*job*/) override { return false; }

      // Serialize object to the given stream
      virtual bool save( CasaSerializer & sz ) const;

      // Get type name of the serialaizable object, used in deserialization to create object with correct type
      virtual std::string typeName() const { return "JobSchedulerLocal"; }

      // Create a new instance and deserialize it from the given stream
      JobSchedulerLocal( CasaDeserializer & dz, unsigned int objVer );

   private:
      class Job;                       // job OS dependent description
      std::vector<Job*> m_jobs;        // array of scheduled jobs
      size_t            m_avCPUs;      // number of cores in system

      size_t            runningJobsNumber(); // get number of running jobs

      JobSchedulerLocal( const JobSchedulerLocal & jbS );
      JobSchedulerLocal & operator = ( const JobSchedulerLocal & jbS );
   };
}

#endif // CASA_CASE_SCHEDULER_LOCAL_H

