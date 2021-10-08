//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by 
// CGI Information Systems & Management Consultants Pvt Ltd.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef __SDUWorkLoadManagerLOCAL_h__
#define __SDUWorkLoadManagerLOCAL_h__

#include "SDUWorkLoadManager.h"

namespace workloadmanagers {

	class WorkLoadManagerForLOCAL : public WorkLoadManager {
	public:
		WorkLoadManagerForLOCAL() = default;
		 ~WorkLoadManagerForLOCAL() final = default;
		 // no copy no move
		 WorkLoadManagerForLOCAL(const WorkLoadManagerForLOCAL&) = delete;
		 WorkLoadManagerForLOCAL& operator=(const WorkLoadManagerForLOCAL&) = delete;
		 std::string JobSubmissionCommand(
			 const std::string& project_name,
			 const std::string& queue_name,
			 int maximum_runtime_limit,
			 const std::string& job_name,
			 const std::string& outfilename,
			 const std::string& errorfilename,
			 const std::string& number_of_cores,
			 const std::string& setSpecificHost,
			 const std::string& resourceSpec, const std::string& cwd,
			 const bool isExlusive,
			 const bool isInteractiveSession,
			 const std::string& appToRun
		 ) override;

	protected:
		bool writeProjectNameSpecification(const std::string& theJobSubmissionProjectName) final;
		bool writeWaitTimeSpecification(int theJobSubmissionWaitTimeSpec) final;
		bool writeJobNameSpecification(const std::string& theJobSubmissionJobName) final;
		bool writeSlotsSpecification(const std::string& theJobSubmissionNProcsSpec) final;
		bool writeOutputLogSpecification(const std::string& theJobSubmissionOutputLogSpec) final;
		bool writeErrorLogSpecification(const std::string& theJobSubmissionErrorLogSpec) final;
		bool writeExlusivitySpecification(bool) final;
		bool writeInteractiveSessionSpecification(bool) final;
		bool writeCWDSpecification(const std::string& theJobSubmissionCWDSpec) final;
		bool writeQueueSpecification(const std::string& theJobSubmissionQueueSpec) final;
		std::string theSchedulerDirective() final;
		std::string theSchedulerJobSubmissionCommand()final;
		std::string JobTerminate() const override { return std::string(); }
		std::string JobStatus() const override { return std::string(); }
		std::string JobStatusFinishedJobs(const std::string& /*JobID*/) const override {return std::string();}
		int getJobIDFromOutputOfSubmissionCommand(const std::string &/*output*/) const override {return -1;}
		enum JobStatus getJobStatusFromOutputOfJobStatusCommand(const std::string &/*output*/) const override {return JobStatus::JobFailed;}
		enum JobStatus getJobStatusFromOutputOfJobStatusFinishedJobsCommand(const std::string &/*output*/) const override {return JobStatus::JobFailed;}
	};
}

#endif
