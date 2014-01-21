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


GeoPhysics::SimpleLithology::SimpleLithology ( Interface::ProjectHandle * projectHandle, 
                                               database::Record *              record ) 
   : Interface::LithoType ( projectHandle, record ) 
   , m_permeability( Permeability::create(
         Interface::LithoType::getPermeabilityModel (),
         this->getPermeabilityAnisotropy (),
         this->getSurfacePorosity () ,
         this->getDepositionalPermeability (),
         this->getPermeabilityRecoveryCoefficient (),
         this->getPermeabilitySensitivityCoefficient (),
         this->getMultipointPorosityValues (),
         this->getMultipointPermeabilityValues (),
         this->getNumberOfMultipointSamplePoints ()
      ) )
{
   m_lithoname = Interface::LithoType::getName ();
   m_density = Interface::LithoType::getDensity ();
   m_heatproduction = Interface::LithoType::getHeatProduction ();
   m_seismicvelocity = Interface::LithoType::getSeismicVelocity ();

   m_specificSurfaceArea = Interface::LithoType::getSpecificSurfArea() * 1000; // convert to m2/kg
   m_geometricVariance = Interface::LithoType::getGeometricVariance();

   m_capC1 = Interface::LithoType::getCapillaryEntryPressureC1();
   m_capC2 = Interface::LithoType::getCapillaryEntryPressureC2();


   //for Brooks-Corey capillary pressure function
   
   m_PcKrModel= Interface::LithoType::getPcKrModel();

   m_depoporosity = this->getSurfacePorosity () / 100.0;
   m_depositionVoidRatio = m_depoporosity / ( 1.0 - m_depoporosity );

   m_compactionincr = 1.0E-08 * this->getExponentialCompactionCoefficient ();
   m_compactiondecr = 0.1 * m_compactionincr;
   m_thermalconductivityval = this->getThermalConductivity ();
   m_thermalcondaniso = this->getThermalConductivityAnisotropy ();

   m_soilMechanicsCompactionCoefficient = this->getSoilMechanicsCompactionCoefficient ();
   m_thermalcondmodel = Interface::TABLE_MODEL;
   m_heatcapmodel = Interface::TABLE_MODEL;

   m_referenceSolidViscosity   = this->getReferenceViscosity ();
   m_lithologyActivationEnergy = this->getViscosityActivationEnergy ();

   /// If value is a null value then DO NOT convert to cauldron units.
   if ( m_lithologyActivationEnergy != Interface::DefaultUndefinedMapValue and m_lithologyActivationEnergy != Interface::DefaultUndefinedScalarValue ) {
      m_lithologyActivationEnergy = 1000.0 * m_lithologyActivationEnergy;
   }

   m_lithologyFractureGradient = this->getHydraulicFracturingPercent ();

   /// If value is a null value then DO NOT convert to cauldron units.
   if ( m_lithologyFractureGradient != Interface::DefaultUndefinedMapValue and m_lithologyFractureGradient != Interface::DefaultUndefinedScalarValue ) {
      m_lithologyFractureGradient = 0.01 * m_lithologyFractureGradient;
   }

   m_minimumMechanicalPorosity = Interface::LithoType::getMinimumMechanicalPorosity ();

   /// If value is a null value then DO NOT convert to cauldron units.
   if ( m_minimumMechanicalPorosity != Interface::DefaultUndefinedMapValue and m_minimumMechanicalPorosity != Interface::DefaultUndefinedScalarValue ) {
      m_minimumMechanicalPorosity = 0.01 * m_minimumMechanicalPorosity;
   }

   loadPropertyTables ();
}


void GeoPhysics::SimpleLithology::loadPropertyTables () {

   Interface::LithologyHeatCapacitySampleList* heatCapacitySamples = getHeatCapacitySamples ();
   Interface::LithologyThermalConductivitySampleList* thermalConductivitySamples = getThermalConductivitySamples ();

   Interface::LithologyHeatCapacitySampleList::const_iterator heatCapacityIter;
   Interface::LithologyThermalConductivitySampleList::const_iterator thermalConductivityIter;

   for ( heatCapacityIter = heatCapacitySamples->begin (); heatCapacityIter != heatCapacitySamples->end (); ++heatCapacityIter ) {
      const Interface::LithologyHeatCapacitySample* sample = *heatCapacityIter;

      m_heatcapacitytbl.addPoint( sample->getTemperature (), sample->getHeatCapacity ());
   }

   for ( thermalConductivityIter = thermalConductivitySamples->begin (); thermalConductivityIter != thermalConductivitySamples->end (); ++thermalConductivityIter ) {
      const Interface::LithologyThermalConductivitySample* sample = *thermalConductivityIter;

      ibs::XF xyPoint( sample->getTemperature (), sample->getThermalConductivity ());

      m_thermCondTbl.push_back(xyPoint);
      m_thermalconductivitytbl.addPoint( sample->getTemperature (), sample->getThermalConductivity ());
   }

   std::sort ( m_thermCondTbl.begin (), m_thermCondTbl.end (), ibs::XYAscendingOrderFunctor ());
   m_heatcapacitytbl.freeze ();
   m_thermalconductivitytbl.freeze ();

   delete heatCapacitySamples;
   delete thermalConductivitySamples;
}



void GeoPhysics::SimpleLithology::setPermeability(
      const string&   faultLithologyName,
      const double    permeabilityAnisotropy,
      const std::vector<double> & porositySamples, const std::vector<double> & permeabilitySamples )
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

void GeoPhysics::SimpleLithology::correctThermCondPoint ( const double correction ) {

   XYContainer::iterator XYIter = m_thermCondTbl.begin();

   while (m_thermCondTbl.end() != XYIter) {
      (*XYIter).setF( (*XYIter).getF () * correction );
      ++XYIter;
   }

} 

void GeoPhysics::SimpleLithology::setName(const string & newName) {
   m_lithoname = newName;
}

const std::string& GeoPhysics::SimpleLithology::getName () const {
   return m_lithoname;
}


double GeoPhysics::SimpleLithology::thermalconductivity ( const double t ) const
{

   switch (m_thermalcondmodel) {

     case Interface::CONSTANT_MODEL : return m_thermalconductivityval;

     case Interface::TABLE_MODEL    : return m_thermalconductivitytbl.compute(t);

     default:
        assert (0);

   }

   // Should not get here! It to stop the compiler from complaining.
   // It is legitimate to have this here because if all cases are covered in the switch statement.
   return 0;
}

double GeoPhysics::SimpleLithology::heatcapacity ( const double t ) const
{
   switch (m_heatcapmodel) {
     case Interface::TABLE_MODEL : return m_heatcapacitytbl.compute(t);
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
  cout << m_depoporosity << " ";
  cout << m_compactionincr << " ";
  cout << m_compactiondecr << " ";
  cout << m_thermalconductivityval << " ";
  //if (isincompressible()){
  //  cout << "Incompressible";
  //}
  cout << endl;
#endif

  cout << image () << endl;

}

std::string GeoPhysics::SimpleLithology::image () const {

   stringstream buffer;

   buffer.precision ( 10 );
   buffer.flags ( ios::scientific );

   buffer << endl;
   buffer << endl;
   buffer << " m_lithoname              " << getName () << " " << endl;
   buffer << " m_thermalcondaniso       " << m_thermalcondaniso << " " << endl;
   buffer << " m_heatproduction         " << m_heatproduction << " " << endl;
   buffer << " m_density                " << m_density << " " << endl;
   buffer << " m_depoporosity           " << m_depoporosity << " " << endl;
   buffer << " m_compactionincr         " << m_compactionincr << " " << endl;
   buffer << " m_compactiondecr         " << m_compactiondecr << " " << endl;
   buffer << " m_thermalconductivityval " << m_thermalconductivityval << " " << endl;
   buffer << " m_permeability           [does not have .image() method yet]" << endl;
   buffer << " is incompressible        " << ( isIncompressible () ? "TRUE" : "FALSE") << endl;

   buffer << " permeability model       ";

   switch ( m_permeability.getPermModel() ) {
     case Interface::SANDSTONE_PERMEABILITY   : buffer << "SANDSTONE_PERMEABILITY"; break;
     case Interface::MUDSTONE_PERMEABILITY    : buffer << "SANDSTONE_PERMEABILITY"; break;
     case Interface::MULTIPOINT_PERMEABILITY  : buffer << "MULTIPOINT_PERMEABILITY"; break;
     case Interface::IMPERMEABLE_PERMEABILITY : buffer << "IMPERMEABLE_PERMEABILITY"; break;
     case Interface::NONE_PERMEABILITY        : buffer << "NONE_PERMEABILITY"; break;
     default : buffer << "UNDEFINED_PERMEABILITY";
   }

   buffer << endl << endl;

   if ( m_permeability.getPermModel() == Interface::MULTIPOINT_PERMEABILITY ) {   
      buffer << "Porosity-Permeability interpolator: [does not have .image() method yet]" << endl;
      buffer << endl << endl;
   }

   return buffer.str ();
   
}

//------------------------------------------------------------//


double GeoPhysics::SimpleLithology::exponentialPorosity ( const double ves,
                                                          const double maxVes,
                                                          const bool   includeChemicalCompaction ) const {


   double calculatedPorosity;
   bool   loadingPhase = ( ves >= maxVes );

   if ( includeChemicalCompaction ) {

      if ( loadingPhase ) {
         calculatedPorosity = ( m_depoporosity - m_minimumMechanicalPorosity ) * exp( -m_compactionincr * maxVes ) + m_minimumMechanicalPorosity;
      } else {
         calculatedPorosity = ( m_depoporosity - m_minimumMechanicalPorosity ) * exp( m_compactiondecr * ( maxVes - ves ) - m_compactionincr * maxVes ) + m_minimumMechanicalPorosity;
      }

   } else {

      if ( loadingPhase ) {
         calculatedPorosity = m_depoporosity * exp( -m_compactionincr * maxVes );
      } else {
         calculatedPorosity = m_depoporosity * exp( m_compactiondecr * ( maxVes - ves ) - m_compactionincr * maxVes );
      }

   }

   return calculatedPorosity;
}


//------------------------------------------------------------//

double GeoPhysics::SimpleLithology::referenceEffectiveStress () const {
   return 1.0e5;
}

//------------------------------------------------------------//


double GeoPhysics::SimpleLithology::soilMechanicsPorosity ( const double ves,
                                                            const double maxVes,
                                                            const bool   includeChemicalCompaction ) const {


   const bool loadingPhase = ( ves >= maxVes );

   const double ves0       = referenceEffectiveStress ();

   /// Depositional void-ratio.
   const double Epsilon100 = m_depositionVoidRatio;


   double calculatedPorosity;
   double voidRatio;
   double M, C;
   double phiMaxVes;
   double phiMinVes;
   double vesUsed;

   const double PercentagePorosityRebound = 0.02; // => %age porosity regain

   if ( loadingPhase ) {
      vesUsed = NumericFunctions::Maximum ( ves, maxVes );
      vesUsed = NumericFunctions::Maximum ( vesUsed, ves0 );

      voidRatio = Epsilon100 - m_soilMechanicsCompactionCoefficient * log ( vesUsed / ves0 );

      calculatedPorosity = voidRatio / ( 1.0 + voidRatio );
   } else {

      voidRatio = Epsilon100 - m_soilMechanicsCompactionCoefficient * log ( NumericFunctions::Maximum ( ves0, maxVes ) / ves0 ); 
      phiMaxVes = voidRatio / ( 1.0 + voidRatio );
      voidRatio = Epsilon100;
      phiMinVes = voidRatio / ( 1.0 + voidRatio );

      M = PercentagePorosityRebound * ( phiMaxVes - phiMinVes ) / ( maxVes - ves0 );

      C = (( 1.0 - PercentagePorosityRebound ) * phiMaxVes * maxVes - 
           phiMaxVes * ves0 + PercentagePorosityRebound * phiMinVes * maxVes ) / 
         ( maxVes - ves0 );

      calculatedPorosity = M * ves + C;
   } 

   // Force porosity to be in range 0.03 .. Surface_Porosity

   if ( includeChemicalCompaction ) {
      calculatedPorosity = NumericFunctions::Maximum ( calculatedPorosity, m_minimumMechanicalPorosity );
   }

   calculatedPorosity = NumericFunctions::Maximum ( calculatedPorosity, MinimumSoilMechanicsPorosity );
   calculatedPorosity = NumericFunctions::Minimum ( calculatedPorosity, m_depoporosity );

   return calculatedPorosity;
}


//------------------------------------------------------------//


double GeoPhysics::SimpleLithology::porosity ( const double ves,
                                               const double maxVes,
                                               const bool   includeChemicalCompaction,
                                               const double chemicalCompaction ) const {

  double calculatedPorosity;

  if ( getPorosityModel () == Interface::EXPONENTIAL_POROSITY ) {
    calculatedPorosity = exponentialPorosity ( ves, maxVes, includeChemicalCompaction );
  } else {
    calculatedPorosity = soilMechanicsPorosity ( ves, maxVes, includeChemicalCompaction );
  }

#if 0
  if ( m_Porosity_Model == Interface::EXPONENTIAL_POROSITY ) {
    calculatedPorosity = exponentialPorosity ( ves, maxVes, includeChemicalCompaction );
  } else {
    calculatedPorosity = soilMechanicsPorosity ( ves, maxVes, includeChemicalCompaction );
  }
#endif

  if ( includeChemicalCompaction ) {
    calculatedPorosity = calculatedPorosity + chemicalCompaction;
    calculatedPorosity = NumericFunctions::Maximum ( calculatedPorosity, GeoPhysics::MinimumPorosity );
  }

  return calculatedPorosity;
}


//------------------------------------------------------------//

void GeoPhysics::SimpleLithology::setChemicalCompactionTerms
   ( const double rockViscosity,
     const double activationEnergy ) {

  m_referenceSolidViscosity   = rockViscosity;
  m_lithologyActivationEnergy = activationEnergy;

}
//------------------------------------------------------------//

const std::string GeoPhysics::SimpleLithology::getThermalCondModelName() const {

  std::string model = "Undefined";
  if ( m_thermalcondmodel == Interface::TABLE_MODEL ) {
    model =  "Legacy";
  } else if ( m_thermalcondmodel == Interface::CONSTANT_MODEL ) {
    model = "Constant";
  } else if ( m_thermalcondmodel == Interface::STANDARD_MODEL ) {
    model = "Standard Conductivity Model";
  } else if ( m_thermalcondmodel == Interface::LOWCOND_MODEL ) {
    model = "Low Conductivity Model";
  } else if ( m_thermalcondmodel == Interface::HIGHCOND_MODEL ) {
    model = "High Conductivity Model";
  } 
  return model;
}

