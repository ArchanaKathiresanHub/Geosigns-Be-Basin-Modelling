//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _GEOPHYSICS__FLUID_TYPE_H_
#define _GEOPHYSICS__FLUID_TYPE_H_

#include "Interface/FluidType.h"
#include "Interface/Interface.h"

#include <memory>

// Forward declarations
namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
   }
}

namespace GeoPhysics
{
   namespace Brine
   {
      class Conductivity;
      class Density;
      class Velocity;
      class Viscosity;
   }
}

namespace ibs
{
   class Interpolator2d;
   class PiecewiseInterpolator;
}

namespace GeoPhysics {

   class FluidType : public DataAccess::Interface::FluidType {

   public :

      /// \brief Make the density function visible.
      using DataAccess::Interface::FluidType::density;

      /// \var DefaultHydrostaticPressureGradient
      /// Assumed pressure gradient used when computing the simple fluid density.
      /// Units are MPa/km.
      static const double DefaultHydrostaticPressureGradient;

      /// \var StandardSurfaceTemperature
      /// Assumed surface temperature when computing the simple fluid density.
      /// Units are Celsius.
      static const double StandardSurfaceTemperature;

      /// \var DefaultStandardDepth
      /// Assumed 'standard' depth at which the parameters for the fluid density
      /// are evaluated.
      /// Units are Metres.
      static const double DefaultStandardDepth;

      /// \var DefaultThermalConductivityCorrectionTemperature
      /// The temperature at the thermal conductivity tables are corrected.
      /// Units are Celsius.
      static const double DefaultThermalConductivityCorrectionTemperature;

      /// Constructor.
      FluidType ( DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record );

      /// Destructor.
      ~FluidType ();

      /// Load the fluid-property tables from the project-file.
      /// These are:
      ///   - heat-capacity;
      ///   - thermal-conductivity;
      void loadPropertyTables ();

      /// return the simple density.
      /// This may be different to what appears in the fluid-io table.
      double getConstantDensity () const;

      /// Over-ride the project file value and set the fluid-density function to constant.
      void setDensityToConstant ();

      /// Compute the fluid density.
      /// If the density calculation model is 'constant' the result here may be different from that in the fluid-io table.
      double density ( const double temperature, const double pressure ) const;

      /// Correct the simple density (density value) of the fluid.
      ///
      /// The fluid density that is used in decompaction and some of the hydrostatic temperature 
      /// calculation is the simple density. This may not be the best choice (the default is 
      /// 1000 Kg/M^3) this function "corrects" this, and uses the Batzle and Wang density function
      /// evaluated at the standard depth, with the standard pressure gradient and the
      /// temperature gradient given in the project3d file.
      void correctSimpleDensity ( const double standardDepth,
                                  const double pressureGradient,
                                  const double surfaceTemperature,
                                  const double temperatureGradient );

      /// \brief Compute the brine density at a defined depth and pressure.
      ///
      /// The temperature is computed from a standard surface-temperature and 
      /// the temperature gradient in the project file.
      double getCorrectedSimpleDensity ( const double standardDepth,
                                         const double pressureGradient,
                                         const double surfaceTemperature,
                                         const double temperatureGradient ) const;

      /// Compute the derivative of the fluid density w.r.t. pressure.
      double computeDensityDerivativeWRTPressure ( const double temperature, const double pressure ) const;

      /// Compute the derivative of the fluid density w.r.t. temperature.
      double computeDensityDerivativeWRTTemperature ( const double temperature, const double pressure ) const;

      /// Compute the fluid viscosity.
      double viscosity ( const double temperature, const double pressure ) const;

      /// Compute the thermal conductivity.
      double thermalConductivity ( const double temperature, const double pressure ) const;

      /// Compute the heat-capacity.
      double heatCapacity ( const double temperature, const double pressure ) const;

      /// Compute the density x heat-capacity, also known as the Volumetric Heat Capacity.
      double densXheatCapacity ( const double temperature, const double pressure, bool includePermafrost = false ) const;

      /// Compute the seismic velocity.
      double seismicVelocity ( const double temperature, const double pressure ) const;

      /// Compute relative premeability (for ice)
      double relativePermeability ( const double temperature, const double pressure ) const;

      bool SwitchPermafrost () const;

   private :

      /// The interpolator for the fluid-heat-capacity table.
      /// It depends on both temperature and pressure.
      mutable std::unique_ptr<ibs::Interpolator2d> m_heatCapacitytbl;

      /// Which calculation model to use for the seismic velocity.
      DataAccess::Interface::CalculationModel m_seismicVelocityCalculationModel;

      /// Which calculation model to use for the density.
      CBMGenerics::waterDensity::FluidDensityModel m_densityCalculationModel;

      double       m_densityVal;
      const double m_salinity;
      double       m_seismicVelocityVal;

      /// For permafrost.
      double m_pressureTerm;
      double m_salinityTerm;
      
      /// For Brine properties
      std::unique_ptr<GeoPhysics::Brine::Conductivity> m_conductivity;
      std::unique_ptr<GeoPhysics::Brine::Density>      m_density;
      std::unique_ptr<GeoPhysics::Brine::Velocity>     m_velocity;
      std::unique_ptr<GeoPhysics::Brine::Viscosity>    m_viscosity;
      std::unique_ptr<GeoPhysics::Brine::Conductivity> m_zeroSalinityConductivity;
      std::unique_ptr<GeoPhysics::Brine::Density>      m_zeroSalinityDensity;

      double solidDensityTimesHeatCapacity ( const double temperature ) const;

      /// Compute the fraction of water as opposed to ice in the pore space.
      double computeTheta ( const double temperature, const double liquidusTemperature ) const;

      /// Compute the derivate of the Theta with respect to Temperature. Theta is the fraction of water
      /// as opposed to ice in the pore space.
      double computeThetaDerivative ( const double temperature, const double liquidusTemperature ) const;

      /// Compute the temperature below which water starts to freeze.
      double getLiquidusTemperature ( const double temperature, const double pressure ) const;

      /// Compute the temperature at which 99% of the water has turned into ice.
      double getSolidusTemperature ( const double liquidusTemperature ) const;

      /// Compute the salinity.
      double salinityConcentration ( const double temperature, const double pressure ) const;

      /// An interpolator for the density of water containing some fraction of ice ( t < 0.0 ).
      /// It depends only on temperature.
      std::unique_ptr<ibs::PiecewiseInterpolator> m_iceDensityInterpolator;

      /// An interpolator for the heat-capacity of water containing some fraction of ice ( t < 0.0 ).
      /// It depends only on temperature.
      std::unique_ptr<ibs::PiecewiseInterpolator> m_iceHeatCapacityInterpolator;

      /// An interpolator for the thermal-conductivity of water containing some fraction of ice ( t < 0.0 ).
      /// It depends only on temperature.
      std::unique_ptr<ibs::PiecewiseInterpolator> m_iceThermalConductivityInterpolator;

      const double m_omega;

   };

} // end GeoPhysics


inline void GeoPhysics::FluidType::setDensityToConstant ()
{
   m_densityCalculationModel = CBMGenerics::waterDensity::Constant;
}

inline double GeoPhysics::FluidType::getConstantDensity () const
{
   return m_densityVal;
}

inline double GeoPhysics::FluidType::relativePermeability (const double , const double ) const
{
   return 1.0;
}

inline bool GeoPhysics::FluidType::SwitchPermafrost() const
{
   return m_projectHandle->getPermafrost();
}

#endif // _GEOPHYSICS__FLUID_TYPE_H_
