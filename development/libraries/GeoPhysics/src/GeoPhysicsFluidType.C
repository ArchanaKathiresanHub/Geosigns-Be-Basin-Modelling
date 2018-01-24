//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <algorithm>
#include "GeoPhysicsFluidType.h"
#include "GeoPhysicalConstants.h"

#include "BrinePhases.h"
#include "BrineConductivity.h"
#include "BrineDensity.h"
#include "BrineVelocity.h"
#include "BrineViscosity.h"
#include "IBSinterpolator2d.h"
#include "Interface/FluidHeatCapacitySample.h"
#include "Interface/FluidThermalConductivitySample.h"
#include "Interface/FluidDensitySample.h"
#include "Interface/FluidType.h"
#include "Interface/Interface.h"
#include "Interface/PermafrostEvent.h"
#include "PiecewiseInterpolator.h"

// utilities library
#include "LogHandler.h"

using namespace DataAccess;

const double GeoPhysics::FluidType::DefaultHydrostaticPressureGradient              = 10.0;
const double GeoPhysics::FluidType::StandardSurfaceTemperature                      = 10.0;
const double GeoPhysics::FluidType::DefaultStandardDepth                            = 2000.0;
const double GeoPhysics::FluidType::DefaultThermalConductivityCorrectionTemperature = 20.0;

GeoPhysics::FluidType::FluidType (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record) :
   DataAccess::Interface::FluidType (projectHandle, record),
   m_heatCapacitytbl( new ibs::Interpolator2d ),
   m_seismicVelocityCalculationModel( getSeismicVelocityCalculationModel() ),
   m_densityCalculationModel( fluidDensityModel() ),
   m_densityVal( Interface::FluidType::density() ),
   m_salinity( salinity() ),
   m_seismicVelocityVal( getConstantSeismicVelocity() ),
   m_hasPermafrost( projectHandle->getPermafrost() && (projectHandle->getPermafrostData() != nullptr) ),
   m_pressureTerm( (m_hasPermafrost && projectHandle->getPermafrostData()->getPressureTerm()) ? 0.073 : 0.0 ),
   m_salinityTerm( (m_hasPermafrost && projectHandle->getPermafrostData()->getSalinityTerm()) ? 0.064 : 0.0 ),
   m_conductivity( new GeoPhysics::Brine::Conductivity() ),
   m_density( new GeoPhysics::Brine::Density() ),
   m_velocity( new GeoPhysics::Brine::Velocity() ),
   m_viscosity( new GeoPhysics::Brine::Viscosity(GeoPhysics::Brine::PhaseStateScalar(m_salinity).getSalinity()) ),
   m_iceDensityInterpolator( new ibs::PiecewiseInterpolator ),
   m_iceHeatCapacityInterpolator( new ibs::PiecewiseInterpolator ),
   m_iceThermalConductivityInterpolator( new ibs::PiecewiseInterpolator ),
   m_omega( 1.5 )
{
   const double temperatureValues [ 5 ] = { -40.0, -30.0, -20.0, -10.0, 0.0 };
   const double densityValues [ 5 ] = { 922.8, 921.6, 920.3, 918.7, 916.7 };
   const double heatCapacityValues [ 5 ] = { 1800.0, 1880.0, 1960.0, 2030.0, 2110.0 };
   const double thermalConductivityValues [ 5 ] = { 2.6, 2.5, 2.4, 2.3, 2.14 };

   m_iceDensityInterpolator->setInterpolation ( 5, temperatureValues, densityValues);
   m_iceHeatCapacityInterpolator->setInterpolation ( 5, temperatureValues, heatCapacityValues);
   m_iceThermalConductivityInterpolator->setInterpolation ( 5, temperatureValues, thermalConductivityValues);
}

GeoPhysics::FluidType::~FluidType ()
{
   // Intentionally left blank
}

void GeoPhysics::FluidType::loadPropertyTables ()
{
   // Load heat-capacity, thermal-conductivity and density tables.
   Interface::FluidHeatCapacitySampleList* heatCapacitySamples;
   Interface::FluidThermalConductivitySampleList* thermalConductivitySamples;

   heatCapacitySamples = m_projectHandle->getFluidHeatCapacitySampleList (m_projectHandle->findFluid (getHeatCapacityFluidName ()));

   thermalConductivitySamples = m_projectHandle->getFluidThermalConductivitySampleList (m_projectHandle->findFluid (getThermalConductivityFluidName ()));
   if ((*thermalConductivitySamples).size () != GeoPhysics::Brine::Conductivity::s_thCondArraySize)
   {
      throw formattingexception::GeneralException() << "\nBasin_Error:  Size of FltThCondIoTbl in project file is not correct\n\n";
   }

   Interface::FluidHeatCapacitySampleList::const_iterator heatCapacitySampleIter;
   Interface::FluidThermalConductivitySampleList::const_iterator thermalConductivitySampleIter;

   for (heatCapacitySampleIter = heatCapacitySamples->begin ();
         heatCapacitySampleIter != heatCapacitySamples->end ();
         ++heatCapacitySampleIter)
   {
      const Interface::FluidHeatCapacitySample * sample = *heatCapacitySampleIter;
      m_heatCapacitytbl->addPoint (sample->getTemperature (), sample->getPressure (), sample->getHeatCapacity ());
   }

   ibs::Interpolator2d thermalConductivitytbl;

   for (thermalConductivitySampleIter = thermalConductivitySamples->begin ();
         thermalConductivitySampleIter != thermalConductivitySamples->end ();
         ++thermalConductivitySampleIter)
   {
      const Interface::FluidThermalConductivitySample * sample = *thermalConductivitySampleIter;
      thermalConductivitytbl.addPoint (sample->getTemperature (), sample->getPressure (), sample->getThermalConductivity ());
   }

   // Pass the table to the m_conductivity object
   m_conductivity->setTable(thermalConductivitytbl);

   delete heatCapacitySamples;
   delete thermalConductivitySamples;
}

double GeoPhysics::FluidType::getLiquidusTemperature( const double temperature, const double pressure ) const
{
   double p = std::max(0.0, pressure);
   return (- m_pressureTerm * p - m_salinityTerm * salinityConcentration( temperature, p ) );
}

double GeoPhysics::FluidType::getSolidusTemperature (const double liquidusTemperature) const
{
   // Ts = Tl - omega * sqrt(- log(solidFractionForFrozen))
   // solidFractionForFrozen = 0.01; omega = 1
   //return liquidusTemperature - 2.14596603; // omega = 1
   //return liquidusTemperature - 4.29193205; // omega = 2
   return liquidusTemperature - 3.21894904; // omega = 1.5
}

double GeoPhysics::FluidType::salinityConcentration( const double temperature, const double pressure ) const
{
   GeoPhysics::Brine::PhaseStateScalar phase( m_salinity );
   phase.set( temperature, pressure );
   GeoPhysics::Brine::PhaseStateScalar phaseZeroSal( 0.0 );
   phaseZeroSal.set( temperature, pressure );
   return m_density->get(phase) - m_density->get(phaseZeroSal);
}

double GeoPhysics::FluidType::density( const double temperature, const double pressure ) const
{
   switch (m_densityCalculationModel)
   {
     case CBMGenerics::waterDensity::Constant   : return m_densityVal;
     case CBMGenerics::waterDensity::Calculated :
     {
        GeoPhysics::Brine::PhaseStateScalar phase( m_salinity );
        phase.set( temperature, pressure );
        return m_density->get(phase);
     }
     default : throw formattingexception::GeneralException() << "\nBasin_Error:  " << __FUNCTION__
                                                             << " - Density calculation model not set\n\n";
   }

   // Added to prevent compiler warning about missing return at end of function.
   return 0.0;
}

void GeoPhysics::FluidType::density( const GeoPhysics::Brine::PhaseStateVec & phases,
                                     ArrayDefs::Real_ptr density ) const
{
   switch (m_densityCalculationModel)
   {
     case CBMGenerics::waterDensity::Constant   :
        std::fill_n(density, phases.getVectorSize(), m_densityVal);
        break;

     case CBMGenerics::waterDensity::Calculated :
        m_density->get( phases, density );
        break;

     default : throw formattingexception::GeneralException() << "\nBasin_Error:  " << __FUNCTION__
                                                             << " - Density calculation model not set\n\n";
   }
}

void GeoPhysics::FluidType::correctSimpleDensity( const double standardDepth,
                                                  const double pressureGradient,
                                                  const double surfaceTemperature,
                                                  const double temperatureGradient )
{
   m_densityVal = getCorrectedSimpleDensity (standardDepth, pressureGradient, surfaceTemperature, temperatureGradient);
}

double GeoPhysics::FluidType::getCorrectedSimpleDensity( const double standardDepth,
                                                         const double pressureGradient,
                                                         const double surfaceTemperature,
                                                         const double temperatureGradient ) const
{
   double result;

   /// The density value should be changed only if the density model is not const.
   /// The user may have set some particular value for this.
   if (m_densityCalculationModel != CBMGenerics::waterDensity::Constant)
   {
      /// The temperature gradient here is already in C/m and so does not need to be converted from C/Km.
      double temperature = surfaceTemperature + standardDepth * temperatureGradient;

      /// The pressure gradient is in MPa/Km and so needs to be scaled to MPa/m.
      double pressure    = standardDepth * pressureGradient * 0.001;

      // Reset the simple-density using the density calculator. Evaluation at the "standard" temperature and pressure.
      GeoPhysics::Brine::PhaseStateScalar phase( m_salinity );
      phase.set(temperature, pressure);
      result = m_density->get(phase);
   } else {
      result = m_densityVal;
   }

   return result;
}


double GeoPhysics::FluidType::computeDensityDerivativeWRTPressure( const double temperature, const double pressure ) const
{
   switch (m_densityCalculationModel)
   {
     case CBMGenerics::waterDensity::Calculated :
     {
        GeoPhysics::Brine::PhaseStateScalar phase( m_salinity );
        phase.set(temperature, pressure);
        return m_density->computeDerivativeP( phase );
     }
     case CBMGenerics::waterDensity::Constant   : return 0.0;
     default : throw formattingexception::GeneralException() << "\nBasin_Error:  " << __FUNCTION__
                                                             << " - Density calculation model not set\n\n";
   }

   // Added to prevent compiler warning about missing return at end of function.
   return 0.0;
}


void GeoPhysics::FluidType::computeDensityDerivativeWRTPressure( const GeoPhysics::Brine::PhaseStateVec & phases,
                                                                 ArrayDefs::Real_ptr densityDerivative ) const
{
   switch (m_densityCalculationModel)
   {
     case CBMGenerics::waterDensity::Calculated :
        m_density->computeDerivativeP( phases, densityDerivative );
        break;

     case CBMGenerics::waterDensity::Constant   :
        std::fill_n(densityDerivative, phases.getVectorSize(), 0.0);
        break;

     default : throw formattingexception::GeneralException() << "\nBasin_Error:  " << __FUNCTION__
                                                             << " - Density calculation model not set\n\n";
   }
}

double GeoPhysics::FluidType::computeDensityDerivativeWRTTemperature( const double temperature, const double pressure ) const
{
   switch (m_densityCalculationModel)
   {
     case CBMGenerics::waterDensity::Calculated :
     {
        GeoPhysics::Brine::PhaseStateScalar phase( m_salinity );
        phase.set( temperature, pressure );
        return m_density->computeDerivativeT( phase );
     }
     case CBMGenerics::waterDensity::Constant   : return 0.0;
     default : throw formattingexception::GeneralException() << "\nBasin_Error:  " << __FUNCTION__
                                                             << " - Density calculation model not set\n\n";
   }

   // Added to prevent compiler warning about missing return at end of function.
   return 0.0;
}

void GeoPhysics::FluidType::computeDensityDerivativeWRTTemperature( const GeoPhysics::Brine::PhaseStateVec & phases,
                                                                    ArrayDefs::Real_ptr densityDerivative ) const
{
   switch (m_densityCalculationModel)
   {
     case CBMGenerics::waterDensity::Calculated :
        m_density->computeDerivativeT( phases, densityDerivative );
        break;

     case CBMGenerics::waterDensity::Constant   :
        std::fill_n(densityDerivative, phases.getVectorSize(), 0.0);
        break;

     default : throw formattingexception::GeneralException() << "\nBasin_Error:  " << __FUNCTION__
                                                             << " - Density calculation model not set\n\n";
   }
}

double GeoPhysics::FluidType::viscosity( const double temperature, const double pressure ) const
{
   GeoPhysics::Brine::PhaseStateScalar phase( m_salinity );
   phase.set( temperature, pressure );
   return m_viscosity->get( phase );
}

void GeoPhysics::FluidType::viscosity( const GeoPhysics::Brine::PhaseStateVec & phases,
                                       ArrayDefs::Real_ptr viscosity ) const
{
   return m_viscosity->get( phases, viscosity);
}

double GeoPhysics::FluidType::thermalConductivity( const double temperature, const double pressure ) const
{
   GeoPhysics::Brine::PhaseStateScalar phaseZeroSal( 0.0 );
   phaseZeroSal.set( temperature, pressure );
   if( m_hasPermafrost )
   {
      const double liquidusTemperature = getLiquidusTemperature( temperature, pressure );
      if (temperature < liquidusTemperature)
      {
         const double theta = computeTheta (temperature, liquidusTemperature);
         return std::pow (m_iceThermalConductivityInterpolator->evaluate (temperature), 1.0 - theta) *
                std::pow (m_conductivity->get( phaseZeroSal ), theta);
      }
   }
   return m_conductivity->get( phaseZeroSal );
}

void GeoPhysics::FluidType::thermalConductivity( const GeoPhysics::Brine::PhaseStateVec & phases,
                                                 ArrayDefs::Real_ptr thConductivty ) const
{
   const int n = phases.getVectorSize();
   const ArrayDefs::Real_ptr temperature = phases.getTemperature();
   const ArrayDefs::Real_ptr pressure    = phases.getPressure();
   for( int i = 0; i < n; ++i )
   {
      thConductivty[i] = thermalConductivity( temperature[i], pressure[i] );
   }
}

double GeoPhysics::FluidType::heatCapacity( const double temperature, const double pressure ) const
{
   return m_heatCapacitytbl->compute(temperature, pressure, ibs::Interpolator2d::constant);
}

void GeoPhysics::FluidType::heatCapacity( const GeoPhysics::Brine::PhaseStateVec & phases,
                                          ArrayDefs::Real_ptr heatCapacity ) const
{
   const int n = phases.getVectorSize();
   const ArrayDefs::Real_ptr temperature = phases.getTemperature();
   const ArrayDefs::Real_ptr pressure    = phases.getPressure();
   for( int i = 0; i < n; ++i )
   {
      heatCapacity[i] = m_heatCapacitytbl->compute( temperature[i], pressure[i], ibs::Interpolator2d::constant );
   }
}

double GeoPhysics::FluidType::densXheatCapacity( const double temperature,
                                                 const double pressure ) const
{
   return densXheatCapacity ( density ( temperature, pressure ), temperature, pressure );
}

double GeoPhysics::FluidType::densXheatCapacity( const double fluidDensity,
                                                 const double temperature,
                                                 const double pressure ) const
{
   // Calculate the volumetric heat capacity (VHC) of water. Salinity is taken into account through density.
   const double waterVHC = m_heatCapacitytbl->compute( temperature, pressure, ibs::Interpolator2d::constant )
                         * fluidDensity;

   if( m_hasPermafrost )
   {
      // Determine the freezing temperature
      const double liquidusTemperature = getLiquidusTemperature( temperature, pressure );

      // When there is no ice, there is only water: return the VHC of water
      if (temperature > liquidusTemperature)
         return waterVHC;

      assert(temperature <= liquidusTemperature);

      // Compute the volumetric heat capacity  of ice
      const double iceVHC = solidDensityTimesHeatCapacity(temperature);

      // Compute theta: the fraction of the water that is in liquid phase
      const double waterFraction = computeTheta(temperature, liquidusTemperature);
      const double iceFraction  = 1.0 - waterFraction;

      // compute the derivative of the ice fraction w.r.t. to temperature
      const double dThetaDT = computeThetaDerivative(temperature, liquidusTemperature);
      const double iceFractionDerivative = - dThetaDT;

      // now we can compute the latent heat term
      const double waterSpecificLatentHeat = 333600.0; // J/kg
      const double iceDensity = m_iceDensityInterpolator->evaluate(temperature);
      const double latentHeatTerm = iceDensity * waterSpecificLatentHeat * iceFractionDerivative;
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

void GeoPhysics::FluidType::densXheatCapacity( const GeoPhysics::Brine::PhaseStateVec & phases,
                                               ArrayDefs::Real_ptr densXheatCap ) const
{
   const int n = phases.getVectorSize();
   const ArrayDefs::Real_ptr temperature = phases.getTemperature();
   const ArrayDefs::Real_ptr pressure    = phases.getPressure();
   for( int i = 0; i < n; ++i )
   {
      densXheatCap[i] = densXheatCapacity( temperature[i], pressure[i] );
   }
}

void GeoPhysics::FluidType::densXheatCapacity( const GeoPhysics::Brine::PhaseStateVec & phases,
                                               ArrayDefs::ConstReal_ptr fluidDensity,
                                               ArrayDefs::Real_ptr densXheatCap ) const
{
   const int n = phases.getVectorSize();
   const ArrayDefs::Real_ptr temperature = phases.getTemperature();
   const ArrayDefs::Real_ptr pressure    = phases.getPressure();

   if ( m_hasPermafrost ) {

      for( int i = 0; i < n; ++i )
      {
         densXheatCap[i] = densXheatCapacity ( fluidDensity [ i ], temperature[i], pressure[i] );
      }

   } else {

      for( int i = 0; i < n; ++i )
      {
         const double heatCapacity = m_heatCapacitytbl->compute( temperature [ i ], pressure [ i ], ibs::Interpolator2d::constant );
         densXheatCap[i] = fluidDensity [ i ] * heatCapacity;
      }

   }
}

double GeoPhysics::FluidType::seismicVelocity( const double temperature, const double pressure ) const
{
   switch (m_seismicVelocityCalculationModel)
   {
     case Interface::CALCULATED_MODEL :
     {
        GeoPhysics::Brine::PhaseStateScalar phase( m_salinity );
        phase.set( temperature, pressure );
        return m_velocity->get( phase );
     }
     case Interface::CONSTANT_MODEL   : return m_seismicVelocityVal;
     default: throw formattingexception::GeneralException() << "\nBasin_Error:  " << __FUNCTION__
                                                            << " - Seismic velocity calculation model not set\n\n";
   }

   // Added to prevent compiler warning about missing return at end of function.
   return 0.0;
}

void GeoPhysics::FluidType::seismicVelocity( const GeoPhysics::Brine::PhaseStateVec & phases,
                                             ArrayDefs::Real_ptr & seismicVelocity ) const
{
   switch (m_seismicVelocityCalculationModel)
   {
     case Interface::CALCULATED_MODEL : m_velocity->get( phases, seismicVelocity );
     case Interface::CONSTANT_MODEL   : std::fill_n(seismicVelocity, phases.getVectorSize(), m_seismicVelocityVal);
     default: throw formattingexception::GeneralException() << "\nBasin_Error:  " << __FUNCTION__
                                                            << " - Seismic velocity calculation model not set\n\n";
   }
}

double GeoPhysics::FluidType::solidDensityTimesHeatCapacity (const double temperature) const
{
   return m_iceDensityInterpolator->evaluate(temperature) * m_iceHeatCapacityInterpolator->evaluate(temperature);
}

double GeoPhysics::FluidType::computeTheta (const double temperature, const double liquidusTemperature) const
{
   if (temperature < liquidusTemperature)
   {
      const double temp = (temperature - liquidusTemperature) / m_omega;
      return std::exp (-temp * temp);
   } else {
      return 1.0;
   }
}

double GeoPhysics::FluidType::computeThetaDerivative (const double temperature, const double liquidusTemperature) const
{
   if (temperature < liquidusTemperature)
   {
      const double temp = (temperature - liquidusTemperature) / m_omega;
      return -2.0 * temp / m_omega * std::exp (-temp * temp);
   } else {
      return 0;
   }
}
