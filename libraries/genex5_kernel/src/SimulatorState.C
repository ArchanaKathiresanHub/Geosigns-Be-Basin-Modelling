#include "SimulatorState.h"
#include "SpeciesState.h"
#include <map>

namespace Genex5
{
SimulatorState::SimulatorState()
{
   for(int i=Genex5::Constants::TOTAL_OM;i<Genex5::Constants::NUMBER_OF_ORGANIC_MATTER_TYPES; i++)
   {
      m_lumpedOMConcentration[i]=0.0;
   }

   m_referenceTime = 0.0;
   m_inorganicDensity = 0.0;

   SetCumQuantity(OilExpelledMassCum);
   SetCumQuantity(OilExpelledVolumeCum);
   SetCumQuantity(HcGasExpelledVolumeCum);
   SetCumQuantity(C614SatPlusAromExpVolCum);
   SetCumQuantity(AromaticsExpelledVolumeCum);
   SetCumQuantity(SaturatesExpelledVolumeCum);

   m_liquidComponents.zero ();
   m_vapourComponents.zero ();

   m_irreducibleWaterSaturation = 1.0;
   m_hcSaturation = 0.0;
   m_tocAtVre05Set = false;
   m_tocAtVre05 = 0.0;

   m_VLSRTemperature = 0.0;
   m_VLReferenceTemperature = 0.0;

   m_retainedVapourVolume = 0.0;
   m_retainedLiquidVolume = 0.0;

   m_InitialKerogenConcentration = 0.0;
   m_meanBulkDensity = 0.0;
   m_usablePorosity = 0.0;
   m_initialToc = 0.0;
   m_currentToc = 0.0;
   m_VLSRTemperature = 0.0;
   m_VLReferenceTemperature = 0.0;

   m_totalGasFromOtgc = 0.0;

   subSurfaceDensities.zero ();



}
SimulatorState::~SimulatorState()
{
   std::map<std::string,SpeciesState*>::iterator itEnd =  m_SpeciesStateBySpeciesName.end();
   for(std::map<std::string,SpeciesState*>::iterator it = m_SpeciesStateBySpeciesName.begin();it != itEnd;++it)
   {
      delete (it->second);
   }
   m_SpeciesStateBySpeciesName.clear();
}
void SimulatorState::SetLumpedConcentrationsToZero()
{
   for(int i = Genex5::Constants::TOTAL_OM;i < Genex5::Constants::NUMBER_OF_ORGANIC_MATTER_TYPES; i++)
   {
      m_lumpedOMConcentration[i] = 0.0;
   }
}
void SimulatorState::AddSpeciesStateByName(const std::string &in_SpeciesName, SpeciesState *theSpeciesState)
{
   std::map<std::string,SpeciesState*>::iterator itB = m_SpeciesStateBySpeciesName.find(in_SpeciesName);
   if(itB == m_SpeciesStateBySpeciesName.end())
   {
      m_SpeciesStateBySpeciesName.insert(std::make_pair(in_SpeciesName,theSpeciesState));
   }
   else
   {
     //throw a warning
   }
}
double SimulatorState::GetSpeciesConcentrationByName(const std::string &in_SpeciesName) const
{
    double ret = 0.0;
    SpeciesNameMap::const_iterator it = m_SpeciesStateBySpeciesName.find(in_SpeciesName);

    if(it != m_SpeciesStateBySpeciesName.end())
    {
      ret = it->second->GetConcentration();
    }
    return ret;
}
SpeciesState *SimulatorState::GetSpeciesStateByName(const std::string &in_SpeciesName)
{
    SpeciesState *ret = 0;
    SpeciesNameMap::iterator it = m_SpeciesStateBySpeciesName.find(in_SpeciesName);

    if(it != m_SpeciesStateBySpeciesName.end())
    {
      ret = it->second;
    }
    return ret;
}
double SimulatorState::ComputeWaso()
{
   return (m_lumpedOMConcentration[Genex5::Constants::AROMATIC_IMMOBILE_OM] / m_lumpedOMConcentration[Genex5::Constants::TOTAL_OM]);
}
double SimulatorState::ComputeWbo()
{
   return (m_lumpedOMConcentration[Genex5::Constants::MOBILE_OM]/ m_lumpedOMConcentration[Genex5::Constants::TOTAL_OM]);
}
void SimulatorState::UpdateLumpedConcentrations(const double &concentration, const double &Oil, const double &AromaticOM)
{
    //Clump(1) = Clump(1) + Conc(L);
   m_lumpedOMConcentration[Genex5::Constants::TOTAL_OM] += concentration;

   m_lumpedOMConcentration[Genex5::Constants::MOBILE_OM] += Oil;

   m_lumpedOMConcentration[Genex5::Constants::AROMATIC_IMMOBILE_OM] += AromaticOM;
}
void SimulatorState::SetCumQuantity(const CumullativeQuantityId &theId, const double &in_value)
{
   m_CumQuantitiesById[theId] = in_value;
} 
double SimulatorState::GetCumQuantity(const CumullativeQuantityId &theId) const
{
   std::map<CumullativeQuantityId,double>::const_iterator resIt =  m_CumQuantitiesById.find(theId);
   double Value = Genex5::Constants::s_undefinedValue;
   if(resIt != m_CumQuantitiesById.end() )
   {
      Value = resIt->second;
   }
   return Value;
}

void SimulatorState::setInorganicDensity ( const double newDensity ) {
   m_inorganicDensity = newDensity;
}

void SimulatorState::setSGMeanBulkDensity ( const double newDensity ) {
   m_meanBulkDensity = newDensity;
}


void SimulatorState::setImmobileSpecies ( const ImmobileSpecies& immobiles ) {
   m_immobileSpecies = immobiles;
}

void SimulatorState::setSubSurfaceDensities ( const PVTPhaseValues& densities ) {
   subSurfaceDensities = densities;
}

const PVTPhaseValues& SimulatorState::getSubSurfaceDensities () const {
   return subSurfaceDensities;
}

void SimulatorState::setLiquidComponents ( const PVTComponents& liquid ) {

   m_liquidComponents = liquid;
}

void SimulatorState::setVapourComponents ( const PVTComponents& vapour ) {
   m_vapourComponents = vapour;
}

void SimulatorState::addLiquidComponents ( const PVTComponents& liquid ) {

   m_liquidComponents += liquid;
}

void SimulatorState::addVapourComponents ( const PVTComponents& vapour ) {
   m_vapourComponents += vapour;
}

void SimulatorState::setRetainedVolumes ( const double retainedVapour,
                                          const double retainedLiquid ) {

   m_retainedVapourVolume = retainedVapour;
   m_retainedLiquidVolume = retainedLiquid;

}

void SimulatorState::setUsablePorosity ( const double usablePorosity ) {
   m_usablePorosity = usablePorosity;
}

void SimulatorState::setHcSaturation ( const double hcSaturation ) {
   m_hcSaturation = hcSaturation;
}

void SimulatorState::setIrreducibleWaterSaturation ( const double irreducibleWaterSaturation ) {
   m_irreducibleWaterSaturation = irreducibleWaterSaturation;
}

void SimulatorState::incrementTotalGasFromOtgc ( const double increment ) {
   m_totalGasFromOtgc += increment;
}

void SimulatorState::setInitialToc ( const double toc ) {
   m_initialToc = toc;
   m_tocAtVre05 = toc;
   m_tocAtVre05Set = false;
} 

void SimulatorState::setCurrentToc ( const double toc ) {
   m_currentToc = toc;
} 

void SimulatorState::setVLReferenceTemperature ( const double vl ) {
   m_VLReferenceTemperature = vl;
}

double SimulatorState::getVLReferenceTemperature () const {
   return m_VLReferenceTemperature;
}


void SimulatorState::setVLSRTemperature ( const double vl ) {
   m_VLSRTemperature = vl;
}


double SimulatorState::getVLSRTemperature () const {
   return m_VLSRTemperature;
}


}

