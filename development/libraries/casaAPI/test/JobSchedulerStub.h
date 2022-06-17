//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "../src/JobScheduler.h"

namespace casa
{
   struct JobStub
   {
      JobScheduler::JobState jobstate = JobScheduler::JobState::NotSubmittedYet;
      int numberOfRuns = 0;
   };

   class JobSchedulerStub : public JobScheduler
   {
   public:
      ~JobSchedulerStub() override {}

      JobID addJob( const std::string & /*cwd*/
                    , const std::string & /*scriptName*/
                    , const std::string & /*jobName*/
                    , int                 /*cpus*/
                    , size_t              /*runTimeLim*/
                    , const std::string & /*scenarioID*/ ) override
      {
         m_jobs.push_back(JobStub());
         return m_jobs.size() - 1;
      }

      void setClusterName(const char *clusterName) override {}
      void sleep(int secs = -1) override {}

      JobState runJob(JobID job) override
      {
         m_jobs[job].numberOfRuns++;
         m_jobs[job].jobstate = job == 1 ? JobState::JobFailed: JobState::JobFinished; // make sure job 1 fails, while all other jobs finish right away
         return m_jobs[job].jobstate;
      }

      JobState stopJob(JobID job) override
      {
         return m_jobs[job].jobstate;
      }

      JobState jobState(JobID job) override
      {
         return m_jobs[job].jobstate;
      }

      bool cpusNumberByScheduler() override
      {
         return false;
      }

      bool hasRunsLeft(JobID job) override
      {
         return m_jobs[job].numberOfRuns < 3;
      }

      // version of serialized object representation
      virtual unsigned int version() const override {
         return 1;
      }

      // Serialize object to the given stream
      bool save( CasaSerializer & sz ) const override
      {
         return true;
      }

      // Get type name of the serializable object, used in deserialization to create object with correct type
      std::string typeName() const override
      {
         return "JobSchedulerStub";
      }

      int numberOfRuns(JobID job)
      {
         return m_jobs[job].numberOfRuns;
      }

   private:
      std::vector<JobStub> m_jobs;
   };

}
