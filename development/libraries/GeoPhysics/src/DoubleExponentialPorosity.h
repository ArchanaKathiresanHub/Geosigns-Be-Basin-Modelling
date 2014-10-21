#ifndef _GEOPHYSICS__DOUBLE_EXPONENTIAL_POROSITY_H_
#define _GEOPHYSICS__DOUBLE_EXPONENTIAL_POROSITY_H_

#include "Porosity.h"



namespace GeoPhysics
{

   class DoubleExponentialPorosity : public Porosity::Algorithm {
   public:
      /// Constructor
      DoubleExponentialPorosity(double depoPorosity,
    	 double minimumMechanicalPorosity,
         double compactionIncrA, 
         double compactionIncrB,
         double compactionDecrA,
         double compactionDecrB );

      /// Return porosity with exponential function
      virtual double porosity(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const;

      /// Return the permeability model
      virtual Porosity::Model  model() const;

      /// Return FullCompThickness
      virtual double fullCompThickness(const double MaxVesValue, const double thickness, const double densitydiff, const double vesScaleFactor, const bool overpressuredCompaction) const;

      /// Return Compaction coefficients (only m_compactionincr for exponential)
      virtual double compactionCoefficient() const{ return 0.0;}
      virtual double compactionCoefficientA() const;
      virtual double compactionCoefficientB() const;

      /// Return PorosityDerivative
      virtual double porosityDerivative(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const;


   private:
      ///class members
      double m_compactionIncrA;
      double m_compactionIncrB;
      double m_compactionDecrA;
      double m_compactionDecrB;
   };
}
#endif
