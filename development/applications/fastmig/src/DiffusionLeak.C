//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "migration.h"
#include "DiffusionLeak.h"

#include <iostream>

#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

#include <algorithm>
#include <limits>
#include <assert.h>

using std::vector;
using std::min;
using std::numeric_limits;

namespace migration {

   const double MA_TO_S = 60 * 60 * 24 * 365.25 * 1e6;
   const double NEARZERO = numeric_limits<double>::min() * 1e10;

   DiffusionLeak::DiffusionLeak(const vector<DiffusionLeak::OverburdenProp>& overburdenProps, 
                                const double& sealFluidDensity, 
                                const double& penetrationDistance, const double& maxPenetrationDistance, 
                                const DiffusionCoefficient& coefficient, const double& maxTimeStep, const double& maxFluxError):
      m_overburdenProps(overburdenProps),
      m_sealFluidDensity(sealFluidDensity),
      m_penetrationDistance(penetrationDistance),
      m_maxPenetrationDistance(maxPenetrationDistance),
      m_coefficient(coefficient),
      m_maxTimeStep(maxTimeStep),
      m_maxFluxError(maxFluxError),
      m_Deff(0),
      m_maxPenetrationDistanceForDeff(-numeric_limits<double>::max())
   {
      m_maxPenetrationDistance = min (m_maxPenetrationDistance, getOverburdenThickness ());
      m_penetrationDistance = min (m_penetrationDistance, m_maxPenetrationDistance);
   }

   /// Compute the leakages for time between intervalStartTime and intervalEndTime.
   ///
   /// Compute the leakages for time between intervalStartTime and intervalEndTime and the following parameters
   /// @param[in] diffusionStartTime:
   /// @param[in] intervalStartTime:
   /// @param[in] intervalEndTime:
   /// @param[in] componentWeight:
   /// @param[in] molarFraction:
   /// @param[in] solubility:
   /// @param[in] surfaceArea:
   /// @param[out] lost:
   ///
   /// Note the signs of the times. timeInterval is in fastmig defined as previous - current. 
   /// Because all times are positive and because previous > current, timeInterval is therefore 
   /// always positive.  m_control.maxTimeStep() is also always positive.  Diffusion leakages 
   /// only depend upon the timeInterval, so the previous snapshot time (or the initial 
   /// stepStartTime in the implementation) is not provided.

   void DiffusionLeak::compute(const double& diffusionStartTime, const double & intervalStartTime, const double & intervalEndTime,
                               const double& componentWeight, const double& molarFraction, const double& solubility, const double& surfaceArea, double& lost, const double& gasRadius)
   {
      assert(intervalStartTime - intervalEndTime > 0.0);

      /// maxError in m s-1
      double maxError = m_maxFluxError / (MA_TO_S * solubility * molarFraction);

      double stepStartTime = intervalStartTime;

      double stepSize = m_maxTimeStep;
      double stepEndTime = intervalStartTime;

      double decreasingComponentWeight = componentWeight;
      // Perform diffusion time steps until timeInterval has been reached
      while (stepStartTime > intervalEndTime && lost < componentWeight)
      {
         assert(componentWeight >= 0.0);
         if (componentWeight == 0.0)
            break;

         assert(m_penetrationDistance <= m_maxPenetrationDistance);

         if (m_penetrationDistance == m_maxPenetrationDistance)
         {
            /// Steady state or when m_penetration distance in transient mode has reached m_maxPenetrationDistance
            stepEndTime = intervalEndTime;

            //the mass lost by diffusion is updated only once
            performDiffusionTimeStep (diffusionStartTime, stepStartTime, stepEndTime, decreasingComponentWeight,
                                      molarFraction, solubility, surfaceArea, lost, gasRadius);
         }
         else
         {
            /// This is the transient case			 

            stepEndTime = stepStartTime - stepSize; // Initial timestep
            stepEndTime = max (stepEndTime, intervalEndTime);
            stepSize = stepStartTime - stepEndTime; 

            assert(stepSize > 0.0);
            assert(stepEndTime < stepStartTime);

            // Try to perform the time step until it succeeds, that is, the error made is 
            // within m_maxError:
            double tmpLost = lost;
            while (!performDiffusionTimeStep(diffusionStartTime, stepStartTime, stepEndTime, decreasingComponentWeight, 
                                             molarFraction, solubility, surfaceArea, lost, gasRadius, &maxError))
            {
               // The error was too large, so reduce the stepSize:
               stepSize /= 2;
               stepEndTime = stepStartTime - stepSize;

               // revert back to the old value of lost
               lost = tmpLost;
            }

            decreasingComponentWeight = componentWeight - lost;

            // Increase the stepSize for the penetrationDistance front may have passed 
            // a formation surface:
            stepSize *= 16;
            stepSize = min(m_maxTimeStep, stepSize);
         }

         stepStartTime = stepEndTime; 
      }

      if (lost > componentWeight) lost = componentWeight;
   }

   bool DiffusionLeak::performDiffusionTimeStep(const double & diffusionStartTime, const double& stepStartTime, const double& stepEndTime, 
                                                const double& componentWeight, const double& molarFraction, const double& solubility, 
                                                const double& surfaceArea, double& lost, const double& gasRadius,const double* maxError)
   {
      assert(stepStartTime > stepEndTime);

      // Deff is the effective diffusion coefficent and effPenetrationDistance is the penetration distance
      double Deff, effPenetrationDistance;

      // compute Deff and penetrationDistance to use for this time step
      if (!updatePenetrationDistance(diffusionStartTime, stepStartTime, stepEndTime, Deff, effPenetrationDistance, gasRadius,
                                     maxError))
         // The error is too large. Let the calling routine reduce its timestep:
      {
         return false;
      }

      // Calculate the flux in weight per second square meter:
      double weight_flux = 0;

      if (Deff > NEARZERO && effPenetrationDistance > NEARZERO)
         // compute the 'steady state' weight_flux
      {
         /// Diffusion flux: solubility here is already in kg m-3 water
         weight_flux = MA_TO_S * solubility * molarFraction * Deff / effPenetrationDistance;
      }

      // Compute the maximal charge reduction given weight_flux:
      double diffused_weight = weight_flux * (stepStartTime - stepEndTime) * surfaceArea;
      
      assert (diffused_weight > 0);

      lost += (diffused_weight > componentWeight) ? componentWeight : diffused_weight;

      return true;
   }

   // Compute the diffusion coefficient and resulting penetrationDistance.
   // Return false if the error is too large
   bool DiffusionLeak::updatePenetrationDistance(const double & diffusionStartTime, const double& stepStartTime, const double& stepEndTime, 
                                                 double& Deff, double& effPenetrationDistance, const double& gasRadius, const double* maxError)
   {
      // use the initial penetrationDistance to compute an initial diffusion coefficient
      Deff = computeDeff(m_penetrationDistance, gasRadius);

      if (m_penetrationDistance < m_maxPenetrationDistance)
      {
         // use the initial diffusion coefficient to compute the penetrationDistance at the end of the timestep.
         double newPenetrationDistance = propagatePenetrationDistance (Deff, diffusionStartTime, stepStartTime, stepEndTime);
         newPenetrationDistance = min (newPenetrationDistance, m_maxPenetrationDistance);

         if (maxError)
         {
            // compute what the diffusion coefficient would be given the computed penetrationDistance
            double newDeff = computeDeff(newPenetrationDistance, gasRadius);

            /// m_penetrationDistance can be zero, resulting in an incorrect effPenetrationDistance
            if (m_penetrationDistance != 0)
               effPenetrationDistance = 0.5 * (m_penetrationDistance + newPenetrationDistance);
            else
               effPenetrationDistance = newPenetrationDistance;

            double approxError = fabs (Deff - newDeff) / effPenetrationDistance;

            // if the error was too large, we need a smaller timestep.
            if (approxError > *maxError)
               return false;
         }

         // Make sure the penetrationDistance doesn't decrease (i.e. we can't use assert(
         // newPenetrationDistance >= m_penetrationDistance) here, as this assumption is not valid):
         if (newPenetrationDistance > m_penetrationDistance)
         {
            // We use in the calculation for the diffusion leakage the average of m_penetrationDistance 
            // and newPenetrationDistance:

            /// m_penetrationDistance can be zero, resulting in an incorrect effPenetrationDistance
            if (m_penetrationDistance != 0)
               effPenetrationDistance = 0.5 * (m_penetrationDistance + newPenetrationDistance);
            else
               effPenetrationDistance = newPenetrationDistance;

            m_penetrationDistance = newPenetrationDistance;
         }
         else
            // In this case, we use for the calculation of the diffusion leakage the previous value 
            // m_penetrationDistance:
            effPenetrationDistance = m_penetrationDistance;
      }
      else
         effPenetrationDistance = m_maxPenetrationDistance;

      return true;
   }

   // Using penetrationDistance as the overburden thickness, compute the effective steady state 
   // diffusion coefficient Deff:
   double DiffusionLeak::computeDeff(const double& penetrationDistance, const double& gasRadius) const
   {
      assert(penetrationDistance >= 0.0);

      if (penetrationDistance <= m_maxPenetrationDistanceForDeff)
         return m_Deff;

      // The formula used in the calculation is:
      //
      //    z             z
      //     tot     --    i
      //    ----  =  >   -----
      //             --
      //    D         i   D
      //     eff           i
      //
      // with i the overburden layers, z_tot the total used thickness and z_i the thickness of a particular 
      // overburden layer within penetrationDistance.

      double sum = 0.0;
      double z_tot = 0.0;

      // Iterate over all the layers of m_overburdenProperties until penetrationDistance is reached:
      vector<OverburdenProp>::const_iterator i = m_overburdenProps.begin();
      assert(i != m_overburdenProps.end());

      // Calculate the penetrationDistance rounded up to the next formation thickness:
      assert((*i).m_thickness > 0.0);
      double remainingPenetrationDistance = max(penetrationDistance, 0.5 * (*i).m_thickness);
      m_maxPenetrationDistanceForDeff = 0.0;

      for (; i != m_overburdenProps.end() && remainingPenetrationDistance > 0.0; ++i)
      {
         assert((*i).m_thickness > 0.0);

         double z_i = min (remainingPenetrationDistance, (*i).m_thickness);

         // Calculate the average formation porosity:
         double formPorosity = Percentage2Fraction * ((*i).m_topPorosity + (*i).m_basePorosity) / 2.0;

         // Calculate the average formation temperature:
         double formTemperature = ((*i).m_topTemperature + (*i).m_baseTemperature) / 2.0;
         
         // Calculate the average formation viscosity
         double formViscosity = ((*i).m_topViscosity + (*i).m_baseViscosity) / 2.0;

         // Calculate the diffusion coefficient dcEff for this formation:
         //double diffusionCoef = m_coefficient.coefficient(formTemperature, formPorosity);
 
         // Calculate diff coefficent with Stokes-Einstein equation
         double diffusionCoef = m_coefficient.coefficient(formTemperature, formPorosity, formViscosity, gasRadius);

         // Keep track of the z_tot and Needed to calculate the overall effective diffusion coefficient Deff:
         sum += z_i / diffusionCoef;

         z_tot += z_i;

         // decrease the loop control variable
         remainingPenetrationDistance -= z_i;
         m_maxPenetrationDistanceForDeff += z_i;
      }

      m_Deff = z_tot / sum;

      return m_Deff;
   }

   double DiffusionLeak::getOverburdenThickness ()
   {
      double thickness = 0;
      vector<OverburdenProp>::const_iterator i;
      for (i = m_overburdenProps.begin(); i != m_overburdenProps.end(); ++i)
      {
         thickness += (*i).m_thickness;
      }

      return thickness;
   }

   // Given the diffusion coefficient, calculate the penetrationDistance at the end of the time step.
   double DiffusionLeak::propagatePenetrationDistance (const double& Deff, const double & diffusionStartTime, const double& stepStartTime, 
                                                       const double& stepEndTime)
   {
      double increment = sqrt (Deff * M_PI * MA_TO_S) *(sqrt (diffusionStartTime - stepEndTime) - sqrt (diffusionStartTime - stepStartTime));
      
      double newPenetrationDistance = m_penetrationDistance + increment;
      return newPenetrationDistance;
   }

} // namespace migration
