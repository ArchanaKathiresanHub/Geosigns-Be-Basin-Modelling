#include "simplelitho.h"

#include <math.h>
#include <assert.h>

#include "utils.h"

#include "Interface/LithologyHeatCapacitySample.h"
#include "Interface/LithologyThermalConductivitySample.h"

#include <sstream>
using std::stringstream;

using namespace DataAccess;


SimpleLitho::SimpleLitho ( const Interface::LithoType* litho ) {

   double* multipointPorosityValues;
   double* multipointPermeabilityValues;


   m_lithoname = litho->getName ();

   m_density = litho->getDensity ();
   m_heatproduction = litho->getHeatProduction ();
   m_depoporosity = litho->getSurfacePorosity () / 100.0;
   m_Deposition_Void_Ratio = m_depoporosity / ( 1.0 - m_depoporosity );

   m_compactionincr = 1.0E-08 * litho->getExponentialCompactionCoefficient ();
   m_compactiondecr = 0.1 * m_compactionincr;
   m_thermalconductivityval = litho->getThermalConductivity ();
   m_thermalcondaniso = litho->getThermalConductivityAnisotropy ();
   m_depopermeability = litho->getDepositionalPermeability ();
   m_permeabilityincr = litho->getPermeabilityRecoveryCoefficient ();
   m_permeabilitydecr = litho->getPermeabilitySensitivityCoefficient ();
   m_permeabilityaniso = litho->getPermeabilityAnisotropy ();
   m_seismicvelocity = litho->getSeismicVelocity ();

   m_permeabilitymodel = litho->getPermeabilityModel ();
   m_Porosity_Model = litho->getPorosityModel ();

//    const std::string& PermMixModel = litho->getAttributeValue ( Interface::PermMixModel ).getString ();

//    if (PermMixModel.find ("Sands") == 0) {
//       m_permeabilitymodel = SAND;
//    } else if (PermMixModel.find ("Shales") == 0) {
//       m_permeabilitymodel = SHALE;
//    } else if (PermMixModel.find ("Impermeable") == 0) {
//       m_permeabilitymodel = IMPERMEABLE;
//    } else if (PermMixModel.find ("None") == 0) {
//       m_permeabilitymodel = NONE;
//    } else if (PermMixModel.find ("Multipoint") == 0) {
//       m_permeabilitymodel = MULTI_POINT;
//    } else {
//       cout << "Permeability Model: " << PermMixModel << " not defined for: " << m_lithoname << endl;
//    }

//    const std::string PorosityModel = litho->getAttributeValue ( Interface::Porosity_Model ).getString ();

//    if ( PorosityModel.find ( "Exponential" ) == 0 ) {
//       m_Porosity_Model = EXPONENTIAL;
//    } else if ( PorosityModel.find ( "Soil_Mechanics" ) == 0 ) {
//       m_Porosity_Model = SOIL_MECHANICS;
//    }

   m_compaction_coefficient_SM = litho->getSoilMechanicsCompactionCoefficient ();
   m_thermalcondmodel = TABLE; 
   m_heatcapmodel = TABLE; 

   Reference_Solid_Viscosity   = litho->getReferenceViscosity ();
   Lithology_Activation_Energy = litho->getViscosityActivationEnergy ();

   /// If value is a null value then DO NOT convert to cauldron units.
   if ( Lithology_Activation_Energy != CAULDRONIBSNULLVALUE and Lithology_Activation_Energy != IBSNULLVALUE ) {
      Lithology_Activation_Energy = 1000.0 * Lithology_Activation_Energy;
   }

   Lithology_Fracture_Gradient = litho->getHydraulicFracturingPercent ();

   /// If value is a null value then DO NOT convert to cauldron units.
   if ( Lithology_Fracture_Gradient != CAULDRONIBSNULLVALUE and Lithology_Fracture_Gradient != IBSNULLVALUE ) {
      Lithology_Fracture_Gradient = 0.01 * Lithology_Fracture_Gradient;
   }

   Minimum_Mechanical_Porosity = litho->getMinimumMechanicalPorosity ();

   /// If value is a null value then DO NOT convert to cauldron units.
   if ( Minimum_Mechanical_Porosity != CAULDRONIBSNULLVALUE and Minimum_Mechanical_Porosity != IBSNULLVALUE ) {
      Minimum_Mechanical_Porosity = 0.01 * Minimum_Mechanical_Porosity;
   }

   if ( m_permeabilitymodel == Interface::MULTIPOINT_PERMEABILITY ) {
      int i;

      multipointPorosityValues = new double [ litho->getNumberOfMultipointSamplePoints ()];
      multipointPermeabilityValues = new double [ litho->getNumberOfMultipointSamplePoints ()];

      Get_Coefficients_From_String ( litho->getMultipointPorosityValues ().c_str (),
                                     litho->getNumberOfMultipointSamplePoints (),
                                     multipointPorosityValues );

      Get_Coefficients_From_String ( litho->getMultipointPermeabilityValues ().c_str (),
                                     litho->getNumberOfMultipointSamplePoints (),
                                     multipointPermeabilityValues );

      for ( i = 0; i < litho->getNumberOfMultipointSamplePoints (); ++i ) {
         multipointPorosityValues [ i ] /= 100.0;
      }

      Interpolant.Set_Interpolation ( PIECEWISE_LINEAR,
                                      litho->getNumberOfMultipointSamplePoints (),
                                      multipointPorosityValues,
                                      multipointPermeabilityValues );

      Interpolant.Compute_Coefficients ();

      // not realy necessary, because the m_depopermeability 
      // is not used if permeability model is a multi-point.
      m_depopermeability = Interpolant.Evaluate ( m_depoporosity );
   }

   if (( m_permeabilitymodel == Interface::IMPERMEABLE_PERMEABILITY or m_permeabilitymodel == Interface::NONE_PERMEABILITY ) and m_permeabilityaniso == 0.0 ) {
      m_permeabilityaniso = 1.0;
   }


   Interface::LithologyHeatCapacitySampleList* heatCapacitySamples = litho->getHeatCapacitySamples ();
   Interface::LithologyThermalConductivitySampleList* thermalConductivitySamples = litho->getThermalConductivitySamples ();

   Interface::LithologyHeatCapacitySampleList::const_iterator heatCapacityIter;
   Interface::LithologyThermalConductivitySampleList::const_iterator thermalConductivityIter;

   for ( heatCapacityIter = heatCapacitySamples->begin (); heatCapacityIter != heatCapacitySamples->end (); ++heatCapacityIter ) {
      const Interface::LithologyHeatCapacitySample* sample = *heatCapacityIter;

      m_heatcapacitytbl.addPoint( sample->getTemperature (), sample->getHeatCapacity ());
   }

   for ( thermalConductivityIter = thermalConductivitySamples->begin (); thermalConductivityIter != thermalConductivitySamples->end (); ++thermalConductivityIter ) {
      const Interface::LithologyThermalConductivitySample* sample = *thermalConductivityIter;


      XY xyPoint( sample->getTemperature (), sample->getThermalConductivity ());

      m_thermCondTbl.push_back(xyPoint);
      m_thermalconductivitytbl.addPoint( sample->getTemperature (), sample->getThermalConductivity ());
   }


   delete heatCapacitySamples;
   delete thermalConductivitySamples;
}


SimpleLitho::SimpleLitho ( const SimpleLitho*               definedLithology,
                           const string&                    faultLithologyName,
                           const double                     permeabilityAnisotropy,
                           const Permeability_Interpolator& newPermeabilities ) {

  ///
  /// Copy all components that are not affected by the permeability.
  ///
  m_thermCondTbl = definedLithology->m_thermCondTbl;
  m_thermalconductivitytbl = definedLithology->m_thermalconductivitytbl;
  m_heatcapacitytbl = definedLithology->m_heatcapacitytbl;

  m_density = definedLithology->m_density;
  m_heatproduction = definedLithology->m_heatproduction;
  m_depoporosity = definedLithology->m_depoporosity;
  m_Deposition_Void_Ratio = m_depoporosity / ( 1.0 - m_depoporosity );
  m_compactionincr = definedLithology->m_compactionincr;
  m_compactiondecr = definedLithology->m_compactiondecr;
  m_thermalconductivityval = definedLithology->m_thermalconductivityval;
  m_thermalcondaniso = definedLithology->m_thermalcondaniso;
  m_seismicvelocity = definedLithology->m_seismicvelocity;
  m_Porosity_Model = definedLithology->m_Porosity_Model;
  m_compaction_coefficient_SM = definedLithology->m_compaction_coefficient_SM;
  m_thermalcondmodel = TABLE; 
  m_heatcapmodel = TABLE; 

  Reference_Solid_Viscosity   = definedLithology->Reference_Solid_Viscosity;
  Lithology_Activation_Energy = definedLithology->Lithology_Activation_Energy;
  Lithology_Fracture_Gradient = definedLithology->Lithology_Fracture_Gradient;
  Minimum_Mechanical_Porosity = definedLithology->Minimum_Mechanical_Porosity;

  ///
  /// The following two are not strictly necessary, as they will never be used.
  ///
  m_permeabilityincr = definedLithology->m_permeabilityincr;
  m_permeabilitydecr = definedLithology->m_permeabilitydecr;

  ///
  /// Now, define the new permeability
  ///
  m_permeabilityaniso = permeabilityAnisotropy;

  m_permeabilitymodel = Interface::MULTIPOINT_PERMEABILITY;
  Interpolant = newPermeabilities;

  ///
  /// Again, this is not strictly neccessary but is added for completness.
  ///
  m_depopermeability = Interpolant.Evaluate ( m_depoporosity );

  ///
  /// We have a new name as well.
  ///
  m_lithoname = faultLithologyName;

}


SimpleLitho::SimpleLitho ( const SimpleLitho* definedLithology,
                           const string& newName ) {

  ///
  /// Copy all components that are not affected by the permeability.
  ///
  m_thermCondTbl = definedLithology->m_thermCondTbl;
  m_thermalconductivitytbl = definedLithology->m_thermalconductivitytbl;
  m_heatcapacitytbl = definedLithology->m_heatcapacitytbl;

  m_density = definedLithology->m_density;
  m_heatproduction = definedLithology->m_heatproduction;
  m_depoporosity = definedLithology->m_depoporosity;
  m_Deposition_Void_Ratio = m_depoporosity / ( 1.0 - m_depoporosity );
  m_compactionincr = definedLithology->m_compactionincr;
  m_compactiondecr = definedLithology->m_compactiondecr;
  m_thermalconductivityval = definedLithology->m_thermalconductivityval;
  m_thermalcondaniso = definedLithology->m_thermalcondaniso;
  m_seismicvelocity = definedLithology->m_seismicvelocity;
  m_Porosity_Model = definedLithology->m_Porosity_Model;
  m_compaction_coefficient_SM = definedLithology->m_compaction_coefficient_SM;
  m_thermalcondmodel = TABLE; 
  m_heatcapmodel = TABLE; 

  Reference_Solid_Viscosity   = definedLithology->Reference_Solid_Viscosity;
  Lithology_Activation_Energy = definedLithology->Lithology_Activation_Energy;
  Lithology_Fracture_Gradient = definedLithology->Lithology_Fracture_Gradient;
  Minimum_Mechanical_Porosity = definedLithology->Minimum_Mechanical_Porosity;

  ///
  /// The following two are not strictly necessary, as they will never be used.
  ///
  m_permeabilityincr  = definedLithology->m_permeabilityincr;
  m_permeabilitydecr  = definedLithology->m_permeabilitydecr;
  m_permeabilityaniso = definedLithology->m_permeabilityaniso;
  m_permeabilitymodel = definedLithology->m_permeabilitymodel;
  m_depopermeability  = definedLithology->m_depopermeability;

  if ( m_permeabilitymodel == Interface::MULTIPOINT_PERMEABILITY ) {
    Interpolant = definedLithology->Interpolant;
  }

  m_lithoname = newName;
}

  
SimpleLitho::~SimpleLitho() {

}

void SimpleLitho::correctThermCondPoint (double correction) {

  XYContainer::iterator XYIter = m_thermCondTbl.begin();
  while (m_thermCondTbl.end() != XYIter) {
    (*XYIter).setY((*XYIter).getY() * correction);
    ++XYIter;
  }
} 
void SimpleLitho::correctThermCondPointHofmeister (double inPressure) {
   // Pa in GPa, K in Kelvin

  XYContainer::iterator XYIter = m_thermCondTbl.begin();
  double temp, klat, krad;
  const double val1 = - (4 * 1.28 + 1 / 3) * 0.00002;
  const double val2 = 4.6 / 128.1;

  while (m_thermCondTbl.end() != XYIter) {
     temp = (*XYIter).getX() + 273.15; // convert C to K
     if(temp > 0.0) {
        klat = 4.70 * pow(298 / temp, 0.25) * exp(val1 * (temp - 298.0)) * (1.0 + val2 * inPressure);
        krad = 1.753E-02 + temp * (-1.0365E-04 + temp * (2.2451E-07 - 3.4071E-11 * temp));
        (*XYIter).setY(klat + krad);
     }
    ++XYIter;
  }
} 
void SimpleLitho::correctThermCondPointXu (double inPressure) {
   // Pa in GPa, K in Kelvin

  XYContainer::iterator XYIter = m_thermCondTbl.begin();
  double temp, termCond;

  while (m_thermCondTbl.end() != XYIter) {
     temp = (*XYIter).getX() + 273.15; // convert C to K
     if(temp > 0.0) {
        termCond = 4.1 * pow(298.0 / temp, 0.493) * (1 + 0.032 * inPressure);
        (*XYIter).setY(termCond);
     }
    ++XYIter;
  }
} 
void SimpleLitho::correctThermCondPointWillis () {
   // Pa in GPa, K in Kelvin

  XYContainer::iterator XYIter = m_thermCondTbl.begin();
  double temp, termCond;

  while (m_thermCondTbl.end() != XYIter) {
     temp = (*XYIter).getX() + 273.15; // convert C to K
     if(temp > 0.0) {
        termCond = 4.6738 + temp * (-0.0069356 + 0.0000031749 * temp);
        (*XYIter).setY(termCond);
     }
    ++XYIter;
  }
} 

double SimpleLitho::permeability ( const double Ves,
                                   const double MaxVes, 
				   const double calculatedPorosity ) const {
  double val = 0.0;

  switch (m_permeabilitymodel) {
    case Interface::SANDSTONE_PERMEABILITY : {

      double deltaphi = calculatedPorosity - m_depoporosity;
      double m = 0.12 + 0.02 * m_permeabilityincr;

      val =  m_depopermeability * pow(10.0, m * deltaphi * 100.0);

#if 0
      val = (m_depopermeability*1000.0)*pow(10.0, m * deltaphi * 100.0);
#endif

      if (val >= 1000.0) val = 1000.0;

      return val;

    }
    case Interface::MUDSTONE_PERMEABILITY : {

      double ves0 = 1.0E+05;
      double cut_off = 0.0;
      
      if ( Ves > cut_off) {
	val = shalepermeability (Ves, MaxVes, ves0);
      } else {
	double a = shalepermeabilityder (cut_off, MaxVes, ves0);
	double b = shalepermeability (cut_off, MaxVes, ves0);
	val = a*(Ves-cut_off)+b;
      }

      if (val >= 1000.0) val = 1000.0;
      return val;

    }
    case Interface::MULTIPOINT_PERMEABILITY : {

      val = exp ( Log_10 * Interpolant.Evaluate ( calculatedPorosity ));

      if (val >= 1000.0) val = 1000.0;

      return val;
    }
    case Interface::IMPERMEABLE_PERMEABILITY: {
      // lithology does have a porosity>0 but permeability=0
      val = 1.0E-09;
      return val;
    }
    case Interface::NONE_PERMEABILITY: {
      // lithology does NOT have a porosity (i.e. = 0) and thus no permeability!
      val = 1.0E-09;

//        double Maximum_Permeability = 1.0e-3;
//        double Minimum_Permeability = 1.0e-7;
//        double VES_Cut_Off = 5.0e6;

//        double Slope = ( Minimum_Permeability - Maximum_Permeability ) / VES_Cut_Off;

//        if ( Ves > VES_Cut_Off ) {
//          val = Minimum_Permeability;
//        } else {
//          val = Slope * Ves + Maximum_Permeability;
//        }


      return val;
    }
    default: {
      PETSC_ASSERT(0);
    }
  }
  return 0;
}


void SimpleLitho::permeabilityDerivative ( const double  Ves,
                                           const double  MaxVes, 
                                           const double  calculatedPorosity, 
                                                 double& Permeability, 
                                                 double& Derivative ) const {

  Permeability = this->permeability (Ves, MaxVes, calculatedPorosity );

  switch (m_permeabilitymodel) {

    case Interface::SANDSTONE_PERMEABILITY : {

      double perm;
      double deltaphi = calculatedPorosity - m_depoporosity;
      double m = 0.12 + 0.02 * m_permeabilityincr;

      perm = m_depopermeability * pow(10.0, m * deltaphi * 100.0);

#if 0
      perm = (m_depopermeability*1000.0)*pow(10.0, m * deltaphi * 100.0);
#endif

      Derivative = Log_10 * m * perm;
      break;
    } 

    case Interface::MUDSTONE_PERMEABILITY : {

      double ves0 = 1.0E+05;
      Derivative = shalepermeabilityder (Ves, MaxVes, ves0);
      break;
    }

    case Interface::MULTIPOINT_PERMEABILITY : {

      Derivative = Log_10 * Interpolant.evaluateDerivative ( calculatedPorosity ) * Permeability;
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
      PETSC_ASSERT(0);
    }
  }

}

double SimpleLitho::shalepermeability(const double Ves, const double MaxVes, 
				      const double ves0) const
{
  double val;

  IBSASSERT(0 != ves0);
  if (Ves >= MaxVes) {
    val = m_depopermeability * pow((Ves+ves0)/ves0, -m_permeabilityincr);
  } else {
    IBSASSERT(MaxVes != -ves0);
    IBSASSERT(0 != MaxVes);
    val = m_depopermeability * pow ((MaxVes+ves0)/ ves0, -m_permeabilityincr) *
      pow ((Ves+ves0)/(MaxVes+ves0), -m_permeabilitydecr);
  }

  return val;
}

double SimpleLitho::shalepermeabilityder(const double Ves, const double MaxVes, 
					 const double ves0) const
{
  double val;

  IBSASSERT(0 != ves0);
  IBSASSERT(0 != (MaxVes+ves0));
  if (Ves >= MaxVes) 
    {
      val = -m_depopermeability * m_permeabilityincr * 
	pow ((Ves+ves0)/ves0, (-m_permeabilityincr - 1.0)) / ves0;
    }
  else
    {
      IBSASSERT(MaxVes != -ves0);
      val = -m_depopermeability * m_permeabilitydecr * 
	pow ((MaxVes+ves0)/ ves0, -m_permeabilityincr) * 
	pow ((Ves+ves0)/(MaxVes+ves0), 
	     (-m_permeabilitydecr-1.0)) / (MaxVes+ves0) ;
    }

  return val;  
}

double SimpleLitho::thermalconductivity(double t)
{
  double val;

  switch (m_thermalcondmodel)
    {
    case CONST: {
      val = m_thermalconductivityval;
      return val;
    }
    case TABLE: {
      val = m_thermalconductivitytbl.compute(t);
      return val;
    }
    default: {
      PETSC_ASSERT(0);
    }
    }
  return 0;
}

double SimpleLitho::heatcapacity(double t)
{
  switch (m_heatcapmodel)
    {
    case TABLE:{
      return m_heatcapacitytbl.compute(t);
    }
    default: {
      PETSC_ASSERT(0);
    }
    }
  return 0;
}

void SimpleLitho::print() {

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
}

std::string SimpleLitho::image () const {

   stringstream buffer;

   buffer.precision ( 10 );
   buffer.flags ( ios::scientific );

   buffer << endl;
   buffer << endl;
   buffer << " m_lithoname              " << m_lithoname << " " << endl;
   buffer << " m_thermalcondaniso       " << m_thermalcondaniso << " " << endl;
   buffer << " m_heatproduction         " << m_heatproduction << " " << endl;
   buffer << " m_permeabilityaniso      " << m_permeabilityaniso << " " << endl;
   buffer << " m_density                " << m_density << " " << endl;
   buffer << " m_depoporosity           " << m_depoporosity << " " << endl;
   buffer << " m_compactionincr         " << m_compactionincr << " " << endl;
   buffer << " m_compactiondecr         " << m_compactiondecr << " " << endl;
   buffer << " m_thermalconductivityval " <<m_thermalconductivityval << " " << endl;
   buffer << " m_depopermeability       " << m_depopermeability << " " << endl;
   buffer << " m_permeabilityincr       " << m_permeabilityincr << " " << endl;
   buffer << " m_permeabilitydecr       " << m_permeabilitydecr << " " << endl;

   buffer << endl << endl;
   buffer << Interpolant.image () << endl;
   buffer << endl << endl;


   return buffer.str ();
   
}


Permeability_Interpolator::Permeability_Interpolator () {
  Porosities       = 0;
  Permeabilities   = 0;
  As               = 0;
  Bs               = 0;
  Cs               = 0;
  Ds               = 0;
  Method           = PIECEWISE_LINEAR;
  Number_Of_Points = 0;
} // end Permeability_Interpolator::constructor


//------------------------------------------------------------//


Permeability_Interpolator::~Permeability_Interpolator () {
  Delete_Coefficients ();
} // end Permeability_Interpolator::destructor


//------------------------------------------------------------//


void Permeability_Interpolator::Delete_Coefficients () {

  if ( Porosities != Null ) {
    delete [] Porosities;
    Porosities = 0;

    delete [] Permeabilities;
    Permeabilities = 0;

    delete [] As;
    delete [] Bs;
    As = 0;
    Bs = 0;
  } // end if
  //
  //
  // The next 2 are not null if the interpolant was a cubic spline
  //
  if ( Cs != Null ) {
    delete [] Cs;
    delete [] Ds;
    Cs = 0;
    Ds = 0;
  } // end if

} // end Permeability_Interpolator::Delete_Coefficients


//------------------------------------------------------------//


double Permeability_Interpolator::Evaluate ( const double Porosity ) const {

  if ( Method == PIECEWISE_LINEAR ) {
    return Evaluate_Piecewise_Linear ( Porosity );
  } else { // Method == Cubic_Spline
    return Evaluate_Cubic_Spline ( Porosity );
  } // end if

} // end Permeability_Interpolator::Evaluate


//------------------------------------------------------------//


double Permeability_Interpolator::evaluateDerivative ( const double Porosity ) const {

  if ( Method == PIECEWISE_LINEAR ) {
    return evaluatePiecewiseLinearDerivative ( Porosity );
  } else { // Method == Cubic_Spline
    return evaluateCubicSplineDerivative ( Porosity );
  } // end if

} // end Permeability_Interpolator::evaluateDerivative


//------------------------------------------------------------//


double Permeability_Interpolator::Evaluate_Piecewise_Linear ( const double Porosity ) const {

  int    Panel_Number = Find_Panel ( Porosity );
  double Result;


  Result = As [ Panel_Number ] + Bs [ Panel_Number ] * Porosity;

  return Result;
} // end Permeability_Interpolator::Evaluate_PIECEWISE_LINEAR


//------------------------------------------------------------//


double Permeability_Interpolator::evaluatePiecewiseLinearDerivative ( const double Porosity ) const {

  int Panel_Number = Find_Panel ( Porosity );

  return Bs [ Panel_Number ];
} // end Permeability_Interpolator::evaluatePiecewiseLinearDerivative


//------------------------------------------------------------//


double Permeability_Interpolator::Evaluate_Cubic_Spline ( const double Porosity ) const {

  int Panel_Number = Find_Panel ( Porosity );
  double Difference;
  double Result;

  if ( Panel_Number == -1 ) {
    Result = M_0 * Porosity + C_0;
  } else if ( Panel_Number == Number_Of_Points ) {
    Result = M_N * Porosity + C_N;
  } else {

    Difference = Porosity - Porosities [ Panel_Number ];
    //
    //
    // Evaluate spline using Horners method. P ( x ) = (( D x + C ) x + B ) x + A
    //
    Result = (( Ds [ Panel_Number ]  * Difference +
                Cs [ Panel_Number ]) * Difference +
                Bs [ Panel_Number ]) * Difference + As [ Panel_Number ];
  } // end if

  return Result;
} // end Permeability_Interpolator::Evaluate_Cubic_Spline


//------------------------------------------------------------//


double Permeability_Interpolator::evaluateCubicSplineDerivative ( const double Porosity ) const {

  int Panel_Number = Find_Panel ( Porosity );
  double Difference;
  double Result;

  if ( Panel_Number == -1 ) {
    Result = M_0;
  } else if ( Panel_Number == Number_Of_Points ) {
    Result = M_N;
  } else {

    Difference = Porosity - Porosities [ Panel_Number ];
    //
    //
    // Evaluate spline using Horners method. P  ( x ) = (( D x + C ) x + B ) x + A
    //                                       P' ( x ) = (( 3 * D x + 2 * C ) x + B
    //
    Result = ( 3.0 * Ds [ Panel_Number ]  * Difference + 2.0 * Cs [ Panel_Number ]) * Difference + Bs [ Panel_Number ];
  } // end if

  return Result;
} // end Permeability_Interpolator::evaluateCubicSplineDerivative


//------------------------------------------------------------//


void Permeability_Interpolator::Set_Interpolation
   ( const Interpolation_Method New_Interpolation_Method,
     const int                  New_Number_Of_Points,
     const double*              New_Porosities,
     const double*              New_Permeabilities ) {

  int I;

  Method           = New_Interpolation_Method;
  Number_Of_Points = New_Number_Of_Points;

  if ( Number_Of_Points < 4 ) {
    //
    //
    // Need at least 4 points for a Cubic Spline interpolant!
    //
    Method = PIECEWISE_LINEAR;
  } else {
    Method = CUBIC_SPLINE;
  } // end if

  Method = PIECEWISE_LINEAR;


  if ( Number_Of_Points != 0 ) {
    Delete_Coefficients ();
  } // end if

  Porosities     = new double [ Number_Of_Points ];
  Permeabilities = new double [ Number_Of_Points ];


  if ( Method == PIECEWISE_LINEAR ) {
    As = new double [ Number_Of_Points - 1 ];
    Bs = new double [ Number_Of_Points - 1 ];
  } else {
    As = new double [ Number_Of_Points ];
    Bs = new double [ Number_Of_Points - 1 ];
    Cs = new double [ Number_Of_Points - 1 ];
    Ds = new double [ Number_Of_Points - 1 ];
  } // end if

  for ( I = 0; I < Number_Of_Points; I++ ) {
    Porosities     [ I ] = New_Porosities     [ I ];
    Permeabilities [ I ] = New_Permeabilities [ I ];
  } // end loop

} // end Permeability_Interpolator::Set_Interpolation


//------------------------------------------------------------//


void Permeability_Interpolator::Compute_Piecewise_Linear_Coefficients () {

  int    I;
  double Divisor;
  // 
  // 
  //             ( x - a )            ( b - x )
  //     P(x) =  --------- * f(b)  +  --------- * f (a)
  //             ( b - a )            ( b - a )
  // 
  // 
  //             x * ( f(b) - f(a))    b * f (a)    a * f (b)
  //          =  ------------------ +  --------  -  ---------
  //                  ( b - a )        ( b - a )    ( b - a )
  // 
  //          = A + B x
  // 
  // 
  //        b * f (a)     a * f (b)
  //   A =  ---------  -  ---------
  //        ( b - a )     ( b - a )
  // 
  // 
  //        ( f(b) - f(a))    
  //   B  =  --------------
  //           ( b - a )
  // 
  // 
  // 
  //  Rearranging the calculation like this reduces the flop count
  //  considerably, from 6 (including a division) to 2, per evaluation.
  // 
  // 
  for ( I = 0; I < Number_Of_Points - 1; I++ ) {
    Divisor = 1.0 / ( Porosities [ I + 1 ] - Porosities [ I ]);

    As [ I ] = Divisor * ( Porosities [ I + 1 ] * Permeabilities [ I     ] - 
                           Porosities [ I     ] * Permeabilities [ I + 1 ]);

    Bs [ I ] = Divisor * ( Permeabilities [ I + 1 ] - Permeabilities [ I ]);

  } // end loop

} // end Permeability_Interpolator::Compute_PIECEWISE_LINEAR_Coefficients


//------------------------------------------------------------//


void Permeability_Interpolator::Compute_Cubic_Spline_Coefficients () {

  int I;
  double* Hs     = new double [ Number_Of_Points - 1 ];
  double* Alphas = new double [ Number_Of_Points - 1 ];
  double* Ls     = new double [ Number_Of_Points ];
  double* Mus    = new double [ Number_Of_Points ];
  double* Zs     = new double [ Number_Of_Points ];

  for ( I = 0; I < Number_Of_Points; I++ ) {
    As [ I ] = Permeabilities [ I ];
  } // end loop

  for ( I = 0; I < Number_Of_Points - 1; I++ ) {
    Hs [ I ] = Porosities [ I + 1 ] - Porosities [ I ];
  } // end loop

  for ( I = 1; I < Number_Of_Points - 1; I++ ) {
    Alphas [ I ] = 3.0 * ( As [ I + 1 ] * Hs [ I - 1 ] - As [ I ] * ( Porosities [ I + 1 ] - Porosities [ I - 1 ]) + As [ I - 1 ] * Hs [ I ]) / 
                   //-------------------------------------------------------------------------------------------------------------------------//
                                                             ( Hs [ I - 1 ] * Hs [ I ]);
  } // end loop

  Ls  [ 0 ] = 1.0;
  Mus [ 0 ] = 0.0;
  Zs  [ 0 ] = 0.0;

  for ( I = 1; I < Number_Of_Points - 1; I++ ) {
    Ls  [ I ] = 2.0 * ( Porosities [ I + 1 ] - Porosities [ I - 1 ]) - Hs [ I - 1 ] * Mus [ I - 1 ];
    Mus [ I ] = Hs [ I ] / Ls [ I ];
    Zs  [ I ] = ( Alphas [ I ] - Hs [ I - 1 ] * Zs [ I - 1 ]) / Ls [ I ];
  } // end loop

  Ls [ Number_Of_Points - 1 ] = 1.0;
  Zs [ Number_Of_Points - 1 ] = 0.0;
  Cs [ Number_Of_Points - 1 ] = 0.0;

  for ( I = Number_Of_Points - 2; I >= 0; I-- ) {
    Cs [ I ] = Zs [ I ] - Mus [ I ] * Cs [ I + 1 ];
    Bs [ I ] = ( As [ I + 1 ] - As [ I ]) / Hs [ I ] - Hs [ I ] * ( Cs [ I + 1 ] + 2.0 * Cs [ I ]) / 3.0;
    Ds [ I ] = ( Cs [ I + 1 ] - Cs [ I ]) / ( 3.0 * Hs [ I ]);
  } // end loop

  delete [] Hs;
  delete [] Alphas;
  delete [] Ls;
  delete [] Mus;
  delete [] Zs;

  //------------------------------------------------------------//
  //
  //
  // Now compute the extrapolants.
  //
  const int N = Number_Of_Points - 1;

  M_0 = Bs [ 0 ];
  C_0 = Permeabilities [ 0 ] - M_0 * Porosities [ 0 ];

  double Difference = Porosities [ N ] - Porosities [ N - 1 ];

  M_N = Bs [ N - 1 ] + 2.0 * Cs [ N - 1 ] * Difference + 2.0 * Ds [ N - 1 ] * Difference * Difference;
  C_N = Permeabilities [ N ] - M_N * Porosities [ N ];

} // end Permeability_Interpolator::Compute_Cubic_Spline_Coefficients


//------------------------------------------------------------//


void Permeability_Interpolator::Compute_Coefficients () {

  if ( Porosities == Null ) {
    //
    //
    // There is nothing to compute!
    //
    return;
  } // end if

  if ( Method == PIECEWISE_LINEAR ) {
    Compute_Piecewise_Linear_Coefficients ();
  } else if ( Method == CUBIC_SPLINE ) {
    Compute_Cubic_Spline_Coefficients ();
  } // end if

} // end Permeability_Interpolator::Compute_Coefficients


//------------------------------------------------------------//


int Permeability_Interpolator::Find_Panel ( const double Porosity ) const {

  int Panel_Number;

  if ( Method == PIECEWISE_LINEAR ) {

    for ( Panel_Number = 1; Panel_Number < Number_Of_Points; Panel_Number++ ) {

      if ( Porosity < Porosities [ Panel_Number ]) {
        return Panel_Number - 1;
      } // end if

    } // end if

    return Number_Of_Points - 2;
  } else {

    for ( Panel_Number = 0; Panel_Number < Number_Of_Points; Panel_Number++ ) {

      if ( Porosity < Porosities [ Panel_Number ]) {
        return Panel_Number - 1;
      } // end if

    } // end if

    return Number_Of_Points;
  } // end if

} // end Permeability_Interpolator::Find_Panel


//------------------------------------------------------------//


void Permeability_Interpolator::Put () const {

  int I;
  #if defined (sgi)
     unsigned long new_options = ios::scientific;
     unsigned long old_options = cout.flags ( new_options );
  #else
     ios::fmtflags new_options = ios::scientific;
     ios::fmtflags old_options = cout.flags ( new_options );
  #endif

  int Old_Precision = cout.precision ( 10 );

  cout << " There are " << Number_Of_Points << " points "  << endl;

  cout << endl << " Porosities | Permeabilities " << endl;

  for ( I = 0; I < Number_Of_Points; I++ ) {
    cout << Porosities [ I ] << "  " << Permeabilities [ I ] << endl;
  } // end loop

  if ( Method == PIECEWISE_LINEAR ) {
    cout << endl << " As      |     Bs  " << endl;

    for ( I = 0; I < Number_Of_Points - 1; I++ ) {
      cout << As [ I ] << "  " << Bs [ I ] << endl;
    } // end loop

  } else {
    cout << endl << " As      |     Bs   |    Cs    | Ds   " << endl;

    for ( I = 0; I < Number_Of_Points - 1; I++ ) {
      cout << As [ I ] << "  " << Bs [ I ] << "  " << Cs [ I ] << "  " << Ds [ I ] << endl;
    } // end loop

  } // end if

  cout.precision ( Old_Precision );
  cout.flags ( old_options );

} // end Permeability_Interpolator::Put

//------------------------------------------------------------//

std::string Permeability_Interpolator::image () const {

   stringstream buffer;
   int I;

   buffer.precision ( 10 );
   buffer.flags ( ios::scientific );

   buffer << " permeability interpolator: " << endl << " There are " << Number_Of_Points << " points "  << endl;
   buffer << endl << " Porosities | Permeabilities " << endl;

   for ( I = 0; I < Number_Of_Points; I++ ) {
      buffer << Porosities [ I ] << "  " << Permeabilities [ I ] << endl;
   }

   if ( Method == PIECEWISE_LINEAR ) {
      buffer << endl << " As      |     Bs  " << endl;

      for ( I = 0; I < Number_Of_Points - 1; I++ ) {
         buffer << As [ I ] << "  " << Bs [ I ] << endl;
      }

   } else {
      buffer << endl << " As      |     Bs   |    Cs    | Ds   " << endl;

      for ( I = 0; I < Number_Of_Points - 1; I++ ) {
         buffer << As [ I ] << "  " << Bs [ I ] << "  " << Cs [ I ] << "  " << Ds [ I ] << endl;
      }

   }

   buffer << endl << endl;

   return buffer.str ();
}

//------------------------------------------------------------//

Permeability_Interpolator& Permeability_Interpolator::operator=( const Permeability_Interpolator& newInterpolator ) {

  if ( Porosities != 0 ) {
    delete [] Porosities;
    delete [] Permeabilities;
  }

  if ( As != 0 ) {
    delete [] As;
    delete [] Bs;
  }

  if ( Cs != 0 ) {
    delete [] Cs;
    delete [] Ds;
  }

  ///
  /// Should I compute the new coefficients before deleting the old stuff?
  ///
  Set_Interpolation ( newInterpolator.Method, newInterpolator.Number_Of_Points, newInterpolator.Porosities, newInterpolator.Permeabilities );
  Compute_Coefficients ();

  return *this;
}

//------------------------------------------------------------//


double SimpleLitho::Exponential_Porosity ( const double VES,
                                           const double Max_VES,
                                           const bool   Include_Chemical_Compaction ) const {


  double calculatedPorosity;
  bool   Loading_Phase = ( VES >= Max_VES );

  if ( Include_Chemical_Compaction ) {

    if ( Loading_Phase ) {
      calculatedPorosity = ( m_depoporosity - Minimum_Mechanical_Porosity ) * exp( -m_compactionincr * Max_VES ) + Minimum_Mechanical_Porosity;
    } else {
      calculatedPorosity = ( m_depoporosity - Minimum_Mechanical_Porosity ) * exp( m_compactiondecr * ( Max_VES - VES ) - m_compactionincr * Max_VES ) + Minimum_Mechanical_Porosity;
    }

  } else {

    if ( Loading_Phase ) {
      calculatedPorosity = m_depoporosity * exp( -m_compactionincr * Max_VES );
    } else {
      calculatedPorosity = m_depoporosity * exp( m_compactiondecr * ( Max_VES - VES ) - m_compactionincr * Max_VES );
    }

  }

  return calculatedPorosity;
}


//------------------------------------------------------------//

double SimpleLitho::referenceEffectiveStress () const {
  return 1.0e5;
}

//------------------------------------------------------------//


double SimpleLitho::Soil_Mechanics_Porosity ( const double VES,
                                              const double Max_VES,
                                              const bool   Include_Chemical_Compaction ) const {


  bool Loading_Phase = ( VES >= Max_VES );
  const double MinimumSoilMechanicsPorosity = 0.03;

  const double VES_0            = referenceEffectiveStress ();

  const double Epsilon_100      = m_Deposition_Void_Ratio;


  double calculatedPorosity;
  double Void_Ratio;
  double M, C;
  double Phi_Max_VES;
  double Phi_Min_VES;
  double VES_Used;

  const double Percentage_Porosity_Rebound = 0.02; // => %age porosity regain


  if ( Loading_Phase ) {
    
    VES_Used = NumericFunctions::Maximum ( VES, Max_VES );
    VES_Used = NumericFunctions::Maximum ( VES_Used, VES_0 );

    Void_Ratio = Epsilon_100 - m_compaction_coefficient_SM * log ( VES_Used / VES_0 );

    calculatedPorosity = Void_Ratio / ( 1.0 + Void_Ratio );

  } else {

    Void_Ratio = Epsilon_100 - m_compaction_coefficient_SM * log ( NumericFunctions::Maximum ( VES_0, Max_VES ) / VES_0 ); 
    Phi_Max_VES = Void_Ratio / ( 1.0 + Void_Ratio );
    Void_Ratio = Epsilon_100;
    Phi_Min_VES = Void_Ratio / ( 1.0 + Void_Ratio );

    M = Percentage_Porosity_Rebound * ( Phi_Max_VES - Phi_Min_VES ) / ( Max_VES - VES_0 );

    C = ( ( 1.0 - Percentage_Porosity_Rebound ) * Phi_Max_VES * Max_VES - 
	  Phi_Max_VES * VES_0 + Percentage_Porosity_Rebound * Phi_Min_VES * Max_VES ) / 
          ( Max_VES - VES_0 );

    calculatedPorosity = M * VES + C;
  } 

  // Force porosity to be in range 0.03 .. Surface_Porosity

  if ( Include_Chemical_Compaction ) {
    calculatedPorosity = NumericFunctions::Maximum ( calculatedPorosity, Minimum_Mechanical_Porosity );
  }

  calculatedPorosity = NumericFunctions::Maximum ( calculatedPorosity, MinimumSoilMechanicsPorosity );
  calculatedPorosity = NumericFunctions::Minimum ( calculatedPorosity, m_depoporosity );

  return calculatedPorosity;
}


//------------------------------------------------------------//


double SimpleLitho::Porosity ( const double VES,
                               const double Max_VES,
                               const bool   Include_Chemical_Compaction,
                               const double Chemical_Compaction ) {

  double calculatedPorosity;

  if ( m_Porosity_Model == Interface::EXPONENTIAL_POROSITY ) {
    calculatedPorosity = Exponential_Porosity ( VES, Max_VES, Include_Chemical_Compaction );
  } else {
    calculatedPorosity = Soil_Mechanics_Porosity ( VES, Max_VES, Include_Chemical_Compaction );
  }

  if ( Include_Chemical_Compaction ) {
    calculatedPorosity = calculatedPorosity + Chemical_Compaction;
    calculatedPorosity = NumericFunctions::Maximum ( calculatedPorosity, Minimum_Porosity );
  }

  return calculatedPorosity;
}


//------------------------------------------------------------//


void SimpleLitho::Set_Chemical_Compaction_Terms 
   ( const double Rock_Viscosity,
     const double Activation_Energy ) {

  Reference_Solid_Viscosity   = Rock_Viscosity;
  Lithology_Activation_Energy = Activation_Energy;

}
