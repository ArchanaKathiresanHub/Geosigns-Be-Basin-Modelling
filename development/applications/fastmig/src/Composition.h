//
// Copyright (C) 2010-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef MIGRATION_COMPOSITION_H
#define MIGRATION_COMPOSITION_H

#include "Biodegrade.h"
#include "migration.h"
#include "DiffusionLeak.h"

// Eospack library
#include "EosPack.h"
using namespace pvtFlash;

// std library
#include <cmath>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <map>
#include <vector>

// CBMGenerics library
#include "ComponentManager.h"
typedef CBMGenerics::ComponentManager::SpeciesNamesId ComponentId;

/// Composition Class
namespace migration
{
   double computeGorm(const Composition& gas, const Composition& oil);

   class Composition
   {
      friend void InitializeRequestTypes (void);
      public:
         /// Constructor
         Composition (void);

         /// Destructor
         virtual ~Composition (void);

         inline void reset       (ComponentId componentId);
         inline void set         (ComponentId componentId, double weight);
         inline void add         (ComponentId componentId, double weight);
         inline void subtract    (ComponentId componentId, double weight);
         inline bool isEmpty     (ComponentId componentId) const;
         inline double getWeight (ComponentId componentId) const;
         inline std::vector<double> getWeights() const;
         inline double moles(ComponentId componentId, const double& gorm) const;

         inline void setViscosity (double viscosity);
         inline double getViscosity (void) const;
         inline void resetViscosity (void);

         inline void setDensity (double density);
         inline double getDensity (void) const;
         inline void resetDensity (void);

         void reset (void);
         void set      (const Composition & composition);
         void add      (const Composition & composition);
         void subtract (const Composition & composition);
         bool isEmpty (void) const;
         double getWeight (void) const;
         void setWeight(const double& weight);

         double getVolume (void) const;
         void setVolume(const double& volume);

         void addFraction      (const Composition & composition, double fraction);
         void subtractFraction (const Composition & composition, double fraction);

         void computeBiodegradation(const double& timeInterval, const double& temperature,
         const Biodegrade& biodegrade, Composition& compositionLost, const double fractionVolumeBiodegraded) const;


         void computeDiffusionLeakages(const double& diffusionStartTime, const double & intervalStartTime, const double & intervalEndTime, const std::vector<double>& solubilities,
         const double& surfaceArea, std::vector<DiffusionLeak*>& diffusionLeaks, const double& gorm,
         Composition* compositionOut, Composition* compositionLost) const;

         void computePVT (double temperature, double pressure, Composition * compositionsOut);

         Composition & operator= (const Composition & original);

      private:

        double m_components[ComponentId::NUMBER_OF_SPECIES];
        double m_density;
        double m_viscosity;
   };

   ostream & operator<< (ostream & stream, Composition & composition);
   ostream & operator<< (ostream & stream, Composition * composition);
}

void migration::Composition::reset (ComponentId componentId)
{
   assert( ComponentId::FIRST_COMPONENT <= componentId && componentId <= ComponentId::LAST_COMPONENT);
   m_components[(int) componentId] = 0;
}

void migration::Composition::set (ComponentId componentId, double weight)
{
   assert( ComponentId::FIRST_COMPONENT <= componentId && componentId <= ComponentId::LAST_COMPONENT);
   if (std::isnan( weight )) weight = 0;
   m_components[(int) componentId] = weight;
   if (m_components[componentId] < 0) m_components[componentId] = 0;
}

void migration::Composition::add (ComponentId componentId, double weight)
{
   assert( ComponentId::FIRST_COMPONENT <= componentId && componentId <= ComponentId::LAST_COMPONENT);
   if (std::isnan( weight )) weight = 0;
   m_components[(int) componentId] += weight;
   if (m_components[componentId] < 0) m_components[componentId] = 0;
}

void migration::Composition::subtract (ComponentId componentId, double weight)
{
   assert( ComponentId::FIRST_COMPONENT <= componentId && componentId <= ComponentId::LAST_COMPONENT);
   if (std::isnan( weight )) weight = 0;
   m_components[(int) componentId] -= weight;
   if (m_components[componentId] < 0) m_components[componentId] = 0;
}

bool migration::Composition::isEmpty (ComponentId componentId) const
{
   assert( ComponentId::FIRST_COMPONENT <= componentId && componentId <= ComponentId::LAST_COMPONENT);
   return (m_components[(int) componentId] <= 0);
}

double migration::Composition::getWeight (ComponentId componentId) const
{
   assert( ComponentId::FIRST_COMPONENT <= componentId && componentId <= ComponentId::LAST_COMPONENT);
   return m_components[(int) componentId];
}

vector<double> migration::Composition::getWeights () const
{
   std::vector<double> weights( ComponentId::NUMBER_OF_SPECIES );
   for (int componentId = ComponentId::FIRST_COMPONENT; componentId <= ComponentId::LAST_COMPONENT; ++componentId)
      weights[componentId] = m_components[(int) componentId];
   return weights;
}

void migration::Composition::setViscosity (double viscosity)
{
   m_viscosity = viscosity;
}

double migration::Composition::getViscosity (void) const
{
   return m_viscosity;
}

void migration::Composition::resetViscosity (void)
{
   m_viscosity = -1;
}

void migration::Composition::setDensity (double density)
{
#if 0
   assert (!std::isnan (density));
#endif
   m_density = density;
}

double migration::Composition::getDensity (void) const
{
   return m_density;
}

void migration::Composition::resetDensity (void)
{
   m_density = 0;
}


#endif // MIGRATION_COMPOSITION_H

