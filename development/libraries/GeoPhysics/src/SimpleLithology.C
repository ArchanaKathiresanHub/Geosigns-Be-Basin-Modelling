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


const double GeoPhysics::SimpleLithology::Log10 = std::log ( 10.0 );


GeoPhysics::SimpleLithology::SimpleLithology ( Interface::ProjectHandle * projectHandle, 
                                               database::Record *              record ) : Interface::LithoType ( projectHandle, record ) {

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
   m_LambdaPc= Interface::LithoType::getExponentLambdaPc();
   m_LambdaKr= Interface::LithoType::getExponentLambdaKr();

   m_depoporosity = this->getSurfacePorosity () / 100.0;
   m_depositionVoidRatio = m_depoporosity / ( 1.0 - m_depoporosity );

   m_compactionincr = 1.0E-08 * this->getExponentialCompactionCoefficient ();
   m_compactiondecr = 0.1 * m_compactionincr;
   m_thermalconductivityval = this->getThermalConductivity ();
   m_thermalcondaniso = this->getThermalConductivityAnisotropy ();
   m_depopermeability = this->getDepositionalPermeability ();
   m_permeabilityincr = this->getPermeabilityRecoveryCoefficient ();
   m_permeabilitydecr = this->getPermeabilitySensitivityCoefficient ();
   m_permeabilityaniso = this->getPermeabilityAnisotropy ();

   m_permeabilityModel = Interface::LithoType::getPermeabilityModel ();

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

   if ( m_permeabilityModel == Interface::MULTIPOINT_PERMEABILITY ) {
      double* multipointPorosityValues;
      double* multipointPermeabilityValues;

      int i;

      multipointPorosityValues = new double [ this->getNumberOfMultipointSamplePoints ()];
      multipointPermeabilityValues = new double [ this->getNumberOfMultipointSamplePoints ()];

      getCoefficientsFromString ( this->getMultipointPorosityValues ().c_str (),
                                  this->getNumberOfMultipointSamplePoints (),
                                  multipointPorosityValues );

      getCoefficientsFromString ( this->getMultipointPermeabilityValues ().c_str (),
                                  this->getNumberOfMultipointSamplePoints (),
                                  multipointPermeabilityValues );

      for ( i = 0; i < this->getNumberOfMultipointSamplePoints (); ++i ) {
         multipointPorosityValues [ i ] /= 100.0;
      }

      m_porosityPermeabilityInterpolant.setInterpolation ( PiecewiseInterpolator::PIECEWISE_LINEAR,
                                                           this->getNumberOfMultipointSamplePoints (),
                                                           multipointPorosityValues,
                                                           multipointPermeabilityValues );

      m_porosityPermeabilityInterpolant.computeCoefficients ();

      // not realy necessary, because the m_depopermeability 
      // is not used if permeability model is a multi-point.
      m_depopermeability = m_porosityPermeabilityInterpolant.evaluate ( m_depoporosity );

      delete [] multipointPorosityValues;
      delete [] multipointPermeabilityValues;

   }

   if (( m_permeabilityModel == Interface::IMPERMEABLE_PERMEABILITY or m_permeabilityModel == Interface::NONE_PERMEABILITY ) and m_permeabilityaniso == 0.0 ) {
      m_permeabilityaniso = 1.0;
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



GeoPhysics::SimpleLithology::SimpleLithology ( const SimpleLithology*       definedLithology,
                                               const string&                faultLithologyName,
                                               const double                 permeabilityAnisotropy,
                                               const PiecewiseInterpolator& newPermeabilities ) : 
   Interface::LithoType ( definedLithology->getProjectHandle (), definedLithology->getRecord ()) {

  ///
  /// Copy all components that are not affected by the permeability.
  ///
  m_thermCondTbl = definedLithology->m_thermCondTbl;
  m_thermalconductivitytbl = definedLithology->m_thermalconductivitytbl;
  m_heatcapacitytbl = definedLithology->m_heatcapacitytbl;

  m_density = definedLithology->m_density;
  m_heatproduction = definedLithology->m_heatproduction;
  m_depoporosity = definedLithology->m_depoporosity;

  m_PcKrModel = definedLithology->getPcKrModel ();
  m_LambdaPc  = definedLithology->getLambdaPc ();
  m_LambdaKr  = definedLithology->getLambdaKr ();

  m_depositionVoidRatio = m_depoporosity / ( 1.0 - m_depoporosity );
  m_compactionincr = definedLithology->m_compactionincr;
  m_compactiondecr = definedLithology->m_compactiondecr;
  m_thermalconductivityval = definedLithology->m_thermalconductivityval;
  m_thermalcondaniso = definedLithology->m_thermalcondaniso;
  m_seismicvelocity = definedLithology->m_seismicvelocity;
  m_soilMechanicsCompactionCoefficient = definedLithology->m_soilMechanicsCompactionCoefficient;
  m_thermalcondmodel = Interface::TABLE_MODEL;
  m_heatcapmodel = Interface::TABLE_MODEL;
  m_specificSurfaceArea = definedLithology->getSpecificSurfArea();
  m_geometricVariance = definedLithology->getGeometricVariance();
  m_capC1 = definedLithology->getCapC1();
  m_capC2 = definedLithology->getCapC2();

  m_referenceSolidViscosity   = definedLithology->m_referenceSolidViscosity;
  m_lithologyActivationEnergy = definedLithology->m_lithologyActivationEnergy;
  m_lithologyFractureGradient = definedLithology->m_lithologyFractureGradient;
  m_minimumMechanicalPorosity = definedLithology->m_minimumMechanicalPorosity;

  ///
  /// The following two are not strictly necessary, as they will never be used.
  ///
  m_permeabilityincr = definedLithology->m_permeabilityincr;
  m_permeabilitydecr = definedLithology->m_permeabilitydecr;

  ///
  /// Now, define the new permeability
  ///
  m_permeabilityaniso = permeabilityAnisotropy;

  m_permeabilityModel = Interface::MULTIPOINT_PERMEABILITY;
  m_porosityPermeabilityInterpolant = newPermeabilities;

  ///
  /// Again, this is not strictly neccessary but is added for completness.
  ///
  m_depopermeability = m_porosityPermeabilityInterpolant.evaluate ( m_depoporosity );

  ///
  /// We have a new name as well.
  ///
  m_lithoname = faultLithologyName;

}


GeoPhysics::SimpleLithology::SimpleLithology ( const SimpleLithology* definedLithology,
                                               const string&          newName )  : 
   Interface::LithoType ( definedLithology->getProjectHandle (), definedLithology->getRecord ()) {

  ///
  /// Copy all components that are not affected by the permeability.
  ///
  m_thermCondTbl = definedLithology->m_thermCondTbl;
  m_thermalconductivitytbl = definedLithology->m_thermalconductivitytbl;
  m_heatcapacitytbl = definedLithology->m_heatcapacitytbl;

  m_density = definedLithology->m_density;
  m_heatproduction = definedLithology->m_heatproduction;
  m_depoporosity = definedLithology->m_depoporosity;

  m_PcKrModel = definedLithology->getPcKrModel ();
  m_LambdaPc  = definedLithology->getLambdaPc ();
  m_LambdaKr  = definedLithology->getLambdaKr ();

  m_depositionVoidRatio = m_depoporosity / ( 1.0 - m_depoporosity );
  m_compactionincr = definedLithology->m_compactionincr;
  m_compactiondecr = definedLithology->m_compactiondecr;
  m_thermalconductivityval = definedLithology->m_thermalconductivityval;
  m_thermalcondaniso = definedLithology->m_thermalcondaniso;
  m_seismicvelocity = definedLithology->m_seismicvelocity;
  m_soilMechanicsCompactionCoefficient = definedLithology->m_soilMechanicsCompactionCoefficient;
  m_thermalcondmodel = Interface::TABLE_MODEL;
  m_heatcapmodel = Interface::TABLE_MODEL;
  m_specificSurfaceArea = definedLithology->getSpecificSurfArea();
  m_geometricVariance = definedLithology->getGeometricVariance();
  m_capC1 = definedLithology->getCapC1();
  m_capC2 = definedLithology->getCapC2();

  m_referenceSolidViscosity   = definedLithology->m_referenceSolidViscosity;
  m_lithologyActivationEnergy = definedLithology->m_lithologyActivationEnergy;
  m_lithologyFractureGradient = definedLithology->m_lithologyFractureGradient;
  m_minimumMechanicalPorosity = definedLithology->m_minimumMechanicalPorosity;

  ///
  /// The following two are not strictly necessary, as they will never be used.
  ///
  m_permeabilityincr  = definedLithology->m_permeabilityincr;
  m_permeabilitydecr  = definedLithology->m_permeabilitydecr;
  m_permeabilityaniso = definedLithology->m_permeabilityaniso;
  m_permeabilityModel = definedLithology->m_permeabilityModel;
  m_depopermeability  = definedLithology->m_depopermeability;

  if ( m_permeabilityModel == Interface::MULTIPOINT_PERMEABILITY ) {
    m_porosityPermeabilityInterpolant = definedLithology->m_porosityPermeabilityInterpolant;
  }

  m_lithoname = newName;
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

const std::string& GeoPhysics::SimpleLithology::getName () const {
   return m_lithoname;
}


double GeoPhysics::SimpleLithology::permeability ( const double ves,
                                                   const double maxVes, 
                                                   const double calculatedPorosity ) const {
  double val = 0.0;

  switch (m_permeabilityModel) {

    case Interface::SANDSTONE_PERMEABILITY : {

      double deltaphi = calculatedPorosity - m_depoporosity;
      double m = 0.12 + 0.02 * m_permeabilityincr;
      val = m_depopermeability * pow(10.0, m * deltaphi * 100.0);

#if 0
      val = (m_depopermeability*1000.0)*pow(10.0, m * deltaphi * 100.0);
#endif

      if (val >= 1000.0) val = 1000.0;

      return val;

    }
    case Interface::MUDSTONE_PERMEABILITY : {

      double ves0 = 1.0E+05;
      double cut_off = 0.0;
      
      if ( ves > cut_off) {
	val = shalepermeability (ves, maxVes, ves0);
      } else {
	double a = shalepermeabilityder (cut_off, maxVes, ves0);
	double b = shalepermeability (cut_off, maxVes, ves0);
	val = a*(ves-cut_off)+b;
      }

      if (val >= 1000.0) val = 1000.0;

      return val;

    }
    case Interface::MULTIPOINT_PERMEABILITY : {

      val = exp ( Log10 * m_porosityPermeabilityInterpolant.evaluate ( calculatedPorosity ));

      return NumericFunctions::Minimum ( val, 1000.0 );
    }
    case Interface::IMPERMEABLE_PERMEABILITY: {
      // lithology has a non-zero porosity but a zero permeability.
      val = 1.0E-09;
      return val;
    }
    case Interface::NONE_PERMEABILITY: {
      // lithology has a zero porosity and thus no permeability!
      val = 1.0E-09;
      return val;
    }
    default: {
       assert ( 0 );
    }
  }
  return 0;
}


void GeoPhysics::SimpleLithology::permeabilityDerivative ( const double  ves,
                                                           const double  maxVes, 
                                                           const double  calculatedPorosity, 
                                                                 double& Permeability, 
                                                                 double& Derivative ) const {

  Permeability = this->permeability ( ves, maxVes, calculatedPorosity );

  switch (m_permeabilityModel) {

    case Interface::SANDSTONE_PERMEABILITY : {

      double perm;
      double deltaphi = calculatedPorosity - m_depoporosity;
      double m = 0.12 + 0.02 * m_permeabilityincr;

      perm =  m_depopermeability * pow ( 10.0, m * deltaphi * 100.0);

#if 0
      perm = (m_depopermeability*1000.0)*pow(10.0, m * deltaphi * 100.0);
#endif

      Derivative = Log10 * m * perm;
      break;
    } 

    case Interface::MUDSTONE_PERMEABILITY : {

      double ves0 = 1.0E+05;
      double maxVesUsed = maxVes;

      if ( maxVes < ves0 ) {
         maxVesUsed = ves0;
      }

      Derivative = shalepermeabilityder (ves, maxVesUsed, ves0);
      break;
    }

    case Interface::MULTIPOINT_PERMEABILITY : {

      Derivative = Log10 * m_porosityPermeabilityInterpolant.evaluateDerivative ( calculatedPorosity ) * Permeability;
      break;
    }

    case Interface::IMPERMEABLE_PERMEABILITY: {
      Derivative = 0.0;
      break;
    }

    case Interface::NONE_PERMEABILITY: {
      Derivative = 0.0;
      break;
    }

    default: {
      assert ( 0 );
    }
  }

}

double GeoPhysics::SimpleLithology::shalepermeability ( const double ves,
                                                        const double maxVes, 
                                                        const double ves0 ) const {
  double val;

  assert ( 0 != ves0 );

  if (ves >= maxVes) {
    val = m_depopermeability * pow((ves+ves0)/ves0, -m_permeabilityincr);
  } else {
    assert ( maxVes != -ves0 );
    assert ( 0 != maxVes );

    val = m_depopermeability * pow ((maxVes+ves0)/ ves0, -m_permeabilityincr) *
          pow ((ves+ves0)/(maxVes+ves0), -m_permeabilitydecr);
  }

  return val;
}

double GeoPhysics::SimpleLithology::shalepermeabilityder ( const double ves,
                                                           const double maxVes, 
                                                           const double ves0 ) const {

   double val;

   assert (0 != ves0);
   assert (0 != (maxVes+ves0));

   if (ves >= maxVes) {
      val = -m_depopermeability * m_permeabilityincr * 
         pow ((ves+ves0)/ves0, (-m_permeabilityincr - 1.0)) / ves0;
   } else {

      assert (maxVes != -ves0);
      val = -m_depopermeability * m_permeabilitydecr * 
         pow (( maxVes + ves0) / ves0, -m_permeabilityincr ) * 
         pow (( ves + ves0 ) / ( maxVes + ves0 ), 
              ( -m_permeabilitydecr - 1.0 )) / ( maxVes + ves0 );
   }

   return val;  
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
  cout << m_depopermeability << " ";
  cout << m_permeabilityincr << " ";
  cout << m_permeabilitydecr << " ";
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
   buffer << " m_permeabilityaniso      " << m_permeabilityaniso << " " << endl;
   buffer << " m_density                " << m_density << " " << endl;
   buffer << " m_depoporosity           " << m_depoporosity << " " << endl;
   buffer << " m_compactionincr         " << m_compactionincr << " " << endl;
   buffer << " m_compactiondecr         " << m_compactiondecr << " " << endl;
   buffer << " m_thermalconductivityval " << m_thermalconductivityval << " " << endl;
   buffer << " m_depopermeability       " << m_depopermeability << " " << endl;
   buffer << " m_permeabilityincr       " << m_permeabilityincr << " " << endl;
   buffer << " m_permeabilitydecr       " << m_permeabilitydecr << " " << endl;
   buffer << " is incompressible        " << ( isIncompressible () ? "TRUE" : "FALSE") << endl;

   buffer << " permeability model       ";

   switch ( m_permeabilityModel ) {
     case Interface::SANDSTONE_PERMEABILITY   : buffer << "SANDSTONE_PERMEABILITY"; break;
     case Interface::MUDSTONE_PERMEABILITY    : buffer << "SANDSTONE_PERMEABILITY"; break;
     case Interface::MULTIPOINT_PERMEABILITY  : buffer << "MULTIPOINT_PERMEABILITY"; break;
     case Interface::IMPERMEABLE_PERMEABILITY : buffer << "IMPERMEABLE_PERMEABILITY"; break;
     case Interface::NONE_PERMEABILITY        : buffer << "NONE_PERMEABILITY"; break;
     default : buffer << "UNDEFINED_PERMEABILITY";
   }

   buffer << endl << endl;

   if ( m_permeabilityModel == Interface::MULTIPOINT_PERMEABILITY ) {   
      buffer << "Porosity-Permeability interpolator: " << endl;
      buffer << m_porosityPermeabilityInterpolant.image () << endl;
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

void GeoPhysics::SimpleLithology::getCoefficientsFromString ( const char*   coefficientString,
                                                              const int     numberOfCoefficients,
                                                              double*&      coefficients ) const {
  if ( numberOfCoefficients == 0 ) {
  
    coefficients = (double*)(0);
    return;

  }

  int I;
  int coefficientStringStart = 0;

  for ( I = 0; I < numberOfCoefficients; I++ ) {

    /* find the position of the first character in the coefficientString */

    while ( coefficientString [ coefficientStringStart ] == ' ' ) {
      coefficientStringStart = coefficientStringStart + 1;
    }

    /* convert string to double */
    //
    coefficients [ I ] = atof ( &coefficientString [ coefficientStringStart ] );

    /* find the next non-blank position in the coefficientString */

    while (( coefficientString [ coefficientStringStart ] != 0 ) &&
           ( coefficientString [ coefficientStringStart ] != ' ' ) ) {
      coefficientStringStart = coefficientStringStart + 1;
    }

  }

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

