#ifndef _GENEX6_KERNEL__SIMULATORSTATE_H_
#define _GENEX6_KERNEL__SIMULATORSTATE_H_

#include <string>
#include <map>
#include<iostream>
#include <fstream>
#include<iomanip>
using namespace std;

#include "SpeciesResult.h"
#include "SpeciesManager.h"
#include "Constants.h"
#include "SimulatorStateBase.h"
#include "Species.h"
#include "GenexResultManager.h"

#include "ImmobileSpecies.h"
#include "PVTCalculator.h"

namespace Genex6
{

#define FIRST_RESULT_ID CBMGenerics::GenexResultManager::OilGeneratedCum
#define LAST_RESULT_ID  CBMGenerics::GenexResultManager::NumberOfResults

class SpeciesState;
class SpeciesResult;

   typedef std::map<std::string, SpeciesState*> SpeciesNameMap;


//!The set of quantities that are required for the initialization of a Simulator and the start of a simulation step
/*!
* SimulatorState encapsulates all the necessary primary variables that are required for a Genex simulation by 
  a Genex6::Simulator. 
  It forms the interface between the Simulator and the SourceRockNode or any other client code. 
  Based on the data of an initilized SimulatorState object Genex6::Simulator can proceed with computing further 
  if an input history is available. 
  The initilization of the SimulatorState object is done through Genex6::Simulator::InitializeSourceRockNode.
  Every Genex6::SourceRockNode has-a SimulatorState object which it uses to inform a Genex6::Simulator about 
  the current starting state when requesting a simulation through Genex6::SourceRockNode::RequestComputation.

*/
class SimulatorState: public SimulatorStateBase
{
public:
   SimulatorState(const SpeciesManager * inSpeciesManager, const int in_numberOfSpecies, const double in_referenceTime);
   SimulatorState(const SimulatorState& in_State);
   virtual ~SimulatorState();
 
   enum CumullativeQuantityId
   { 
      OilExpelledMassCum = 0,               
      OilExpelledVolumeCum, 
      HcGasExpelledVolumeCum, 
      WetGasExpelledVolumeCum,
      C614SatPlusAromExpVolCum,
      AromaticsExpelledVolumeCum,
      SaturatesExpelledVolumeCum,
      NumberOfResults
   };

   double ComputeWaso();
   double ComputeWbo();

   void SetLumpedConcentrationsToZero();
   void SetResultsToZero();

   void UpdateLumpedConcentrations(const double concentration, 
                                   const double Oil, const double AromaticOM);

   void SetResult(const int theId, const double theResult);
   double GetResult(const int theId) const;

   void computeShaleGasResults ();
 
   /// \brief Sets the species value, indicated by the theId.
   void setShaleGasResult( const int theId, const double theResult );

   /// \brief Add to the current species value, indicated by the theId.
   void addShaleGasResult( const int theId, const double theResult );

   /// \brief Gets the species value, indicated by the theId.
   double getShaleGasResult( const int theId ) const;
 
   void SetThickness( const double theThickness );
   void SetConcki( const double theConcki );
   void SetConckiThickness( const double theConcki, const double theThickness );
   double GetThickness() const;
   double GetConcki() const;

   void SetOM();

   void SetCumQuantity(const CumullativeQuantityId theId, const double in_value = 0.0);  
   double GetCumQuantity(const CumullativeQuantityId theId) const;
   
   int getNumberOfSpecies() const;
   double GetInitialKerogenConcentration() const;
   void SetInitialKerogenConcentration(const double in_KerogenConc);

   /// \brief Set all the values accumulated for the time-interval (one or more genex time-steps) to be zero.
   void resetIntervalCumulative ();

   /// \brief Increment the values for the time-interval (may be one or more genex time-steps).
   void incrementIntervalQuantity ( const CumullativeQuantityId id,
                                    const double                increment );

   /// \brief Increment the values for the time-interval (may be one or more genex time-steps).
   void incrementIntervalSpeciesQuantity ( const int    id,
                                           const double increment );

   /// \brief Get a value for the time-interval.
   double getIntervalCumulative ( const CumullativeQuantityId id ) const;

   /// \brief Get a value for the time-interval.
   double getIntervalSpeciesCumulative ( const CBMGenerics::ComponentManager::SpeciesNamesId id ) const;

   const SpeciesResult&  GetSpeciesResult( const int theId ) const; 

   SpeciesResult&  GetSpeciesResult( const int theId ); 

   /// \brief Return const reference to species-manager.
   const SpeciesManager * getSpeciesManager () const;

   void PrintBenchmarkModelConcData(ofstream &outputfile) const;
   void PrintBenchmarkModelFluxData(ofstream &outputfile) const;
   void PrintSensitivityResultsOnFile(ofstream &outputfile) const;
   void PrintBenchmarkModelCumExpData(ofstream &outputfile) const;

   //void PostProcessTimeStepComputation ( const SpeciesManager * speciesManager );
   void PostProcessTimeStep(Species &theSpecies, const double in_dT);
   void PostProcessTimeStepComputation ( SimulatorState * subSimulatorState1 = 0, SimulatorState * subSimulatorState2 = 0,
                                         const double fraction1 = 0.0, const double fraction2 = 0.0);

   /// \brief Update state after a shale-gas time-step.
   ///
   /// Computes the compound results.
   void postProcessShaleGasTimeStep ( ChemicalModel *chemicalMode, const double in_dT );

   void ComputeFirstTimeInstance(ChemicalModel *theChmod);

   void SetExmTot(const double in_ExmTot);
   double GetTotalMass() const; 

   double GetExmTot();
   double GetOilExpelledMassInst();
   double GetOilExpelledVolumeInst();
   double GetHcGasExpelledVolumeInst();
   double GetWetGasExpelledVolumeInst();
   double GetC614SatPlusAromExpVolInst();
   double GetAromaticsExpelledVolumeInst();
   double GetSaturatesExpelledVolumeInst();
   double GetGroupResult(const int theId);
   void SetSpeciesTimeStepVariablesToZero();

   void initSpeciesUltimateMass ( const SpeciesManager& speciesManager );
   
   double ComputeKerogenTransformatioRatio ( const SpeciesManager& speciesManager,
                                             int aSimulationType); 

   double ComputeDiffusionConcDependence(const double in_Waso);


   // Adsorption related.

   // Where do these really belong?
   ImmobileSpecies& getImmobileSpecies ();

   void setImmobileSpecies ( const ImmobileSpecies& immobiles );
   void setImmobileDensitiesMixed ( SimulatorState * aState1,  SimulatorState * aState2, const double f1, const double f2 );
   void setImmobileRetainedMixed  ( SimulatorState * aState1,  SimulatorState * aState2, const double f1, const double f2 );

   /// \brief returns the amount generated of HC generated for each species.
   PVTComponents getGeneratedComponents () const;


   void setSubSurfaceDensities ( const PVTPhaseValues& densities );

   void setSubSurfaceDensities ( const PVTPhaseValues& densities1, const double scale1, const PVTPhaseValues& densities21, const double scale21 );

   const PVTPhaseValues& getSubSurfaceDensities () const;

   void setLiquidComponents ( const PVTComponents& liquid1,  const double scale1, const PVTComponents& liquid2,  const double scale2 );

   void setLiquidComponents ( const PVTComponents& liquid );

   void addLiquidComponents ( const PVTComponents& liquid );

   const PVTComponents& getLiquidComponents () const;


   void setVapourComponents ( const PVTComponents& vapour );
   void setVapourComponents ( const PVTComponents& vapour1,  const double scale1, const PVTComponents& vapour2,  const double scale2 );

   void addVapourComponents ( const PVTComponents& vapour );

   const PVTComponents& getVapourComponents () const;


   void setRetainedVolumes ( const double retainedVapour,
                             const double retainedLiquid );

   void setRetainedVapourVolume ( const double retainedVapourVolume);

   double getRetainedVapourVolume () const;

   double getRetainedLiquidVolume () const;

   void setRetainedLiquidVolume ( const double retainedLiquidVolume);

   void setEffectivePorosity ( const double effectivePorosity );

   double getEffectivePorosity () const;


   void setHcSaturation ( const double hcSaturation );

   double getHcSaturation () const;

   void setIrreducibleWaterSaturation ( const double irreducibleWaterSaturation );

   double getIrreducibleWaterSaturation () const;


   /// Return the total gas that is generated frmo otgc in mass.
   double getTotalGasFromOtgc () const;

   void setTotalGasFromOtgc ( const double totalgasFromOTGC );

   /// Increment the total gas frmo otgc.
   void incrementTotalGasFromOtgc ( const double increment );


   // Is there a better way of exposing this to the world?
   const SpeciesState* const* getSpeciesNameMap () const;


   void setVLReferenceTemperature ( const double vl );

   double getVLReferenceTemperature () const;

   void setVLSRTemperature ( const double vl );

   double getVLSRTemperature () const;

   double getHC() const;
   double getOC() const;

   void setH2SFromGenex ( const double h2s );
   void setH2SFromOtgc ( const double h2s );

   void addH2SFromGenex ( const double h2s );
   void addH2SFromOtgc ( const double h2s );

   double getH2SFromGenex () const;
   double getH2SFromOtgc () const;

private:

   void mixIntervalResults ( SimulatorState * inSimulatorState1,
                             SimulatorState * inSimulatorState2,
                             const double fraction1,
                             const double fraction2 );

   const SpeciesManager * m_speciesManager;
   SpeciesResult * m_SpeciesResults;

   int m_numberOfSpecies;
   double m_thickness;
   double m_concki;

   double m_lumpedOMConcentration[Genex6::Constants::NUMBER_OF_ORGANIC_MATTER_TYPES];
   double m_InitialKerogenConcentration;

   double m_ResultsByResultId[CBMGenerics::GenexResultManager::NumberOfResults];
   double m_shaleGasResultsByResultId[CBMGenerics::GenexResultManager::NumberOfResults];
  
   double m_CumQuantitiesById[NumberOfResults];

   double m_intervalCumulativeQuantities [ NumberOfResults ];
   double m_intervalCumulativeSpecies [ CBMGenerics::ComponentManager::NumberOfOutputSpecies ];

   double m_TotalRetainedOM;
   double m_MobilOMConc;

   //Variables initialised set to Zero in ChemicalModel::SetSpeciesInitialState,
   //updated for each call of Species::ComputeTimeStep  
   double s_ExmTot;
   double s_OilExpelledMassInst;
   double s_OilExpelledVolumeInst;
   double s_HcGasExpelledVolumeInst;
   double s_WetGasExpelledVolumeInst;
   double s_C614SatPlusAromExpVolInst;
   double s_AromaticsExpelledVolumeInst;
   double s_SaturatesExpelledVolumeInst;

   double s_GroupResults[LAST_RESULT_ID - FIRST_RESULT_ID];

   void AddExmTot(const double theExmTot); 
   void AddOilExpelledMassInst( const double theOilExpelledMassInst);
   void AddOilExpelledVolumeInst( const double theOilExpelledVolumeInst);
   void AddC614SatPlusAromExpVolInst( const double theC614SatPlusAromExpVolInst);
   void AddAromaticsExpelledVolumeInst( const double theAromaticsExpelledVolumeInst);
   void AddSaturatesExpelledVolumeInst( const double theSaturatesExpelledVolumeInst);
   void AddHcGasExpelledVolumeInst( const double theHcGasExpelledVolumeInst);
   void AddWetGasExpelledVolumeInst( const double theWetGasExpelledVolumeInst);
   void AddGroupResult(const int theId, const double theGroupResult);

   double m_AtomHR;
   double m_AtomCR;
   double m_AtomOR;
   // atomic ratios of OM (if immobility >= ActCrit)
   double m_OC;
   double m_HC;

   // Adsorption related.

   PVTPhaseValues subSurfaceDensities;
   PVTComponents m_liquidComponents;
   PVTComponents m_vapourComponents;
   ImmobileSpecies m_immobileSpecies; // set from ChemicalModel

   double m_irreducibleWaterSaturation;
   double m_hcSaturation;


   double m_retainedVapourVolume;
   double m_retainedLiquidVolume;
   double m_effectivePorosity;
   double m_VLSRTemperature;
   double m_VLReferenceTemperature;

   double m_totalGasFromOtgc;
   double m_h2sFromGenex;
   double m_h2sFromOtgc;


   /// Mix results of two genex runs as fraction1 * result1 + fraction2 * result2
   void mixResults ( SimulatorState * inSimulatorState1, SimulatorState * inSimulatorState2,
                     const double fraction1, const double fraction2 );
};
inline int SimulatorState::getNumberOfSpecies( ) const
{
   return m_numberOfSpecies; 
}
inline SpeciesResult &SimulatorState::GetSpeciesResult( const int theId )
{
   return m_SpeciesResults[theId - 1]; // count from 0
}

inline const SpeciesResult& SimulatorState::GetSpeciesResult( const int theId ) const{
   return m_SpeciesResults[theId - 1]; // count from 0
}

inline void SimulatorState::SetThickness( const double theThickness )
{
   m_thickness = theThickness;
}
inline void SimulatorState::SetConcki( const double theConcki )
{
   m_concki = theConcki;
}
inline void SimulatorState::SetConckiThickness( const double theConcki, const double theThickness )
{
   m_concki = theConcki;
   m_thickness = theThickness;
}
inline double SimulatorState::GetThickness() const
{
   return m_thickness;
}
inline double SimulatorState::GetConcki() const
{
   return m_concki;
}
inline double SimulatorState::GetInitialKerogenConcentration() const
{
   return m_InitialKerogenConcentration;
}
inline void SimulatorState::SetInitialKerogenConcentration(const double in_KerogenConc)
{
   m_InitialKerogenConcentration = in_KerogenConc * m_thickness;
}

inline double SimulatorState::GetTotalMass() const
{
    return m_TotalRetainedOM + s_ExmTot;
}
inline void SimulatorState::SetOM()
{
  m_TotalRetainedOM = m_lumpedOMConcentration[Genex6::Constants::TOTAL_OM] * m_thickness * m_concki;
  m_MobilOMConc = m_lumpedOMConcentration[Genex6::Constants::MOBILE_OM];
}
inline void SimulatorState::SetCumQuantity(const CumullativeQuantityId theId, 
                                           const double in_value)
{
   m_CumQuantitiesById[theId] = in_value;
} 
inline double SimulatorState::GetCumQuantity(const CumullativeQuantityId theId) const
{
   return m_CumQuantitiesById[theId];
}
inline void SimulatorState::SetResult(const int theId, const double theResult)
{
   m_ResultsByResultId[theId] = theResult;
}
inline double SimulatorState::GetResult(const int theId) const
{
   return m_ResultsByResultId[theId];
}

inline void SimulatorState::setShaleGasResult(const int theId, const double theResult)
{
   m_shaleGasResultsByResultId[theId] = theResult;
}
inline void SimulatorState::addShaleGasResult(const int theId, const double theResult)
{
   m_shaleGasResultsByResultId[theId] += theResult;
}
inline double SimulatorState::getShaleGasResult(const int theId) const
{
   return m_shaleGasResultsByResultId[theId];
}

inline double SimulatorState::GetGroupResult(const int theId)
{
   return s_GroupResults[theId - FIRST_RESULT_ID];
}
inline void SimulatorState::UpdateLumpedConcentrations(const double concentration, 
                                                       const double Oil, 
                                                       const double AromaticOM)
{
    //Clump(1) = Clump(1) + Conc(L);
   m_lumpedOMConcentration[Genex6::Constants::TOTAL_OM] += concentration;

   m_lumpedOMConcentration[Genex6::Constants::MOBILE_OM] += Oil;

   m_lumpedOMConcentration[Genex6::Constants::AROMATIC_IMMOBILE_OM] += AromaticOM;
}
inline void SimulatorState::AddExmTot(const double theExmTot) 
{
   s_ExmTot += theExmTot;
}
inline void SimulatorState::AddOilExpelledMassInst(const double theOilExpelledMassInst) 
{
   s_OilExpelledMassInst += theOilExpelledMassInst;
}
inline void SimulatorState::AddOilExpelledVolumeInst(const double theOilExpelledVolumeInst) 
{
   s_OilExpelledVolumeInst += theOilExpelledVolumeInst;
}
inline void SimulatorState::AddC614SatPlusAromExpVolInst(const double theC614SatPlusAromExpVolInst) 
{
   s_C614SatPlusAromExpVolInst += theC614SatPlusAromExpVolInst;
}
inline void SimulatorState::AddAromaticsExpelledVolumeInst(const double theAromaticsExpelledVolumeInst) 
{
   s_AromaticsExpelledVolumeInst += theAromaticsExpelledVolumeInst;
}
inline void SimulatorState::AddSaturatesExpelledVolumeInst(const double theSaturatesExpelledVolumeInst) 
{
   s_SaturatesExpelledVolumeInst += theSaturatesExpelledVolumeInst;
}
inline void SimulatorState::AddHcGasExpelledVolumeInst(const double theHcGasExpelledVolumeInst) 
{
   s_HcGasExpelledVolumeInst += theHcGasExpelledVolumeInst;
}
inline void SimulatorState::AddWetGasExpelledVolumeInst(const double theWetGasExpelledVolumeInst) 
{
   s_WetGasExpelledVolumeInst += theWetGasExpelledVolumeInst;
}
inline void SimulatorState::AddGroupResult(const int theId, const double theGroupResult) 
{
   s_GroupResults[theId - FIRST_RESULT_ID]  += theGroupResult;
}   
inline void SimulatorState::initSpeciesUltimateMass( const SpeciesManager& speciesManager )
{
   m_UltimateMassesBySpeciesName [ speciesManager.getPreasphaltId () - 1] = 1.0;
}

inline ImmobileSpecies & SimulatorState::getImmobileSpecies () {
   return m_immobileSpecies;
}


inline const PVTComponents& SimulatorState::getLiquidComponents () const {
   return m_liquidComponents;
}

inline const PVTComponents& SimulatorState::getVapourComponents () const {
   return m_vapourComponents;
}

inline double SimulatorState::getHcSaturation () const {
   return m_hcSaturation;
}

inline double SimulatorState::getRetainedVapourVolume () const {
   return m_retainedVapourVolume;
}

inline double SimulatorState::getRetainedLiquidVolume () const {
   return m_retainedLiquidVolume;
}

inline double SimulatorState::getEffectivePorosity () const {
   return m_effectivePorosity;
}

inline double SimulatorState::getIrreducibleWaterSaturation () const {
   return m_irreducibleWaterSaturation;
}

inline double SimulatorState::getTotalGasFromOtgc () const {
   return m_totalGasFromOtgc;
}

inline const SpeciesState* const* SimulatorState::getSpeciesNameMap () const {
   return m_SpeciesStateBySpeciesName;
}

inline double SimulatorState::getHC() const {
   return m_HC;
}

inline double SimulatorState::getOC() const {
   return m_OC;
}


inline double SimulatorState::getH2SFromGenex () const {
   return m_h2sFromGenex;
}

inline double SimulatorState::getH2SFromOtgc () const {
   return m_h2sFromOtgc;
}

inline const SpeciesManager * SimulatorState::getSpeciesManager () const {
   return m_speciesManager;
}

}
#endif // _GENEX6_KERNEL__SIMULATOR_STATE_H_
