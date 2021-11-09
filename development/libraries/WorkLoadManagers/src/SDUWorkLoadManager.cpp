//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SDUWorkLoadManager.h"
#include "SDUWorkLoadManagerLSF.h"
#include "SDUWorkLoadManagerSLURM.h"
#include "SDUWorkLoadManagerLOCAL.h"
#include <cstdio>
#include <iostream>
#include <fstream>

#ifndef WIN32
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
#endif


workloadmanagers::WorkLoadManager::WorkLoadManager(const std::string& JobScriptName)
{
	if (!JobScriptName.empty()) {
		// not empty else create the script with default name
		JobSubmissionScriptName = JobScriptName;
	}
	// has to go derived class
	// theWlMVariables.setClusterName(getenv("LSF_CLUSTER"));
	CreateTheFileStream();
}

workloadmanagers::WorkLoadManager::~WorkLoadManager()
{
	if (deleteTheJobSubmissionScript() == 2) {
		std::cerr<< "(WARNING) Something is not right while deleting the job submission file!" <<std::endl;
	}
	if (JobSubmissionScriptStream)
	{
		delete JobSubmissionScriptStream;
	}
}

std::unique_ptr<workloadmanagers::WorkLoadManager> workloadmanagers::WorkLoadManager::Create(const std::string& JobSubmissionScriptName, WorkLoadManagerType type)
{
	/* All the following env variables are present on servers
		LSF_BINDIR = / glb / apps / hpc / lsfprod / 10.1 / linux2.6 - glibc2.3 - x86_64 / bin
		LSF_CLUSTER = HOUGDC
		LSF_ENVDIR = / glb / apps / hpc / lsfprod / conf
		LSF_INSTALL = / glb / apps / hpc / lsfprod / 10.1 / install
		LSF_LIBDIR = / glb / apps / hpc / lsfprod / 10.1 / linux2.6 - glibc2.3 - x86_64 / lib
		LSF_PROFILE = / glb / apps / hpc / lsfprod / conf / profile.lsf
		LSF_SERVERDIR = / glb / apps / hpc / lsfprod / 10.1 / linux2.6 - glibc2.3 - x86_64 / etc
		LSF_TOP = / glb / apps / hpc / lsfprod
		LSF_VERSION = 10.1.0.6
		
	*/

	if (type == WorkLoadManagerType::AUTO)
	{
		// First we check is the user want to use a LOCAL machine
		bool found = false;

		if (getenv("LOCAL"))
		{
			// Here we don't check for the emptiness; cause the user can just $ export LOCAL="" should be sufficient
			type = WorkLoadManagerType::LOCAL;
			found = true;
		}
#ifndef WIN32
		if(!found)
		{
			std::string result = exec("which sbatch");
			if (!result.empty()
					|| access("/usr/bin/srun", 0) != -1
					|| (getenv("SLURM_INSTALL") && !std::string(std::getenv("SLURM_INSTALL")).empty()))
			{
				type = WorkLoadManagerType::SLURM;
				found = true;
			}
		}
#endif
		if (!found)
		{
			if (getenv("LSF_INSTALL") && !std::string(std::getenv("LSF_INSTALL")).empty())// avoid an empty string literal)// avoid an empty string literal
			{
				type = WorkLoadManagerType::IBM_LSF;
				found = true;
			}
		}
	}
	
	// Factory method to create objects of different types. 
	// Change is required only in this function to create a new object type
	// Use of make_unique<> is a better version but compatible with C++14 and above
	// Implement it once the bootstrap is upgraded to accommodate C++14 or above

	switch (type)
	{
	case workloadmanagers::WorkLoadManagerType::LOCAL:
		return std::unique_ptr<workloadmanagers::WorkLoadManagerForLOCAL>(new workloadmanagers::WorkLoadManagerForLOCAL);
	case workloadmanagers::WorkLoadManagerType::IBM_LSF:

		return std::unique_ptr<workloadmanagers::WorkLoadManagerForLSF>(new workloadmanagers::WorkLoadManagerForLSF(JobSubmissionScriptName));
	case workloadmanagers::WorkLoadManagerType::SLURM:
		return std::unique_ptr<workloadmanagers::WorkLoadManagerForSLURM>(new workloadmanagers::WorkLoadManagerForSLURM(JobSubmissionScriptName));
	case workloadmanagers::WorkLoadManagerType::OTHER:
		// to do
		break;
	case workloadmanagers::WorkLoadManagerType::AUTO:
		// If none of the WLM environment variables were not found then the AUTO sets to a Local run
		return std::unique_ptr<workloadmanagers::WorkLoadManagerForLOCAL>(new workloadmanagers::WorkLoadManagerForLOCAL);
	default:
		return std::unique_ptr<workloadmanagers::WorkLoadManagerForLOCAL>(new workloadmanagers::WorkLoadManagerForLOCAL);
	}

	return nullptr;
}

// Gettig current working directory using boost::filesystem
std::string workloadmanagers::GetCurrentWorkingDir()
{
	return boost::filesystem::current_path().string();
}

std::string workloadmanagers::WorkLoadManager::JobSubmissionCommand(const std::string& project_name,
	const std::string& queue_name, int maximum_runtime_limit_seconds,
	const std::string& job_name, const std::string& outfilename, const std::string& errorfilename, const std::string& number_of_cores, 
	const std::string& setSpecificHost, const std::string& resourceSpec, const std::string& cwd, const bool isExlusive, const bool isInteractiveSession,
	const std::string& appToRun)
{
	bool isSuccess = writeProjectNameSpecification(project_name);
	if(isSuccess)
		isSuccess = writeQueueSpecification(queue_name);
	if (isSuccess)
		isSuccess = writeSlotsSpecification(number_of_cores);
	if (isSuccess && maximum_runtime_limit_seconds > 0)
		isSuccess = writeWaitTimeSpecification(maximum_runtime_limit_seconds);
	if (isSuccess)
		isSuccess = writeJobNameSpecification(job_name);

	// The following are optional parameters that don't need check for success
	if (isSuccess)
	{
		writeOutputLogSpecification(outfilename);
		writeErrorLogSpecification(errorfilename);
		writeExlusivitySpecification(isExlusive);
		writeInteractiveSessionSpecification(isInteractiveSession);
	}

	if (cwd.empty() && isSuccess)
	{
			isSuccess = writeCWDSpecification("\""+workloadmanagers::GetCurrentWorkingDir()+"\"");
	}
	else if (isSuccess)
	{
			isSuccess = writeCWDSpecification(cwd);
	}
	if (isSuccess)
		(*JobSubmissionScriptStream) << appToRun << '\n';
	else
		throw WLMException();

	JobSubmissionScriptStream->close();
	// Should check if the file was created with the contents ?
	return theSchedulerJobSubmissionCommand() + "\"" + JobSubmissionScriptName + "\"";
}


std::string workloadmanagers::WorkLoadManager::JobTerminationCommand()
{
	return JobTerminate();
}

std::string workloadmanagers::WorkLoadManager::JobStatusCommand()
{
	return JobStatus();
}

std::string workloadmanagers::WorkLoadManager::JobStatusCommandFinishedJobs(const std::string& JobID)
{
	return JobStatusFinishedJobs(JobID);
}

std::ofstream* workloadmanagers::WorkLoadManager::getTheFileStream(void) const
{
	return JobSubmissionScriptStream;
}

const workloadmanagers::allThingsWlm& workloadmanagers::WorkLoadManager::getTheWlMVariables(void) const
{
	// TODO: insert return statement here
	return theWlMVariables;
}

void workloadmanagers::WorkLoadManager::setTheMlMVariables(const allThingsWlm& theVars)
{
	theWlMVariables = theVars;
}

void workloadmanagers::WorkLoadManager::CreateTheFileStream(void)
{
	JobSubmissionScriptStream = new std::ofstream(JobSubmissionScriptName);
	if (!JobSubmissionScriptStream) { std::cerr << "Error creating " + JobSubmissionScriptName; return; }

	if (JobSubmissionScriptStream->is_open()) { // check for successful opening
		(*JobSubmissionScriptStream) << "#!/bin/bash\n#\n";
	}
	else
	{
		throw WLMException();
	}
}

int workloadmanagers::WorkLoadManager::deleteTheJobSubmissionScript(void) const
{
	if (JobSubmissionScriptStream) {
		std::cout << std::ifstream(JobSubmissionScriptName.c_str()).rdbuf() << '\n'; // print file

		std::remove(JobSubmissionScriptName.c_str()); // delete file

		bool failed = !std::ifstream(JobSubmissionScriptName.c_str());
		if (failed) { std::cerr << ("Don't be ALARMED: the jobSubmission file with the above details was deleted!\n"); return 1; }
		//the two is the problem...
		return 2;
	}
	else
	{
		// This is for LOCAL when there is no file to delete
		return 3;
	}
}
