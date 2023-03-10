//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "BiodegradeConverter.h"

//utilities
#include "LogHandler.h"

static const double MIN_BIO_TEMP = 0.0;
static const double MAX_BIO_TEMP = 100.0;
static const double MIN_TEMP_CONSTANT = 0.0;
static const double MAX_TEMP_CONSTANT = 100.0;
static const double MIN_TIME_CONSTANT = 0.0;
static const double MAX_TIME_CONSTANT = 100.0;
static const double MIN_BIO_RATE = 0.0;
static const double MAX_BIO_RATE = 10000.0;

void Prograde::BiodegradeConverter::upgradeBioConstants(const std::string & BioConsName, const double BioConsValue, double & BioConsFromP3dFile)
{
   if (BioConsName == "MaxBioTemp") {
      if (BioConsFromP3dFile < MIN_BIO_TEMP || BioConsFromP3dFile > MAX_BIO_TEMP) {
         LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> Biodegradation Constant " << BioConsName <<" is updated from "<< BioConsFromP3dFile <<" to " << BioConsValue<<" as the value was not in valid range [" << MIN_BIO_TEMP << ", " << MAX_BIO_TEMP << "]";
         BioConsFromP3dFile = BioConsValue;
      }
   }
   else if (BioConsName == "TempConstant") {
      if (BioConsFromP3dFile < MIN_TEMP_CONSTANT || BioConsFromP3dFile > MAX_TEMP_CONSTANT) {
		 LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> Biodegradation Constant " << BioConsName << " is updated from " << BioConsFromP3dFile << " to " << BioConsValue << " as the value was not in valid range [" << MIN_TEMP_CONSTANT << ", " << MAX_TEMP_CONSTANT << "]";
         BioConsFromP3dFile = BioConsValue;
      }
   }
   else if (BioConsName == "TimeConstant") {
      if (BioConsFromP3dFile < MIN_TIME_CONSTANT || BioConsFromP3dFile > MAX_TIME_CONSTANT) {
		 LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> Biodegradation Constant " << BioConsName << " is updated from " << BioConsFromP3dFile << " to " << BioConsValue << " as the value was not in valid range [" << MIN_TIME_CONSTANT << ", " << MAX_TIME_CONSTANT << "]";
         BioConsFromP3dFile = BioConsValue;
      }
   }
   else if (BioConsName == "BioRate") {
      if (BioConsFromP3dFile < MIN_BIO_RATE || BioConsFromP3dFile > MAX_BIO_RATE) {
		 LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> Biodegradation Constant " << BioConsName << " is updated from " << BioConsFromP3dFile << " to " << BioConsValue << " as the value was not in valid range [" << MIN_BIO_RATE << ", " << MAX_BIO_RATE << "]";
         BioConsFromP3dFile = BioConsValue;
      }
   }
   else
      if (BioConsValue != BioConsFromP3dFile) {
		 LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> Upgrading Biodegradation Constant " << BioConsName << " from " << BioConsFromP3dFile << " to " << BioConsValue << " as a non standard value was detected!";
         BioConsFromP3dFile = BioConsValue;
      }
}
