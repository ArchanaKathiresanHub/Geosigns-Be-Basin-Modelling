#include "GeoPhysicsFluidType.h"
#include "GeoPhysicsProjectHandle.h"

#include "GeoPhysicalConstants.h"

#include <cmath>
#include <assert.h>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "Interface/FluidHeatCapacitySample.h"
#include "Interface/FluidThermalConductivitySample.h"
#include "Interface/FluidDensitySample.h"
#include "Interface/FluidType.h"
#include "Interface/Interface.h"
#include "Interface/PermafrostEvent.h"

#include "NumericFunctions.h"

using namespace DataAccess;

const double GeoPhysics::FluidType::DefaultHydrostaticPressureGradient              = 10.0;
const double GeoPhysics::FluidType::StandardSurfaceTemperature                      = 10.0;
const double GeoPhysics::FluidType::DefaultStandardDepth                            = 2000.0;
const double GeoPhysics::FluidType::DefaultThermalConductivityCorrectionTemperature = 20.0;

GeoPhysics::FluidType::FluidType ( Interface::ProjectHandle * projectHandle, database::Record * record ) : DataAccess::Interface::FluidType ( projectHandle, record ) {

   m_densityVal = Interface::FluidType::density ();
   m_salinity = salinity ();
   m_densityCalculationModel = fluidDensityModel ();

   m_seismicVelocityVal = getConstantSeismicVelocity ();
   m_seismicVelocityCalculationModel = getSeismicVelocityCalculationModel ();

   double term = pow ( m_salinity, 0.8 ) - 0.17;

   m_precomputedViscosityTerm1 = 0.42 * term * term + 0.045;
   m_precomputedViscosityTerm2 = 1.65 + 91.9 * m_salinity * m_salinity * m_salinity;

   m_omega = 1.5;

   double temperatureValues [ 5 ] = { -40.0, -30.0, -20.0, -10.0, 0.0 };
   double densityValues [ 5 ] = { 922.8, 921.6, 920.3, 918.7, 916.7 };
   double heatCapacityValues [ 5 ] = { 1800.0, 1880.0, 1960.0, 2030.0, 2110.0 }; // 2960.0, 2030.0, 2110.0 };
   double thermalConductivityValues [ 5 ] = { 2.6, 2.5, 2.4, 2.3, 2.14 };

   m_iceDensityInterpolator.setInterpolation ( ibs::PiecewiseInterpolator::PIECEWISE_LINEAR, 5, temperatureValues, densityValues );
   m_iceDensityInterpolator.computeCoefficients ();

   m_iceHeatCapacityInterpolator.setInterpolation ( ibs::PiecewiseInterpolator::PIECEWISE_LINEAR, 5, temperatureValues, heatCapacityValues );
   m_iceHeatCapacityInterpolator.computeCoefficients ();

   m_iceThermalConductivityInterpolator.setInterpolation ( ibs::PiecewiseInterpolator::PIECEWISE_LINEAR, 5, temperatureValues, thermalConductivityValues );
   m_iceThermalConductivityInterpolator.computeCoefficients ();

   m_pressureTerm = 0.0;
   m_salinityTerm = 0.0;
}

GeoPhysics::FluidType::~FluidType () {
}

void GeoPhysics::FluidType::loadPropertyTables () {

   // Load heat-capacity, thermal-conductivity and density tables.
   Interface::FluidHeatCapacitySampleList* heatCapacitySamples;
   Interface::FluidThermalConductivitySampleList* thermalConductivitySamples;
   Interface::FluidDensitySampleList* densitySamples;

   heatCapacitySamples = m_projectHandle->getFluidHeatCapacitySampleList ( m_projectHandle->findFluid ( getHeatCapacityFluidName ()));
   thermalConductivitySamples = m_projectHandle->getFluidThermalConductivitySampleList ( m_projectHandle->findFluid ( getThermalConductivityFluidName ()));
   densitySamples = m_projectHandle->getFluidDensitySampleList ( m_projectHandle->findFluid ( getDensityFluidName ()));
 
   Interface::FluidHeatCapacitySampleList::const_iterator heatCapacitySampleIter;
   Interface::FluidThermalConductivitySampleList::const_iterator thermalConductivitySampleIter;
   Interface::FluidDensitySampleList::const_iterator densitySampleIter;

   for ( heatCapacitySampleIter = heatCapacitySamples->begin ();
         heatCapacitySampleIter != heatCapacitySamples->end ();
         ++heatCapacitySampleIter ) {

      const Interface::FluidHeatCapacitySample * sample = *heatCapacitySampleIter;

      m_heatCapacitytbl.addPoint ( sample->getTemperature (), sample->getPressure (), sample->getHeatCapacity ());
   }

   for ( thermalConductivitySampleIter = thermalConductivitySamples->begin ();
         thermalConductivitySampleIter != thermalConductivitySamples->end ();
         ++thermalConductivitySampleIter ) {

      const Interface::FluidThermalConductivitySample * sample = *thermalConductivitySampleIter;

      m_thermalConductivitytbl.addPoint ( sample->getTemperature (), sample->getThermalConductivity ());
   }

   // This table must be loaded before the density-x-heat-capacity is computed since it may be used in this calculation.
   for ( densitySampleIter = densitySamples->begin ();
         densitySampleIter != densitySamples->end ();
         ++densitySampleIter ) {

      const Interface::FluidDensitySample * sample = *densitySampleIter;

      m_densitytbl.addPoint ( sample->getTemperature (), sample->getPressure (), sample->getDensity ());
   }

   // Would it be better to use the density that will used in the computation?
   for ( heatCapacitySampleIter = heatCapacitySamples->begin ();
         heatCapacitySampleIter != heatCapacitySamples->end ();
         ++heatCapacitySampleIter ) {

      const Interface::FluidHeatCapacitySample * sample = *heatCapacitySampleIter;

      const double temperature = sample->getTemperature ();
      const double pressure    = sample->getPressure ();

      m_densXheatCapacitytbl.addPoint ( temperature, pressure, sample->getHeatCapacity () * densityFromTable ( temperature, pressure ));
   }

   // Load data for permafrost modelling
   if( m_projectHandle->getPermafrostData() != 0 ) {
      if( m_projectHandle->getPermafrostData()->getPressureTerm() ) {
         m_pressureTerm = 0.073;
      }
      if( m_projectHandle->getPermafrostData()->getSalinityTerm() ) {
         m_salinityTerm = 0.064;
      }
    } 

   delete heatCapacitySamples;
   delete thermalConductivitySamples;
   delete densitySamples;
}

double GeoPhysics::FluidType::getLiquidusTemperature ( const double temperature, const double pressure ) const {
   
   // return ( - 0.073 * pressure - 0.064 * salinityConcentration ( temperature, pressure ));

   double p = NumericFunctions::Maximum ( 0.0, pressure );

   // NLSAY3: Debugging
/*   if ( pressure<0 ) {std::cout << "Pressure: " << pressure << " LiquidusTemperature: " << - m_pressureTerm * pressure - m_salinityTerm * salinityConcentration ( temperature, pressure ) << std::endl;}*/

   return ( - m_pressureTerm * p - m_salinityTerm * salinityConcentration ( temperature, p ));

}
    
double GeoPhysics::FluidType::getSolidusTemperature ( const double liquidusTemperature ) const {
   
   // Ts = Tl - omega * sqrt( - log( solidFractionForFrozen ))
   // solidFractionForFrozen = 0.01; omega = 1
   //return liquidusTemperature - 2.14596603; // omega = 1
   //return liquidusTemperature - 4.29193205; // omega = 2
   return liquidusTemperature - 3.21894904; // omega = 1.5
}

double GeoPhysics::FluidType::getConstantDensity () const {
   return m_densityVal;
}

void GeoPhysics::FluidType::setDensityToConstant () {
   m_densityCalculationModel = CBMGenerics::waterDensity::Constant;
}

// double GeoPhysics::FluidType::getSimpleSeismicVelocity () const {
//    return m_seismicVelocityVal;
// }

double GeoPhysics::FluidType::salinityConcentration( const double temperature, const double pressure ) const {
   
   const double p = NumericFunctions::Maximum ( 0.0, pressure );
  
   return densityBatzleWang ( temperature, p, m_salinity ) - densityBatzleWang ( temperature, p, 0.0 );

}

double GeoPhysics::FluidType::density ( const double temperature, const double pressure ) const {

   switch ( m_densityCalculationModel ) {
     case CBMGenerics::waterDensity::Constant   : return m_densityVal;
     case CBMGenerics::waterDensity::Calculated : return densityBatzleWang ( temperature, pressure );
     default : assert ( 0 );
   }

   // Added to prevent compiler warning about missing return at end of function.
   return 0.0;
}

double GeoPhysics::FluidType::densityFromTable ( const double temperature, const double pressure ) const {
   return m_densitytbl.compute ( temperature, pressure );
}

void GeoPhysics::FluidType::correctSimpleDensity ( const double standardDepth,
                                                   const double pressureGradient,
                                                   const double surfaceTemperature,
                                                   const double temperatureGradient ) {

   /// The density value should be changed only if the density model is not const.
   /// The user may have set some particular value for this.
   if ( m_densityCalculationModel != CBMGenerics::waterDensity::Constant ) {

      /// The temperature gradient here is already in C/m and so does not need to be converted from C/Km.
      double temperature = surfaceTemperature + standardDepth * temperatureGradient;

      /// The pressure gradient is in MPa/Km and so needs to be scaled to MPa/m.
      double pressure    = standardDepth * pressureGradient * 0.001;

      // reset the simple-density to the Batzle and Wang fnuction evaluated at the "standard" temperature and pressure.
      m_densityVal = densityBatzleWang ( temperature, pressure );
   }

}

double GeoPhysics::FluidType::computeDensityDerivativeWRTPressure ( const double temperature,
                                                                    const double pressure ) const {

   switch ( m_densityCalculationModel ) {
     case CBMGenerics::waterDensity::Calculated : return computeDensityDerivativeWRTPressureBatzleWang ( temperature, pressure );
     case CBMGenerics::waterDensity::Constant   : return 0.0;
     default : assert ( 0 );
   }

   // Added to prevent compiler warning about missing return at end of function.
   return 0.0;
}

double GeoPhysics::FluidType::computeDensityDerivativeWRTTemperature ( const double temperature,
                                                                       const double pressure ) const {

   switch ( m_densityCalculationModel ) {
     case CBMGenerics::waterDensity::Calculated : return computeDensityDerivativeWRTTemperatureBatzleWang ( temperature, pressure );
     case CBMGenerics::waterDensity::Constant   : return 0.0;
     default : assert ( 0 );
   }

   // Added to prevent compiler warning about missing return at end of function.
   return 0.0;
}

double GeoPhysics::FluidType::viscosity ( const double temperature ) const {

   switch ( getViscosityModel ()) {
     case Interface::BATTZLE_WANG_VISCOSITY : return viscosityBatzleWang ( temperature );
     case Interface::TEMIS_PACK_VISCOSITY   : return viscosityTemisPack ( temperature );
     default:
        assert ( 0 );
   }

   // Added to prevent compiler warning about missing return at end of function.
   return 0.0;
}

double GeoPhysics::FluidType::thermalConductivity ( const double temperature, const double pressure ) const {

   if( m_projectHandle->getPermafrost() ) {  
      const double liquidusTemperature = getLiquidusTemperature( temperature, pressure );
      
      if ( temperature < liquidusTemperature ) {
         const double theta = computeTheta ( temperature, liquidusTemperature );
         
         return pow ( m_iceThermalConductivityInterpolator.evaluate ( temperature ), 1.0 - theta ) *
            pow ( m_thermalConductivitytbl.compute ( temperature, ibs::Interpolator::constant ), theta );
      } 
   }
   return m_thermalConductivitytbl.compute ( temperature, ibs::Interpolator::constant );
}

double GeoPhysics::FluidType::heatCapacity ( const double temperature,
                                             const double pressure ) const {

   return m_heatCapacitytbl.compute ( temperature, pressure, ibs::Interpolator2d::constant );
}

double GeoPhysics::FluidType::densXheatCapacity ( const double temperature,
                                                  const double pressure,
                                                  bool includePermafrost ) const {

   // Lookup the volumetric heat capacity (VHC) of water. 
   const double waterVHC = m_densXheatCapacitytbl.compute ( temperature, pressure, ibs::Interpolator2d::constant );
 
   if( includePermafrost ) {

      // Determine the freezing temperature
      const double liquidusTemperature = getLiquidusTemperature( temperature, pressure );
      
      // When there is no ice, there is only water: return the VHC of water
      if (temperature > liquidusTemperature)
         return waterVHC;

      assert( temperature <= liquidusTemperature );

      // Compute the volumetric heat capacity  of ice
      const double iceVHC = solidDensityTimesHeatCapacity ( temperature );

      // Compute theta: the fraction of the water that is in liquid phase
      const double theta = computeTheta ( temperature, liquidusTemperature );
      const double waterFraction = theta;
      const double iceFraction  = 1.0 - theta;

      // compute the derivative of the ice fraction w.r.t. to temperature
      const double dThetaDT = computeThetaDerivative ( temperature, liquidusTemperature );
      const double iceFractionDerivative = - dThetaDT;

      // now we can compute the latent heat term
      static const double WaterSpecificLatentHeat = 333600.0; // J/kg
      const double iceDensity = m_iceDensityInterpolator.evaluate ( temperature ) ;
      const double latentHeatTerm = iceDensity * WaterSpecificLatentHeat * iceFractionDerivative;
      assert( latentHeatTerm <= 0.0 );

      // return the volumetric heat capacity of the mixture minus the latent heat term
      return waterFraction * waterVHC + iceFraction * iceVHC - latentHeatTerm;
   } 
   else
   {
      // return just the volumetric heat capacity of water
      return  waterVHC;
   }
}

double GeoPhysics::FluidType::seismicVelocity ( const double temperature,
                                                const double pressure ) const {

   switch ( m_seismicVelocityCalculationModel ) {
     case Interface::CALCULATED_MODEL : return seismicVelocityBatzleWang ( temperature, pressure );
     case Interface::CONSTANT_MODEL   : return m_seismicVelocityVal;
     default: assert ( 0 );
   }

   // Added to prevent compiler warning about missing return at end of function.
   return 0.0;
}

double GeoPhysics::FluidType::seismicVelocityBatzleWang ( const double temperature,
                                                          const double pressure ) const {

   const double& t = temperature;
   const double& p = pressure;
   const double& s = m_salinity;

   const double t2 = t * t;
   const double t3 = t2 * t;

   const double p2 = p * p;
   const double p3 = p2 * p;

   double seisvel;
  
   // Use Horners method here, it may improve precision and speed.
   seisvel = 1402.85 + t * ( 4.871 - 0.04783 * t + 1.487e-4 * t2 - 2.197e-7 * t3 );
   seisvel += p * ( 1.524 + 3.437e-3 * p - 1.197e-5 * p2 );
   seisvel += p * t * ( -0.0111 + t * 2.747e-4 - t2 * 6.503e-7 + t3 * 7.987e-10 );
   seisvel += p2 * t * ( 1.739e-4 - 2.135e-6 * t - 1.455e-8 * t2 + 5.230e-11 * t3 );
   seisvel += p3 * t * ( -1.628e-6 + 1.237e-8 * t + 1.327e-10 * t2 - 4.614e-13 * t3 );

   if ( s != 0.0 ) {
      seisvel += s * ( 1170.0 - 9.6 * t + 0.055 * t2 - 8.5e-5 * t3 + 2.6 * p - 0.0029 * p * t - 0.0476 * p2 ) +
         pow ( s, 1.5 ) * ( 780.0 - 10.0 * p + 0.16 * p2 )-820.0 * s * s;
   }
  
   return seisvel;
}

double GeoPhysics::FluidType::densityBatzleWang ( const double temperature,
                                                  const double pressure,
                                                  const double salinity ) const {
   const double& t = temperature;
   const double& p = pressure;
   const double& s = salinity;

   double dens;

   dens = 1000.0 * ( s * s * ( 0.44 - 0.0033 * t )
                     + p * p * ( -3.33e-7 - 2.0e-9 * t )+
                     1.75e-9 * ( -1718.91 + t ) * ( -665.977 + t ) * ( 499.172 + t )+
                     s * ( 0.668 + 0.00008 * t + 3.0e-6 * t * t )+
                     p * ( s * ( 0.0003 - 0.000013 * t ) + s * s * ( -0.0024 + 0.000047 * t )-
                           1.3e-11 * ( -1123.64 + t ) * ( 33476.2 - 107.125 * t + t * t )));
   return dens;

}
double GeoPhysics::FluidType::densityBatzleWang ( const double temperature,
                                                  const double pressure ) const {

   return densityBatzleWang( temperature, pressure, m_salinity );
}

double GeoPhysics::FluidType::computeDensityDerivativeWRTPressureBatzleWang ( const double temperature,
                                                                              const double pressure ) const
{

   const double& t = temperature;
   const double& p = pressure;
   const double& s = m_salinity;

   double densder;

   densder = (1.e-6) * ( 489.0 - 0.666 * p - 2.0 * t - 0.004 * p * t + 0.016 * t * t - 0.000013 * t * t * t );
   densder = 1000.0 * ( densder + (1e-6) * s * ( 300.0 - 2400.0 * s - 13.0 * t + 47.0 * s * t ));

   return densder;
}

double GeoPhysics::FluidType::computeDensityDerivativeWRTTemperatureBatzleWang ( const double temperature,
                                                                                 const double pressure ) const
{

   const double& t = temperature;
   const double& p = pressure;
   const double& s = m_salinity;

   double densder;

   densder = (1.e-6)*(-80.0 - 2.0 * p - 0.002 * p * p - 6.6 * t + 0.032 * p * t + 0.00525 * t * t - 0.000039 * p * t * t );
   densder = 1000.0 * ( densder + (1.e-6) * s * ( 80.0 - 13.0 * p - 3300.0 * s + 6.0 * t + 47.0 * p * s ));

   return densder;
}

double GeoPhysics::FluidType::viscosityBatzleWang ( const double temperature ) const {

   const double  t = NumericFunctions::Maximum<double> ( 0.0, temperature );
   const double& s = m_salinity;

   //
   // For expression see:
   //
   // Michael Batzle and Zhijing Wang,
   // Seismic properties of pore fluids.
   double visc;

   double Term1 = pow ( s, 0.8 ) - 0.17;

   double Term2 = ( 0.42 * Term1 * Term1 + 0.045 ) * pow ( t, 0.8 );

   visc = 0.1 + 0.333 * s + (1.65 + 91.9 * s * s * s ) * exp ( -Term2 );

   return 0.001 * visc;


#if 0
   const double  t = NumericFunctions::Maximum<double> ( 0.0, temperature );
   const double& s = m_salinity;

   //
   // For expression see:
   //
   // Michael Batzle and Zhijing Wang,
   // Seismic properties of pore fluids.
   //
   double visc;


//    double Term1 = pow ( s, 0.8 ) - 0.17;
//    double Term2 = ( 0.42 * Term1 * Term1 + 0.045 ) * pow ( t, 0.8 );
//    visc = 0.1 + 0.333 * s + (1.65 + 91.9 * s * s * s ) * exp ( -Term2 );

   double Term2 = m_precomputedViscosityTerm1 * pow ( t, 0.8 );

   visc = 0.1 + 0.333 * s + m_precomputedViscosityTerm2 * exp ( -Term2 );

   return 0.001 * visc;
#endif
}


double GeoPhysics::FluidType::viscosityTemisPack ( const double temperature ) const {

   double visc;

   visc = 0.0145 * std::exp ( 1260.0 / ( temperature + CelciusToKelvin ));
   return 0.001 * visc;
}

void GeoPhysics::FluidType::asString ( std::string& str ) const {

   std::ostringstream buf;

   buf << "GeoPhysics::FluidType: ";
   buf << getName () << std::endl;
   buf << " Density calculation mode : " 
       << ( m_densityCalculationModel == CBMGenerics::waterDensity::Calculated ? "Batzle-Wang" : "Constant" ) << std::endl;
   buf << " Simple density           : " << m_densityVal << std::endl;
   buf << " Seismic velocity mode    : " 
       << ( m_seismicVelocityCalculationModel == DataAccess::Interface::CALCULATED_MODEL ? "Batzle-Wang" : "Constant" ) << std::endl;
   buf << " Simple Seismic velocity  : " << m_seismicVelocityVal << std::endl;
   buf << " Salinity                 : " << m_salinity << std::endl;

   str = buf.str ();

}

double GeoPhysics::FluidType::solidDensityTimesHeatCapacity ( const double temperature ) const {

   double usedTemperature = temperature; //NumericFunctions::Maximum ( temperature, 40.0 );

   // return 916.0 * 2110.0;
   return m_iceDensityInterpolator.evaluate ( usedTemperature ) * m_iceHeatCapacityInterpolator.evaluate ( usedTemperature );
}

double GeoPhysics::FluidType::computeTheta ( const double temperature, const double liquidusTemperature ) const {

   if ( temperature < liquidusTemperature ) { 
      const double temp = ( temperature - liquidusTemperature ) / m_omega; // not necessarily divide by m_omega ( = 1.0 )
      return exp ( -temp * temp );
   } else {
      return 1.0;
   }
}

double GeoPhysics::FluidType::computeThetaDerivative ( const double temperature, const double liquidusTemperature ) const {

   if ( temperature < liquidusTemperature ) { 
      const double temp = ( temperature - liquidusTemperature ) / m_omega; // not necessarily divide by m_omega ( = 1.0 )
      return -2.0 * temp / m_omega * exp ( -temp * temp );
   } else {
      return 0;
   }
}

double GeoPhysics::FluidType::relativePermeability ( const double temperature, const double pressure ) const {
 
/*   if( m_projectHandle->getPermafrost() ) {
      
      const double liquidusTemperature = getLiquidusTemperature( temperature, NumericFunctions::Maximum ( 0.0, pressure ) );
      const double solidusTemperature  = liquidusTemperature - 10.0; // = getSolidusTemperature( liquidusTemperature ); // NLSAY3: improve stability by extending the temperature range
      
      if( temperature >= liquidusTemperature ) {
         return 1.0;
      } else if( temperature <= solidusTemperature ) {
         return 1.0e-6;
      } else {
         if( solidusTemperature != liquidusTemperature ) {
            return ( - (1.0-1.0e-6) / ( solidusTemperature - liquidusTemperature )) * ( temperature - liquidusTemperature ) + 1.0;
         } 
      }
  }*/
   return 1.0;
}

bool GeoPhysics::FluidType::SwitchPermafrost() const {

   return m_projectHandle->getPermafrost();
}
