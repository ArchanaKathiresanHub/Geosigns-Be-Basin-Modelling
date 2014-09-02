#ifndef _GEOPHYSICS__SOIL_MECHANICS_POROSITY_H_
#define _GEOPHYSICS__SOIL_MECHANICS_POROSITY_H_

#include "Porosity.h"
#include "CompoundLithology.h"
#include <cmath>

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

      /// Class members
      double  m_depoporosity;
      double  m_minimumMechanicalPorosity;
      double  m_soilMechanicsCompactionCoefficient;
      double  m_depositionVoidRatio;
   };
}
#endif
