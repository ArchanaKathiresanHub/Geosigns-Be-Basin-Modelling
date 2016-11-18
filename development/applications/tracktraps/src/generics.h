//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef GENERICS_H
#define GENERICS_H

#include <string>
using namespace std;

#include "ComponentManager.h"

using namespace CBMGenerics;


extern const double MolarWeights[ComponentManager::NUMBER_OF_SPECIES];

extern const string CEPPrefix;
extern const string MassPrefix;
extern const string VolumePrefix;
extern const string DensityPrefix;
extern const string ViscosityPrefix;

extern const string StockTankPhaseNames[ComponentManager::NUMBER_OF_PHASES][ComponentManager::NUMBER_OF_PHASES];

extern const double UndefinedValue;
extern const double StockTankPressure;
extern const double StockTankTemperature;

extern double Accumulate (double values[], int numberOfValues);

#endif // GENERICS_H

