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

#ifndef __SDUWorkLoadManager_h__
#define __SDUWorkLoadManager_h__

#include "boost/filesystem.hpp"
#include <iostream>
#include <string>
#include <memory>
#include <fstream> // the class declarations for file stream objects

namespace workloadmanagers
{
	/// <summary>
	/// A Simple WLM Exception class
	/// </summary>
	struct WLMException : public std::exception
	{
		const char* what() const throw() override
		{
			return "WLM Exception!";
		}
	};

	enum class WorkLoadManagerType {
		LOCAL, IBM_LSF, SLURM, OTHER, AUTO
	};

	enum class JobStatus
	{
		JobFailed, JobPending, JobRunning, JobFinished, JobStatusUnknown
	};

	std::string GetCurrentWorkingDir();

	/// <summary>
	/// This is the storage class for all variables related
	/// to HPC
	/// </summary>
	class allThingsWlm
	{
	public:
		int getNProcs(void) const { return m_nprocs; };
		void setNProcs(int nprocs) { m_nprocs = nprocs; };

		std::string getErrorLogName(void) const { return m_errorLog; };
		std::string getOutputLogName(void) const { return m_outputLog; };

		bool getIsExlusiveFlag(void) const { return m_isExclusive; };
		void setIsExlusiveFlag(bool IsExlusive) { m_isExclusive = IsExlusive; };

		bool getInteractiveSession(void) const { return m_isInteractiveSession; };
		void setInteractiveSession(bool IsInteractive) { m_isInteractiveSession = IsInteractive; };
		std::string getClusterName(void)const { return m_clusterName; }
		void setClusterName(const std::string& clstrNm) { m_clusterName = clstrNm; };

	private:
		int m_nprocs = 1;
		std::string m_errorLog = "ERROR.log";
		std::string m_outputLog = "OUTPUT.log";
		bool m_isExclusive = false;
		bool m_isInteractiveSession = false;
		std::string m_queue = "default.q";
		std::string m_jobName = "myHPCJob";
		std::string m_clusterName = "LOCAL";
	};

	/// <summary>
	/// This is the base class for writing
	/// can not be a class member object; unless of a non-copyable class
	/// job-submission scripts for various workload management systems
	/// </summary>
	class WorkLoadManager {

	public:
		explicit WorkLoadManager(const std::string& JobScriptName);
		WorkLoadManager() = default;
		virtual ~WorkLoadManager();
		// no copy no move
		WorkLoadManager(const WorkLoadManager&) = delete;
		WorkLoadManager& operator=(const WorkLoadManager&) = delete;

		// The factory
		static std::unique_ptr<WorkLoadManager> Create(const std::string& JobSubmissionScriptName, WorkLoadManagerType type=WorkLoadManagerType::AUTO);
		// This method has to be re-factored to have a concise parameter list like <allThingsWlm>
		virtual std::string JobSubmissionCommand(const std::string& project_name,
			const std::string& queue_name,
			int maximum_runtime_limit_second,
			const std::string& job_name,
			const std::string& outfilename,
			const std::string& errorfilename,
			const std::string& number_of_cores,
			const std::string& setSpecificHost,
			const std::string& resourceSpec,
			const std::string& cwd,
			const bool isExlusive,
			const bool isInteractiveSession,
			const std::string& appToRun

		);
		virtual std::string JobTerminationCommand();
		virtual std::string JobStatusCommand();
		virtual std::string JobStatusCommandFinishedJobs(const std::string& JobID);
		virtual int getJobIDFromOutputOfSubmissionCommand(const std::string& output) const = 0;
		virtual JobStatus getJobStatusFromOutputOfJobStatusCommand(const std::string& output) const = 0;
		virtual JobStatus getJobStatusFromOutputOfJobStatusFinishedJobsCommand(const std::string& output) const = 0;

	protected:
		virtual bool writeProjectNameSpecification(const std::string & theJobSubmissionProjectName) = 0;
		/// <summary>
		/// Set the maximum runtime limit
		/// </summary>
		/// <param name="theJobSubmissionWaitTimeSpec"></param>
		/// <returns></returns>
		virtual bool writeWaitTimeSpecification(int theJobSubmissionWaitTimeSpec) = 0;
		virtual bool writeJobNameSpecification(const std::string& theJobSubmissionJobName) = 0;

		/// <summary>
		/// For WorkLoadManagers this parameter is the number of slots
		/// alloted to the job. This is typically equal to the number of 
		/// task spawned by MPI 
		/// </summary>
		/// <param name="theJobSubmissionWaitTimeSpec"></param>
		/// <returns></returns>
		virtual bool writeSlotsSpecification(const std::string& theJobSubmissionNProcsSpec) = 0;
		virtual bool writeOutputLogSpecification(const std::string& theJobSubmissionOutputLogSpec) = 0;
		virtual bool writeErrorLogSpecification(const std::string& theJobSubmissionErrorLogSpec) = 0;
		virtual bool writeExlusivitySpecification(bool isExclusive) = 0;
		virtual bool writeInteractiveSessionSpecification(bool isInteractiveSession) = 0;
		virtual bool writeCWDSpecification(const std::string& theJobSubmissionCWDSpec) = 0;
		virtual bool writeQueueSpecification(const std::string& theJobSubmissionQueueSpec) = 0;

		virtual std::string JobTerminate() const = 0;
		virtual std::string JobStatus() const = 0;
		virtual std::string JobStatusFinishedJobs(const std::string& JobID) const = 0;

		virtual std::string theSchedulerDirective() = 0;
		virtual std::string theSchedulerJobSubmissionCommand() = 0;
		std::ofstream* getTheFileStream(void)const;		
		const allThingsWlm& getTheWlMVariables(void)const;
		void setTheMlMVariables(const allThingsWlm& theVars);

	private:
		std::ofstream* JobSubmissionScriptStream = nullptr;
		int deleteTheJobSubmissionScript(void)const;
		void CreateTheFileStream(void);
		std::string JobSubmissionScriptName = "myJob.sh";
		allThingsWlm theWlMVariables;
				
	};

}
#endif
