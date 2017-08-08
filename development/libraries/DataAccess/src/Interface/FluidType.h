#ifndef _DISTRIBUTEDDATAACCESS_INTERFACE_FLUID_TYPE_H_
#define _DISTRIBUTEDDATAACCESS_INTERFACE_FLUID_TYPE_H_

#include "Interface/DAObject.h"
#include "Interface/ProjectHandle.h"
#include "../../../CBMGenerics/src/waterDensity.h"

namespace DataAccess {

   namespace Interface {

      class FluidType: public DAObject
      {
      public:

         FluidType(ProjectHandle* projectHandle, database::Record* record);
         ~FluidType();

         /// \name General accessor functions.
         /// @{

         virtual const std::string& getName () const;

         virtual CBMGenerics::waterDensity::FluidDensityModel fluidDensityModel () const;

         virtual double density () const;

         virtual double salinity () const;


         virtual CalculationModel getSeismicVelocityCalculationModel () const;

         virtual double getConstantSeismicVelocity () const;
   
         /// @}

         /// \name Heat-capacity attributes.
         /// @{

         /// Name of fluid used for heat capacity samples.
         virtual const std::string& getHeatCapacityFluidName () const;

         /// The fluid used for heat capacity samples.
         virtual const FluidType* getHeatCapacityFluidType () const;

         /// Return a list of the heat-capacity samples.
         ///
         /// This list must be deleted after use.
         virtual FluidHeatCapacitySampleList* getFluidHeatCapacitySamples () const;

//          /// Return a constans reference to the interpolator for the heat-capacity.
//          virtual const TableInterpolator2D& getHeatCapacityInterpolator () const;

//          /// Return the interpolated value of the heat-capacity interpolator at the temperature and pressure.
//          virtual double getHeatCapacity ( const double temperature, const double pressure ) const;

         /// \brief Set the viscosity model to be used.
         void setViscosityModel ( const ViscosityModel model );

         /// \brief Get the viscosity model to be used.
         ViscosityModel getViscosityModel () const;


         /// @}
         /// \name Thermal-conductivity attributes.
         /// @{

         /// Name of fluid used for thermal-conductivity samples.
         virtual const std::string& getThermalConductivityFluidName () const;

         /// The fluid used for thermal-conductivity samples.
         virtual const FluidType* getThermalConductivityFluidType () const;

         /// Return a list of the thermal-conductivity samples.
         ///
         /// This list must be deleted after use.
         virtual FluidThermalConductivitySampleList* getFluidThermalConductivitySamples () const;

         /// @}
         /// \name Density attributes.
         /// @{

         /// Name of fluid used for demsity samples.
         virtual const std::string& getDensityFluidName () const;

         /// The fluid used for demsity samples.
         virtual const FluidType* getDensityFluidType () const;

         /// @}


      private :

         CBMGenerics::waterDensity::FluidDensityModel m_fluidDensityModel;
         CalculationModel m_seismicVelocityCalculationModel;
         ViscosityModel   m_viscosityModel;

      };

   }

} // namespace DataAccess::Interface

#endif
