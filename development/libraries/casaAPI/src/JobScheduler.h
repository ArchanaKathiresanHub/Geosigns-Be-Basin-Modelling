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
/// @brief This file keeps declaration the base class for job scheduler
///        It is used by casa::RunManager and is not intended be exposed through CASA API
#ifndef CASA_CASE_SCHEDULER_H
#define CASA_CASE_SCHEDULER_H

// CMB
#include "ErrorHandler.h"

// CASA
#include "CasaSerializer.h"
// STL
#include <string>

namespace casa
{

   // This class provides OS independent access to the job scheduler to RunManager.
   class JobScheduler : public CasaSerializable
   {
   public:
      enum JobState
      {
         NotSubmittedYet, // job wasn't submitted yet
         JobSucceeded,    // job finished OK
         JobFailed,       // job crashed
         JobPending,      // job is waiting to run on cluster
         JobRunning,      // job is running on cluster
         JobFinished,     // job is finished or crashed
         Unknown          // job status is unknown
      };

      typedef size_t JobID ;

      // Constructor/Destructor
      virtual ~JobScheduler() { ; }


      // get cluster name
      virtual void setClusterName( const char * clusterName ) = 0;
 
      // get cluster name
      std::string clusterName() { return m_clusterName; }

      // Add job to the list
      virtual JobID addJob( const std::string & cwd, const std::string & scriptName, const std::string & jobName, int cpus ) = 0;

      // Restore job state by analysing output files
      virtual JobState restoreJobState( const std::string & cwd, const std::string & scriptName, const std::string & jobName );

      // run job
      virtual JobState runJob( JobID job ) = 0;

      // get job state
      virtual JobState jobState( JobID job ) = 0;

      // should mpirun command contains -np CPUS or number of cpus will be requested by the job scheduler
      virtual bool cpusNumberByScheduler() = 0;

      // Wait a bit (~10 sec) before asking about job state again
      // for the LOCAL cluster - do nothing
      virtual void sleep() = 0;

      static JobScheduler *  load( CasaDeserializer & dz, const char * objName );

   protected:
      JobScheduler() { ; }

      std::string m_clusterName; // name of the cluster. If not set obtained automatically through LSF API

   private:      
      JobScheduler( const JobScheduler & jbS );
      JobScheduler & operator = ( const JobScheduler & jbS );
   };
}

#endif // CASA_CASE_SCHEDULER_H

