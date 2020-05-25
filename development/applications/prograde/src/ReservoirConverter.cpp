//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ReservoirConverter.h"

//cmbAPI
#include "ReservoirManager.h"

//utilities
#include "LogHandler.h"

using namespace mbapi;

void Prograde::ReservoirConverter::trapCapacityLogic(const double valueProject3d, double & globalValue) {
  
   if (valueProject3d < globalValue) globalValue = valueProject3d;
   
}
void Prograde::ReservoirConverter::blockingPermeabilityLogic(const double valueProject3d, double & globalValue) {
  
   if ( (valueProject3d != globalValue)) {
      globalValue = 1e-9;
   }
   else {
      globalValue = valueProject3d;
   }

}
void Prograde::ReservoirConverter::bioDegradIndLogic(const int valueProject3d, int & globalValue) {
   
   if (valueProject3d) globalValue = valueProject3d;
}
void Prograde::ReservoirConverter::oilToGasCrackingIndLogic(const int valueProject3d, int & globalValue) {
   
   if (valueProject3d) globalValue = valueProject3d;
}
void Prograde::ReservoirConverter::blockingIndLogic(const int valueProject3d, int & globalValue) {
   
   if (!valueProject3d) globalValue = valueProject3d;
}

double Prograde::ReservoirConverter::upgradeActivityStartAge(const std::string & resMode, const double & stratDepoAge, double & resActivityStartAge)
{
	double originalActivityStartAge = resActivityStartAge;
	if (resMode.compare("ActiveFrom") == 0 && resActivityStartAge > stratDepoAge)
	{
		resActivityStartAge = stratDepoAge;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original activityStartAge ("<< originalActivityStartAge <<" Ma) is greater than the reservoir depo age ("<< stratDepoAge<<" Ma). Resetting its value to depoAge of "<< stratDepoAge <<" Ma ";
	}
	return resActivityStartAge;
}
