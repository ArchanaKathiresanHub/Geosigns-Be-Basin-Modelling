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

#include "SDUWorkLoadManagerLSF.h"

std::string workloadmanagers::WorkLoadManagerForLSF::theSchedulerDirective()
{
	return std::string("#BSUB");
}

std::string workloadmanagers::WorkLoadManagerForLSF::theSchedulerJobSubmissionCommand()
{
	//-K Sends the message "Waiting for dispatch" to the terminal when you submit the job. 
	//Sends the message "Job is finished" to the terminal when the job is done (if LSB_SUBK_SHOW_JOBID is enabled in the 
	//lsf.conf file or as an environment variable, also displays the job ID when the job is done)
	return std::string("bsub <");
	// Note -K can not be used with options: -I, -Ip, -IS, -ISp, -ISs, -Is, -IX.
}

workloadmanagers::WorkLoadManagerForLSF::WorkLoadManagerForLSF(const std::string&  JobSubmissionScriptName) :
	WorkLoadManager(JobSubmissionScriptName)
{}

bool workloadmanagers::WorkLoadManagerForLSF::writeProjectNameSpecification(const std::string& theJobSubmissionProjectName)
{
	if ((*getTheFileStream()).is_open()) { // check for successful opening
        (*getTheFileStream()) << theSchedulerDirective() << "\t -P " << theJobSubmissionProjectName << '\n';
		return true;
	}
    return false;
}

// use estimated runtime time -We instead of the runtime limit, 
// -W which will kill job after the elapsed time
// 
bool workloadmanagers::WorkLoadManagerForLSF::writeWaitTimeSpecification(const std::string& theJobSubmissionWaitTimeSpec)
{
	if ((*getTheFileStream()).is_open()) { // check for successful opening
		(*getTheFileStream()) << theSchedulerDirective() << "\t -We " << theJobSubmissionWaitTimeSpec << '\n';
		return true;
	}
    return false;
}

bool workloadmanagers::WorkLoadManagerForLSF::writeJobNameSpecification(const std::string& theJobSubmissionJobName)
{
	if ((*getTheFileStream()).is_open()) { // check for successful opening
		(*getTheFileStream()) << theSchedulerDirective() << "\t -J " << theJobSubmissionJobName << '\n';
		return true;
	}
    return false;
}

bool workloadmanagers::WorkLoadManagerForLSF::writeSlotsSpecification(const std::string& theJobSubmissionNProcsSpec)
{
	if ((*getTheFileStream()).is_open()) { // check for successful opening
		(*getTheFileStream()) << theSchedulerDirective() << "\t -n " << theJobSubmissionNProcsSpec << '\n';
		return true;
	}
    return false;
}

bool workloadmanagers::WorkLoadManagerForLSF::writeOutputLogSpecification(const std::string& theJobSubmissionOutputLogSpec)
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

bool workloadmanagers::WorkLoadManagerForLSF::writeErrorLogSpecification(const std::string& theJobSubmissionErrorLogSpec)
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

bool workloadmanagers::WorkLoadManagerForLSF::writeExlusivitySpecification(bool isExclusive)
{
	if ((*getTheFileStream()).is_open() && isExclusive) { // check for successful opening
		(*getTheFileStream()) << theSchedulerDirective() << "\t -x " << '\n';
		return true;
	}
    return false;
}

bool workloadmanagers::WorkLoadManagerForLSF::writeInteractiveSessionSpecification(bool isInteractive)
{
	if ((*getTheFileStream()).is_open() && isInteractive) { // check for successful opening
		(*getTheFileStream()) << theSchedulerDirective() << "\t -Is " << '\n';
		return true;
	}
    return false;
}

bool workloadmanagers::WorkLoadManagerForLSF::writeCWDSpecification(const std::string& theJobSubmissionCWDSpec)
{
	if ((*getTheFileStream()).is_open()) { // check for successful opening
		(*getTheFileStream()) << theSchedulerDirective() << "\t -cwd " << theJobSubmissionCWDSpec << '\n';
		return true;
	}
	return false;
}

bool workloadmanagers::WorkLoadManagerForLSF::writeQueueSpecification(const std::string& theJobSubmissionQueueSpec)
{
	if ((*getTheFileStream()).is_open()) { // check for successful opening
		(*getTheFileStream()) << theSchedulerDirective() << "\t -q " << theJobSubmissionQueueSpec << '\n';
		return true;
	}
    return false;
}
