//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "gtest/gtest.h"
#include "SDUWorkLoadManager.h"
#include <iostream>
#include <vector>

/*
 * It will iterate through all the lines in file and
 * put them in given vector
 */
namespace workloadmanagers{
	// Variables used by the unit tests
	static const std::string wlmScriptPath("cldrn.sh");
	static const int numProc = 4;
	static const std::string CLDRN_BIN("CLDRN_BIN");
	static const std::string MPI_BIN("MPI_BIN");
	static const std::string cldrnRunMode("cldrnRunMode");
	static const std::string workingDirectory("WorkingDirectory");
	static const std::string filePath("filePath");
	static const std::string outputLocal("MPI_BIN -n 4 CLDRN_BIN -project filePath cldrnRunMode");

	// Expected vector of strings for the unit tests of LSF
	static const std::vector<std::string> expVecOfStrLSF = {
		"#!/bin/bash",
		"#",
		"#BSUB\t -P cldrn",
		"#BSUB\t -q default.q",
		"#BSUB\t -n 4",
		"#BSUB\t -J ctcPressureJob",
		"#BSUB\t -o out.log",
		"#BSUB\t -e err.log",
		"#BSUB	 -K ",
		"#BSUB\t -cwd \"" + boost::filesystem::current_path().string() + "\"",
		"MPI_BIN -n 4 CLDRN_BIN -project filePath cldrnRunMode"
	};

	// Expected vector of strings for the unit tests of LSF
	static const std::vector<std::string> expNoOutLogVecOfStrLSF = {
		"#!/bin/bash",
		"#",
		"#BSUB\t -P cldrn",
		"#BSUB\t -q default.q",
		"#BSUB\t -n 4",
		"#BSUB\t -J ctcPressureJob",+
		"#BSUB\t -e err.log",
		"#BSUB	 -K ",
		"#BSUB\t -cwd \"" + boost::filesystem::current_path().string() + "\"",
		"MPI_BIN -n 4 CLDRN_BIN -project filePath cldrnRunMode"
	};

	// Expected vector of strings for the unit tests of SLURM
	static const std::vector<std::string> expVecOfStrSLURM = {
		"#!/bin/bash",
		"#",
		"#SBATCH\t -A cldrn",
		"#SBATCH\t -p pt",
		"#SBATCH\t -n 4",
		"#SBATCH\t --job-name ctcPressureJob",
		"#SBATCH\t -o out.log",
		"#SBATCH\t -e err.log",
		"#SBATCH\t -W",
		"#SBATCH\t -D \"" + boost::filesystem::current_path().string() + "\"",
		"MPI_BIN -n 4 CLDRN_BIN -project filePath cldrnRunMode"
	};

	// Expected vector of strings for the unit tests of SLURM
	static const std::vector<std::string> expNoOutLogVecOfStrSLURM = {
		"#!/bin/bash",
		"#",
		"#SBATCH\t -A cldrn",
		"#SBATCH\t -p pt",
		"#SBATCH\t -n 4",
		"#SBATCH\t --job-name ctcPressureJob",
		"#SBATCH\t -e err.log",
		"#SBATCH\t -W",
		"#SBATCH\t -D \"" + boost::filesystem::current_path().string() + "\"",
		"MPI_BIN -n 4 CLDRN_BIN -project filePath cldrnRunMode"
	};

	// The following function reads the contents of the job submission script and stores in a vector of strings
	bool getFileContent(std::string fileName, std::vector<std::string>& vecOfStrs)
	{
		// Open the File
		std::ifstream in(fileName.c_str());
		// Check if object is valid
		if (!in)
		{
			std::cerr << "Cannot open the File : " << fileName << std::endl;
			return false;
		}
		std::string str;
		// Read the next line from File until it reaches the end.
		while (std::getline(in, str))
		{
			// Line contains string of length > 0 then save it in vector
			if (str.size() > 0)
				vecOfStrs.push_back(str);
		}
		//Close The File
		in.close();
		return true;
	}
}

using namespace workloadmanagers;

TEST(CTCWorkLoadManager, Job_Submission_Local) {
	auto wlm = workloadmanagers::WorkLoadManager::Create("cldrn.sh", workloadmanagers::WorkLoadManagerType::LOCAL);

	// always check for nullptr from Create
	if (wlm) {
		std::string runPT = wlm->JobSubmissionCommand("cldrn", "default.q", 1800, "ctcPressureJob", "out.log",
			"err.log", std::to_string(numProc), "", "", workingDirectory, false, false, (MPI_BIN + " -n " +
				std::to_string(numProc) + ' ' + CLDRN_BIN + " -project " + filePath + " " + cldrnRunMode)
		)
			;

		EXPECT_EQ(runPT, outputLocal);
		EXPECT_EQ(wlm->JobTerminationCommand(), std::string());
		EXPECT_EQ(wlm->JobStatusCommand(), std::string());
	}
}

TEST(CTCWorkLoadManager, Job_Submission_LSF) {
	std::vector<std::string> vecOfStr;
	auto wlm = workloadmanagers::WorkLoadManager::Create("cldrn.sh", workloadmanagers::WorkLoadManagerType::IBM_LSF);

	if (wlm) {
		std::string runPT = wlm->JobSubmissionCommand("cldrn", "", 1800, "ctcPressureJob", "out.log",
			"err.log", std::to_string(numProc), "", "", "", false, false, (MPI_BIN + " -n " +
				std::to_string(numProc) + ' ' + CLDRN_BIN + " -project " + filePath + " " + cldrnRunMode)
		);

		// Get the contents of file in a vector
		bool result = getFileContent(wlmScriptPath, vecOfStr);

		ASSERT_EQ(vecOfStr.size(), expVecOfStrLSF.size()) << "Vectors are of unequal length";
		for (int i = 0; i < vecOfStr.size(); ++i) {
			EXPECT_EQ(vecOfStr[i], expVecOfStrLSF[i]) << "Vectors differ at index " << i;
		}

		EXPECT_EQ(wlm->JobTerminationCommand(), "bkill ");
		EXPECT_EQ(wlm->JobStatusCommand(), "bjobs -a -hms -o 'stat' -json ");
	}
}

TEST(CTCWorkLoadManager, Job_Submission_no_outlog_LSF) {
	std::vector<std::string> vecOfStr;
	auto wlm = workloadmanagers::WorkLoadManager::Create("cldrn.sh", workloadmanagers::WorkLoadManagerType::IBM_LSF);

	if (wlm) {
		std::string runPT = wlm->JobSubmissionCommand("cldrn", "default.q", 1800, "ctcPressureJob", "",
			"err.log", std::to_string(numProc), "", "", "", false, false, (MPI_BIN + " -n " +
				std::to_string(numProc) + ' ' + CLDRN_BIN + " -project " + filePath + " " + cldrnRunMode)
		);

		// Get the contents of file in a vector
		bool result = getFileContent(wlmScriptPath, vecOfStr);

		ASSERT_EQ(vecOfStr.size(), expNoOutLogVecOfStrLSF.size()) << "Vectors are of unequal length";
		for (int i = 0; i < vecOfStr.size(); ++i) {
			EXPECT_EQ(vecOfStr[i], expNoOutLogVecOfStrLSF[i]) << "Vectors differ at index " << i;
		}

		EXPECT_EQ(wlm->JobTerminationCommand(), "bkill ");
		EXPECT_EQ(wlm->JobStatusCommand(), "bjobs -a -hms -o 'stat' -json ");
	}
}

TEST(CTCWorkLoadManager, Job_Submission_SLURM) {
	std::vector<std::string> vecOfStr;
	auto wlm = workloadmanagers::WorkLoadManager::Create("cldrn.sh", workloadmanagers::WorkLoadManagerType::SLURM);

	if (wlm) {
		std::string runPT = wlm->JobSubmissionCommand("cldrn", "pt", 1800, "ctcPressureJob", "out.log",
			"err.log", std::to_string(numProc), "", "", "", false, true, (MPI_BIN + " -n " +
				std::to_string(numProc) + ' ' + CLDRN_BIN + " -project " + filePath + " " + cldrnRunMode)
		);

		// Get the contents of file in a vector
		bool result = getFileContent(wlmScriptPath, vecOfStr);

		ASSERT_EQ(vecOfStr.size(), expVecOfStrSLURM.size()) << "Vectors are of unequal length";
		for (int i = 0; i < vecOfStr.size(); ++i) {
			EXPECT_EQ(vecOfStr[i], expVecOfStrSLURM[i]) << "Vectors differ at index " << i;
		}

		EXPECT_EQ(wlm->JobTerminationCommand(), std::string("scancel "));
		EXPECT_EQ(wlm->JobStatusCommand(), std::string("squeue -o \"%.6i %t\" -j "));
	}
}

TEST(CTCWorkLoadManager, Job_Submission_no_outlog_SLURM) {
	std::vector<std::string> vecOfStr;
	auto wlm = workloadmanagers::WorkLoadManager::Create("cldrn.sh", workloadmanagers::WorkLoadManagerType::SLURM);

	if (wlm) {
		std::string runPT = wlm->JobSubmissionCommand("cldrn", "", 1800, "ctcPressureJob", "",
			"err.log", std::to_string(numProc), "", "", "", false, false, (MPI_BIN + " -n " +
				std::to_string(numProc) + ' ' + CLDRN_BIN + " -project " + filePath + " " + cldrnRunMode)
		);

		// Get the contents of file in a vector
		bool result = getFileContent(wlmScriptPath, vecOfStr);

		ASSERT_EQ(vecOfStr.size(), expNoOutLogVecOfStrSLURM.size()) << "Vectors are of unequal length";
		for (int i = 0; i < vecOfStr.size(); ++i) {
			EXPECT_EQ(vecOfStr[i], expNoOutLogVecOfStrSLURM[i]) << "Vectors differ at index " << i;
		}

		EXPECT_EQ(wlm->JobTerminationCommand(), std::string("scancel "));
		EXPECT_EQ(wlm->JobStatusCommand(), std::string("squeue -o \"%.6i %t\" -j "));
	}
}

TEST(CTCWorkLoadManager, Get_JobID_LSF) {
	// Given
	auto wlm = workloadmanagers::WorkLoadManager::Create("cldrn.sh", workloadmanagers::WorkLoadManagerType::IBM_LSF);
	if (!wlm)
	{
		FAIL() << "WorkLoadManager was not initialized correctly";
	}

	// When
	const std::string commandOutput = "Job <20326251> is submitted to default queue <default.q>.";
	const int JobID = wlm->getJobIDFromOutputOfSubmissionCommand(commandOutput);

	// Then
	EXPECT_EQ(JobID, 20326251);
}

TEST(CTCWorkLoadManager, Get_JobID_SLURM) {
	// Given
	auto wlm = workloadmanagers::WorkLoadManager::Create("cldrn.sh", workloadmanagers::WorkLoadManagerType::SLURM);
	if (!wlm)
	{
		FAIL() << "WorkLoadManager was not initialized correctly";
	}

	// When
	const std::string commandOutput = "Submitted batch job 34987";
	const int JobID = wlm->getJobIDFromOutputOfSubmissionCommand(commandOutput);

	// Then
	EXPECT_EQ(JobID, 34987);
}

TEST(CTCWorkLoadManager, Get_JobStatus_Running_SLURM) {
	// Given
	auto wlm = workloadmanagers::WorkLoadManager::Create("cldrn.sh", workloadmanagers::WorkLoadManagerType::SLURM);
	if (!wlm)
	{
		FAIL() << "WorkLoadManager was not initialized correctly";
	}

	// When
	const std::string commandOutput = " JOBID ST\n   437 R\n";
	const JobStatus status = wlm->getJobStatusFromOutputOfJobStatusCommand(commandOutput);

	// Then
	EXPECT_EQ(status, JobStatus::JobRunning);
}

TEST(CTCWorkLoadManager, Get_JobStatusFinishedJobs_Finished_SLURM) {
	// Given
	auto wlm = workloadmanagers::WorkLoadManager::Create("cldrn.sh", workloadmanagers::WorkLoadManagerType::SLURM);
	if (!wlm)
	{
		FAIL() << "WorkLoadManager was not initialized correctly";
	}

	// When
	const std::string commandOutput = "451.batch     COMPLETED      0:0";
	const JobStatus status = wlm->getJobStatusFromOutputOfJobStatusFinishedJobsCommand(commandOutput);

	// Then
	EXPECT_EQ(status, JobStatus::JobFinished);
}

TEST(CTCWorkLoadManager, Get_JobStatus_Pending_SLURM) {
	// Given
	auto wlm = workloadmanagers::WorkLoadManager::Create("cldrn.sh", workloadmanagers::WorkLoadManagerType::SLURM);
	if (!wlm)
	{
		FAIL() << "WorkLoadManager was not initialized correctly";
	}

	// When
	const std::string commandOutput = "JOBID ST\n 65543 PD\n";
	const JobStatus status = wlm->getJobStatusFromOutputOfJobStatusCommand(commandOutput);

	// Then
	EXPECT_EQ(status, JobStatus::JobPending);
}

TEST(CTCWorkLoadManager, Get_JobStatus_Failed_SLURM) {
	// Given
	auto wlm = workloadmanagers::WorkLoadManager::Create("cldrn.sh", workloadmanagers::WorkLoadManagerType::SLURM);
	if (!wlm)
	{
		FAIL() << "WorkLoadManager was not initialized correctly";
	}

	// When
	const std::string commandOutput = "JOBID ST\n 65543 F\n";
	const JobStatus status = wlm->getJobStatusFromOutputOfJobStatusCommand(commandOutput);

	// Then
	EXPECT_EQ(status, JobStatus::JobFailed);
}

TEST(CTCWorkLoadManager, Get_JobStatus_Finished_SLURM) {
	// Given
	auto wlm = workloadmanagers::WorkLoadManager::Create("cldrn.sh", workloadmanagers::WorkLoadManagerType::SLURM);
	if (!wlm)
	{
		FAIL() << "WorkLoadManager was not initialized correctly";
	}

	// When
	const std::string commandOutput = "JOBID ST\n 65543 CD\n";
	const JobStatus status = wlm->getJobStatusFromOutputOfJobStatusCommand(commandOutput);

	// Then
	EXPECT_EQ(status, JobStatus::JobFinished);
}

TEST(CTCWorkLoadManager, Get_JobStatus_Unknown_SLURM) {
	// Given
	auto wlm = workloadmanagers::WorkLoadManager::Create("cldrn.sh", workloadmanagers::WorkLoadManagerType::SLURM);
	if (!wlm)
	{
		FAIL() << "WorkLoadManager was not initialized correctly";
	}

	// When
	const std::string commandOutput = "JOBID ST\n"; // There should always be one JobID
	const JobStatus status = wlm->getJobStatusFromOutputOfJobStatusCommand(commandOutput);

	// Then
	EXPECT_EQ(status, JobStatus::JobStatusUnknown);
}
