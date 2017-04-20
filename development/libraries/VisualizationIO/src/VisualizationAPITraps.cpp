//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "VisualizationAPI.h"
#include <assert.h>
#include <new>
#include <algorithm>
#include <cstring>
#include <boost/foreach.hpp>

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4290)
#endif

using namespace CauldronIO;
using namespace std;

//////////////////////////////////////////////////////////////////////////
/// Trapper implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Trapper::Trapper() 
{
   std::memset((void*)this, 0, sizeof(Trapper));
   
   m_ID = -1;
   m_persistentID = -1;
   m_downstreamTrapperID = -1;
}

CauldronIO::Trapper::Trapper(int ID, int persistentID) 
{
   std::memset((void*)this, 0, sizeof(Trapper));
   
   m_ID = ID;
   m_persistentID = persistentID;
   m_downstreamTrapperID = -1;
}

void CauldronIO::Trapper::setPersistentId(int id)
{
   m_persistentID = id;
}

int CauldronIO::Trapper::getPersistentID() const
{
   return m_persistentID;
}

std::shared_ptr<const Trapper> CauldronIO::Trapper::getDownStreamTrapper() const
{
   return m_downstreamTrapper;
}

void CauldronIO::Trapper::setDownStreamTrapper(std::shared_ptr<const Trapper> trapper)
{
   m_downstreamTrapper = trapper;
}

void CauldronIO::Trapper::setDownStreamTrapperID(int persistentID)
{
   m_downstreamTrapperID = persistentID;
}

int CauldronIO::Trapper::getDownStreamTrapperID() const
{
   return m_downstreamTrapperID;
}

void CauldronIO::Trapper::setSolutionGasVolume(float solutionGasVolume) 
{
   m_solutionGasVolume = solutionGasVolume;
}

float CauldronIO::Trapper::getSolutionGasVolume() const 
{
   return m_solutionGasVolume;
}

void CauldronIO::Trapper::setSolutionGasDensity(float solutionGasDensity) 
{
   m_solutionGasDensity = solutionGasDensity;
}

float CauldronIO::Trapper::getSolutionGasDensity() const
{
   return m_solutionGasDensity;
}

void CauldronIO::Trapper::setSolutionGasViscosity(float solutionGasViscosity) 
{
   m_solutionGasViscosity = solutionGasViscosity;
}

float CauldronIO::Trapper::getSolutionGasViscosity() const 
{
   return m_solutionGasViscosity;
}

void CauldronIO::Trapper::setSolutionGasMass(double solutionGasMass, CauldronIO::SpeciesNamesId compId)
{
   m_solutionGasMasses[compId] = solutionGasMass;
}

double CauldronIO::Trapper::getSolutionGasMass(CauldronIO::SpeciesNamesId compId) const
{
   return m_solutionGasMasses[compId];
}

void CauldronIO::Trapper::setSolutionGasMass(float solutionGasMass)
{
   m_solutionGasMass = solutionGasMass;
}

float CauldronIO::Trapper::getSolutionGasMass() const
{
   return m_solutionGasMass;
}

void CauldronIO::Trapper::setFreeGasVolume(float freeGasVolume) 
{
   m_freeGasVolume = freeGasVolume;
}

float CauldronIO::Trapper::getFreeGasVolume() const 
{
   return m_freeGasVolume;
}

void CauldronIO::Trapper::setFreeGasDensity(float freeGasDensity) 
{
   m_freeGasDensity = freeGasDensity;
}

float CauldronIO::Trapper::getFreeGasDensity() const
{
   return m_freeGasDensity;
}

void CauldronIO::Trapper::setFreeGasViscosity(float freeGasViscosity) 
{
   m_freeGasViscosity = freeGasViscosity;
}

float CauldronIO::Trapper::getFreeGasViscosity() const 
{
   return m_freeGasViscosity;
}

void CauldronIO::Trapper::setFreeGasMass(double freeGasMass, CauldronIO::SpeciesNamesId compId)
{
   m_freeGasMasses[compId] = freeGasMass;
}

double CauldronIO::Trapper::getFreeGasMass(CauldronIO::SpeciesNamesId compId) const
{
   return m_freeGasMasses[compId];
}

void CauldronIO::Trapper::setFreeGasMass(float freeGasMass)
{
   m_freeGasMass = freeGasMass;
}

float CauldronIO::Trapper::getFreeGasMass() const
{
   return m_freeGasMass;
}

void CauldronIO::Trapper::setCondensateVolume(float condensateVolume) 
{
   m_condensateVolume = condensateVolume;
}

float CauldronIO::Trapper::getCondensateVolume() const 
{
   return m_condensateVolume;
}

void CauldronIO::Trapper::setCondensateDensity(float condensateDensity) 
{
   m_condensateDensity = condensateDensity;
}

float CauldronIO::Trapper::getCondensateDensity() const
{
   return m_condensateDensity;
}

void CauldronIO::Trapper::setCondensateViscosity(float condensateViscosity) 
{
   m_condensateViscosity = condensateViscosity;
}

float CauldronIO::Trapper::getCondensateViscosity() const 
{
   return m_condensateViscosity;
}

void CauldronIO::Trapper::setCondensateMass(double condensateMass, CauldronIO::SpeciesNamesId compId)
{
   m_condensateMasses[compId] = condensateMass;
}

double CauldronIO::Trapper::getCondensateMass(CauldronIO::SpeciesNamesId compId) const
{
   return m_condensateMasses[compId];
}

void CauldronIO::Trapper::setCondensateMass(float condensateMass)
{
   m_condensateMass = condensateMass;
}

float CauldronIO::Trapper::getCondensateMass() const
{
   return m_condensateMass;
}

void CauldronIO::Trapper::setStockTankOilVolume(float stockTankOilVolume) 
{
   m_stockTankOilVolume = stockTankOilVolume;
}

float CauldronIO::Trapper::getStockTankOilVolume() const 
{
   return m_stockTankOilVolume;
}

void CauldronIO::Trapper::setStockTankOilDensity(float stockTankOilDensity) 
{
   m_stockTankOilDensity = stockTankOilDensity;
}

float CauldronIO::Trapper::getStockTankOilDensity() const
{
   return m_stockTankOilDensity;
}

void CauldronIO::Trapper::setStockTankOilViscosity(float stockTankOilViscosity) 
{
   m_stockTankOilViscosity = stockTankOilViscosity;
}

float CauldronIO::Trapper::getStockTankOilViscosity() const 
{
   return m_stockTankOilViscosity;
}

void CauldronIO::Trapper::setStockTankOilMass(double stockTankOilMass, CauldronIO::SpeciesNamesId compId)
{
    m_stockTankOilMasses[compId] = stockTankOilMass;
}

double CauldronIO::Trapper::getStockTankOilMass(CauldronIO::SpeciesNamesId compId) const
{
   return m_stockTankOilMasses[compId];
}

void CauldronIO::Trapper::setStockTankOilMass(float stockTankOilMass)
{
   m_stockTankOilMass = stockTankOilMass;
}

float CauldronIO::Trapper::getStockTankOilMass() const
{
   return m_stockTankOilMass;
}

void CauldronIO::Trapper::setMassLiquid(float massLiquid) {
   m_massLiquid = massLiquid;
}

void CauldronIO::Trapper::setMassVapour(float massVapour) {
   m_massVapour = massVapour;
}

void CauldronIO::Trapper::setViscosityLiquid(float viscosityLiquid) {
   m_viscosityLiquid = viscosityLiquid;
}

void CauldronIO::Trapper:: setViscosityVapour(float viscosityVapour) {
   m_viscosityVapour = viscosityVapour;
}

void CauldronIO::Trapper::setDensityLiquid(float densityLiquid) {
   m_densityLiquid = densityLiquid;
}

void CauldronIO::Trapper::setDensityVapour(float densityVapour) {
   m_densityVapour = densityVapour;
}

void CauldronIO::Trapper::setOilAPI(float oilAPI) {
   m_oilAPI = oilAPI;
}

void CauldronIO::Trapper::setCGR(float cgr) {
   m_cgr = cgr;
}

void CauldronIO::Trapper::setGOR(float gor) {
   m_gor = gor;
}

void CauldronIO::Trapper::setBuoyancy(float buoyancy) {
   m_buoyancy = buoyancy;
}

void CauldronIO::Trapper::setPorosity(float porosity) {
   m_porosity = porosity;
}

float CauldronIO::Trapper::getMassLiquid() const {
   return m_massLiquid; 
}

float CauldronIO::Trapper::getMassVapour() const {
   return m_massVapour;
}

float CauldronIO::Trapper::getViscosityLiquid() const {
   return m_viscosityLiquid;
}

float CauldronIO::Trapper:: getViscosityVapour() const {
   return m_viscosityVapour;
}

float CauldronIO::Trapper::getDensityLiquid() const {
   return m_densityLiquid; 
}

float CauldronIO::Trapper::getDensityVapour() const {
   return m_densityVapour; 
}

float CauldronIO::Trapper::getOilAPI() const {
   return m_oilAPI; 
}

float CauldronIO::Trapper::getCGR() const {
   return m_cgr;
}

float CauldronIO::Trapper::getGOR() const {
   return m_gor;
}


float CauldronIO::Trapper::getBuoyancy() const {
   return m_buoyancy; 
}


float CauldronIO::Trapper::getPorosity() const {
   return m_porosity; 
}

void CauldronIO::Trapper::setID(int id)
{
   m_ID = id;
}

int CauldronIO::Trapper::getID() const
{
   return m_ID;
}

const std::string CauldronIO::Trapper::getReservoirName() const
{
   return std::string(m_reservoir);
}

void CauldronIO::Trapper::setReservoirName(const std::string& reservoirName)
{
   std::strcpy(m_reservoir, reservoirName.c_str());
}

float CauldronIO::Trapper::getSpillDepth() const
{
   return m_spillDepth;
}

void CauldronIO::Trapper::setSpillDepth(float depth)
{
   m_spillDepth = depth;
}
void CauldronIO::Trapper::getSpillPointPosition(float& posX, float& posY) const
{
   posX = m_spillPositionX;
   posY = m_spillPositionY;
}

void CauldronIO::Trapper::setSpillPointPosition(float posX, float posY)
{
   m_spillPositionX = posX;
   m_spillPositionY = posY;
}

float CauldronIO::Trapper::getDepth() const
{
   return m_depth;
}

void CauldronIO::Trapper::setDepth(float depth)
{
   m_depth = depth;
}

void CauldronIO::Trapper::getPosition(float& posX, float& posY) const
{
   posX = m_positionX;
   posY = m_positionY;
}

void CauldronIO::Trapper::setPosition(float posX, float posY)
{
   m_positionX = posX;
   m_positionY = posY;
}

void CauldronIO::Trapper::setGOC(float goc)
{
   m_goc = goc;
}

float CauldronIO::Trapper::getGOC() const
{
   return m_goc;
}

void CauldronIO::Trapper::setOWC(float woc)
{
   m_owc = woc;
}

float CauldronIO::Trapper::getOWC() const
{
   return m_owc;
}

void CauldronIO::Trapper::setVolumeOil(float volumeLiquid) {
   m_volumeLiquid = volumeLiquid;
}

void CauldronIO::Trapper::setVolumeGas(float volumeVapour) {
   m_volumeVapour = volumeVapour;
}

void CauldronIO::Trapper::setCEPOil(float cepLiquid) {
   m_cepLiquid = cepLiquid;
}

void CauldronIO::Trapper::setCEPGas(float cepVapour) {
   m_cepVapour = cepVapour;
}

void CauldronIO::Trapper::setCriticalTemperatureOil(float criticalTemperatureLiquid) {
   m_criticalTemperatureLiquid = criticalTemperatureLiquid;
}

void CauldronIO::Trapper::setCriticalTemperatureGas(float criticalTemperatureVapour) {
   m_criticalTemperatureVapour = criticalTemperatureVapour;
}

void CauldronIO::Trapper::setInterfacialTensionOil(float interfacialTensionLiquid) {
   m_interfacialTensionLiquid = interfacialTensionLiquid;
}

void CauldronIO::Trapper::setInterfacialTensionGas(float interfacialTensionVapour) {
   m_interfacialTensionVapour = interfacialTensionVapour;
}

void CauldronIO::Trapper::setFracturePressure(float fracturePressure) {
   m_fracturePressure = fracturePressure;
}

void CauldronIO::Trapper::setPressure(float pressure) {
   m_pressure = pressure;
}

void CauldronIO::Trapper::setTemperature(float temperature) {
   m_temperature = temperature;
}

void CauldronIO::Trapper::setPermeability(float permeability) {
   m_permeability = permeability;
}

void CauldronIO::Trapper::setSealPermeability(float sealPermeability) {
   m_sealPermeability = sealPermeability;
}

void CauldronIO::Trapper::setNetToGross(float netToGross) {
   m_netToGross = netToGross;
}

void CauldronIO::Trapper::setAge(float age) {
   m_age = age;
}

void CauldronIO::Trapper::setWCSurface(float wcSurface) {
   m_wcSurface = wcSurface;
}

void CauldronIO::Trapper::setTrapCapacity(float trapCapacity) {
   m_trapCapacity = trapCapacity;
}

float CauldronIO::Trapper::getVolumeOil() const {
   return m_volumeLiquid;
}

float CauldronIO::Trapper::getVolumeGas() const {
   return m_volumeVapour;
}

float CauldronIO::Trapper::getCEPOil() const {
   return m_cepLiquid;
}

float CauldronIO::Trapper::getCEPGas() const {
   return m_cepVapour;
}

float CauldronIO::Trapper::getCriticalTemperatureOil() const {
   return m_criticalTemperatureLiquid;
}

float CauldronIO::Trapper::getCriticalTemperatureGas() const {
   return m_criticalTemperatureVapour;
}

float CauldronIO::Trapper::getInterfacialTensionOil() const {
   return m_interfacialTensionLiquid;
}

float CauldronIO::Trapper::getInterfacialTensionGas() const {
   return m_interfacialTensionVapour;
}

float CauldronIO::Trapper::getFracturePressure() const {
   return m_fracturePressure;
}

float CauldronIO::Trapper::getWCSurface() const {
   return m_wcSurface;
}

float CauldronIO::Trapper::getTrapCapacity() const {
   return m_trapCapacity;
}

float CauldronIO::Trapper::getPressure() const {
   return m_pressure;
}

float CauldronIO::Trapper::getTemperature() const {
   return m_temperature;
}

float CauldronIO::Trapper::getPermeability() const {
   return m_permeability;
}

float CauldronIO::Trapper::getSealPermeability() const {
   return m_sealPermeability;
}

float CauldronIO::Trapper::getNetToGross() const {
   return m_netToGross;
}

float CauldronIO::Trapper::getAge() const {
   return m_age;
}

//////////////////////////////////////////////////////////////////////////
/// Trap implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Trap::Trap()
{
   std::memset((void*)this, 0, sizeof(Trap));
   
   m_ID = -1;
}

CauldronIO::Trap::Trap(int ID) 
{
   std::memset((void*)this, 0, sizeof(Trap));
   
   m_ID = ID;
}

void CauldronIO::Trap::setMass(double mass, CauldronIO::SpeciesNamesId compId)
{
   m_masses[compId] = mass;
}

double CauldronIO::Trap::getMass(CauldronIO::SpeciesNamesId compId) const
{
   return m_masses[compId];
}

void CauldronIO::Trap::setFractSealStrength(float fractSealStrength) {
   m_fractSealStrength = fractSealStrength;
}

float CauldronIO::Trap::getFractSealStrength() const {
   return m_fractSealStrength;
}

void CauldronIO::Trap::setID(int id)
{
   m_ID = id;
}

const std::string CauldronIO::Trap::getReservoirName() const
{
   return std::string( m_reservoir );
}

int CauldronIO::Trap::getID() const
{
   return m_ID;
}

void CauldronIO::Trap::setReservoirName(const std::string& reservoirName)
{
   std::strcpy(m_reservoir, reservoirName.c_str());
}

float CauldronIO::Trap::getSpillDepth() const
{
   return m_spillDepth;
}

void CauldronIO::Trap::setSpillDepth(float depth)
{
   m_spillDepth = depth;
}

void CauldronIO::Trap::getSpillPointPosition(float& posX, float& posY) const
{
   posX = m_spillPositionX;
   posY = m_spillPositionY;
}

void CauldronIO::Trap::setSpillPointPosition(float posX, float posY)
{
   m_spillPositionX = posX;
   m_spillPositionY = posY;
}

float CauldronIO::Trap::getDepth() const
{
   return m_depth;
}

void CauldronIO::Trap::setDepth(float depth)
{
   m_depth = depth;
}

void CauldronIO::Trap::getPosition(float& posX, float& posY) const
{
   posX = m_positionX;
   posY = m_positionY;
}

void CauldronIO::Trap::setPosition(float posX, float posY)
{
   m_positionX = posX;
   m_positionY = posY;
}

void CauldronIO::Trap::setGOC(float goc)
{
   m_goc = goc;
}

float CauldronIO::Trap::getGOC() const
{
   return m_goc;
}

void CauldronIO::Trap::setOWC(float woc)
{
   m_owc = woc;
}

float CauldronIO::Trap::getOWC() const
{
   return m_owc;
}

void CauldronIO::Trap::setVolumeOil(float volumeLiquid) {
   m_volumeLiquid = volumeLiquid;
}

void CauldronIO::Trap::setVolumeGas(float volumeVapour) {
   m_volumeVapour = volumeVapour;
}

void CauldronIO::Trap::setCEPOil(float cepLiquid) {
   m_cepLiquid = cepLiquid;
}

void CauldronIO::Trap::setCEPGas(float cepVapour) {
   m_cepVapour = cepVapour;
}

void CauldronIO::Trap::setCriticalTemperatureOil(float criticalTemperatureLiquid) {
   m_criticalTemperatureLiquid = criticalTemperatureLiquid;
}

void CauldronIO::Trap::setCriticalTemperatureGas(float criticalTemperatureVapour) {
   m_criticalTemperatureVapour = criticalTemperatureVapour;
}

void CauldronIO::Trap::setInterfacialTensionOil(float interfacialTensionLiquid) {
   m_interfacialTensionLiquid = interfacialTensionLiquid;
}

void CauldronIO::Trap::setInterfacialTensionGas(float interfacialTensionVapour) {
   m_interfacialTensionVapour = interfacialTensionVapour;
}

void CauldronIO::Trap::setFracturePressure(float fracturePressure) {
   m_fracturePressure = fracturePressure;
}

void CauldronIO::Trap::setPressure(float pressure) {
   m_pressure = pressure;
}

void CauldronIO::Trap::setTemperature(float temperature) {
   m_temperature = temperature;
}

void CauldronIO::Trap::setPermeability(float permeability) {
   m_permeability = permeability;
}

void CauldronIO::Trap::setSealPermeability(float sealPermeability) {
   m_sealPermeability = sealPermeability;
}

void CauldronIO::Trap::setNetToGross(float netToGross) {
   m_netToGross = netToGross;
}

void CauldronIO::Trap::setAge(float age) {
   m_age = age;
}

void CauldronIO::Trap::setWCSurface(float wcSurface) {
   m_wcSurface = wcSurface;
}

void CauldronIO::Trap::setTrapCapacity(float trapCapacity) {
	m_trapCapacity = trapCapacity;
}

float CauldronIO::Trap::getVolumeOil() const {
   return m_volumeLiquid; 
}

float CauldronIO::Trap::getVolumeGas() const {
   return m_volumeVapour; 
}

float CauldronIO::Trap::getCEPOil() const {
   return m_cepLiquid;
}

float CauldronIO::Trap::getCEPGas() const {
   return m_cepVapour;
}

float CauldronIO::Trap::getCriticalTemperatureOil() const {
   return m_criticalTemperatureLiquid;
}

float CauldronIO::Trap::getCriticalTemperatureGas() const {
   return m_criticalTemperatureVapour;
}

float CauldronIO::Trap::getInterfacialTensionOil() const {
   return m_interfacialTensionLiquid; 
}

float CauldronIO::Trap::getInterfacialTensionGas() const {
   return m_interfacialTensionVapour; 
}

float CauldronIO::Trap::getFracturePressure() const {
   return m_fracturePressure;
}

float CauldronIO::Trap::getWCSurface() const {
   return m_wcSurface;
}

float CauldronIO::Trap::getTrapCapacity() const {
   return m_trapCapacity;
}

float CauldronIO::Trap::getPressure() const {
   return m_pressure;
}

float CauldronIO::Trap::getTemperature() const {
   return m_temperature;
}

float CauldronIO::Trap::getPermeability() const {
   return m_permeability;
}

float CauldronIO::Trap::getSealPermeability() const {
   return m_sealPermeability;
}

float CauldronIO::Trap::getNetToGross() const {
   return m_netToGross;
}

float CauldronIO::Trap::getAge() const {
   return m_age;
}


#ifdef _MSC_VER
#pragma warning (pop)
#endif

