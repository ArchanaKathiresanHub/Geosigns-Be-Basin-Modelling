#ifndef SIMULATORSTATEBASE_H
#define SIMULATORSTATEBASE_H

#include <string>
#include <map>
#include<iostream>
#include <fstream>
#include<iomanip> 
#include "SpeciesState.h"
#include "SpeciesManager.h"
#include "ChemicalModel.h"

using namespace std;

namespace Genex6
{
class SpeciesState;
class Species; 
class SimulatorStateBase
{
public:
   SimulatorStateBase(const double in_referenceTime = 0.0);
   virtual ~SimulatorStateBase();

   double GetSpeciesConcentrationByName(const int in_SpeciesName) const;
   SpeciesState *GetSpeciesStateById(const int in_SpeciesId);
   void AddSpeciesStateById(const int in_SpeciesId, SpeciesState *theSpeciesState);
   void GetSpeciesStateConcentrations(const ChemicalModel* chemicalModel, std::map<string, double> &currentSpeciesConcs) const;
   void GetSpeciesStateConcentrations( double * currentSpeciesConcs ) const;
   
   double GetReferenceTime() const;
   int GetTimeStep() const;
   void SetReferenceTime(const double in_ReferenceTime);

   double * getSpeciesUltimateMasses();
   double getSpeciesUltimateMassByName(const int name) const;

   bool isInitialized(){return m_isInitialized;}
   void setInitializationStatus(bool status){m_isInitialized = status;}

   double getMaxPrecokeTransfRatio();
   void   setMaxPrecokeTransfRatio(const double inMaxPrecokeTransRatios);
   double getMaxCoke2TransTransfRatio();
   void   setMaxCoke2TransTransfRatio(const double inMaxCoke2TransTransfRatio);
   double getCarrierBedPermeability() const;
   void   setCarrierBedPermeability(const double inCarrierBedPermeability);

   virtual void ComputeFirstTimeInstance(ChemicalModel *theChmod) = 0;
   virtual void SetSpeciesTimeStepVariablesToZero() = 0;
   virtual void PostProcessTimeStep(Species &aSpecies, const double in_dT, const double temp) = 0;
   virtual double ComputeWaso() = 0;
   virtual double ComputeWbo() = 0;
   virtual void SetLumpedConcentrationsToZero() = 0;
   virtual void SetResultsToZero() = 0;
   virtual void initSpeciesUltimateMass ( const SpeciesManager& speciesManager ) = 0;

   virtual double ComputeKerogenTransformatioRatio ( const SpeciesManager& speciesManager,
                                                     int aSimulationType) = 0; 

   virtual double ComputeDiffusionConcDependence(const double in_Waso, const bool isGx7) = 0;

   virtual double getTotalOilForTSR() const;
   virtual void   incTotalOilForTSR( double /* currentConc */ ) {};
   virtual void   setTotalOilForTSR( double /* currentConc */ ) {};

   void setInitialToc ( const double toc );

   double getInitialToc () const;

   void setTocAtVre05 ( const double toc );

   double getTocAtVre05 () const;

   bool tocAtVre05Set () const;

   void setCurrentToc ( const double toc );

   double getCurrentToc () const;

   void setInorganicDensity ( const double inInorganicDensity );
   double getInorganicDensity () const;


protected:
   bool m_isInitialized; 
   double m_referenceTime; 
   int m_timeStep; // current tumeStep number

   double m_UltimateMassesBySpeciesName[Genex6::SpeciesManager::numberOfSpecies];
   SpeciesState * m_SpeciesStateBySpeciesName[Genex6::SpeciesManager::numberOfSpecies];

private:
   double m_maxprecokeTransformationRatio;
   double m_maxcoke2TransformationRatio;

   double m_carrierBedPermeability;

   void clearSpeciesState();

   double m_initialToc;
   double m_currentToc;
   double m_tocAtVre05;
   bool   m_tocAtVre05Set;
   double m_InorganicDensity;

};
inline void SimulatorStateBase::SetReferenceTime(const double in_ReferenceTime)
{
  m_referenceTime = in_ReferenceTime; 
  ++ m_timeStep;
}
inline int SimulatorStateBase::GetTimeStep() const
{
   return m_timeStep;
}
inline double SimulatorStateBase::GetReferenceTime() const
{
   return m_referenceTime;
}
inline double SimulatorStateBase::getMaxPrecokeTransfRatio()
{
   return m_maxprecokeTransformationRatio;
}
inline void SimulatorStateBase::setMaxPrecokeTransfRatio(const double inMaxPrecokeTransRatios)
{
   m_maxprecokeTransformationRatio = inMaxPrecokeTransRatios;
}
inline double SimulatorStateBase::getMaxCoke2TransTransfRatio()
{
   return m_maxcoke2TransformationRatio;
}
inline void SimulatorStateBase::setMaxCoke2TransTransfRatio(const double inMaxCoke2TransTransfRatio)
{
   m_maxcoke2TransformationRatio = inMaxCoke2TransTransfRatio;
}

inline void SimulatorStateBase::setCarrierBedPermeability(const double inCarrierBedPermeability)
{
   m_carrierBedPermeability = inCarrierBedPermeability;
}

inline double SimulatorStateBase::getCarrierBedPermeability() const
{
   return m_carrierBedPermeability;
}

inline double SimulatorStateBase::getInitialToc () const {
   return m_initialToc;
} 

inline double SimulatorStateBase::getCurrentToc () const {
   return m_currentToc;
} 

inline void SimulatorStateBase::setInorganicDensity (const double inInorganicDensity) {
   m_InorganicDensity = inInorganicDensity;
} 

inline double SimulatorStateBase::getInorganicDensity () const {
   return m_InorganicDensity;
} 

inline void SimulatorStateBase::setTocAtVre05 ( const double toc ) {

   if ( not m_tocAtVre05Set ) {
      m_tocAtVre05 = toc;
      m_tocAtVre05Set = true;
   }

} 

inline double SimulatorStateBase::getTocAtVre05 () const {
   return m_tocAtVre05;
}

inline bool SimulatorStateBase::tocAtVre05Set () const {
   return m_tocAtVre05Set;
}

inline double SimulatorStateBase::getTotalOilForTSR() const {
   return 0.0;
}


}
#endif
