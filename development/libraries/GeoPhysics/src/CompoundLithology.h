//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef GEOPHYSICS__COMPOUND_LITHOLOGY_H
#define GEOPHYSICS__COMPOUND_LITHOLOGY_H

//std library
#include <string>
#include <vector>

// Eospack library
#include "EosPack.h"

// CBMGenerics library
#include "ComponentManager.h"
typedef CBMGenerics::ComponentManager::PhaseId PhaseId;
#include "AbstractCompoundLithology.h"
#include "ArrayDefinitions.h"
#include "CompoundLithologyComposition.h"
#include "CompoundProperty.h"
#include "GeoPhysicsFluidType.h"
#include "Interface/Interface.h"
#include "SeismicVelocity.h"
#include "SimpleLithology.h"
#include "MultiCompoundProperty.h"

#include "PermeabilityMixer.h"
#include "AlignedWorkSpaceArrays.h"

namespace GeoPhysics {
   class ProjectHandle;
}

namespace GeoPhysics {


   class CompoundLithology : public DataModel::AbstractCompoundLithology {

   public:

      /// \brief Create typedef for the workspace of vectors used in intermediate permeability calculation.
      typedef PermeabilityMixer::PermeabilityWorkSpaceArrays PermeabilityWorkSpaceArrays;

      CompoundLithology(GeoPhysics::ProjectHandle* projectHandle);

      virtual ~CompoundLithology();

      CompoundLithologyComposition getComposition() const;

      SimpleLithology * getSimpleLithology() const;

      /// Return a string representation of the compound-lithology.
      std::string image() const;

      /// Add a simple-lithology to the compound lithology and its fraction.
      void addLithology(SimpleLithology* a_lithology, const double a_percentage);

      void addThermCondPointN(const double temp, const double thermcond);

      void addThermCondPointP(const double temp, const double thermcond);

      void createThCondTbl();

      /// Return the density of the rock matrix.
      double density() const;

      /// Return the depositional-permeability value for the compound-lithology.
      double depositionalPermeability() const;

      /// Return the surface-porosity value for the compound-lithology.
      double surfacePorosity() const;

      /// Get the compound-lithology surface porosity.
      void getSurfacePorosity(CompoundProperty& porosity) const;

      /// Calculate the heat-capacity of the compound-lithology.
      double heatcapacity(const double temperature) const;

      /// Calculate the density times heat-capacity of the compound-lithology.
      double densityXheatcapacity(const double temperature, const double pressure) const;

      /// \bried Calculate the density times heat-capacity of the compound-lithology for a vector of values.
      void densityXheatcapacity ( const unsigned int       size,
                                  ArrayDefs::ConstReal_ptr temperature,
                                  ArrayDefs::ConstReal_ptr pressure,
                                  ArrayDefs::Real_ptr      densityXHeatCap ) const;

      /// Return the heat-production value for the compound-lithology.
      double heatproduction() const final;

      /// Return the capillary entry pressure coefficient C1
      double capC1() const;

      /// Return the capillary entry pressure coefficient C2
      double capC2() const;

      /// \brief Return 10^-c2.
      ///
      /// This is required in order to evaluate the entry pressure.
      /// It saves on a pow each time the entry pressure is computed.
      double tenPowerCapC2() const;

      /// Return the Brooks-Corey parameters
      string PcKrModel() const;
      double LambdaPc() const;
      double LambdaKr() const;

      double hydrostatFullCompThickness(const double maxVes,
                                        const double thickness,
                                        const double densitydiff,
                                        const bool   overpressuredCompaction) const;

      /// Return whether or not the lithology is in-compressible.
      bool isIncompressible() const;

      /// Return whether or not the basement lithology
      bool isBasement() const;

      /// Compute the porosity.
      double porosity(const double sigma,
         const double sigma_max,
         const bool   includeChemicalCompaction,
         const double chemicalCompactionTerm) const;

      /// Re-calculate the compound-lithology properties after a simple-lithology has been added.
      bool reCalcProperties();

      /// Set the layer mixing model and layering index.
      void setMixModel(const std::string& mixmodel, double layeringIndex);

      /// Return the solid seismic-velocity value for the compound-lithology.
      double seismicVelocitySolid() const;

      /// Return the SeismicVelocity object for the compound-lithology.
      const SeismicVelocity& seismicVelocity() const;

      /// Return the thermal-conductivity anisotropy.
      double thermalcondaniso() const;

      /// Return the simple thermal-conductivity value.
      double thermalconductivityval() const;

      /// Return the thermal-conductivity normal (vertical) value.
      double thermalconductivityN(const double temperature) const;

      /// Return the thermal-conductivity plane (horizontal) value.
      double thermalconductivityP(const double temperature) const;

      /// Calculate the bulk-density times heat-capacity.
      // Should this be a lithology function?
      void calcBulkDensXHeatCapacity(const FluidType* fluid,
                                     const double  Porosity,
                                     const double  Pressure,
                                     const double  Temperature,
                                     const double  LithoPressure,
                                     double& BulkDensXHeatCapacity) const;

      /// \bried Calculate the bulk-density times heat-capacity for a vector of values.
      void calcBulkDensXHeatCapacity ( const unsigned int  size,
                                       const double*       fluidDensityXHeatCap,
                                       const double*       porosity,
                                       const double*       pressure,
                                       const double*       temperature,
                                       const double*       lithostaticPressure,
                                       ArrayDefs::Real_ptr bulkDensXHeatCapacity,
                                       ArrayDefs::Real_ptr matrixDensXHeatCapWorkSpace ) const;

      /// \brief Compute the density of the lithology.
      ///
      /// The lithologies "Crust", "Litho. Mantle" and ALCBasalt can only appear
      /// in the basement. In ALC mode the density of these lithologies depends on
      /// temperature and lithostatic-pressure.
      double computeDensity ( const double temperature,
                              const double lithoPressure ) const;


      /// \brief Calculate the bulk density.
      ///
      /// This function is to be used only in alc mode, where the density
      /// in the basement lithologies depends on the pressure and temperature.
      // Should this be a lithology function?
      void calcBulkDensity ( const FluidType* fluid,
                             const double     porosity,
                             const double     porePressure,
                             const double     temperature,
                             const double     lithoPresure,
                                   double&    bulkDensity ) const;

      /// \brief Calculate the bulk density.
      ///
      /// This function is to be used only in coupled mode when ALC is not enabled.
      /// The density of the lithologies in the basement lithologies is constant.
      void calcBulkDensity ( const FluidType* fluid,
                             const double     porosity,
                             const double     porePressure,
                             const double     temperature,
                                   double&    bulkDensity ) const;

      /// \brief Calculate the bulk density.
      ///
      /// This function is to be used only in hydrostatic mode and when ALC is not enabled.
      /// The density of the lithologies in the basement lithologies is constant.
      void calcBulkDensity ( const FluidType* fluid,
                             const double     porosity,
                                   double&    bulkDensity ) const;

      /// Calculate the bulk thermal-conductivity.
      // Should this be a lithology function?
      void calcBulkThermCondNP(const FluidType* fluid,
                               const double  Porosity,
                               const double  Temperature,
                               const double  PorePressue,
                               double& BulkTHCondN,
                               double& BulkTHCondP) const;

      /// \brief Calculate the bulk thermal-conductivity for a vector of values.
      void calcBulkThermCondNP ( const unsigned int size,
                                 const double* fluidThermalConductivity,
                                 const double* porosity,
                                 const double* temperature,
                                 const double* porePressue,
                                 double*       bulkThermalCondN,
                                 double*       bulkThermalCondP ) const;


      /// thermal permeability for basement lithology
      void calcBulkThermCondNPBasement(const FluidType* fluid,
                                       double Porosity, double Temperature, double LithoPressure,
                                       double &BulkTHCondN, double &BulkTHCondP) const;

      /// \brief thermal permeability for basement lithology for a vector of values.
      void calcBulkThermCondNPBasement ( const unsigned int size,
                                         const double* temperature,
                                         const double* lithostaticPressure,
                                         double*       bulkThermalCondN,
                                         double*       bulkThermalCondP ) const;


      /// thermal permeability for ALC Basalt  lithology
      void calcBulkThermCondNPBasalt( double Temperature, double LithoPressure, double &BulkTHCondN, double &BulkTHCondP) const;

      /// Calculate the bulk heat-production.
      void calcBulkHeatProd(const double Porosity, double &BulkHeatProd) const;

      double computeSegmentThickness(const double topMaxVes,
                                     const double bottomMaxVes,
                                     const double densityDifference,
                                     const double solidThickness) const;

      double computeSegmentThickness(const double topMaxVes,
                                     const double bottomMaxVes,
                                     const double topVes,
                                     const double bottomVes,
                                     const double densityDifference,
                                     const double solidThickness) const;

      // compute capillary entry pressure coefficients
      void mixCapillaryEntryPressureCofficients();

      // mix the capillary pressure function parameters
      void mixBrooksCoreyParameters();

      const string getThermalModel() const;

      double computePorosityDerivativeWRTVes(const double ves,
                                             const double maxVes,
                                             const bool   includeChemicalCompaction,
                                             const double chemicalCompactionTerm) const;


      /// Return the reference-effective-stress.
      ///
      /// Used in computing the soil-mechanics void-ratio (porosity).
      double referenceEffectiveStress() const;

      /// Return the reference-void-ratio.
      ///
      /// Used in computing the soil-mechanics void-ratio (porosity).
      double referenceVoidRatio() const;
      ///Return the activation energy
      ///
      ///Used for the chemical compaction computation with Schneider algorithm
      double activationEnergy() const;

      ///Return the reference viscosity
      ///
      ///Used for the chemical compaction computation with Schneider algorithm
      double referenceViscosity() const;

      ///Return the size of the grains of quartz
      ///
      ///Used for the chemical compaction computation with Walderhaug algorithm
      double quartzGrainSize() const;

      ///Return the fraction of quartz
      ///
      ///Used for the chemical compaction computation with Walderhaug algorithm
      double quartzFraction() const;

      ///Return the fraction of quartz surface area coated by clay
      ///
      ///Used for the chemical compaction computation with Walderhaug algorithm
      double coatingClayFactor() const;

      ///Return the temperature of intrusion for a sill
      ///
      ///Used for the sill intrusion tool
      double igneousIntrusionTemperature() const;

      /// Return the fracture-pressure scaling-value.
      double fracturedPermeabilityScaling() const;

      bool hasHydraulicallyFractured(const double hydrostaticPressure,
                                     const double porePressure,
                                     const double lithostaticPressure) const;

      /// Compute the lithology-dependant fracture pressure.
      double fracturePressure(const double hydrostaticPressure,
                              const double lithostaticPressure) const;

      /// Compute the compound-porosity value.
      ///
      /// This compute the individual, simple-lithology porosities and the mixed value.
      void getPorosity(const double            ves,
                       const double            maxVes,
                       const bool              includeChemicalCompaction,
                       const double            chemicalCompactionTerm,
                             CompoundProperty& Porosity) const;

      /// \brief Compute the compound-porosity for an array of values.
      ///
      /// \param  [in] size                      The number of values in the array.
      /// \param  [in] ves                       The ves values for which the porosity is to be calculated.
      /// \param  [in] maxVes                    The max-ves values for which the porosity is to be calculated.
      /// \param  [in] includeChemicalCompaction Indicate whether or not chemical compaction is to be included.
      /// \param  [in] chemicalCompactionTerm    The chemical compaction values for which the porosity is to be calculated.
      /// \param [out] porosity                  The mixed porosity values.
      void getPorosity ( const unsigned int       size,
                         ArrayDefs::ConstReal_ptr ves,
                         ArrayDefs::ConstReal_ptr maxVes,
                         const bool               includeChemicalCompaction,
                         ArrayDefs::ConstReal_ptr chemicalCompactionTerm,
                         ArrayDefs::Real_ptr      porosity ) const;

      /// \brief Compute the compound-porosity for an array of values.
      ///
      /// \param  [in] size                      The number of values in the array.
      /// \param  [in] ves                       The ves values for which the porosity is to be calculated.
      /// \param  [in] maxVes                    The max-ves values for which the porosity is to be calculated.
      /// \param  [in] includeChemicalCompaction Indicate whether or not chemical compaction is to be included.
      /// \param  [in] maxVes                    The chemical compaction values for which the porosity is to be calculated.
      /// \param [out] porosities                The calculated single and mixed porosity values.
      void getPorosity ( const unsigned int       size,
                         ArrayDefs::ConstReal_ptr ves,
                         ArrayDefs::ConstReal_ptr maxVes,
                         const bool               includeChemicalCompaction,
                         ArrayDefs::ConstReal_ptr chemicalCompactionTerm,
                         MultiCompoundProperty&   porosities,
                         ArrayDefs::Real_ptr      porosityDerivative = nullptr ) const;

      /// Calculate the permeability value using the compound porosity.
      void calcBulkPermeabilityNP(const double            ves,
                                  const double            maxVes,
                                  const CompoundProperty& Porosity,
                                  double&           Permeability_Normal,
                                  double&           Permeability_Plane) const;

      /// \brief Calculate the permeability value using the compound porosity.
      ///
      /// \param  [in] size               The number of values in the array.
      /// \param  [in] ves                The ves values for which the porosity is to be calculated.
      /// \param  [in] maxVes             The max-ves values for which the porosity is to be calculated.
      /// \param  [in] porosities         The porosity calcualted at the ves and max-ves values.
      /// \param [out] permeabilityNormal The permeability calculated in the vertical direction.
      /// \param [out] permeabilityPlane  The permeability calculated in the horizontal direction.
      /// \param [in,out] workSpace       A set of work-space vectors used to store intermediate results.
      /// \pre The dimension of all vectors is the same and is parameter size.
      void calcBulkPermeabilityNP ( const unsigned int           size,
                                    ArrayDefs::ConstReal_ptr     ves,
                                    ArrayDefs::ConstReal_ptr     maxVes,
                                    const MultiCompoundProperty& porosities,
                                    ArrayDefs::Real_ptr          permeabilityNormal,
                                    ArrayDefs::Real_ptr          permeabilityPlane,
                                    PermeabilityWorkSpaceArrays& workSpace ) const;


      /// Calculate the permeability value using the simple porosity.
      void calcBulkPermeabilityNP(const double            ves,
                                  const double            maxVes,
                                  const double            porosity,
                                  double&           permeabilityNormal,
                                  double&           permeabilityPlane) const;


      /// Calculate the permeability-derivative value with respect to ves using the compound porosity.
      /// It returns derivatives computed both in normal and plane direction
      // The handled mixing modes are HOMOGENEOUS and LAYERED
      void calcBulkPermeabilityNPDerivativeWRTVes(const double            ves,
                                                  const double            maxVes,
                                                  const CompoundProperty& Porosity,
                                                  const double            porosityDerivativeWrtVes,
                                                  double&                 Permeability_Derivative_Normal,
                                                  double&                 Permeability_Derivative_Plane) const;

      /// Integrate the chemical-compaction terms.
      double integrateChemicalCompaction(const double Time_Step,
                                         const double ves,
                                         const double Porosity,
                                         const double Temperature) const;

      /// Indicate that this lithology is used as a fault lithology.
      void makeFault(const bool newFaultValue);

      /// Return whether of not this lithology is a fault.
      bool isFault() const;

      /// Set the value of m_isLegacy to determine which behaviour to use for
      ///    1.Mixing of lithologies
      ///    2.Minimum porosity behaviour
      void setIsLegacy( bool isLegacy );

      void setMinimumPorosity(DataAccess::Interface::PorosityModel porosityModel, double  surfaceVoidRatio, double soilMechanicsCompactionCoefficient);

      /// \brief Return the number of simple lithologies the compound lithology is made from.
      size_t getNumberOfSimpleLithologies () const;

   protected:

	   void setChemicalCompactionTerms(const double rockViscosity,
		   const double activationEnergy);

	   double exponentialDecompactionFunction(const double ves) const;

      /// Decide whether or not you can mix the lithologies depending on the porosity models
      /// Return true if mixing is ok return false otherwise
      bool allowableMixing() const;

      void mixSurfacePorosity(DataAccess::Interface::PorosityModel porosityModel, double & surfacePorosity, double & surfaceVoidRatio);

      // Decide which porosity model should be used, choose the model with the main percentage
      /// According to allowableMixing() function, the chosen model is also the only with non nul percentage
      void mixPorosityModel(DataAccess::Interface::PorosityModel & porosityModel);

      void mixCompactionCoefficients(double & compactionincr, double & compactionincrA,
    		  double & compactionincrB,
    		  double & compactiondecr,
    		  double & compactiondecrA,
    		  double & compactiondecrB,
    		  double & soilMechanicsCompactionCoefficient);

      void setMinimumExponentialPorosity();

      void setMinimumSoilMechanicsPorosity(double surfaceVoidRatio, double soilMechanicsCompactionCoefficient);


      // these are the properties in the simple lithology
      // in this compound lithology these values will be derived
      // from the simple lithology properties.
      double           m_density;
      double           m_depositionalPermeability;
      double           m_heatProduction;
      double           m_seismicVelocitySolid;
      double           m_nExponentVelocity;
      double           m_permeabilityincr;
      double           m_permeabilitydecr;
      double           m_thermalConductivityValue;
      double           m_thermalConductivityAnisotropy;
      double           m_capC1;
      double           m_capC2;
      double           m_tenPowerCapC2;
      //Brooks-Corey
      string           m_PcKrModel;
      double           m_LambdaKr;
      double           m_LambdaPc;

      // These are mutable because the compute function in the interpolator is not const.
      mutable ibs::Interpolator     m_thermcondntbl;
      mutable ibs::Interpolator     m_thermcondptbl;

      double m_fracturedPermeabilityScalingValue;
      double m_lithologyFractureGradient;

      double m_referenceSolidViscosity;
      double m_lithologyActivationEnergy;
      // For chemical compaction with Walderhaug model
      double m_quartzGrainSize;
      double m_quartzFraction;
      double m_coatingClayFactor;
      // For sill intrusion
      double m_igneousIntrusionTemperature;

      DataAccess::Interface::MixModelType m_mixmodeltype; //!< Mixing model: used to mix permeability

      double m_layeringIndex;//!< Permeability layering index used to compute permeability mixing

      // typedef std::vector<double> anisotropyContainer;
      // anisotropyContainer m_componentAnisotropy;

      typedef std::vector<SimpleLithology*> compContainer;
      compContainer m_lithoComponents;

      typedef std::vector<double> percentContainer;
      percentContainer m_componentPercentage;

      /// Only effects the calculation of the permeability.
      /// Fault lithologies will always be modelled as a homogeneous mixing for the permeability,
      /// but the thermal conductivity may still be layered.
      bool m_isFaultLithology;

      /// Holds the porosities of the individual simple lithologies, evaluated at a ves that the
      /// compound lithology has the minimum porosity.
      CompoundProperty minimumCompoundPorosity;

      GeoPhysics::ProjectHandle* m_projectHandle;

      Porosity m_porosity;

   private:

      /*!
       * \brief Compute the solid modulus of the lithology according to the following mixing rules.
       * \details If the compound lithology is HOMOGENEOUS or UNDEFINED we use the geometric mean.
       *   If the compound lithology is LAYERED we use the harmonic mean.
       */
      double mixModulusSolid() const;

      /// The object used to compute the velocity (by using m_seismicVelocity.seismicVelocity() method).
      SeismicVelocity m_seismicVelocity;

      bool m_isBasementLithology;

      bool m_isLegacy; /*!< Legacy behaviour for minimum porosity?
                        *Flag for new rock property library( and new migration engine )
                        * 0 is the revised minimum porosity behaviour and additional mixing models
                        * 1 is simple minimum porosity behaviour and 2 mixing models */

      double m_mixHorizonExp;//!< Permeability horizontal mixing exponent , equals (1-m_mixVerticalExp)/2 */
      double m_inverseMixHorizonExp;/// 1 / m_mixHorizonExp

      double m_mixVerticalExp;//!< Permeability vertical mixing exponent, equals 1-2*m_mixHorizonExp */
      double m_inverseMixVerticalExp; /// 1 / m_mixVerticalExp

      /// \brief Performs mixing of permeability values.
      PermeabilityMixer m_permeabilityMixer;

   };


}

//------------------------------------------------------------//
//      Inline Functions
//------------------------------------------------------------//


inline bool GeoPhysics::CompoundLithology::isFault() const {
   return m_isFaultLithology;
}

inline double GeoPhysics::CompoundLithology::fracturedPermeabilityScaling() const {
   return m_fracturedPermeabilityScalingValue;
}

inline void GeoPhysics::CompoundLithology::addThermCondPointN(const double temp, const double thermcond) {
   m_thermcondntbl.addPoint(temp, thermcond);
}

inline void GeoPhysics::CompoundLithology::addThermCondPointP(const double temp, const double thermcond) {
   m_thermcondptbl.addPoint(temp, thermcond);
}

inline double GeoPhysics::CompoundLithology::density() const {
   return m_density;
}

inline double GeoPhysics::CompoundLithology::depositionalPermeability() const {
   return m_depositionalPermeability;
}

inline double GeoPhysics::CompoundLithology::surfacePorosity() const {
   return m_porosity.getSurfacePorosity();
}

inline double GeoPhysics::CompoundLithology::heatproduction() const {
   return m_heatProduction;
}

inline double GeoPhysics::CompoundLithology::seismicVelocitySolid() const {
	return m_seismicVelocitySolid;
}

inline const GeoPhysics::SeismicVelocity& GeoPhysics::CompoundLithology::seismicVelocity() const {
	return m_seismicVelocity;
}

inline double GeoPhysics::CompoundLithology::thermalcondaniso() const {
   return m_thermalConductivityAnisotropy;
}

inline double GeoPhysics::CompoundLithology::thermalconductivityval() const {
   return m_thermalConductivityValue;
}

inline double GeoPhysics::CompoundLithology::thermalconductivityN(const double temperature) const {
   return m_thermcondntbl.compute(temperature);
}

inline double GeoPhysics::CompoundLithology::thermalconductivityP(const double temperature) const {
   return m_thermcondptbl.compute(temperature);
}

inline double GeoPhysics::CompoundLithology::capC1() const {
   return m_capC1;
}

inline double GeoPhysics::CompoundLithology::capC2() const {
   return m_capC2;
}

inline double GeoPhysics::CompoundLithology::tenPowerCapC2() const {
   return m_tenPowerCapC2;
}


inline string GeoPhysics::CompoundLithology::PcKrModel() const {
   return m_PcKrModel;
}

inline double GeoPhysics::CompoundLithology::LambdaPc() const {
   return m_LambdaPc;
}

inline double GeoPhysics::CompoundLithology::LambdaKr() const {
   return m_LambdaKr;
}


inline double GeoPhysics::CompoundLithology::referenceEffectiveStress() const {
   return 1.0e5;

}

inline double GeoPhysics::CompoundLithology::referenceVoidRatio() const {
   return m_porosity.getSurfacePorosity() / (1.0 - m_porosity.getSurfacePorosity());

#if 0
   return m_porosity.getSurfacePorosity() / (1.0 - m_porosity.getSurfacePorosity());
#endif

}

inline double GeoPhysics::CompoundLithology::activationEnergy() const {
   return m_lithologyActivationEnergy;
}

inline double GeoPhysics::CompoundLithology::referenceViscosity() const{
   return m_referenceSolidViscosity;
}

inline double GeoPhysics::CompoundLithology::quartzGrainSize() const{
   return m_quartzGrainSize;
}

inline double GeoPhysics::CompoundLithology::coatingClayFactor() const{
   return m_coatingClayFactor;
}

inline double GeoPhysics::CompoundLithology::quartzFraction() const{
   return m_quartzFraction;
}

inline double GeoPhysics::CompoundLithology::igneousIntrusionTemperature() const{
   return m_igneousIntrusionTemperature;
}

inline void GeoPhysics::CompoundLithology::setIsLegacy( bool isLegacy )
{
   m_isLegacy = isLegacy;
}

inline size_t GeoPhysics::CompoundLithology::getNumberOfSimpleLithologies () const {
   return m_lithoComponents.size ();
}

#endif // _GEOPHYSICS__COMPOUND_LITHOLOGY_H_
