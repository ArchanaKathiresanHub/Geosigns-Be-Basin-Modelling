#include "generics.h"
#include <assert.h>

#include <map>
using namespace std;

const double MolarWeights[ComponentManager::NUMBER_OF_SPECIES] =
{
   16.043, 30.070, 44.097, 58.124, 72.151, 28.013, 44.009, 128.259, 254.503, 464.909, 122.281, 153.256, 187.336
};

const string CEPPrefix = "CEP";
const string MassPrefix = "Mass";
const string VolumePrefix = "Volume";
const string DensityPrefix = "Density";
const string ViscosityPrefix = "Viscosity";

const string StockTankPhaseNames[ComponentManager::NUMBER_OF_PHASES][ComponentManager::NUMBER_OF_PHASES] =
{
   { "FreeGas", "Condensate" },
   { "SolutionGas", "StockTankOil" }
};

double Accumulate(double values[], int numberOfValues)
{
	int i;

	double accumulatedValue = 0;

	for (i = 0; i < numberOfValues; ++i)
	{
		accumulatedValue += values[i];
	}

	return accumulatedValue;
}
