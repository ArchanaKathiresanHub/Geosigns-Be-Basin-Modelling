#ifndef _GEOPHYSICS__EXPONENTIAL_POROSITY_H_
#define _GEOPHYSICS__EXPONENTIAL_POROSITY_H_

#include "Porosity.h"



namespace GeoPhysics
{

   class ExponentialPorosity : public Porosity::Algorithm {
   public:
      /// Constructor
      ExponentialPorosity(double depoPorosity,
    	 double minimumMechanicalPorosity,
         double compactionIncr,
         double compactionDecr);

      /// Return porosity with exponential function
      virtual double porosity(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const;

      /// \brief Determine if the porosity model is incompressible.
      virtual bool isIncompressible () const;

      /// Return the permeability model
      virtual Porosity::Model  model() const;

      /// Return FullCompThickness
      virtual double fullCompThickness(const double MaxVesValue, const double thickness, const double densitydiff, const double vesScaleFactor, const bool overpressuredCompaction) const;

      /// Return Compaction coefficients
      virtual double compactionCoefficient() const;
      
      /// Return Compaction coefficients 
      virtual double compactionCoefficientA() const { return 0.0; }

      /// Return Compaction coefficients 
      virtual double compactionCoefficientB() const { return 0.0; }

      /// Return PorosityDerivative
      virtual double porosityDerivative(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const;
      

   private:
      ///class members
      double  m_compactionIncr;
      double  m_compactionDecr;
   };
}
#endif
