#ifndef _GENERICS_H_
#define _GENERICS_H_

#include <string>
using namespace std;

#include "ComponentManager.h"
#include "consts.h"
using namespace CBMGenerics;


extern const double MolarWeights[ComponentManager::NumberOfOutputSpecies];
extern const string TableSpeciesNames[ComponentManager::NumberOfSpecies];

extern const string CEPPrefix;
extern const string MassPrefix;
extern const string VolumePrefix;
extern const string DensityPrefix;
extern const string ViscosityPrefix;

extern const string StockTankPhaseNames[ComponentManager::NumberOfPhases][ComponentManager::NumberOfPhases];

extern const double UndefinedValue;
extern const double StockTankPressure;
extern const double StockTankTemperature;

extern double Accumulate (double values[], int numberOfValues);

#endif // _GENERICS_H_

