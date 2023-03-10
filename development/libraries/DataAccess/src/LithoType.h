//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _INTERFACE_LITHOTYPE_H_
#define _INTERFACE_LITHOTYPE_H_

#include "DAObject.h"
#include "Interface.h"

namespace DataAccess
{
   namespace Interface
   {
      /// A LithoType object contains information on a specific lithotype
      class LithoType : public DAObject
      {
      public:
         LithoType (ProjectHandle& projectHandle, database::Record * record);
         virtual ~LithoType (void);


         /// Return the name of this LithoType
         virtual const std::string & getName (void) const;

         /// Return the value of the specified attribute
         const AttributeValue & getLithoTypeAttributeValue (LithoTypeAttributeId attributeId) const;

         /// Return the hydraulic fracturing percentage.
         ///
         /// Value is a percentage, so must be scaled by 0.01 when used.
         virtual double getHydraulicFracturingPercent () const;

         /// Return the reference viscosity.
         virtual double getReferenceViscosity () const;

         /// Return the viscosity activation energy.
         virtual double getViscosityActivationEnergy () const;

         /// Return the minimum mechanical porosity.
         virtual double getMinimumMechanicalPorosity () const;

         /// return the density of the rock matrix.
         virtual double getDensity () const;

         /// Return the heat production.
         virtual double getHeatProduction () const;

         /// Return the thermal conductivity.
         virtual double getThermalConductivity () const;

         /// Return the thermal conductivity anisotropy.
         virtual double getThermalConductivityAnisotropy () const;


         /// Return the surface porosity.
         virtual double getSurfacePorosity () const;

         /// Return the porosity model.
         virtual PorosityModel getPorosityModel () const;

         /// Return the sinlge exponential compaction coefficient.
         virtual double getExponentialCompactionCoefficient () const;

         /// Return the double exponential compaction coefficient A.
         virtual double getExponentialCompactionCoefficientA () const;

         /// Return the double exponential compaction coefficient B.
         virtual double getExponentialCompactionCoefficientB () const;

         /// Return the double exponential compaction ratio.
         virtual double getExponentialCompactionRatio() const;

         /// Return the soil mechanics compaction coefficient.
         virtual double getSoilMechanicsCompactionCoefficient () const;

         /// Return the size of the grain of quartz for chemical compaction
         virtual double getQuartzGrainSize () const;

         ///Return the volume fraction of quartz in the facies
         virtual double getQuartzFraction () const;

         ///Return the fraction of quartz surface area coated by clay
         virtual double getClayCoatingFactor () const;

         /// Return the permeability model.
         virtual PermeabilityModel getPermeabilityModel () const;

         /// Return the deposition permeability.
         virtual double getDepositionalPermeability () const;

         /// Return the number of sample points used to describe the porosity-permeability function.
         virtual int getNumberOfMultipointSamplePoints () const;

         /// Return a constant reference to the porosity value std::string.
         ///
         /// Contains the porosity values for the multipoint porosity-permeability function.
         virtual const std::string& getMultipointPorosityValues () const;

         /// Return a constant reference to the multi point permeability value std::string.
         ///
         /// Contains the permeability values for the multipoint porosity-permeability function.
         virtual const std::string& getMultipointPermeabilityValues () const;

         /// Return the permeability anisotropy.
         virtual double getPermeabilityAnisotropy () const;

         /// Return the permeability sensitivity coefficient.
         ///
         /// In the GUI the permeability model "SANDS" has a field "clay fraction"
         /// this field and the permeability sensitivity coefficient hold the value
         /// that has been input.
         virtual double getPermeabilitySensitivityCoefficient () const;

         /// Return the permeabilty recovery coefficient.
         virtual double getPermeabilityRecoveryCoefficient () const;

         /// Return the seismic velocity.
         virtual double getSeismicVelocity () const;

         /// Return the n exponent parameter used for the Kennan's viscosity computation mode.
         virtual double getSeismicVelocityExponent() const;

         /// Return the capillary entry pressure c1.
         virtual double getCapillaryEntryPressureC1 () const;

         /// Return the capillary entry pressure c2.
         virtual double getCapillaryEntryPressureC2 () const;

   //==Brooks-Corey capillary function parameters

         /// Return residual saturation.
         virtual double getExponentLambdaKr () const;

   /// Return the exponent lambda.
         virtual double getExponentLambdaPc () const;

         /// Return the capillary entry pressure.
         virtual PcKrModel getPcKrModel () const;

         /// Return a list of sample points used to describe how the heat-capacity changes w.r.t. temperature.
         virtual LithologyHeatCapacitySampleList * getHeatCapacitySamples () const;

         /// Return a list of sample points used to describe how the thermal-conductivity changes w.r.t. temperature.
         virtual LithologyThermalConductivitySampleList * getThermalConductivitySamples () const;
         /// Return the fore-ground colour.
         ///
         /// This may be a named colour or its rgb value.
         virtual const std::string& getForegroundColour () const;

         /// Return the back-ground colour.
         ///
         /// This may be a named colour or its rgb value.
         virtual const std::string& getBackgroundColour () const;

         /// Return the pixmap.
         virtual const std::string& getPixmap () const;


         /// Is the behaviour legacy (1) or new rock property feature (0)?
         virtual bool getLegacy() const;

         /// Return the temperature of the sill intrusion when intruded
         virtual double getIgneousIntrusionTemperature () const;

         /// Print the attributes of this LithoType
         // May not work if user application is compiled under IRIX with CC -lang:std
         virtual void printOn (std::ostream &) const;

         virtual void asString (std::string &) const;

      protected:

         LithoType ( const LithoType& litho );

      private:

         static const std::string s_attributeNames[];

         PorosityModel     m_porosityModel;
         PermeabilityModel m_permeabilityModel;
         PcKrModel m_pcKrModel;
      };
   }
}

//Some comment for testing tf compare
#endif // _INTERFACE_LITHOTYPE_H_
