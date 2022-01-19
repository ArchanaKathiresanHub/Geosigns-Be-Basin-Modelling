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

#include "SDUWorkLoadManagerSLURM.h"

#include <sstream>

std::string workloadmanagers::WorkLoadManagerForSLURM::theSchedulerDirective()
{
	return std::string("#SBATCH");
}

std::string workloadmanagers::WorkLoadManagerForSLURM::theSchedulerJobSubmissionCommand()
{
	return std::string("sbatch ");//srun??
}

std::string workloadmanagers::WorkLoadManagerForSLURM::JobTerminate() const
{
	return std::string("scancel ");
}

std::string workloadmanagers::WorkLoadManagerForSLURM::JobStatus() const
{
	return std::string("squeue -o \"%.6i %t\" -j " );
}

std::string workloadmanagers::WorkLoadManagerForSLURM::JobStatusFinishedJobs(const std::string& JobID) const
{
	return std::string("sacct -n -b -j " + JobID + ".batch");
}

int workloadmanagers::WorkLoadManagerForSLURM::getJobIDFromOutputOfSubmissionCommand(const std::string& output) const
{
	// output is in format "Submitted batch job 34987", so we separate with spaces and take the last string
	// to get the ID
	std::stringstream stream(output);
	std::string lastString;
	while (getline(stream, lastString, ' ')){}

	return std::stoi(lastString);
}

workloadmanagers::JobStatus workloadmanagers::WorkLoadManagerForSLURM::getJobStatusFromOutputOfJobStatusCommand(const std::string& output) const
{
	std::string jobStatusString = getJobStatusString(output);

	if (jobStatusString == "R")
	{
		return JobStatus::JobRunning;
	}
	else if (jobStatusString == "F")
	{
		return JobStatus::JobFailed;
	}
	else if (jobStatusString == "CD")
	{
		return JobStatus::JobFinished;
	}
	else if (jobStatusString == "PD")
	{
		return JobStatus::JobPending;
	}

	return JobStatus::JobStatusUnknown;
}

workloadmanagers::JobStatus workloadmanagers::WorkLoadManagerForSLURM::getJobStatusFromOutputOfJobStatusFinishedJobsCommand(const std::string& output) const
{
	std::string jobStatusString = getJobStatusStringFinishedJobs(output);

	if (jobStatusString == "FAILED" || jobStatusString == "CANCELLED")
	{
		return JobStatus::JobFailed;
	}
	else if (jobStatusString == "COMPLETED")
	{
		return JobStatus::JobFinished;
	}

	return JobStatus::JobStatusUnknown;
}

std::string workloadmanagers::WorkLoadManagerForSLURM::getJobStatusString(const std::string& output) const
{
	// Format of the command output string:
	// JOBID ST
	// 99999 R
	// Where we want to extract the R

	// Get the last line ('99999 R')
	std::stringstream lineStream(output);
	std::string infoLine;
	int counter = 0;
	// Get the second line of the output ('99999 R')
	while (getline(lineStream, infoLine, '\n') && counter < 1){counter++;}

	// Get the last word ('R')
	std::string lastWord;
	std::stringstream wordStream(infoLine);
	while (getline(wordStream, lastWord, ' ')){}

	return lastWord;
}

std::string workloadmanagers::WorkLoadManagerForSLURM::getJobStatusStringFinishedJobs(const std::string& output) const
{
	// Format of the command output string:
	// JobID.batch     COMPLETED      0:0
	// Where we want to extract the COMPLETED

	// Get the second word ('COMPLETED')
	std::string secondWord;
	std::stringstream wordStream(output);
	int counter = 0;
	while (counter < 2 && getline(wordStream, secondWord, ' '))
	{
		if (secondWord != "")
		{
			counter++;
		}
	}

	return secondWord;
}

workloadmanagers::WorkLoadManagerForSLURM::WorkLoadManagerForSLURM(const std::string& JobSubmissionScriptName) : 
	WorkLoadManager(JobSubmissionScriptName)
{}

bool workloadmanagers::WorkLoadManagerForSLURM::writeProjectNameSpecification(const std::string& theJobSubmissionProjectName)
{
	if ((*getTheFileStream()).is_open()) { // check for successful opening
        (*getTheFileStream()) << theSchedulerDirective() << "\t -A " << theJobSubmissionProjectName << '\n';
		return true;
	}
    return false;
}

bool workloadmanagers::WorkLoadManagerForSLURM::writeWaitTimeSpecification(int theJobSubmissionWaitTimeSpec)
{
#ifdef WLM_RUNTIME
	int hours = theJobSubmissionWaitTimeSpec / 3600;
	theJobSubmissionWaitTimeSpec %= 3600;
	int minutes = theJobSubmissionWaitTimeSpec / 60;
	theJobSubmissionWaitTimeSpec %= 60;
	int seconds = theJobSubmissionWaitTimeSpec / 60;

	if ((*getTheFileStream()).is_open()) { // check for successful opening
		(*getTheFileStream()) << theSchedulerDirective() << "\t -t " << hours << ":" << minutes << ":" << seconds << '\n'; //-t hh:mm:ss
		return true;
	}
    return false;
#else
	return true;
#endif
}

bool workloadmanagers::WorkLoadManagerForSLURM::writeJobNameSpecification(const std::string& theJobSubmissionJobName)
{
	if ((*getTheFileStream()).is_open()) { // check for successful opening
		(*getTheFileStream()) << theSchedulerDirective() << "\t --job-name " << theJobSubmissionJobName << '\n';
		return true;
	}
    return false;
}

bool workloadmanagers::WorkLoadManagerForSLURM::writeSlotsSpecification(const std::string& theJobSubmissionNProcsSpec)
{
	if ((*getTheFileStream()).is_open()) { // check for successful opening
		(*getTheFileStream()) << theSchedulerDirective() << "\t -n " << theJobSubmissionNProcsSpec << '\n';
		return true;
	}
    return false;
}

bool workloadmanagers::WorkLoadManagerForSLURM::writeOutputLogSpecification(const std::string& theJobSubmissionOutputLogSpec)
{
	if (boost::filesystem::exists(theJobSubmissionOutputLogSpec)) {
		std::cout << "\n Output file \"out.log\" of previous run exists. Deleting it.\n\n";
		boost::filesystem::remove(theJobSubmissionOutputLogSpec);
	}

	if ((*getTheFileStream()).is_open() && !theJobSubmissionOutputLogSpec.empty()) { // check for successful opening
		(*getTheFileStream()) << theSchedulerDirective() << "\t -o " << theJobSubmissionOutputLogSpec << '\n';
		return true;
	}
    return false;
}

bool workloadmanagers::WorkLoadManagerForSLURM::writeErrorLogSpecification(const std::string& theJobSubmissionErrorLogSpec)
{
	if (boost::filesystem::exists(theJobSubmissionErrorLogSpec)) {
		std::cout << "\n Error file \"err.log\" of previous run exists. Deleting it.\n\n";
		boost::filesystem::remove(theJobSubmissionErrorLogSpec);
	}

	if ((*getTheFileStream()).is_open() && !theJobSubmissionErrorLogSpec.empty()) { // check for successful opening
		(*getTheFileStream()) << theSchedulerDirective() << "\t -e " << theJobSubmissionErrorLogSpec << '\n';
		return true;
	}
    return false;
}

bool workloadmanagers::WorkLoadManagerForSLURM::writeExlusivitySpecification(bool isExclusive)
{
	if ((*getTheFileStream()).is_open() && isExclusive) { // check for successful opening
		(*getTheFileStream()) << theSchedulerDirective() << "\t --exclusive " << '\n';
		return true;
	}
    return false;
}

bool workloadmanagers::WorkLoadManagerForSLURM::writeWaitForJobToFinish(bool doWait)
{
	/// Do not exit until the submitted job terminates. 
	/// The exit code of the sbatch command will be the same as the exit code of the submitted job
	if ((*getTheFileStream()).is_open() && doWait) { // check for successful opening
		(*getTheFileStream()) << theSchedulerDirective() << "\t -W" << '\n';
		return true;
	}
    return false;
}

// there is no Interactive way to JobSubmission on SLURM
bool workloadmanagers::WorkLoadManagerForSLURM::writeInteractiveSessionSpecification(bool isInteractive)
{
	return false;
}

bool workloadmanagers::WorkLoadManagerForSLURM::writeCWDSpecification(const std::string& theJobSubmissionCWDSpec)
{
	if ((*getTheFileStream()).is_open()) { // check for successful opening
		(*getTheFileStream()) << theSchedulerDirective() << "\t -D " << theJobSubmissionCWDSpec << '\n';
		return true;
	}
	return false;
}

bool workloadmanagers::WorkLoadManagerForSLURM::writeQueueSpecification(const std::string& theJobSubmissionQueueSpec)
{
	const std::string queue = theJobSubmissionQueueSpec.empty() ? "pt" : theJobSubmissionQueueSpec;
	if ((*getTheFileStream()).is_open()) { // check for successful opening
		(*getTheFileStream()) << theSchedulerDirective() << "\t -p " << queue << '\n';
		return true;
	}
    return false;
}
