//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Genex0dSimulator.h"

#include "CommonDefinitions.h"
#include "Genex0dInputData.h"
#include "Genex0dSourceRock.h"

// cmbAPI
#include "ErrorHandler.h"

// database
#include "cauldronschemafuncs.h"
#include "ProjectFileHandler.h"
#include "LangmuirAdsorptionIsothermSample.h"
#include "IrreducibleWaterSaturationSample.h"
//
#include "SimulationDetails.h"

// CBMGenerics
#include "ComponentManager.h"
#include "GenexResultManager.h"

// utilities
#include "LogHandler.h"

#include <sstream>

namespace Genex0d
{

Genex0dSimulator::Genex0dSimulator(database::ProjectFileHandlerPtr database,
                                   const std::string & name,
                                   const DataAccess::Interface::ObjectFactory* objectFactory)
try :
  GeoPhysics::ProjectHandle{database, name, objectFactory},
  m_gnx0dSourceRock{nullptr},
  m_registeredProperties{},
  m_shaleProperties{},
  m_requestedProperties{},
  m_expelledToCarrierBedProperties{},
  m_expelledToSourceRockProperties{},
  m_expelledToCarrierBedPropertiesS{},
  m_expelledToSourceRockPropertiesS{}
{
}
catch (const ErrorHandler::Exception & ex)
{
  throw ex;
}

Genex0dSimulator::~Genex0dSimulator()
{
}

Genex0dSimulator* Genex0dSimulator::CreateFrom(const std::string & fileName, DataAccess::Interface::ObjectFactory* objectFactory)
{
  return dynamic_cast<Genex0dSimulator*>(DataAccess::Interface::OpenCauldronProject (fileName, objectFactory));
}

bool Genex0dSimulator::run(const DataAccess::Interface::Formation* formation, const Genex0dInputData& inData, unsigned int indI, unsigned int indJ,
                           double thickness, double inorganicDensity, const std::vector<double>& time,
                           const std::vector<double>& temperature, const std::vector<double>& pressure, const std::vector<double>& VRE,
                           const std::vector<double>& porePressure, const std::vector<double>& permeability, const std::vector<double>& porosity)
{
  registerProperties();

  setRequestedOutputProperties();
  m_gnx0dSourceRock.reset(new Genex0dSourceRock(*this, inData, indI, indJ));
  if (m_gnx0dSourceRock == nullptr)
  {
    return false;
  }

  m_gnx0dSourceRock->initializeInputs(thickness, inorganicDensity, time, temperature, pressure, VRE, porePressure, permeability, porosity);

  if (!computeSourceRock(formation))
  {
    return false;
  }

  LogHandler(LogHandler::INFO_SEVERITY) << "Saving Genex0d results to disk ... \n";
  return true;
}

bool Genex0dSimulator::saveTo(const std::string & outputFileName)
{
  return saveToFile(outputFileName);
}

void Genex0dSimulator::setLangmuirData(const std::string& adsorptionFunctionTPVData, const std::string& langmuirName)
{
  deleteLangmuirIsotherms();
  m_langmuirIsotherms.clear();
  std::stringstream stream(adsorptionFunctionTPVData);

  std::string temperature;
  std::string pressure;
  std::string volume;
  while (stream >> temperature >> pressure >> volume)
  {
    m_langmuirIsotherms.push_back(new DataAccess::Interface::LangmuirAdsorptionIsothermSample(*this, langmuirName, std::stod(temperature), std::stod(pressure), std::stod(volume)));
  }
}

void Genex0d::Genex0dSimulator::setIrreducibleWaterSaturationData(const std::string &irreducibleWaterSaturationData)
{
  deleteIrreducibleWaterSaturationSample();

  if (irreducibleWaterSaturationData != "")
  {
    std::stringstream stream(irreducibleWaterSaturationData);
    std::string coeffA;
    std::string coeffB;

    stream >> coeffA >> coeffB;

    m_irreducibleWaterSample = new DataAccess::Interface::IrreducibleWaterSaturationSample(*this, std::stod(coeffA), std::stod(coeffB));
  }
}

bool Genex0dSimulator::computeSourceRock(const DataAccess::Interface::Formation * aFormation)
{
  m_gnx0dSourceRock->setFormationData(aFormation);
  return m_gnx0dSourceRock->compute();
}

void Genex0dSimulator::setRequestedOutputProperties()
{
  setRequestedSpeciesOutputProperties();
  CBMGenerics::GenexResultManager & theResultManager = CBMGenerics::GenexResultManager::getInstance();

  database::Table * timeIoTbl = getTable("FilterTimeIoTbl");
  database::Table::iterator tblIter;

  for (tblIter = timeIoTbl->begin(); tblIter != timeIoTbl->end(); ++tblIter)
  {
    database::Record * filterTimeIoRecord = *tblIter;
    const std::string & outPutOption = database::getOutputOption(filterTimeIoRecord);
    const std::string & propertyName = database::getPropertyName (filterTimeIoRecord);

    if (!isPropertyRegistered(propertyName))
    {
      continue;
    }

    if (outPutOption != "None")
    {
      if (theResultManager.getResultId(propertyName) != -1)
      {
        m_requestedProperties.push_back(propertyName);
        theResultManager.SetResultToggleByName(propertyName, true);
      }
      else if (propertyName == "SourceRockEndMember1" || propertyName == "SourceRockEndMember2" or propertyName == "TOC")
      {
        m_requestedProperties.push_back(propertyName);
      }
    }
    else if ((propertyName == theResultManager.GetResultName(CBMGenerics::GenexResultManager::HcGasExpelledCum)) ||
             (propertyName == theResultManager.GetResultName(CBMGenerics::GenexResultManager::OilExpelledCum)))
    {
      m_requestedProperties.push_back(propertyName);
      theResultManager.SetResultToggleByName(propertyName, true);
    }
    else if (theResultManager.getResultId(propertyName) != -1)
    { // calculate (not output) all if output history
      theResultManager.SetResultToggleByName(propertyName, true);
    }
  }
}

void Genex0dSimulator::setRequestedSpeciesOutputProperties()
{
  CBMGenerics::ComponentManager & theManager = CBMGenerics::ComponentManager::getInstance();

  for (int i = 0; i < CBMGenerics::ComponentManager::NUMBER_OF_SPECIES; ++i)
  {
    if(!theManager.isSulphurComponent(i))
    {
      m_expelledToCarrierBedProperties.push_back(theManager.getSpeciesOutputPropertyName(i, false));
      m_expelledToSourceRockProperties.push_back(theManager.getSpeciesOutputPropertyName(i, true));
    }
    m_expelledToCarrierBedPropertiesS.push_back(theManager.getSpeciesOutputPropertyName(i, false));
    m_expelledToSourceRockPropertiesS.push_back(theManager.getSpeciesOutputPropertyName(i, true));
  }
}

void Genex0dSimulator::registerProperties()
{
  CBMGenerics::ComponentManager & theManager = CBMGenerics::ComponentManager::getInstance();
  CBMGenerics::GenexResultManager & theResultManager = CBMGenerics::GenexResultManager::getInstance();

  for (int i = 0; i < CBMGenerics::ComponentManager::NUMBER_OF_SPECIES; ++i)
  {
    m_registeredProperties.insert(theManager.getSpeciesOutputPropertyName(i, false));
  }

  for (int i = 0; i < CBMGenerics::ComponentManager::NUMBER_OF_SPECIES; ++i)
  {
    m_registeredProperties.insert(theManager.getSpeciesOutputPropertyName(i, true));
  }

  for (int i = 0; i < CBMGenerics::GenexResultManager::NumberOfResults; ++i)
  {
    m_registeredProperties.insert(theResultManager.GetResultName(i));
  }

  // Adding all possible species that can be used in the adsorption process.
  for (int i = 0; i < CBMGenerics::ComponentManager::NUMBER_OF_SPECIES; ++i)
  {
    m_registeredProperties.insert(theManager.getSpeciesName(i) + "Retained");
    m_registeredProperties.insert(theManager.getSpeciesName(i) + "Adsorped");
    m_registeredProperties.insert(theManager.getSpeciesName(i) + "AdsorpedExpelled");
    m_registeredProperties.insert(theManager.getSpeciesName(i) + "AdsorpedFree");
  }

  m_registeredProperties.insert("HcSaturation");
  m_registeredProperties.insert("ImmobileWaterSat");
  m_registeredProperties.insert("FractionOfAdsorptionCap");
  m_registeredProperties.insert("HcVapourSat");
  m_registeredProperties.insert("HcLiquidSat");
  m_registeredProperties.insert("AdsorptionCapacity");
  m_registeredProperties.insert("RetainedGasVolumeST");
  m_registeredProperties.insert("RetainedOilVolumeST");
  m_registeredProperties.insert("GasExpansionRatio_Bg");
  m_registeredProperties.insert("Oil2GasGeneratedCumulative");
  m_registeredProperties.insert("TotalGasGeneratedCumulative");
  m_registeredProperties.insert("RetainedOilApiSR");
  m_registeredProperties.insert("RetainedCondensateApiSR");
  m_registeredProperties.insert("RetainedGorSR");
  m_registeredProperties.insert("RetainedCgrSR");
  m_registeredProperties.insert("OverChargeFactor");
  m_registeredProperties.insert("PorosityLossFromPyroBitumen");
  m_registeredProperties.insert("H2SRisk");

  m_registeredProperties.insert("SourceRockEndMember1");
  m_registeredProperties.insert("SourceRockEndMember2");

  m_registeredProperties.insert("TOC");
}

bool Genex0dSimulator::isPropertyRegistered(const std::string & propertyName)
{
  std::unordered_set<std::string>::const_iterator it = m_registeredProperties.find(propertyName);
  return it != m_registeredProperties.end();
}

} // namespace genex0d
