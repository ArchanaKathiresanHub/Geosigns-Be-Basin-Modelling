#ifndef _GEOPHYSICS__SOIL_MECHANICS_POROSITY_H_
#define _GEOPHYSICS__SOIL_MECHANICS_POROSITY_H_

#include "Porosity.h"



namespace GeoPhysics
{

   class soilMechanicsPorosity : public Porosity::Algorithm {
   public:
      /// Constructor
      soilMechanicsPorosity(double depoPorosity,
         double minimumMechanicalPorosity,
         double soilMechanicsCompactionCoefficient,
         double depositionVoidRatio);

      /// Return porosity with soilMechanicsPorosity
      virtual double porosity(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const;

      /// \brief Determine if the porosity model is incompressible.
      virtual bool isIncompressible () const;

      /// Return the permeability model
      virtual Porosity::Model  model() const;

      /// Return Compaction coefficients 
      virtual double compactionCoefficient() const;
      
      /// Return Compaction coefficients 
      virtual double compactionCoefficientA() const { return 0.0; }
                  
      /// Return Compaction coefficients 
      virtual double compactionCoefficientB() const { return 0.0; }

      /// Return PorosityDerivative
      virtual double porosityDerivative(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const;


   private:

      /// Class members
      double  m_soilMechanicsCompactionCoefficient;
      double  m_depositionVoidRatio;
   };
}
#endif
