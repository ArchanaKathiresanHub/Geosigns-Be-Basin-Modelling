#include "generics.h"
#include <assert.h>

#include <map>
using namespace std;

const double MolarWeights[ComponentManager::NumberOfSpecies] =
{
   16.043, 30.070, 44.097, 58.124, 72.151, 28.013, 44.009, 128.259, 254.503, 464.909, 122.281, 153.256, 187.336
};

const string TableSpeciesNames[ComponentManager::NumberOfSpecies] = 
{
   "asphaltenes", "resins",
   "C15Aro", "C15Sat",
   "C6_14Aro", "C6_14Sat",
   "C5", "C4", "C3", "C2", "C1",
   "COx", "N2", "H2S",
   "LSC", "C15AT", "C6_14BT", "C6_14DBT", "C6_14BP", "C15AroS", "C15SatS", "C6_14SatS", "C6_14AroS"
};

const string CEPPrefix = "CEP";
const string MassPrefix = "Mass";
const string VolumePrefix = "Volume";
const string DensityPrefix = "Density";
const string ViscosityPrefix = "Viscosity";

const string StockTankPhaseNames[ComponentManager::NumberOfPhases][ComponentManager::NumberOfPhases] =
{
   { "FreeGas", "Condensate" },
   { "SolutionGas", "StockTankOil" }
};

double Accumulate (double values[], int numberOfValues)
{
   int i;

   double accumulatedValue = 0;

   for (i = 0; i < numberOfValues; ++i)
   {
      accumulatedValue += values[i];
   }

   return accumulatedValue;
}
