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

#include <boost/property_tree/json_parser.hpp>

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

std::string workloadmanagers::WorkLoadManagerForLSF::JobTerminate() const
{
	return std::string("bkill ");
}

std::string workloadmanagers::WorkLoadManagerForLSF::JobStatus() const
{
	return std::string("bjobs -a -hms -o 'stat' -json ");
}

int workloadmanagers::WorkLoadManagerForLSF::getJobIDFromOutputOfSubmissionCommand(const std::string& commandOutput) const
{
  // The output string has the following format: Job <JobID> is submitted to queue <default.q>.
  const std::string JobID = commandOutput.substr(commandOutput.find_first_of("<") + 1, commandOutput.find_first_of(">") - (commandOutput.find_first_of("<") + 1));
  return std::stoi(JobID);
}

workloadmanagers::JobStatus workloadmanagers::WorkLoadManagerForLSF::getJobStatusFromOutputOfJobStatusCommand(const std::string& output) const
{
  std::stringstream ss;
  ss << output;

  boost::property_tree::ptree pt;
  boost::property_tree::read_json(ss, pt);

  auto records = pt.get_child("RECORDS");
  std::string status;
  for (const auto& record : records)
  {
    status = record.second.get<std::string>("STAT");
  }

  if (status == "EXIT")
  {
    return JobStatus::JobFailed;
  }
  else if (status == "PEND")
  {
    return JobStatus::JobPending;
  }
  else if (status == "RUN")
  {
   return JobStatus::JobRunning;
  }
  else if (status == "DONE")
  {
    return JobStatus::JobFinished;
  }
  else
  {
    return JobStatus::JobFailed;
  }
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
bool workloadmanagers::WorkLoadManagerForLSF::writeWaitTimeSpecification(int theJobSubmissionWaitTimeSpec)
{
#ifdef WLM_RUNTIME
	int hours = theJobSubmissionWaitTimeSpec / 3600;
	theJobSubmissionWaitTimeSpec %= 3600;
	int minutes = theJobSubmissionWaitTimeSpec / 60;

	if ((*getTheFileStream()).is_open()) { // check for successful opening
		(*getTheFileStream()) << theSchedulerDirective() << "\t -We " << hours << ":" << minutes << '\n';
		return true;
	}
    return false;
#else
	return true;
#endif
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

bool workloadmanagers::WorkLoadManagerForLSF::writeWaitForJobToFinish(bool)
{
	if ((*getTheFileStream()).is_open()) { // check for successful opening
		(*getTheFileStream()) << theSchedulerDirective() << "\t -K \n" ;
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
	const std::string queue = theJobSubmissionQueueSpec.empty() ? "default.q" : theJobSubmissionQueueSpec;
	if ((*getTheFileStream()).is_open()) { // check for successful opening
		(*getTheFileStream()) << theSchedulerDirective() << "\t -q " << queue << '\n';
		return true;
	}
	return false;
}
