#ifndef _GENEX5_SIMULATORSTATE_H
#define _GENEX5_SIMULATORSTATE_H


#include <string>
#include <map>
#include<iostream>
#include <fstream>
#include<iomanip>
using namespace std;


#include "Constants.h"
#include "ImmobileSpecies.h"
#include "PVTCalculator.h"

namespace Genex5
{
class SpeciesState;

   typedef std::map<std::string, SpeciesState*> SpeciesNameMap;

//!The set of quantities that are required for the initialization of a Simulator and the start of a simulation step
/*!
* SimulatorState encapsulates all the necessary primary variables that are required for a Genex simulation by a Genex5::Simulator. 

It forms the interface between the Simulator and the SourceRockNode or any other client code. 

Based on the data of an initilized SimulatorState object Genex5::Simulator can proceed with computing further if an input history is available. 

The initilization of the SimulatorState object is done through Genex5::Simulator::InitializeSourceRockNode.

Every Genex5::SourceRockNode has-a SimulatorState object which it uses to inform a Genex5::Simulator about the current starting state when requesting a simulation through Genex5::SourceRockNode::RequestComputation.

*/
class SimulatorState
{
public:
   SimulatorState();
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

   void setInorganicDensity ( const double newDensity );
   double getInorganicDensity () const;

   void setSGMeanBulkDensity ( const double newDensity );
   double getSGMeanBulkDensity () const;

   double GetTotalOMConc()const;
   double GetMobileOMConc()const;
   double GetSpeciesConcentrationByName(const std::string &in_SpeciesName) const ;
   SpeciesState *GetSpeciesStateByName(const std::string &in_SpeciesName);
   void AddSpeciesStateByName(const std::string &in_SpeciesName, SpeciesState *theSpeciesState);

   void UpdateLumpedConcentrations(const double &concentration, const double &Oil, const double &AromaticOM);
   void SetLumpedConcentrationsToZero();
   double GetReferenceTime() const;
   void SetReferenceTime(const double in_ReferenceTime);

   void SetCumQuantity(const CumullativeQuantityId &theId, const double &in_value = 0.0);  
   double GetCumQuantity(const CumullativeQuantityId &theId) const;
   
   double GetInitialKerogenConcentration() const;
   void SetInitialKerogenConcentration(const double &in_KerogenConc);

   // Where do these really belong?
   ImmobileSpecies& getImmobileSpecies ();

   void setImmobileSpecies ( const ImmobileSpecies& immobiles );


   void setSubSurfaceDensities ( const PVTPhaseValues& densities );

   const PVTPhaseValues& getSubSurfaceDensities () const;

   void setLiquidComponents ( const PVTComponents& liquid );

   void addLiquidComponents ( const PVTComponents& liquid );

   const PVTComponents& getLiquidComponents () const;


   void setVapourComponents ( const PVTComponents& vapour );

   void addVapourComponents ( const PVTComponents& vapour );

   const PVTComponents& getVapourComponents () const;


   void setRetainedVolumes ( const double retainedVapour,
                             const double retainedLiquid );

   double getRetainedVapourVolume () const;

   double getRetainedLiquidVolume () const;

   void setUsablePorosity ( const double usablePorosity );

   double getUsablePorosity () const;


   void setHcSaturation ( const double hcSaturation );

   double getHcSaturation () const;

   void setIrreducibleWaterSaturation ( const double irreducibleWaterSaturation );

   double getIrreducibleWaterSaturation () const;


   void setInitialToc ( const double toc );

   double getInitialToc () const;


   void setTocAtVre05 ( const double toc );

   double getTocAtVre05 () const;

   bool tocAtVre05Set () const;


   void setCurrentToc ( const double toc );

   double getCurrentToc () const;

   /// Return the total gas that is generated frmo otgc in mass.
   double getTotalGasFromOtgc () const;

   /// Increment the total gas frmo otgc.
   void incrementTotalGasFromOtgc ( const double increment );



   // Is there a better way of exposing this to the world?
   const SpeciesNameMap& getSpeciesNameMap () const;


   void setVLReferenceTemperature ( const double vl );

   double getVLReferenceTemperature () const;

   void setVLSRTemperature ( const double vl );

   double getVLSRTemperature () const;


private:

   double m_referenceTime;       //timestep size Dt=   SourceRockNodeInput::m_currentTime - SourceRockNodeState::m_referenceTime
   double m_lumpedOMConcentration[Genex5::Constants::NUMBER_OF_ORGANIC_MATTER_TYPES];
   double m_InitialKerogenConcentration;
   double m_inorganicDensity;
   double m_meanBulkDensity;
   ImmobileSpecies m_immobileSpecies;

   double m_retainedVapourVolume;
   double m_retainedLiquidVolume;
   double m_usablePorosity;
   double m_initialToc;
   double m_currentToc;
   double m_VLSRTemperature;
   double m_VLReferenceTemperature;

   double m_totalGasFromOtgc;

   double m_tocAtVre05;
   bool   m_tocAtVre05Set;

   SpeciesNameMap                          m_SpeciesStateBySpeciesName;
   std::map<CumullativeQuantityId, double> m_CumQuantitiesById;

   SimulatorState(const SimulatorState &);
   SimulatorState & operator=(const SimulatorState &);


   PVTPhaseValues subSurfaceDensities;
   PVTComponents m_liquidComponents;
   PVTComponents m_vapourComponents;

   double m_irreducibleWaterSaturation;
   double m_hcSaturation;
   
};



inline ImmobileSpecies& SimulatorState::getImmobileSpecies () {
   return m_immobileSpecies;
}

inline double SimulatorState::GetInitialKerogenConcentration() const
{
   return m_InitialKerogenConcentration;
}
inline void SimulatorState::SetInitialKerogenConcentration(const double &in_KerogenConc)
{
   m_InitialKerogenConcentration = in_KerogenConc;
}
inline double SimulatorState::GetMobileOMConc()const
{
  return  m_lumpedOMConcentration[Genex5::Constants::MOBILE_OM];
}
inline double SimulatorState::GetTotalOMConc()const
{
  return m_lumpedOMConcentration[Genex5::Constants::TOTAL_OM];
}
inline void SimulatorState::SetReferenceTime(const double in_ReferenceTime)
{
  m_referenceTime = in_ReferenceTime; 
}
inline double SimulatorState::GetReferenceTime() const
{
   return m_referenceTime;
}

inline double SimulatorState::getInorganicDensity () const {
   return m_inorganicDensity;
}

inline double SimulatorState::getSGMeanBulkDensity () const {
   return m_meanBulkDensity;
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

inline double SimulatorState::getUsablePorosity () const {
   return m_usablePorosity;
}

inline double SimulatorState::getIrreducibleWaterSaturation () const {
   return m_irreducibleWaterSaturation;
}

inline double SimulatorState::getInitialToc () const {
   return m_initialToc;
} 

inline double SimulatorState::getCurrentToc () const {
   return m_currentToc;
} 

inline void SimulatorState::setTocAtVre05 ( const double toc ) {

   if ( not m_tocAtVre05Set ) {
      m_tocAtVre05 = toc;
      m_tocAtVre05Set = true;
   }

} 

inline double SimulatorState::getTocAtVre05 () const {
   return m_tocAtVre05;
}

inline bool SimulatorState::tocAtVre05Set () const {
   return m_tocAtVre05Set;
}

inline double SimulatorState::getTotalGasFromOtgc () const {
   return m_totalGasFromOtgc;
}



inline const SpeciesNameMap& SimulatorState::getSpeciesNameMap () const {
   return m_SpeciesStateBySpeciesName;
}

}
#endif
