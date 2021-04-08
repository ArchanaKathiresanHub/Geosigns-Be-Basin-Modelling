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

#include "SDUWorkLoadManagerLOCAL.h"

std::string workloadmanagers::WorkLoadManagerForLOCAL::theSchedulerDirective()
{
	return std::string("");
}

std::string workloadmanagers::WorkLoadManagerForLOCAL::theSchedulerJobSubmissionCommand()
{
	return std::string("");
}

std::string workloadmanagers::WorkLoadManagerForLOCAL::JobSubmissionCommand(const std::string& project_name, const std::string& queue_name, 
    const std::string& maximum_runtime_limit, const std::string& job_name, const std::string& outfilename, const std::string& errorfilename, 
    const std::string& number_of_cores, const std::string& setSpecificHost, const std::string& resourceSpec, const std::string& cwd,
    const bool isExlusive, const bool isInteractiveSession, const std::string& appToRun)
{
	bool isSuccess = writeProjectNameSpecification(project_name);
	if (!isSuccess)
		isSuccess = writeQueueSpecification(queue_name);
	if (!isSuccess)
		isSuccess = writeSlotsSpecification(number_of_cores);
	if (!isSuccess)
		isSuccess = writeWaitTimeSpecification(maximum_runtime_limit);
	if (!isSuccess)
		isSuccess = writeJobNameSpecification(job_name);
	if (!isSuccess)
		isSuccess = writeOutputLogSpecification(outfilename);
	if (!isSuccess)
		isSuccess = writeErrorLogSpecification(errorfilename);
	// The following are optional parameters that don't need check for success
	if (!isSuccess)
		writeExlusivitySpecification(isExlusive);
	if (!isSuccess)
		writeInteractiveSessionSpecification(isInteractiveSession);

	if (cwd.empty() && !isSuccess)
	{
		isSuccess = writeCWDSpecification(workloadmanagers::GetCurrentWorkingDir());
	}
	else if (!isSuccess)
	{
		isSuccess = writeCWDSpecification(cwd);
	}
	else
		throw WLMException();

    return appToRun;
}

bool workloadmanagers::WorkLoadManagerForLOCAL::writeProjectNameSpecification(const std::string& theJobSubmissionProjectName)
{
    return false;
}

bool workloadmanagers::WorkLoadManagerForLOCAL::writeWaitTimeSpecification(const std::string& theJobSubmissionWaitTimeSpec)
{
    return false;
}

bool workloadmanagers::WorkLoadManagerForLOCAL::writeJobNameSpecification(const std::string& theJobSubmissionJobName)
{
    return false;
}

bool workloadmanagers::WorkLoadManagerForLOCAL::writeSlotsSpecification(const std::string& theJobSubmissionNProcsSpec)
{
    return false;
}

bool workloadmanagers::WorkLoadManagerForLOCAL::writeOutputLogSpecification(const std::string& theJobSubmissionOutputLogSpec)
{
    return false;
}

bool workloadmanagers::WorkLoadManagerForLOCAL::writeErrorLogSpecification(const std::string& theJobSubmissionErrorLogSpec)
{
    return false;
}

bool workloadmanagers::WorkLoadManagerForLOCAL::writeExlusivitySpecification(bool isExclusive)
{
    return false;
}

bool workloadmanagers::WorkLoadManagerForLOCAL::writeInteractiveSessionSpecification(bool isInteractive)
{
    return false;
}

bool workloadmanagers::WorkLoadManagerForLOCAL::writeCWDSpecification(const std::string& theJobSubmissionCWDSpec)
{
	return false;
}

bool workloadmanagers::WorkLoadManagerForLOCAL::writeQueueSpecification(const std::string& theJobSubmissionQueueSpec)
{
    return false;
}
