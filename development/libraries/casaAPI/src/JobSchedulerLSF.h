//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file JobSchedulerLSF.h
/// @brief This file keeps declaration the job scheduler which uses LSF to submit jobs\n
///        It is used by casa::RunManager and is not intended be exposed through CASA API
#ifndef CASA_CASE_SCHEDULER_LSF_H
#define CASA_CASE_SCHEDULER_LSF_H

#include "JobScheduler.h"

#include <vector>

namespace casa
{

   // This class provides OS independent access to the job scheduler to RunManager.
   class JobSchedulerLSF : public JobScheduler
   {
   public:

      // Constructor/Destructor
      JobSchedulerLSF( const std::string & clusterName = "" );
      virtual ~JobSchedulerLSF();


      // get cluster name
      virtual void setClusterName( const char * clusterName ) { m_clusterName = clusterName; }

      // Add job to the list
      virtual JobID addJob( const std::string & cwd, const std::string & scriptName, const std::string & jobName, int cpus );
      
      // run job
      virtual JobState runJob( JobID job );

      // get job state
      virtual JobState jobState( JobID job );

      // should mpirun command contains -np CPUS or number of cpus will be requested by the job scheduler
      virtual bool cpusNumberByScheduler() { return true; }

      // Wait a bit (~10 sec) before asking about job state again
      // for the LOCAL cluster - do nothing
      virtual void sleep();

      // Print all parameters of LSF batch system
      void printLSFBParametersInfo();

      // Serialization / Deserialization

      // version of serialized object representation
      virtual unsigned int version() const { return 0; }

      // Serialize object to the given stream
      virtual bool save( CasaSerializer & sz, unsigned int version ) const;

      // Create a new instance and deserialize it from the given stream
      JobSchedulerLSF( CasaDeserializer & dz, unsigned int objVer );

   private:
      class Job;                       // job OS dependent description
      std::vector<Job*> m_jobs;        // array of scheduled jobs
      
      JobSchedulerLSF( const JobSchedulerLSF & jbS );
      JobSchedulerLSF & operator = ( const JobSchedulerLSF & jbS );
   };
}

#endif // CASA_CASE_SCHEDULER_LSF_H

