//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once
#ifndef _WIN32

#include "JobScheduler.h"
#include <memory>

namespace workloadmanagers
{
  class WorkLoadManager;
}

namespace casa
{

  struct Job
  {
    std::string cwd;
    std::string scriptName;
    std::string jobName;
    int cpus;
    size_t runTimeLim;
    JobScheduler::JobState jobstate;
    int ClusterJobID = -1;
    int numberOfRuns = 0;
  };

  class JobSchedulerCluster : public JobScheduler
  {
  public:
    JobSchedulerCluster( const std::string & clusterName = "" );
    JobSchedulerCluster( CasaDeserializer & dz, unsigned int objVer );

    ~JobSchedulerCluster() override;

    JobID addJob( const std::string & cwd
                , const std::string & scriptName
                , const std::string & jobName
                , int                 cpus
                , size_t              runTimeLim
                , const std::string & scenarioID ) override;

    void setClusterName(const char *clusterName) override {}
    void sleep(int secs = -1) override;

    JobState runJob(JobID job) override;
    JobState stopJob(JobID job) override;
    JobState jobState(JobID job) override;

    bool cpusNumberByScheduler() override {return false;}

    bool hasRunsLeft(JobID job) override;

    // version of serialized object representation
    virtual unsigned int version() const override { return 1; }

    // Serialize object to the given stream
    bool save( CasaSerializer & sz ) const override {return true; }

    // Get type name of the serialaizable object, used in deserialization to create object with correct type
    std::string typeName() const override { return "JobSchedulerCluster"; }

  private:
    std::vector<Job> m_jobs;
    std::unique_ptr<workloadmanagers::WorkLoadManager> workLoadManager_;
  };

}

#endif
