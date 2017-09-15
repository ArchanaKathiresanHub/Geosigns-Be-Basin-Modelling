//
// Copyright (C) 2010-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Composition.h"
#include "EosPack.h"
#include "migration.h"
#include "rankings.h"


#ifdef DEBUG_PVT
#include "waterDensity.h"
#endif

// std library
#include <assert.h>
#include <iostream>
#include <vector>
using namespace std;

#include "ConstantsPhysics.h"
using Utilities::Physics::GasRadius;
#include "ConstantsMathematics.h"
using Utilities::Maths::MegaPaToPa;
using Utilities::Maths::CelciusToKelvin;

using namespace migration;
using namespace CBMGenerics;

double migration::computeGorm(const Composition& gas, const Composition& oil)
{
   vector<double> gasWeights = gas.getWeights();
   vector<double> oilWeights = oil.getWeights();

   vector<double> weights(NumComponents, 0.0);
   for (int i = 0; i < NumComponents; ++i) {
      weights[i] += gasWeights[i];
      weights[i] += oilWeights[i];
   }
      
   double gorm = pvtFlash::gorm(weights);
   return gorm;
}

Composition::Composition (void)
{
   reset ();
}

Composition::~Composition ()
{
}

void Composition::reset (void)
{
   for (unsigned int componentId = 0; componentId < NumComponents; ++componentId)
   {
      reset ((ComponentId) componentId);
   }
   resetDensity ();
   resetViscosity ();
}

void Composition::set (const Composition & composition)
{
   for (unsigned int componentId = 0; componentId < NumComponents; ++componentId)
   {
      set ((ComponentId) componentId, composition.getWeight ((ComponentId) componentId));
   }
}

void Composition::add (const Composition & composition)
{
   for (unsigned int componentId = 0; componentId < NumComponents; ++componentId)
   {
      add ((ComponentId) componentId, composition.getWeight ((ComponentId) componentId));
   }
}

void Composition::subtract (const Composition & composition)
{
   for (unsigned int componentId = 0; componentId < NumComponents; ++componentId)
   {
      subtract ((ComponentId) componentId, composition.getWeight ((ComponentId) componentId));
   }
}

void Composition::addFraction (const Composition & composition, double fraction)
{
   for (unsigned int componentId = 0; componentId < NumComponents; ++componentId)
   {
      add ((ComponentId) componentId, fraction * composition.getWeight ((ComponentId) componentId));
   }
}

void Composition::subtractFraction (const Composition & composition, double fraction)
{
   for (unsigned int componentId = 0; componentId < NumComponents; ++componentId)
   {
      subtract ((ComponentId) componentId, fraction * composition.getWeight ((ComponentId) componentId));
   }
}

bool Composition::isEmpty (void) const
{
   for (unsigned int componentId = 0; componentId < NumComponents; ++componentId)
   {
      if (!isEmpty ((ComponentId) componentId)) return false;
   }
   return true;
}

double Composition::getWeight (void) const
{
   double total = 0;
   for (unsigned int componentId = 0; componentId < NumComponents; ++componentId)
   {
      total += getWeight ((ComponentId) componentId);
   }

   return total;
}

void Composition::setWeight (const double& weight)
{
   double correction = weight / getWeight();
   for (int componentId = 0; componentId < NumComponents; ++componentId)
   {
      set((ComponentId) componentId, getWeight ((ComponentId) componentId) * correction);
   }
}

double Composition::moles(ComponentId componentId, const double& gorm) const
{
   assert( ComponentId::FIRST_COMPONENT <= componentId && componentId <= ComponentId::LAST_COMPONENT);
   pvtFlash::EosPack& eosPack = pvtFlash::EosPack::getInstance();
   return getWeight(componentId) / eosPack.getMolWeightLumped(componentId, gorm);
}

double Composition::getVolume (void) const
{
   double weight = getWeight ();
   if (weight == 0) return 0;
   
   return weight / getDensity ();
}

void Composition::setVolume (const double& volume)
{
   // We can't deal here with the situation volume > 0.0 and getVolume() == 0.0:
   assert(!(getVolume() == 0.0 && volume > 0.0));

   // If volume is equal to zero, make everything zero:
   double correction = volume != 0.0 ? 
      volume / getVolume() :
      0.0;
   for (int componentId = 0; componentId < NumComponents; ++componentId)
   {
      set((ComponentId) componentId, getWeight ((ComponentId) componentId) * correction);
   }
}

// Compute the lost composition due to biodegradation: switch between the legacy and new biodegradation module depending on the m_legacyMigration flag  
void Composition::computeBiodegradation(const double& timeInterval, const double& temperature,
   const Biodegrade& biodegrade, Composition& compositionLost, const double fractionVolumeBiodegraded, const bool legacyMigration) const
{
   double inputComponents[NumComponents];
   double lostComponents[NumComponents];

   int component;

   for (component = 0; component < NumComponents; ++component)
   {
      inputComponents[component] = getWeight ((ComponentId) component) * fractionVolumeBiodegraded;
      lostComponents[component] = 0.0;
   }

   if (legacyMigration)
   {
      biodegrade.calculateLegacy (timeInterval, temperature, inputComponents, lostComponents);
   }
   else
   {
      biodegrade.calculate (timeInterval, temperature, inputComponents, lostComponents);
   }
   
   for (component = 0; component < NumComponents; ++component)
   {
      compositionLost.set ((ComponentId) component, lostComponents[component]);
   }
}

void Composition::computeDiffusionLeakages(const double& diffusionStartTime, const double & intervalStartTime, const double & intervalEndTime,
      const vector<double>& solubilities, const double& surfaceArea, vector<DiffusionLeak*>& diffusionLeaks, const double& gorm, 
      Composition* compositionOut, Composition* compositionLost) const
{
#ifdef NOTONLYC1C2C3
   assert (solubilities.size () == diffusionLeaks.size ());
#endif
   assert (NumComponents >= diffusionLeaks.size ());

   double totalMoles = 0.0;

   unsigned int component;
   for (component = 0; component < NumComponents; ++component)
   {
      totalMoles += moles ((ComponentId) component, gorm);
   }
   
   assert (totalMoles >= 0.0);

   if (totalMoles == 0.0)
      return;

   unsigned int index;
   for (index = 0; index < diffusionLeaks.size (); ++index)
   {
      ComponentId componentId = (ComponentId) (ComponentId::C1 - index);
      double lost = 0.0;
      double molarFraction = moles (componentId, gorm) / totalMoles;

      assert (molarFraction >= 0.0);

      if (molarFraction == 0.0)
         continue;

      double gasRadius = GasRadius[index];

      diffusionLeaks[index]->compute (diffusionStartTime, intervalStartTime, intervalEndTime, getWeight (componentId), molarFraction,
                                      solubilities[index], surfaceArea, lost, gasRadius);

      compositionOut->set (componentId, getWeight (componentId) - lost);
      compositionLost->set (componentId, lost);

   }
}

void Composition::computePVT (double temperature, double pressure, Composition * compositionsOut)
{
   double inputComponents[NumComponents];
   double outputViscosities[NumPhases];
   double outputDensities[NumPhases];
   double outputComponents[NumPhases][NumComponents];

   static double accumError = 0;
   static double accumLoss = 0;

   double totalMass = .0;

   temperature = Max (temperature, StockTankTemperature);
   pressure = Max (pressure, StockTankPressure);


   // Prepare input for PvtPack calculations
   unsigned int phase;
   for (phase = 0; phase < NumPhases; phase++)
   {
      for (unsigned int component = 0; component < NumComponents; component++)
      {
         if (phase == 0)
         {
            totalMass += getWeight ((ComponentId) component);
            inputComponents[component] = getWeight ((ComponentId) component);
            if (std::isnan( inputComponents[component] )) inputComponents[component] = 0;
         }

         // initialize
         outputComponents[phase][component] = 0.0;
      }

      compositionsOut[phase].reset ();
      assert (compositionsOut[phase].getWeight () == 0.0);

      compositionsOut[phase].setViscosity (.0);
      compositionsOut[phase].setDensity (.0);
   }


   // if there is no component we should not run PVT at all it returns
   // nand values!!! -> the trap remains still empty!
   if (totalMass < MinimumMass)
   {
      // return, Compositions are clean (empty)
      return;
   }

   bool flashed = pvtFlash::EosPack::getInstance().computeWithLumping (temperature + CelciusToKelvin, pressure * MegaPaToPa, inputComponents, outputComponents, outputDensities, outputViscosities);
#if 1
   if (!flashed)
   {
      cerr << "PVT FAILED: T = " << temperature + CelciusToKelvin << " K, P = " << pressure * MegaPaToPa << " Pa, Components = ";

      for (unsigned int comp = 0; comp < NumComponents; comp++)
      {
	      cerr << getWeight ((ComponentId) comp) << ", ";
      }
      cerr << endl;
      cerr.flush ();
   }
#endif

   // convert PvtPack output to Composition content
   for (phase = 0; phase < NumPhases; phase++)
   {
      compositionsOut[phase].setDensity (outputDensities[phase]);
      compositionsOut[phase].setViscosity (outputViscosities[phase]);

      for (unsigned int component = 0; component < NumComponents; component++)
      {
         // compositionsOut[phase].add ((ComponentId) component, 0.5 * inputComponents[component]);
         compositionsOut[phase].add ((ComponentId) component, outputComponents[phase][component]);
      }
   }

   // error handling
   double pvtError = getWeight ();

   for (phase = 0; phase < NumPhases; phase++)
   {
      pvtError -= compositionsOut[phase].getWeight ();
   }

   accumError += Abs (pvtError);
   accumLoss += -pvtError;

#ifdef DEBUG_PVT
   double fluidDensity = CBMGenerics::waterDensity::compute(CBMGenerics::waterDensity::Calculated, 0, .22, temperature, pressure);
   if (fluidDensity < compositionsOut[OIL].getDensity ())
   {
      cerr << GetRank () << "::PVT: WARNING: calculated oil phase density (" << compositionsOut[OIL].getDensity () <<
	 ") exceeds calculated fluid density (" << fluidDensity << ")" << endl;
      cerr << GetRank () << "::PVT:          Components = ";

      for (unsigned int component = 0; component < NumComponents; ++component)
      {
	 cerr << getWeight ((ComponentId) component) << ", ";
      }
      cerr << endl;

      cerr << GetRank () << "::PVT:          T = " << temperature << " C, P = " << pressure <<
	 " MPa, weights out = (" << compositionsOut[GAS].getWeight () <<
	 ", " << compositionsOut[OIL].getWeight () << ")" <<
	 ", densities out = (" << compositionsOut[GAS].getDensity () <<
	 ", " << compositionsOut[OIL].getDensity () << ")" << endl;

      cerr.flush ();
   }
#endif

#if 0
   static int number = 0;

   if (compositionsOut[OIL].getDensity() > 1200.0) {
      cerr << "PVT number: " << number << endl;
      cerr << "WARNING: oil density: " << compositionsOut[OIL].getDensity() << 
	" is higher than water density: " << 1074.0 << "." << endl;
      cerr << "         Temperature: " << temperature << " C , pressure: " << pressure << " MPa." << endl;
      cerr << "Composition of oil:" << endl;
      vector<double> oilWeights = compositionsOut[OIL].getWeights();
      for (int i = 0; i < oilWeights.size(); ++i)
         cerr << ComponentIdNames[i] << ": " << oilWeights[i] << " kg." << endl;
   }
   ++number;
#endif
}

Composition & Composition::operator= (const Composition & original)
{
   if (this != & original)
   {
      set (original);
      setDensity (original.getDensity ());
      setViscosity (original.getViscosity ());
   }

   return * this;
}

ostream & migration::operator<< (ostream & stream, Composition &composition)
{
      return stream << & composition;
}

ostream & migration::operator<< (ostream & stream, Composition * composition)
{
   if (!composition)
   {
      stream << "null";
   }
   else
   {
      for (int component = ComponentId::FIRST_COMPONENT; component < ComponentId::NUMBER_OF_SPECIES; ++component)
      {
	 stream << composition->getWeight ((ComponentId) component) << ", ";
      }
      stream << "V: " << composition->getViscosity () << ", ";
      stream << "D: " << composition->getDensity ();
   }
   return stream;
}
