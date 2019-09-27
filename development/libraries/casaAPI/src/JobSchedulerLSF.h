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
      virtual JobID addJob( const std::string & cwd
                          , const std::string & scriptName
                          , const std::string & jobName
                          , int                 cpus
                          , size_t              runTimeLim
                          , const std::string & scenarioID
                          );

      // run job
      virtual JobState runJob( JobID job );

      // stop submitted job
      virtual JobState stopJob( JobID job );

      // get job state
      virtual JobState jobState( JobID job );

      // get internal job ID for logging
      virtual std::string schedulerJobID( JobID job );

      // should mpirun command contains -np CPUS or number of cpus will be requested by the job scheduler
      virtual bool cpusNumberByScheduler() { return false; }

      // Wait a bit (~10 sec) before asking about job state again
      // for the LOCAL cluster - do nothing
      // secs -1 - use default value for scheduler, 0 or positive number seconds to sleep
      virtual void sleep( int secs = -1 );

      // Print all parameters of LSF batch system
      void printLSFBParametersInfo();

      // Defines resource request string for LSF job scheduler in the same format as bsub -R option
      void setResourceRequirements( const std::string & resReqStr ) { m_resReqStr = resReqStr; }

      // Get current resource request string for LSF
      std::string resourceRequirements() { return m_resReqStr; }

      // Serialization / Deserialization

      // version of serialized object representation
      virtual unsigned int version() const { return 1; }

      // Serialize object to the given stream
      virtual bool save( CasaSerializer & sz ) const;

      // Get type name of the serialaizable object, used in deserialization to create object with correct type
      virtual const char * typeName() const { return "JobSchedulerLSF"; }

       // Create a new instance and deserialize it from the given stream
      JobSchedulerLSF( CasaDeserializer & dz, unsigned int objVer );

   private:
      class Job;                       // job OS dependent description
      friend class Job;

      std::vector<Job*> m_jobs;        // array of scheduled jobs
      std::string       m_resReqStr;   // resource request string in format bsusb -R option

      std::string       m_sla;         // keep SLA to pass the next job
      std::string       m_prjGrp;      // keep project group to pas to the next job
      JobSchedulerLSF( const JobSchedulerLSF & jbS );
      JobSchedulerLSF & operator = ( const JobSchedulerLSF & jbS );
   };
}

#endif // CASA_CASE_SCHEDULER_LSF_H

