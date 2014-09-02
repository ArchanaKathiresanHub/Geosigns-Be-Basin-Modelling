#ifndef _GEOPHYSICS__EXPONENTIAL_POROSITY_H_
#define _GEOPHYSICS__EXPONENTIAL_POROSITY_H_

#include "Porosity.h"
#include <cmath>


namespace GeoPhysics
{

   class ExponentialPorosity : public Porosity::Algorithm {
   public:
      /// Constructor
      ExponentialPorosity(double depoPorosity,
         double minimumMechanicalPorosity,
         double compactionincr,
         double compactiondecr);

      /// Return porosity with exponential function
      virtual double porosity(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const;

      /// Return the surfacePorosity
      virtual double surfacePorosity() const;

      /// Return the permeability model
      virtual Porosity::Model  model() const;

      /// Return FullCompThickness
      virtual double FullCompThickness(const double MaxVesValue, const double thickness, const double densitydiff, const double vesScaleFactor, const bool overpressuredCompaction) const;

      /// Return Compaction coefficients (only m_compactionincr for exponential)
      virtual double CompactionCoefficent() const;

      /// Return DVoidRatioDVes
      virtual double DVoidRatioDVes(const double computedVoidRatio, const double ves, const double maxVes, const bool loadingPhase) const;

      /// Return PorosityDerivativeWrtVes
      virtual double PorosityDerivativeWrtVes(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const;

      /// Return PorosityDerivative
      virtual double PorosityDerivative(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const;

      /// Return DVoidRatioDP
      virtual double DVoidRatioDP(const double ves, const double maxVes, const bool loadingPhase, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const;
      
      /// Return minimumMechanicalPorosity
      virtual double  MinimumMechanicalPorosity( ) const;

   private:
      ///class members
      double  m_depoporosity;
      double  m_minimumMechanicalPorosity;
      double  m_compactionincr;
      double  m_compactiondecr;
   };
}
#endif
