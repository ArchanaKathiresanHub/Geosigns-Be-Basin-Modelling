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
   void PostProcessTimeStep(Species & /*theSpecies */, const double /* in_dT */, const double /* temp */){};
 
   void ComputeFirstTimeInstance(ChemicalModel * /* theChmod */){};

   double ComputeWaso() {return 0.0;}
   double ComputeWbo()  {return 0.0;}

   void SetLumpedConcentrationsToZero(){};
   void SetResultsToZero(){ };
   double ComputeKerogenTransformatioRatio( const SpeciesManager& /* speciesManager */,int /* aSimulationType */ ){ return 0.0; }; 
   double ComputeDiffusionConcDependence(const double /* in_Waso */, const bool /*isGx7 */ ){ return 0.0; }; 

   double getTotalOilForTSR () const;
   void incTotalOilForTSR ( double curentConc );
   void setTotalOilForTSR ( double curentConc );

protected:
   void initSpeciesUltimateMass ( const SpeciesManager& speciesManager );
private:

   double m_concTotalOilForTSR;
};

inline double SimulatorState::getTotalOilForTSR () const {
   return m_concTotalOilForTSR;
}

inline void SimulatorState::incTotalOilForTSR ( double curentConc ) {
   m_concTotalOilForTSR += curentConc;
}

inline void SimulatorState::setTotalOilForTSR ( double curentConc ) {
   m_concTotalOilForTSR = curentConc;
}

} 
#endif // _OTGC_KERNEL6__SIMULATOR_STATE_H_
 
