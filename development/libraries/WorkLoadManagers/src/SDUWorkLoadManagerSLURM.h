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

#ifndef __SDUWorkLoadManagerSLURM_h__
#define __SDUWorkLoadManagerSLURM_h__


#include "SDUWorkLoadManager.h"

namespace workloadmanagers {

	class WorkLoadManagerForSLURM : public WorkLoadManager {
	public:
		explicit WorkLoadManagerForSLURM(const std::string& JobSubmissionScriptName);
		 ~WorkLoadManagerForSLURM() final = default;
		 // no copy no move
		 WorkLoadManagerForSLURM(const WorkLoadManagerForSLURM&) = delete;
		 WorkLoadManagerForSLURM& operator=(const WorkLoadManagerForSLURM&) = delete;

	protected:
		bool writeProjectNameSpecification(const std::string& theJobSubmissionProjectName) final;
		bool writeWaitTimeSpecification(const std::string& theJobSubmissionWaitTimeSpec) final;
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
		 std::string JobTerminate() const override { return std::string(); };
		 std::string JobStatus() const override { return std::string(); };

	};
}

#endif
