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

std::string workloadmanagers::WorkLoadManagerForSLURM::theSchedulerDirective()
{
	return std::string("#SBATCH");
}

std::string workloadmanagers::WorkLoadManagerForSLURM::theSchedulerJobSubmissionCommand()
{
	return std::string("sbatch ");//srun??
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

bool workloadmanagers::WorkLoadManagerForSLURM::writeWaitTimeSpecification(const std::string& theJobSubmissionWaitTimeSpec)
{
	if ((*getTheFileStream()).is_open()) { // check for successful opening
		(*getTheFileStream()) << theSchedulerDirective() << "\t -t " << theJobSubmissionWaitTimeSpec << '\n'; //-t hh:mm:ss
		return true;
	}
    return false;
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

bool workloadmanagers::WorkLoadManagerForSLURM::writeInteractiveSessionSpecification(bool isInteractive)
{
	if ((*getTheFileStream()).is_open() && isInteractive) { // check for successful opening
		(*getTheFileStream()) << theSchedulerDirective() << "\t --pty " << '\n';
		return true;
	}
    return false;
}

bool workloadmanagers::WorkLoadManagerForSLURM::writeCWDSpecification(const std::string& theJobSubmissionCWDSpec)
{
	if ((*getTheFileStream()).is_open()) { // check for successful opening
		(*getTheFileStream()) << theSchedulerDirective() << "\t -cwd " << theJobSubmissionCWDSpec << '\n';
		return true;
	}
	return false;
}

bool workloadmanagers::WorkLoadManagerForSLURM::writeQueueSpecification(const std::string& theJobSubmissionQueueSpec)
{
	if ((*getTheFileStream()).is_open()) { // check for successful opening
		(*getTheFileStream()) << theSchedulerDirective() << "\t -q " << theJobSubmissionQueueSpec << '\n';
		return true;
	}
    return false;
}
