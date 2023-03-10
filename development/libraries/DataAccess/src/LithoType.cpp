//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <assert.h>

#include <iostream>
#include <sstream>
using namespace std;

#include "database.h"
#include "cauldronschemafuncs.h"
#include "LogHandler.h"

using namespace database;

#include "LithoType.h"
#include "LithologyHeatCapacitySample.h"
#include "LithologyThermalConductivitySample.h"
#include "ProjectHandle.h"
#include "RunParameters.h"

using namespace DataAccess;
using namespace Interface;

LithoType::LithoType (ProjectHandle& projectHandle, Record * record) : DAObject (projectHandle, record)
{

   //for Pc - Kr models
   const std::string& pcKrModelStr = database::getPcKrModel(m_record);
    if ( pcKrModelStr == "Brooks_Corey" ) {
      m_pcKrModel = BROOKS_COREY;
   } else {
      std::cout << "Basin_Error:  Error in Pc-Kr model " << pcKrModelStr << endl;
      // Error
   }

   const std::string& porosityModelStr     = database::getPorosity_Model ( m_record );
   const std::string& permeabilityModelStr = database::getPermMixModel ( m_record );

   if ( porosityModelStr == "Exponential" ) {
      m_porosityModel = EXPONENTIAL_POROSITY;
      if (getSurfacePorosity() == 0 && getExponentialCompactionCoefficient() != 0)
      {
          LogHandler(LogHandler::WARNING_SEVERITY) << "Lithotype " << getName() <<
              " has a null surface porosity and a non-null compaction coefficient.";
      }
   } else if ( porosityModelStr == "Soil_Mechanics" ) {
      m_porosityModel = SOIL_MECHANICS_POROSITY;
   }
   else if (porosityModelStr == "Double_Exponential") {
       m_porosityModel = DOUBLE_EXPONENTIAL_POROSITY;
       if (getSurfacePorosity() == 0 && (getExponentialCompactionCoefficientA() != 0 || getExponentialCompactionCoefficientB() != 0))
       {
           LogHandler(LogHandler::WARNING_SEVERITY) << "Lithotype " << getName() <<
               " has a null surface porosity and at least one non-null compaction coefficient.";
       }
   } else {
      std::cout << "Basin_Error: Error in porosity model " << porosityModelStr << ". Using Exponential porosity model as the default value." << endl;
      // Error
      m_porosityModel = EXPONENTIAL_POROSITY;
   }

   if ( permeabilityModelStr == "Sands" ) {
      m_permeabilityModel = SANDSTONE_PERMEABILITY;
   } else if ( permeabilityModelStr == "Shales" ) {
      m_permeabilityModel = MUDSTONE_PERMEABILITY;
   } else if ( permeabilityModelStr == "Impermeable" ) {
      m_permeabilityModel = IMPERMEABLE_PERMEABILITY;
   } else if ( permeabilityModelStr == "None" ) {
      m_permeabilityModel = NONE_PERMEABILITY;
   } else if ( permeabilityModelStr == "Multipoint" ) {
      m_permeabilityModel = MULTIPOINT_PERMEABILITY;
   } else {
      std::cout << "Basin_Error: Error in permeability model " << permeabilityModelStr << ". Using None as the default value." << endl;
      // Error
      m_permeabilityModel = NONE_PERMEABILITY;
   }

}

LithoType::~LithoType (void)
{
}

LithoType::LithoType(const LithoType& litho) :
  DAObject( litho )
{
  m_porosityModel = litho.m_porosityModel;
  m_permeabilityModel = litho.m_permeabilityModel;
  m_pcKrModel = litho.m_pcKrModel;
}

// the names herein must have the same order as LithoType::AttributeId.
const string LithoType::s_attributeNames[] =
{
   // Strings
   "Lithotype", "Description", "DefinedBy", "DefinitionDate", "LastChangedBy", "LastChangedDate",
   "FgColour", "BgColour", "Pixmap", "HeatProdSource", "PermMixModel", "Porosity_Model",
   "Multipoint_Porosity", "Multipoint_Permeability",
   // Doubles
   "Density", "HeatProd", "SurfacePorosity", "CompacCoefES", "CompacCoefESA", "CompacCoefESB", "StpThCond",
   "ThCondAnisotropy", "DepoPerm", "PermDecrStressCoef", "PermIncrRelaxCoef", "PermAnisotropy",
   "SeisVelocity", "NExponentVelocity", "CapC1", "CapC2", "Compaction_Coefficient_SM", "HydraulicFracturingPercent",
   "ReferenceSolidViscosity", "ActivationEnergy", "MinimumPorosity", "CompacRatioES",
   // Ints
   "UserDefined", "Number_Of_Data_Points"
};



const string & LithoType::getName (void) const
{
   return database::getLithotype (m_record);
}

const AttributeValue & LithoType::getLithoTypeAttributeValue (LithoTypeAttributeId attributeId) const
{
   return getAttributeValue (s_attributeNames[attributeId], 0);
}

double LithoType::getHydraulicFracturingPercent () const {
   return database::getHydraulicFracturingPercent ( m_record );
}

double LithoType::getReferenceViscosity () const {
   return database::getReferenceSolidViscosity ( m_record );
}

double LithoType::getViscosityActivationEnergy () const {
   return database::getActivationEnergy ( m_record );
}

double LithoType::getMinimumMechanicalPorosity () const {
   return database::getMinimumPorosity ( m_record );
}

double LithoType::getIgneousIntrusionTemperature () const {
   return database::getIgneousIntrusionTemperature ( m_record );
}

double LithoType::getDensity () const {
   return database::getDensity ( m_record );
}

double LithoType::getHeatProduction () const {
   return database::getHeatProd ( m_record );
}

double LithoType::getThermalConductivity () const {
   return database::getStpThCond ( m_record );
}

double LithoType::getThermalConductivityAnisotropy () const {
   return database::getThCondAnisotropy ( m_record );
}

double LithoType::getSurfacePorosity () const {
   return database::getSurfacePorosity ( m_record );
}

PorosityModel LithoType::getPorosityModel () const {
   return m_porosityModel;
}

const std::string& LithoType::getMultipointPorosityValues () const {
   return database::getMultipoint_Porosity ( m_record );
}

double LithoType::getExponentialCompactionCoefficient () const {
   return database::getCompacCoefES ( m_record );
}

double LithoType::getExponentialCompactionCoefficientA () const {
   return database::getCompacCoefESA ( m_record );
}

double LithoType::getExponentialCompactionCoefficientB () const {
   return database::getCompacCoefESB ( m_record );
}

double LithoType::getExponentialCompactionRatio() const {
   return database::getCompacRatioES ( m_record );
}

double LithoType::getSoilMechanicsCompactionCoefficient () const {
   return database::getCompaction_Coefficient_SM ( m_record );
}

double LithoType::getQuartzGrainSize () const {
   return database::getQuartzGrainSize( m_record );
}

double LithoType::getQuartzFraction () const {
   return database::getQuartzFraction( m_record );
}

double LithoType::getClayCoatingFactor () const {
   return database::getClayCoatingFactor( m_record );
}

PermeabilityModel LithoType::getPermeabilityModel () const {
   return m_permeabilityModel;
}

int LithoType::getNumberOfMultipointSamplePoints () const {
   return database::getNumber_Of_Data_Points ( m_record );
}

double LithoType::getDepositionalPermeability () const {
   return database::getDepoPerm ( m_record );
}

const std::string& LithoType::getMultipointPermeabilityValues () const {
   return database::getMultipoint_Permeability ( m_record );
}

double LithoType::getPermeabilityAnisotropy () const {
   return database::getPermAnisotropy ( m_record );
}

double LithoType::getPermeabilitySensitivityCoefficient () const {
   return database::getPermDecrStressCoef ( m_record );
}

double LithoType::getPermeabilityRecoveryCoefficient () const {
   return database::getPermIncrRelaxCoef ( m_record );
}

double LithoType::getSeismicVelocity () const {
   return database::getSeisVelocity ( m_record );
}

double LithoType::getSeismicVelocityExponent() const {
  return database::getSeisVelocityExponent( m_record );
}

double LithoType::getCapillaryEntryPressureC1 () const {
   return database::getCapC1 ( m_record );
}

double LithoType::getCapillaryEntryPressureC2 () const {
   return database::getCapC2 ( m_record );
}

//for Brooks-Corey function

PcKrModel  LithoType::getPcKrModel () const
{
   //return database::getPcKrModel ( m_record );
   return m_pcKrModel;
}

double  LithoType::getExponentLambdaPc () const
{
   return database::getLambdaPc ( m_record );
}

double  LithoType::getExponentLambdaKr () const
{
   return database::getLambdaKr ( m_record );
}

LithologyHeatCapacitySampleList * LithoType::getHeatCapacitySamples () const {
   return getProjectHandle().getLithologyHeatCapacitySampleList ( this );
}

LithologyThermalConductivitySampleList * LithoType::getThermalConductivitySamples () const {
   return getProjectHandle().getLithologyThermalConductivitySampleList ( this );
}

const std::string& LithoType::getForegroundColour () const {
   return database::getFgColour ( m_record );
}

const std::string& LithoType::getBackgroundColour () const {
   return database::getBgColour ( m_record );
}

const std::string& LithoType::getPixmap () const {
   return database::getPixmap ( m_record );
}

bool LithoType::getLegacy() const {
   return getProjectHandle().getRunParameters()->getLegacy( );
}

void LithoType::printOn (ostream & ostr) const
{
   string str;
   asString (str);
   ostr << str << endl;
}

void LithoType::asString (string & str) const
{
   std::ostringstream buf;

   buf << "LithoType:";
   buf << " name = " << getName ();
   buf << endl;

   str = buf.str ();

}
