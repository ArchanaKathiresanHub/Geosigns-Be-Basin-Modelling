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

using namespace mbapi;

void Prograde::ReservoirConverter::trapCapacityLogic(const double valueProject3d, double & globalValue) {
  
   if (valueProject3d < globalValue) globalValue = valueProject3d;
   
}
void Prograde::ReservoirConverter::blockingPermeabilityLogic(const size_t resId, const double valueProject3d, double & globalValue) {
  
   if (resId > 0 && (valueProject3d != globalValue)) {
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
