//                                                                      
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

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

// utilities library
#include "LogHandler.h"

using namespace DataAccess;

const double GeoPhysics::FluidType::DefaultHydrostaticPressureGradient              = 10.0;
const double GeoPhysics::FluidType::StandardSurfaceTemperature                      = 10.0;
const double GeoPhysics::FluidType::DefaultStandardDepth                            = 2000.0;
const double GeoPhysics::FluidType::DefaultThermalConductivityCorrectionTemperature = 20.0;

GeoPhysics::FluidType::FluidType (Interface::ProjectHandle * projectHandle, database::Record * record) : DataAccess::Interface::FluidType (projectHandle, record) {

   m_densityVal = Interface::FluidType::density ();
   m_salinity = salinity ();
   m_densityCalculationModel = fluidDensityModel ();

   m_seismicVelocityVal = getConstantSeismicVelocity ();
   m_seismicVelocityCalculationModel = getSeismicVelocityCalculationModel ();

   double term = pow (m_salinity, 0.8) - 0.17;

   m_precomputedViscosityTerm1 = 0.42 * term * term + 0.045;
   m_precomputedViscosityTerm2 = 1.65 + 91.9 * m_salinity * m_salinity * m_salinity;

   m_omega = 1.5;

   double temperatureValues [ 5 ] = { -40.0, -30.0, -20.0, -10.0, 0.0 };
   double densityValues [ 5 ] = { 922.8, 921.6, 920.3, 918.7, 916.7 };
   double heatCapacityValues [ 5 ] = { 1800.0, 1880.0, 1960.0, 2030.0, 2110.0 }; // 2960.0, 2030.0, 2110.0 };
   double thermalConductivityValues [ 5 ] = { 2.6, 2.5, 2.4, 2.3, 2.14 };

   m_iceDensityInterpolator.setInterpolation ( 5, temperatureValues, densityValues);
   m_iceHeatCapacityInterpolator.setInterpolation ( 5, temperatureValues, heatCapacityValues);
   m_iceThermalConductivityInterpolator.setInterpolation ( 5, temperatureValues, thermalConductivityValues);

   m_pressureTerm = 0.0;
   m_salinityTerm = 0.0;
}

GeoPhysics::FluidType::~FluidType () {
}

void GeoPhysics::FluidType::loadPropertyTables ()
{
   // Load heat-capacity, thermal-conductivity and density tables.
   Interface::FluidHeatCapacitySampleList* heatCapacitySamples;
   Interface::FluidThermalConductivitySampleList* thermalConductivitySamples;

   heatCapacitySamples = m_projectHandle->getFluidHeatCapacitySampleList (m_projectHandle->findFluid (getHeatCapacityFluidName ()));

   thermalConductivitySamples = m_projectHandle->getFluidThermalConductivitySampleList (m_projectHandle->findFluid (getThermalConductivityFluidName ()));
   if ((*thermalConductivitySamples).size () != GeoPhysics::BrineConductivity::s_thCondArraySize)
   {
      throw formattingexception::GeneralException() << "\nMeSsAgE ERROR  Size of FltThCondIoTbl in project file is not correct\n\n";
   }

   Interface::FluidHeatCapacitySampleList::const_iterator heatCapacitySampleIter;
   Interface::FluidThermalConductivitySampleList::const_iterator thermalConductivitySampleIter;

   for (heatCapacitySampleIter = heatCapacitySamples->begin ();
         heatCapacitySampleIter != heatCapacitySamples->end ();
         ++heatCapacitySampleIter) {

      const Interface::FluidHeatCapacitySample * sample = *heatCapacitySampleIter;

      m_heatCapacitytbl.addPoint (sample->getTemperature (), sample->getPressure (), sample->getHeatCapacity ());
   }

   ibs::Interpolator2d thermalConductivitytbl;

   for (thermalConductivitySampleIter = thermalConductivitySamples->begin ();
         thermalConductivitySampleIter != thermalConductivitySamples->end ();
         ++thermalConductivitySampleIter) {

      const Interface::FluidThermalConductivitySample * sample = *thermalConductivitySampleIter;

      thermalConductivitytbl.addPoint (sample->getTemperature (), sample->getPressure (), sample->getThermalConductivity ());
   }

   // Pass the table to the m_conductivity object
   m_conductivity.setTable (thermalConductivitytbl);

   // Load data for permafrost modelling
   if(m_projectHandle->getPermafrostData() != 0) {
      if(m_projectHandle->getPermafrostData()->getPressureTerm()) {
         m_pressureTerm = 0.073;
      }
      if(m_projectHandle->getPermafrostData()->getSalinityTerm()) {
         m_salinityTerm = 0.064;
      }
    }

   delete heatCapacitySamples;
   delete thermalConductivitySamples;
}

double GeoPhysics::FluidType::getLiquidusTemperature (const double temperature, const double pressure) const {

   double p = NumericFunctions::Maximum (0.0, pressure);

   return (- m_pressureTerm * p - m_salinityTerm * salinityConcentration (temperature, p));
}

double GeoPhysics::FluidType::getSolidusTemperature (const double liquidusTemperature) const {

   // Ts = Tl - omega * sqrt(- log(solidFractionForFrozen))
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

double GeoPhysics::FluidType::salinityConcentration(const double temperature, const double pressure) const {

   const double p = NumericFunctions::Maximum (0.0, pressure);

   return m_density.phaseChange (temperature, p, m_salinity) - m_density.phaseChange (temperature, p, 0.0);

}

double GeoPhysics::FluidType::density (const double temperature, const double pressure) const {

   switch (m_densityCalculationModel) {
     case CBMGenerics::waterDensity::Constant   : return m_densityVal;
     case CBMGenerics::waterDensity::Calculated : return m_density.phaseChange (temperature, pressure, m_salinity);
     default : assert (0);
   }

   // Added to prevent compiler warning about missing return at end of function.
   return 0.0;
}

void GeoPhysics::FluidType::correctSimpleDensity (const double standardDepth,
                                                   const double pressureGradient,
                                                   const double surfaceTemperature,
                                                   const double temperatureGradient) {
   m_densityVal = getCorrectedSimpleDensity (standardDepth, pressureGradient, surfaceTemperature, temperatureGradient);
}

double GeoPhysics::FluidType::getCorrectedSimpleDensity (const double standardDepth,
                                                          const double pressureGradient,
                                                          const double surfaceTemperature,
                                                          const double temperatureGradient) const {

   double result;

   /// The density value should be changed only if the density model is not const.
   /// The user may have set some particular value for this.
   if (m_densityCalculationModel != CBMGenerics::waterDensity::Constant) {

      /// The temperature gradient here is already in C/m and so does not need to be converted from C/Km.
      double temperature = surfaceTemperature + standardDepth * temperatureGradient;

      /// The pressure gradient is in MPa/Km and so needs to be scaled to MPa/m.
      double pressure    = standardDepth * pressureGradient * 0.001;

      // Reset the simple-density using the density calculator. Evaluation at the "standard" temperature and pressure.
      result = m_density.phaseChange (temperature, pressure, m_salinity);
   } else {
      result = m_densityVal;
   }

   return result;
}


double GeoPhysics::FluidType::computeDensityDerivativeWRTPressure (const double temperature,
                                                                    const double pressure) const {

   switch (m_densityCalculationModel) {
     case CBMGenerics::waterDensity::Calculated : return m_density.computeDerivativeP (temperature, pressure, m_salinity);
     case CBMGenerics::waterDensity::Constant   : return 0.0;
     default : assert (0);
   }

   // Added to prevent compiler warning about missing return at end of function.
   return 0.0;
}

double GeoPhysics::FluidType::computeDensityDerivativeWRTTemperature (const double temperature,
                                                                       const double pressure) const {

   switch (m_densityCalculationModel) {
     case CBMGenerics::waterDensity::Calculated : return m_density.computeDerivativeT (temperature, pressure, m_salinity);
     case CBMGenerics::waterDensity::Constant   : return 0.0;
     default : assert (0);
   }

   // Added to prevent compiler warning about missing return at end of function.
   return 0.0;
}

double GeoPhysics::FluidType::viscosity (const double temperature, const double pressure) const {

	return m_viscosity.phaseChange (temperature, pressure, m_salinity);
}

double GeoPhysics::FluidType::thermalConductivity (const double temperature, const double pressure) const {

   if(m_projectHandle->getPermafrost()) {
      const double liquidusTemperature = getLiquidusTemperature(temperature, pressure);

      if (temperature < liquidusTemperature) {
         const double theta = computeTheta (temperature, liquidusTemperature);

         return pow (m_iceThermalConductivityInterpolator.evaluate (temperature), 1.0 - theta) *
            pow (m_conductivity.phaseChange (temperature, pressure, 0.0), theta);
      }
   }
   return m_conductivity.phaseChange (temperature, pressure, 0.0);
}

double GeoPhysics::FluidType::heatCapacity (const double temperature,
                                             const double pressure) const {

   return m_heatCapacitytbl.compute (temperature, pressure, ibs::Interpolator2d::constant);
}

double GeoPhysics::FluidType::densXheatCapacity (const double temperature,
                                                  const double pressure,
                                                  bool includePermafrost) const {

	// Calculate the volumetric heat capacity (VHC) of water. Salinity is taken into account through density.
   const double waterVHC = m_heatCapacitytbl.compute(temperature, pressure, ibs::Interpolator2d::constant) * density(temperature, pressure);

   if(includePermafrost) {

      // Determine the freezing temperature
      const double liquidusTemperature = getLiquidusTemperature(temperature, pressure);

      // When there is no ice, there is only water: return the VHC of water
      if (temperature > liquidusTemperature)
         return waterVHC;

      assert(temperature <= liquidusTemperature);

      // Compute the volumetric heat capacity  of ice
      const double iceVHC = solidDensityTimesHeatCapacity (temperature);

      // Compute theta: the fraction of the water that is in liquid phase
      const double theta = computeTheta (temperature, liquidusTemperature);
      const double waterFraction = theta;
      const double iceFraction  = 1.0 - theta;

      // compute the derivative of the ice fraction w.r.t. to temperature
      const double dThetaDT = computeThetaDerivative (temperature, liquidusTemperature);
      const double iceFractionDerivative = - dThetaDT;

      // now we can compute the latent heat term
      static const double WaterSpecificLatentHeat = 333600.0; // J/kg
      const double iceDensity = m_iceDensityInterpolator.evaluate (temperature) ;
      const double latentHeatTerm = iceDensity * WaterSpecificLatentHeat * iceFractionDerivative;
      assert(latentHeatTerm <= 0.0);

      // return the volumetric heat capacity of the mixture minus the latent heat term
      return waterFraction * waterVHC + iceFraction * iceVHC - latentHeatTerm;
   }
   else
   {
      // return just the volumetric heat capacity of water
      return  waterVHC;
   }
}

double GeoPhysics::FluidType::seismicVelocity (const double temperature,
                                                const double pressure) const {

   switch (m_seismicVelocityCalculationModel) {
     case Interface::CALCULATED_MODEL : return m_velocity.phaseChange (temperature, pressure, m_salinity);
     case Interface::CONSTANT_MODEL   : return m_seismicVelocityVal;
     default: assert (0);
   }

   // Added to prevent compiler warning about missing return at end of function.
   return 0.0;
}

double GeoPhysics::FluidType::solidDensityTimesHeatCapacity (const double temperature) const {

   double usedTemperature = temperature; //NumericFunctions::Maximum (temperature, 40.0);

   // return 916.0 * 2110.0;
   return m_iceDensityInterpolator.evaluate (usedTemperature) * m_iceHeatCapacityInterpolator.evaluate (usedTemperature);
}

double GeoPhysics::FluidType::computeTheta (const double temperature, const double liquidusTemperature) const {

   if (temperature < liquidusTemperature) {
      const double temp = (temperature - liquidusTemperature) / m_omega; // not necessarily divide by m_omega (= 1.0)
      return exp (-temp * temp);
   } else {
      return 1.0;
   }
}

double GeoPhysics::FluidType::computeThetaDerivative (const double temperature, const double liquidusTemperature) const {

   if (temperature < liquidusTemperature) {
      const double temp = (temperature - liquidusTemperature) / m_omega; // not necessarily divide by m_omega (= 1.0)
      return -2.0 * temp / m_omega * exp (-temp * temp);
   } else {
      return 0;
   }
}

double GeoPhysics::FluidType::relativePermeability (const double temperature, const double pressure) const {
   return 1.0;
}

bool GeoPhysics::FluidType::SwitchPermafrost() const {

   return m_projectHandle->getPermafrost();
}
