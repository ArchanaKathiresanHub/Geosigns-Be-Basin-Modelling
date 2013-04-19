#ifndef _OTGC_KERNEL6__SIMULATOR_STATE_H_
#define _OTGC_KERNEL6__SIMULATOR_STATE_H_


#include <string>
#include <map>
#include <vector>
#include<iostream>
#include <fstream>
#include<iomanip>
#include "SimulatorStateBase.h"
#include "SpeciesManager.h"

using namespace std;
using namespace Genex6;

namespace OTGC6
{
class SpeciesState;

//! Simulator is the main controlling class 
/*! 
   Provides the core computational functionality 
*/
class SimulatorState : public Genex6::SimulatorStateBase
{
public:
   SimulatorState();
   SimulatorState(const double inReferenceTime, const Species ** SpeciesInChemicalModel,
                  const std::map<string, double> &initSpeciesConcs);

   SimulatorState(const double inReferenceTime, const Species ** SpeciesInChemicalModel,
                  const double * initSpeciesConcs);

   virtual ~SimulatorState(){};

   void SetSpeciesTimeStepVariablesToZero(){};
   void PostProcessTimeStep(Species &theSpecies, const double in_dT){};
 
   void ComputeFirstTimeInstance(ChemicalModel *theChmod = NULL){};

   double ComputeWaso() {return 0.0;}
   double ComputeWbo()  {return 0.0;}

   void SetLumpedConcentrationsToZero(){};
   void SetResultsToZero(){};
   double ComputeKerogenTransformatioRatio( const SpeciesManager& speciesManager,
                                            int aSimulationType ){ return 0.0; }; 
   double ComputeDiffusionConcDependence(const double in_Waso){ return 0.0; }; 
protected:
   void initSpeciesUltimateMass ( const SpeciesManager& speciesManager );

};

} 
#endif // _OTGC_KERNEL6__SIMULATOR_STATE_H_
 
