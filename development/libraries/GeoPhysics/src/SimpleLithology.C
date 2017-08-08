//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "SimpleLithology.h"

#include <cmath>
#include <assert.h>
#include <stdlib.h>

#include "Interface/Interface.h"
#include "Interface/LithologyHeatCapacitySample.h"
#include "Interface/LithologyThermalConductivitySample.h"

#include "NumericFunctions.h"
#include "GeoPhysicalConstants.h"

#include <sstream>
#include <algorithm>
using std::stringstream;

using namespace DataAccess;
using namespace ibs;


GeoPhysics::SimpleLithology::SimpleLithology(Interface::ProjectHandle * projectHandle,
   database::Record *              record)
   : Interface::LithoType(projectHandle, record)
   , m_permeability(Permeability::create(
   Interface::LithoType::getPermeabilityModel(),
   this->getPermeabilityAnisotropy(),
   this->getSurfacePorosity(),
   this->getDepositionalPermeability(),
   this->getPermeabilityRecoveryCoefficient(),
   this->getPermeabilitySensitivityCoefficient(),
   this->getMultipointPorosityValues(),
   this->getMultipointPermeabilityValues(),
   this->getNumberOfMultipointSamplePoints()
   ))
   //construct porosity object
   , m_porosity(Porosity::create(Interface::LithoType::getPorosityModel(),
   this->getSurfacePorosity() / 100,
   Interface::LithoType::getMinimumMechanicalPorosity() / 100.0,
   1.0E-08 * this->getExponentialCompactionCoefficient(),
   1.0E-08 * this->getExponentialCompactionCoefficientA(),
   1.0E-08 * this->getExponentialCompactionCoefficientB(),
   0.1 * 1.0E-08 * this->getExponentialCompactionCoefficient(),
   0.1 * 1.0E-08 * this->getExponentialCompactionCoefficientA(),
   0.1 * 1.0E-08 * this->getExponentialCompactionCoefficientB(),
   this->getSoilMechanicsCompactionCoefficient(),
   Interface::LithoType::getLegacy()
   ))
{
   m_lithoname = Interface::LithoType::getName();
   m_density = Interface::LithoType::getDensity();
   m_heatproduction = Interface::LithoType::getHeatProduction();
   m_seismicvelocity = Interface::LithoType::getSeismicVelocity();
   m_nExponentVelocity = Interface::LithoType::getSeismicVelocityExponent();

   m_capC1 = Interface::LithoType::getCapillaryEntryPressureC1();
   m_capC2 = Interface::LithoType::getCapillaryEntryPressureC2();

   //for Brooks-Corey capillary pressure function

   m_PcKrModel = Interface::LithoType::getPcKrModel();
   m_LambdaPc = Interface::LithoType::getExponentLambdaPc();
   m_LambdaKr = Interface::LithoType::getExponentLambdaKr();

   m_thermalconductivityval = this->getThermalConductivity();
   m_thermalcondaniso = this->getThermalConductivityAnisotropy();

   m_thermalcondmodel = Interface::TABLE_MODEL;
   m_heatcapmodel = Interface::TABLE_MODEL;

   m_referenceSolidViscosity = this->getReferenceViscosity();
   m_lithologyActivationEnergy = this->getViscosityActivationEnergy();

   /// If value is a null value then DO NOT convert to cauldron units.
   if (m_lithologyActivationEnergy != Interface::DefaultUndefinedMapValue and m_lithologyActivationEnergy != Interface::DefaultUndefinedScalarValue) {
      m_lithologyActivationEnergy = 1000.0 * m_lithologyActivationEnergy;
   }

   m_lithologyFractureGradient = this->getHydraulicFracturingPercent();

   /// If value is a null value then DO NOT convert to cauldron units.
   if (m_lithologyFractureGradient != Interface::DefaultUndefinedMapValue and m_lithologyFractureGradient != Interface::DefaultUndefinedScalarValue) {
      m_lithologyFractureGradient = 0.01 * m_lithologyFractureGradient;
   }

   m_minimumMechanicalPorosity = Interface::LithoType::getMinimumMechanicalPorosity();

   /// If value is a null value then DO NOT convert to cauldron units.
   if (m_minimumMechanicalPorosity != Interface::DefaultUndefinedMapValue and m_minimumMechanicalPorosity != Interface::DefaultUndefinedScalarValue) {
      m_minimumMechanicalPorosity = 0.01 * m_minimumMechanicalPorosity;
   }

   loadPropertyTables();
}

GeoPhysics::SimpleLithology::SimpleLithology ( const SimpleLithology& litho ) :
   Interface::LithoType ( litho.getProjectHandle(), litho.getRecord ()),
   m_thermCondTbl ( litho.m_thermCondTbl ),
   m_density ( litho.m_density ),

   m_heatproduction ( litho.m_heatproduction ),
   m_seismicvelocity ( litho.m_seismicvelocity ),
   m_nExponentVelocity ( litho.m_nExponentVelocity ),

   m_thermalcondaniso ( litho.m_thermalcondaniso ),
   m_thermalconductivityval ( litho.m_thermalconductivityval ),
   m_capC1 ( litho.m_capC1 ),
   m_capC2 ( litho.m_capC2 ),

   m_PcKrModel ( litho.m_PcKrModel ),
   m_LambdaPc ( litho.m_LambdaPc ),
   m_LambdaKr ( litho.m_LambdaKr ),



   m_thermalconductivitytbl ( litho.m_thermalconductivitytbl ),
   m_heatcapacitytbl ( litho.m_heatcapacitytbl ),

   m_referenceSolidViscosity ( litho.m_referenceSolidViscosity ),
   m_lithologyActivationEnergy ( litho.m_lithologyActivationEnergy ),
   m_lithologyFractureGradient ( litho.m_lithologyFractureGradient ),
   m_minimumMechanicalPorosity ( litho.m_minimumMechanicalPorosity ),

   m_permeability ( Permeability::create( litho.getPermeabilityModel(),
                                          litho.getPermeabilityAnisotropy(),
                                          litho.getSurfacePorosity(),
                                          litho.getDepositionalPermeability(),
                                          litho.getPermeabilityRecoveryCoefficient(),
                                          litho.getPermeabilitySensitivityCoefficient(),
                                          litho.getMultipointPorosityValues(),
                                          litho.getMultipointPermeabilityValues(),
                                          litho.getNumberOfMultipointSamplePoints())),

   m_porosity ( Porosity::create ( litho.getPorosityModel ( ),
                                   litho.getSurfacePorosity ( ) / 100.0,
                                   litho.getMinimumMechanicalPorosity( ) / 100.0,
                                   1.0E-8 * litho.getExponentialCompactionCoefficient ( ),
                                   1.0E-8 * litho.getExponentialCompactionCoefficientA ( ),
                                   1.0E-8 * litho.getExponentialCompactionCoefficientB ( ),
                                   0.1 * 1.0E-8 * litho.getExponentialCompactionCoefficient ( ),
                                   0.1 * 1.0E-8 * litho.getExponentialCompactionCoefficientA ( ),
                                   0.1 * 1.0E-8 * litho.getExponentialCompactionCoefficientB ( ),
                                   litho.getSoilMechanicsCompactionCoefficient ( ),
                                   Interface::LithoType::getLegacy() ) ),

   m_lithoname ( litho.m_lithoname ),
   m_thermalcondmodel ( litho.m_thermalcondmodel ),
   m_heatcapmodel ( litho.m_heatcapmodel )

{
}


void GeoPhysics::SimpleLithology::loadPropertyTables() {

   Interface::LithologyHeatCapacitySampleList* heatCapacitySamples = getHeatCapacitySamples();
   Interface::LithologyThermalConductivitySampleList* thermalConductivitySamples = getThermalConductivitySamples();

   Interface::LithologyHeatCapacitySampleList::const_iterator heatCapacityIter;
   Interface::LithologyThermalConductivitySampleList::const_iterator thermalConductivityIter;

   for (heatCapacityIter = heatCapacitySamples->begin(); heatCapacityIter != heatCapacitySamples->end(); ++heatCapacityIter) {
      const Interface::LithologyHeatCapacitySample* sample = *heatCapacityIter;

      m_heatcapacitytbl.addPoint(sample->getTemperature(), sample->getHeatCapacity());
   }

   for (thermalConductivityIter = thermalConductivitySamples->begin(); thermalConductivityIter != thermalConductivitySamples->end(); ++thermalConductivityIter) {
      const Interface::LithologyThermalConductivitySample* sample = *thermalConductivityIter;

      ibs::XF xyPoint(sample->getTemperature(), sample->getThermalConductivity());

      m_thermCondTbl.push_back(xyPoint);
      m_thermalconductivitytbl.addPoint(sample->getTemperature(), sample->getThermalConductivity());
   }

   std::sort(m_thermCondTbl.begin(), m_thermCondTbl.end(), ibs::XYAscendingOrderFunctor());
   m_heatcapacitytbl.freeze();
   m_thermalconductivitytbl.freeze();

   delete heatCapacitySamples;
   delete thermalConductivitySamples;
}



void GeoPhysics::SimpleLithology::setPermeability(
   const string&   faultLithologyName,
   const double    permeabilityAnisotropy,
   const std::vector<double> & porositySamples, const std::vector<double> & permeabilitySamples)
{

   ///
   /// Now, define the new permeability
   ///
   m_permeability = Permeability::createMultiPoint(
      permeabilityAnisotropy,
      this->getDepositionalPermeability(),
      porositySamples,
      permeabilitySamples
      );

   ///
   /// We have a new name as well.
   ///
   m_lithoname = faultLithologyName;
}


GeoPhysics::SimpleLithology::~SimpleLithology() {

}

void GeoPhysics::SimpleLithology::correctThermCondPoint(const double correction) {

   XYContainer::iterator XYIter = m_thermCondTbl.begin();

   while (m_thermCondTbl.end() != XYIter) {
      (*XYIter).setF((*XYIter).getF() * correction);
      ++XYIter;
   }

}

void GeoPhysics::SimpleLithology::setName(const string & newName) {
   m_lithoname = newName;
}

const std::string& GeoPhysics::SimpleLithology::getName() const {
   return m_lithoname;
}


double GeoPhysics::SimpleLithology::thermalconductivity(const double t) const
{

   switch (m_thermalcondmodel) {

   case Interface::CONSTANT_MODEL: return m_thermalconductivityval;

   case Interface::TABLE_MODEL: return m_thermalconductivitytbl.compute(t);

   default:
      assert(0);

   }

   // Should not get here! It to stop the compiler from complaining.
   // It is legitimate to have this here because if all cases are covered in the switch statement.
   return 0;
}

double GeoPhysics::SimpleLithology::heatcapacity(const double t) const
{
   switch (m_heatcapmodel) {
   case Interface::TABLE_MODEL: return m_heatcapacitytbl.compute(t);
   default:    assert(0);
   }

   // Should not get here! It to stop the compiler from complaining.
   // It is legitimate to have this here because if all cases are covered in the switch statement.
   return 0;
}

void GeoPhysics::SimpleLithology::print() const {

#if 0
   cout << m_lithoname << " ";
   cout << m_thermalcondaniso << " ";
   cout << m_heatproduction << " ";
   cout << m_permeabilityaniso << " ";
   cout << m_density << " ";
   cout << m_porosity.getSurfacePorosity() << " ";
   cout << m_thermalconductivityval << " ";
   //if (isincompressible()){
   //  cout << "Incompressible";
   //}
   cout << endl;
#endif

   cout << image() << endl;

}

std::string GeoPhysics::SimpleLithology::image() const {

   stringstream buffer;

   buffer.precision(10);
   buffer.flags(ios::scientific);

   buffer << endl;
   buffer << endl;
   buffer << " m_lithoname              " << getName() << " " << endl;
   buffer << " m_thermalcondaniso       " << m_thermalcondaniso << " " << endl;
   buffer << " m_heatproduction         " << m_heatproduction << " " << endl;
   buffer << " m_density                " << m_density << " " << endl;
   buffer << " porosity                 " << m_porosity.getSurfacePorosity() << " " << endl;
   buffer << " m_compactionincr         " << m_porosity.getCompactionCoefficient() << " " << endl;
   buffer << " m_compactiondecr         " << 0.1 * m_porosity.getCompactionCoefficient() << " " << endl;
   buffer << " m_thermalconductivityval " << m_thermalconductivityval << " " << endl;
   buffer << " m_permeability           [does not have .image() method yet]" << endl;

   buffer << " permeability model       ";

   switch (m_permeability.getPermModel()) {
   case Interface::SANDSTONE_PERMEABILITY: buffer << "SANDSTONE_PERMEABILITY"; break;
   case Interface::MUDSTONE_PERMEABILITY: buffer << "SANDSTONE_PERMEABILITY"; break;
   case Interface::MULTIPOINT_PERMEABILITY: buffer << "MULTIPOINT_PERMEABILITY"; break;
   case Interface::IMPERMEABLE_PERMEABILITY: buffer << "IMPERMEABLE_PERMEABILITY"; break;
   case Interface::NONE_PERMEABILITY: buffer << "NONE_PERMEABILITY"; break;
   default: buffer << "UNDEFINED_PERMEABILITY";
   }

   switch (m_porosity.getPorosityModel()) {
   case DataAccess::Interface::EXPONENTIAL_POROSITY: buffer << "EXPONENTIAL_POROSITY"; break;
   case DataAccess::Interface::SOIL_MECHANICS_POROSITY: buffer << "SOIL_MECHANICS_POROSITY"; break;
   case DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY: buffer << "DOUBLE_EXPONENTIAL_POROSITY"; break;
   default: buffer << "UNDEFINED_POROSITY";
   }

   buffer << endl << endl;

   if (m_permeability.getPermModel() == Interface::MULTIPOINT_PERMEABILITY) {
      buffer << "Porosity-Permeability interpolator: [does not have .image() method yet]" << endl;
      buffer << endl << endl;
   }

   return buffer.str();

}


double GeoPhysics::SimpleLithology::referenceEffectiveStress() const {
   return 1.0e5;
}

double GeoPhysics::SimpleLithology::porosity(const double ves,
   const double maxVes,
   const bool   includeChemicalCompaction,
   const double chemicalCompaction) const {

   return m_porosity.calculate(ves, maxVes, includeChemicalCompaction, chemicalCompaction);

}

void GeoPhysics::SimpleLithology::porosity ( const unsigned int       size,
                                             ArrayDefs::ConstReal_ptr ves,
                                             ArrayDefs::ConstReal_ptr maxVes,
                                             const bool               includeChemicalCompaction,
                                             ArrayDefs::ConstReal_ptr chemicalCompactionTerm,
                                             ArrayDefs::Real_ptr      porosity ) const {
   m_porosity.calculate ( size, ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm, porosity );
}

//------------------------------------------------------------//

void GeoPhysics::SimpleLithology::setChemicalCompactionTerms
(const double rockViscosity,
const double activationEnergy) {

   m_referenceSolidViscosity = rockViscosity;
   m_lithologyActivationEnergy = activationEnergy;

}
//------------------------------------------------------------//

const std::string GeoPhysics::SimpleLithology::getThermalCondModelName() const {

   std::string model = "Undefined";
   if (m_thermalcondmodel == Interface::TABLE_MODEL) {
      model = "Legacy";
   }
   else if (m_thermalcondmodel == Interface::CONSTANT_MODEL) {
      model = "Constant";
   }
   else if (m_thermalcondmodel == Interface::STANDARD_MODEL) {
      model = "Standard Conductivity Model";
   }
   else if (m_thermalcondmodel == Interface::LOWCOND_MODEL) {
      model = "Low Conductivity Model";
   }
   else if (m_thermalcondmodel == Interface::HIGHCOND_MODEL) {
      model = "High Conductivity Model";
   }
   return model;
}
