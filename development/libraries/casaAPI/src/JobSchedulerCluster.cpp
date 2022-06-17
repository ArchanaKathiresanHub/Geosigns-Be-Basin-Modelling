//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _WIN32

#include "JobSchedulerCluster.h"

#include "SDUWorkLoadManager.h"

#include <boost/property_tree/json_parser.hpp>

#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

static void Wait( int sec ) { sleep( sec ); }

namespace casa
{
std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr && result.size() < 200) {
        result += buffer.data();
    }
    return result;
}

  JobSchedulerCluster::JobSchedulerCluster(const std::string& /*clusterName*/)
  {
  }

  JobSchedulerCluster::JobSchedulerCluster(CasaDeserializer& /*dz*/, unsigned int /*objVer*/)
  {

  }

  JobSchedulerCluster::~JobSchedulerCluster(){}

  JobScheduler::JobID JobSchedulerCluster::addJob(const std::string& cwd, const std::string& scriptName, const std::string& jobName, int cpus, size_t runTimeLim, const std::string& scenarioID)
  {
    Job job;
    job.cwd = cwd;
    job.scriptName = scriptName;
    job.jobName = jobName;
    job.cpus = cpus;
    job.runTimeLim = runTimeLim;
    job.jobstate = JobState::NotSubmittedYet;
    job.ClusterJobID = -1;

    m_jobs.push_back(job);

    return m_jobs.size() - 1;
  }

  JobScheduler::JobState JobSchedulerCluster::runJob(JobScheduler::JobID job)
  {
    workLoadManager_.release();
    workLoadManager_ = workloadmanagers::WorkLoadManager::Create("run.sh", workloadmanagers::WorkLoadManagerType::AUTO);
    auto command = workLoadManager_->JobSubmissionCommand("cldrn", "", -1, m_jobs[job].jobName, "out.log",
                                                          "err.log", std::to_string(m_jobs[job].cpus), "", "", m_jobs[job].cwd, false, false,
                                                          m_jobs[job].scriptName).c_str();

#ifndef _WIN32
  chmod( "run.sh", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
#endif

    std::string result = exec(command);
    std::cout << result << std::endl;
    m_jobs[job].ClusterJobID = workLoadManager_->getJobIDFromOutputOfSubmissionCommand(result);

    m_jobs[job].jobstate = jobState(job);
    m_jobs[job].numberOfRuns++;
    return m_jobs[job].jobstate;
  }

  JobScheduler::JobState JobSchedulerCluster::stopJob(JobScheduler::JobID job)
  {
    system((workLoadManager_->JobTerminationCommand() + std::to_string(m_jobs[job].ClusterJobID)).c_str());
    return m_jobs[job].jobstate;
  }
  
  void JobSchedulerCluster::sleep( int secs )
  {
    if (      secs < 0 ) { Wait( 10 );   }
    else if ( secs > 0 ) { Wait( secs ); }
  }

  JobScheduler::JobState JobSchedulerCluster::jobState(JobScheduler::JobID job)
  {
    if (m_jobs[job].ClusterJobID == -1)
    {
      return m_jobs[job].jobstate;
    }

    try
    {
      std::string result = exec((workLoadManager_->JobStatusCommand() + std::to_string(m_jobs[job].ClusterJobID)).c_str());
      workloadmanagers::JobStatus status = workLoadManager_->getJobStatusFromOutputOfJobStatusCommand(result);

      if (status == workloadmanagers::JobStatus::JobStatusUnknown)
      {
        std::string result = exec((workLoadManager_->JobStatusCommandFinishedJobs(std::to_string(m_jobs[job].ClusterJobID))).c_str());

        status = workLoadManager_->getJobStatusFromOutputOfJobStatusFinishedJobsCommand(result);
      }


      if (status == workloadmanagers::JobStatus::JobFailed)
      {
        m_jobs[job].jobstate = JobFailed;
      }
      else if (status == workloadmanagers::JobStatus::JobPending)
      {
        m_jobs[job].jobstate = JobPending;

      }
      else if (status == workloadmanagers::JobStatus::JobRunning)
      {
        m_jobs[job].jobstate = JobRunning;
      }
      else if (status == workloadmanagers::JobStatus::JobFinished)
      {
        m_jobs[job].jobstate = JobFinished;
      }
    } catch (...)
    {
      return m_jobs[job].jobstate;
    }

    return m_jobs[job].jobstate;
  }

  bool JobSchedulerCluster::hasRunsLeft(JobScheduler::JobID job)
  {
     return m_jobs[job].numberOfRuns < 3;
  }
}
#endif
