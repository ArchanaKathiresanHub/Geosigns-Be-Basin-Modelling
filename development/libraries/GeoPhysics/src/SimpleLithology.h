#ifndef _GEOPHYSICS__SIMPLE_LITHOLOGY_H_
#define _GEOPHYSICS__SIMPLE_LITHOLOGY_H_

#include <vector>
#include <string>

#include "database.h"

#include "Interface/LithoType.h"
#include "Interface/Interface.h"

#include "IBSinterpolator.h"
#include "PiecewiseInterpolator.h"
#include "Permeability.h"
#include "Porosity.h"

namespace GeoPhysics {

   /// A simple-lithology object extends the DAL::LithoType, adding functionality 
   /// to enable the calculation of various geo-physical quantities.
   class SimpleLithology : public DataAccess::Interface::LithoType {

   public:


      SimpleLithology(DataAccess::Interface::ProjectHandle* projectHandle,
         database::Record*                     record);

      SimpleLithology ( const SimpleLithology& litho );

      virtual ~SimpleLithology();

      // Change the name of a lithology
      void setName(const string & newName);

      /// Change the permeability interpolator.
      void setPermeability(const string & faultLithologyName, const double permeabilityAnisotropy,
         const std::vector<double> & porositySamples, const std::vector<double> & permeabilitySamples
         );

      /// Correct the thermal conductivity tables.
      void correctThermCondPoint(const double correction);

      vector<ibs::XF>& getThermCondTbl();

      /// Return thermal conductivity model
      DataAccess::Interface::CalculationModel getThermalCondModel() const;

      virtual bool setThermalModel(const string& aThermalModel) { (void)aThermalModel; return true; };
      /// Return thermal conductivity model
      const std::string getThermalCondModelName() const;

      /// Return compaction coefficient for single exponential model.
      double getCompCoeff() const;
      
      /// Return compaction coefficient A for double exponential model.
      double getCompCoeffA() const;
      
      /// Return compaction coefficient B for double exponential model.
      double getCompCoeffB() const;

      /// Return depositional porosity.
      double getDepoPoro() const;

      /// Return porosity model
      DataAccess::Interface::PorosityModel getPoroModel() const;

      /// Return permeability model
      DataAccess::Interface::PermeabilityModel getPermeabilityModel() const;

      /// Return the depositional permeability.
      double getDepoPerm() const;

      /// Return the anisotropy of the permeability.
      double getPermAniso() const;

      /// Return the solid density.
      double getDensity()  const;

      /// Return the thermal conductivity anisotropy.
      double getThCondAn() const;

      /// Return the thermal conductivity value.
      double getThCondVal() const;

      /// Return the heat production value.
      double getHeatProduction() const;

      /// Return the seismic velocity value.
      double getSeismicVelocity() const;

	  /// Return the n exponent value used for the Lorcan's velocity computation mode.
	  double getVelocityExponent() const;

      /// Return the specific surface area.
      double getSpecificSurfArea() const;

      /// Return the geometric variance of the grain size distribution.
      double getGeometricVariance () const;
  
      virtual double getDensity( double t, double p ) const { (void) t; (void) p; return m_density; }
      
      /// Return the C1 - capillary entry pressure coefficient
      double getCapC1() const;

      /// Return the C2 - capillary entry pressure coefficient
      double getCapC2() const;


      //Brooks-Corey parameters
      DataAccess::Interface::PcKrModel getPcKrModel() const;
      double getLambdaPc() const;
      double getLambdaKr() const;


      /// Compute the heat-capacity from the heat-capacity table.
      virtual double heatcapacity(const double t) const;

      /// Determine whether or not the lithology is incompressible.
      bool isIncompressible() const;

      /// Return the name of the lithology.
      const std::string& getName() const;

      /// Compute the permeability.
      double permeability(const double ves,
         const double maxVes,
         const double calculatedPorosity) const;

      /// Compte the derivative of the permeability function.
      void permeabilityDerivative(const double  ves,
         const double  maxVes,
         const double  calculatedPorosity,
         const double  porosityDerivativeWrtVes,
         double& Permeability,
         double& Derivative) const;

      /// Print the string representation of the lithology to standard out.
      void print() const;

      /// Return the string representation of the lithology.
      std::string image() const;

      /// Compute the thermal conductivity from the tables.
      ///
      /// NOTE: This function uses the uncorrected table values.
      double thermalconductivity(const double t) const;
      virtual double thermalconductivity(double t, double p) const {
         (void) p;
         return thermalconductivity(t);
      }

      friend bool operator== (const SimpleLithology& l1,
         const SimpleLithology& l2);


      //------------------------------------------------------------//

      /// Compute the porosity.
      double porosity(const double ves,
         const double maxVes,
         const bool   includeChemicalCompaction,
         const double chemicalCompaction) const;

      /// Return the reference effective stress.
      ///
      /// This is the for the soil-mechanics porosity-model.
      double referenceEffectiveStress() const;

      void setChemicalCompactionTerms(const double rockViscosity,
         const double activationEnergy);

      /// Return the reference solid viscosity.
      double getReferenceSolidViscosity() const;

      /// Return the lithology activation energy.
      double getLithologyActivationEnergy() const;

      /// Return the fracture gradient.
      double getLithologyFractureGradient() const;

   private:

      /// Loads the heat-capacity and thermal-conductivity tables.
      void loadPropertyTables();

      typedef std::vector<ibs::XF> XYContainer;
      XYContainer m_thermCondTbl;

      double            m_density;

      double            m_heatproduction;
      double            m_seismicvelocity;
	  double            m_nExponentVelocity;

      double            m_thermalcondaniso;
      double            m_thermalconductivityval;
      double            m_specificSurfaceArea;
      double            m_geometricVariance;
      double            m_capC1;
      double            m_capC2;

      //Brooks-Corey parameters

      DataAccess::Interface::PcKrModel m_PcKrModel;
      double m_LambdaPc;
      double m_LambdaKr;

      mutable ibs::Interpolator m_thermalconductivitytbl;
      mutable ibs::Interpolator m_heatcapacitytbl;

      double m_referenceSolidViscosity;
      double m_lithologyActivationEnergy;
      double m_lithologyFractureGradient;
      double m_minimumMechanicalPorosity;

      Permeability m_permeability;
      Porosity     m_porosity;

      protected:
      string            m_lithoname;
      DataAccess::Interface::CalculationModel  m_thermalcondmodel;
      DataAccess::Interface::CalculationModel  m_heatcapmodel;
   };


}

//------------------------------------------------------------//
//   Inline functions.
//------------------------------------------------------------//

inline std::vector<ibs::XF>& GeoPhysics::SimpleLithology::getThermCondTbl() {
   return m_thermCondTbl;
}

inline DataAccess::Interface::CalculationModel GeoPhysics::SimpleLithology::getThermalCondModel() const {
   return m_thermalcondmodel;
}

inline DataAccess::Interface::PermeabilityModel GeoPhysics::SimpleLithology::getPermeabilityModel() const {
   return m_permeability.getPermModel();
}

inline double GeoPhysics::SimpleLithology::getDepoPerm() const {
   return m_permeability.getDepoPerm();
}

inline double GeoPhysics::SimpleLithology::getDepoPoro() const {
   return m_porosity.getSurfacePorosity();
}

inline double GeoPhysics::SimpleLithology::getCompCoeff() const {
   return m_porosity.getCompactionCoefficient();
}

inline double GeoPhysics::SimpleLithology::getCompCoeffA() const {
   return m_porosity.getCompactionCoefficientA();
}

inline double GeoPhysics::SimpleLithology::getCompCoeffB() const {
   return m_porosity.getCompactionCoefficientB();
}

inline DataAccess::Interface::PorosityModel GeoPhysics::SimpleLithology::getPoroModel() const{
   return m_porosity.getPorosityModel();
}

inline double GeoPhysics::SimpleLithology::getPermAniso() const {
   return m_permeability.getPermAniso();
}

inline double GeoPhysics::SimpleLithology::getDensity()  const {
   return m_density;
}

inline double GeoPhysics::SimpleLithology::getThCondAn() const {
   return m_thermalcondaniso;
}

inline double GeoPhysics::SimpleLithology::getThCondVal() const {
   return m_thermalconductivityval;
}

inline double GeoPhysics::SimpleLithology::getHeatProduction() const {
   return m_heatproduction;
}

inline double GeoPhysics::SimpleLithology::getSeismicVelocity() const {
   return m_seismicvelocity;
}

inline double GeoPhysics::SimpleLithology::getVelocityExponent() const {
	return m_nExponentVelocity;
}

inline bool GeoPhysics::SimpleLithology::isIncompressible() const {
   return (m_porosity.getCompactionCoefficient() == 0.0);
}

inline double GeoPhysics::SimpleLithology::getReferenceSolidViscosity() const {
   return m_referenceSolidViscosity;
}

inline double GeoPhysics::SimpleLithology::getLithologyActivationEnergy() const {
   return m_lithologyActivationEnergy;
}

inline double GeoPhysics::SimpleLithology::getLithologyFractureGradient() const {
   return m_lithologyFractureGradient;
}

inline double GeoPhysics::SimpleLithology::getSpecificSurfArea() const {
   return  m_specificSurfaceArea;
}

inline double GeoPhysics::SimpleLithology::getGeometricVariance() const {
   return  m_geometricVariance;
}

inline double GeoPhysics::SimpleLithology::getCapC1() const {
   return  m_capC1;
}

inline double GeoPhysics::SimpleLithology::getCapC2() const {
   return  m_capC2;
}

inline DataAccess::Interface::PcKrModel GeoPhysics::SimpleLithology::getPcKrModel() const {
   return  m_PcKrModel;
}

inline double GeoPhysics::SimpleLithology::getLambdaPc() const {
   return  m_LambdaPc;
}

inline double GeoPhysics::SimpleLithology::getLambdaKr() const {
   return  m_LambdaKr;
}

inline double GeoPhysics::SimpleLithology::permeability(const double ves, const double maxVes, const double calculatedPorosity) const
{
   return m_permeability.permeability(ves, maxVes, calculatedPorosity);
}

/// Compte the derivative of the permeability function.
inline void GeoPhysics::SimpleLithology::permeabilityDerivative(
   const double ves,
   const double maxVes,
   const double calculatedPorosity,
   const double porosityDerivativeWrtVes,
   double& permeability,
   double& derivative) const
{
   m_permeability.permeabilityDerivative(ves, maxVes, calculatedPorosity, porosityDerivativeWrtVes, permeability, derivative);
}


#endif // _GEOPHYSICS__SIMPLE_LITHOLOGY_H_
