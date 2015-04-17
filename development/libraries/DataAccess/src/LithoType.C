#include <assert.h>
#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
      #include <sstream>
      using namespace std;
      #define USESTANDARD
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
      #include<strstream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   #include <sstream>
   using namespace std;
   #define USESTANDARD
#endif // sgi

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Interface/LithoType.h"
#include "Interface/LithologyHeatCapacitySample.h"
#include "Interface/LithologyThermalConductivitySample.h"
#include "Interface/ProjectHandle.h"

using namespace DataAccess;
using namespace Interface;

LithoType::LithoType (ProjectHandle * projectHandle, Record * record) : DAObject (projectHandle, record)
{

   //for Pc - Kr models
   const std::string& pcKrModelStr = database::getPcKrModel(m_record);
    if ( pcKrModelStr == "Brooks_Corey" ) {
      m_pcKrModel = BROOKS_COREY;
   } else {
      std::cout << " Error in Pc-Kr model " << pcKrModelStr << endl;
      // Error
   }

   const std::string& porosityModelStr     = database::getPorosity_Model ( m_record );
   const std::string& permeabilityModelStr = database::getPermMixModel ( m_record );

   if ( porosityModelStr == "Exponential" ) {
      m_porosityModel = EXPONENTIAL_POROSITY;
   } else if ( porosityModelStr == "Soil_Mechanics" ) {
      m_porosityModel = SOIL_MECHANICS_POROSITY;
   } else if ( porosityModelStr == "Double_Exponential") {
     m_porosityModel = DOUBLE_EXPONENTIAL_POROSITY;
   } else {
      std::cout << " Error in porosity model " << porosityModelStr << ". Using Exponential porosity model as the default value." << endl;
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
      std::cout << " Error in permeability model " << permeabilityModelStr << ". Using None as the default value." << endl;
      // Error
      m_permeabilityModel = NONE_PERMEABILITY;
   }

}

LithoType::~LithoType (void)
{
}

// the names herein must have the same order as LithoType::AttributeId.
const string LithoType::s_attributeNames[] =
{
   // Strings
   "Lithotype", "Description", "DefinedBy", "DefinitionDate", "LastChangedBy", "LastChangedDate",
   "FgColour", "BgColour", "Pixmap", "HeatProdSource", "PermMixModel", "Porosity_Model",
   "Multipoint_Porosity", "Multipoint_Permeability",
   // Doubles
   "Density", "HeatProd", "SurfacePorosity", "CompacCoefES", "CompacCoefESA","CompacCoefESB", "CompacCoefSC", "CompacCoefFM", "StpThCond",
   "ThCondAnisotropy", "DepoPerm", "PermDecrStressCoef", "PermIncrRelaxCoef", "PermAnisotropy",
   "SeisVelocity", "CapC1", "CapC2", "Compaction_Coefficient_SM", "HydraulicFracturingPercent",
   "ReferenceSolidViscosity", "ActivationEnergy", "MinimumPorosity", "SpecSurfArea", "GeoVariance",
   // Ints
   "UserDefined", "Number_Of_Data_Points"
};



const string & LithoType::getName (void) const
{
   return database::getLithotype (m_record);
}

const AttributeValue & LithoType::getAttributeValue (LithoTypeAttributeId attributeId) const
{
   return DAObject::getAttributeValue (s_attributeNames[attributeId], 0);
}

AttributeType LithoType::getAttributeType (LithoTypeAttributeId attributeId) const
{
   return DAObject::getAttributeType (s_attributeNames[attributeId]);
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

double LithoType::getSoilMechanicsCompactionCoefficient () const {
   return database::getCompaction_Coefficient_SM ( m_record );
}

double LithoType::getCompacCoefSC () const {
   return database::getCompacCoefSC ( m_record );
}

double LithoType::getCompacCoefFM () const {
   return database::getCompacCoefFM ( m_record );
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

double LithoType::getSpecificSurfArea () const {
   return database::getSpecSurfArea ( m_record );
}

double LithoType::getGeometricVariance () const {
   return database::getGeoVariance ( m_record );
}

LithologyHeatCapacitySampleList * LithoType::getHeatCapacitySamples () const {
   return m_projectHandle->getLithologyHeatCapacitySampleList ( this );
}

LithologyThermalConductivitySampleList * LithoType::getThermalConductivitySamples () const {
   return m_projectHandle->getLithologyThermalConductivitySampleList ( this );
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

void LithoType::printOn (ostream & ostr) const
{
   string str;
   asString (str);
   ostr << str << endl;
}

void LithoType::asString (string & str) const
{
#ifdef USESTANDARD
   ostringstream buf;
#else
   strstream buf;
#endif

   buf << "LithoType:";
   buf << " name = " << getName ();
   buf << endl;

   str = buf.str ();
#ifndef USESTANDARD
   buf.rdbuf ()->freeze (0);
#endif
}
