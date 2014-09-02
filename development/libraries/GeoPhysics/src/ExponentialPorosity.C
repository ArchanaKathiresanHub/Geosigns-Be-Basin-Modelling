#include "ExponentialPorosity.h"
#include "GeoPhysicalConstants.h"
#include "NumericFunctions.h"
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <limits>

using namespace DataAccess;


namespace GeoPhysics
{

   ///Parameters from constructor
   ExponentialPorosity::ExponentialPorosity(double depoPorosity, double minimumMechanicalPorosity, double compactionincr, double compactiondecr)
      : m_depoporosity(depoPorosity)
      , m_minimumMechanicalPorosity(minimumMechanicalPorosity)
      , m_compactionincr(compactionincr)
      , m_compactiondecr(compactiondecr)
   {}

   ///Exponential porosity function
   double ExponentialPorosity::porosity(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const {

      double calculatedPorosity;

      bool   loadingPhase = (ves >= maxVes);

      if (includeChemicalCompaction) {

         if (loadingPhase) {
            calculatedPorosity = (m_depoporosity - m_minimumMechanicalPorosity) * exp(-m_compactionincr * maxVes) + m_minimumMechanicalPorosity;
         }
         else {
            calculatedPorosity = (m_depoporosity - m_minimumMechanicalPorosity) * exp(m_compactiondecr * (maxVes - ves) - m_compactionincr * maxVes) + m_minimumMechanicalPorosity;
         }

      }
      else {

         if (loadingPhase) {
            calculatedPorosity = m_depoporosity * exp(-m_compactionincr * maxVes);
         }
         else {
            calculatedPorosity = m_depoporosity * exp(m_compactiondecr * (maxVes - ves) - m_compactionincr * maxVes);
         }
      }

      if (includeChemicalCompaction) {
         calculatedPorosity = calculatedPorosity + chemicalCompactionTerm;
         calculatedPorosity = NumericFunctions::Maximum(calculatedPorosity, MinimumPorosity);
      }

      return calculatedPorosity;
   }

   ///surfacePorosity
   double ExponentialPorosity::surfacePorosity() const
   {
      return m_depoporosity;
   }

   Porosity::Model ExponentialPorosity::model() const
   {
      return DataAccess::Interface::EXPONENTIAL_POROSITY;
   }

   ///FullCompThickness
   double ExponentialPorosity::FullCompThickness(const double MaxVesValue, const double thickness, const double densitydiff, const double vesScaleFactor, const bool overpressuredCompaction) const {

      double c1;
      double c2;
      double Solid_Thickness;

      if (m_compactionincr == 0) {
         Solid_Thickness = thickness * (1.0 - m_depoporosity);
      }
      else {
         c1 = AccelerationDueToGravity * densitydiff * m_compactionincr;

         c2 = (1.0 - exp(-c1 * thickness)) * m_depoporosity
            * exp(-m_compactionincr * MaxVesValue);

         Solid_Thickness = log(1.0 - c2) / c1 + thickness;
      }

      return Solid_Thickness;
   }

   ///CompactionCoefficent
   double ExponentialPorosity::CompactionCoefficent() const {
      return m_compactionincr;
   }

   ///DVoidRatioDVes
   double ExponentialPorosity::DVoidRatioDVes(const double computedVoidRatio, const double ves, const double maxVes, const bool loadingPhase) const {

      /* This is the derivative of the void-ratio w.r.t. ves with singular point */
      /* If loadingPhase is TRUE then maxVes = ves */

      double dpsi, cC1, cC2, dpsidphi;

      //
      //  d psi       d psi   d phi
      //  -------  = ------   ------
      //  d sigma     d phi   d sigma
      //

      cC1 = computedVoidRatio / (1.0 + computedVoidRatio);
      cC2 = 1.0 - cC1;
      dpsidphi = 1.0 / (cC2 * cC2);

      if (loadingPhase) {
         dpsi = -m_compactionincr * cC1 * dpsidphi;
      }
      else {
         dpsi = -m_compactiondecr * cC1 * dpsidphi;
      }

      return dpsi;
   }

   ///PorosityDerivativeWrtVes
   double ExponentialPorosity::PorosityDerivativeWrtVes(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const {

      if (ves >= maxVes) {
         return -m_compactionincr;
      }
      else {
         return -m_compactiondecr;
      }
   }

   ///PorosityDerivative
   double ExponentialPorosity::PorosityDerivative(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const {

      //
      //
      //   d Phi   d Phi   d ves       d Phi
      //   ----- = ----- x -----  =  - -----  = cc * Phi
      //     dp    d ves     dp        d ves
      //
      //
      //
      double porosityDerivative;
      double porosityValue;

      porosityValue = porosity(ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm);

      if (ves >= maxVes) {
         porosityDerivative = m_compactionincr * porosityValue;
      }
      else {
         porosityDerivative = m_compactiondecr * porosityValue;
      }

      return porosityDerivative;
   }

   ///DVoidRatioDP
   double ExponentialPorosity::DVoidRatioDP(const double ves, const double maxVes, const bool loadingPhase, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const {

      const double Biot = 1.0;

      double psi, dpsi;

      /* This routine will return the derivative of the void-ratio (dpsi) */
      /* using a cut-off value of psi.  */

      double min_ves = 1.0e5; // 

      //    double min_ves = 
      //      Exponential_Minimum_Stress ( m_depositionalPorosity, maxVes, m_compactionincr, m_compactiondecr, loadingPhase );

      double ves_used = NumericFunctions::Maximum(ves, min_ves);

      double computedPorosity = porosity(ves_used, maxVes, includeChemicalCompaction, chemicalCompactionTerm);

      psi = computedPorosity / (1.0 - computedPorosity);

      dpsi = -Biot * DVoidRatioDVes(psi, ves_used, maxVes, loadingPhase);

      return dpsi;
   }

   ///MinimumMechanicalPorosity
   double ExponentialPorosity::MinimumMechanicalPorosity( ) const
   {
      return m_minimumMechanicalPorosity;
   }

}
