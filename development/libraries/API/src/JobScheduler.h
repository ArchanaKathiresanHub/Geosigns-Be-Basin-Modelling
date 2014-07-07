//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file JobScheduler.h
/// @brief This file keeps declaration the job scheduler which hides all system dependent implementation.\n
///        It is used by casa::RunManager and is not intended be exposed through CASA API
#ifndef CASA_CASE_SCHEDULER_H
#define CASA_CASE_SCHEDULER_H

#include <vector>
#include <string>

namespace casa
{

   // This class provides OS independent access to the job scheduler to RunManager.
   class JobScheduler
   {
   public:
      typedef enum
      {
         NotSubmittedYet, // job wasn't submitted yet
         JobSucceeded,    // job finished OK
         JobFailed,       // job crashed
         JobPending,      // job is waiting to run on cluster
         JobRunning,      // job is running on cluster
         JobFinished,     // job is finished or crashed
         Unknown          // job status is unknown
      } JobState;

      typedef size_t JobID ;

      // Constructor/Destructor
      JobScheduler( const std::string & clusterName = "" );
      ~JobScheduler();

      // get cluster name
      std::string clusterName() { return m_clusterName; }

      // Add job to the list
      JobID addJob( const std::string & cwd, const std::string & scriptName, const std::string & jobName, int cpus );
      
      // run job
      void runJob( JobID job );

      // get job state
      JobState jobState( JobID job );

      // Print all parameters of LSF batch system
      void printLSFBParametersInfo();

   private:
      class Job;                       // job OS dependent description

      std::vector<Job*> m_jobs;        // array of scheduled jobs
      std::string       m_clusterName; // name of the cluster. If not set obtained automaticly through LSF API
   };
}

#endif // CASA_CASE_SCHEDULER_H

