//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "Permeability.h"
#include "PermeabilitySandStone.h"
#include "PermeabilityMudStone.h"
#include "PermeabilityMultiPoint.h"
#include "PermeabilityImpermeable.h"

#include <sstream>

namespace GeoPhysics
{

Permeability
   :: Permeability(  double permAniso, Algorithm * algorithm)
   : m_algorithm(algorithm)
   , m_permeabilityAniso(permAniso)                               
{}

Permeability Permeability::create(
         Model permeabilityModel, double permAniso,
         double depoPorosityPercentage, double depoPermeability, 
         double permeabilityIncr, double permeabilityDecr,
         const std::string & porosityMultiPointSamplesString,
         const std::string & permeabilityMultiPointSamplesString,
         int numberOfMultiPointSamples
         )
{
   if (permAniso <= 0.0) permAniso = 1.0;
  switch(permeabilityModel)
  {
     case DataAccess::Interface::SANDSTONE_PERMEABILITY:
        return Permeability( permAniso, new PermeabilitySandStone(depoPorosityPercentage/100.0, depoPermeability, permeabilityIncr) );

     case DataAccess::Interface::MUDSTONE_PERMEABILITY:
        return Permeability( permAniso,new PermeabilityMudStone(depoPermeability, permeabilityIncr, permeabilityDecr));

     case DataAccess::Interface::MULTIPOINT_PERMEABILITY: 
        {
           std::vector<double> porositySamples 
              = parseCoefficientsFromString( porosityMultiPointSamplesString   );
           std::vector<double> permeabilitySamples
              = parseCoefficientsFromString( permeabilityMultiPointSamplesString  );

           porositySamples.resize( numberOfMultiPointSamples );
           permeabilitySamples.resize( numberOfMultiPointSamples );

           // convert porosity from percentages into fractions
           for (int i = 0; i < numberOfMultiPointSamples; ++i)
              porositySamples[i] /= 100.0;

           return Permeability( permAniso, new PermeabilityMultiPoint( depoPorosityPercentage / 100.0 , porositySamples, permeabilitySamples));
        }

      case DataAccess::Interface::IMPERMEABLE_PERMEABILITY:
      case DataAccess::Interface::NONE_PERMEABILITY:
        {
           return Permeability( permAniso, new PermeabilityImpermeable( depoPermeability, permeabilityModel) );
        }

      default:
        assert(false);
  }

  return Permeability(permAniso, 0);
}

Permeability Permeability::createMultiPoint(double permAniso,
                                            double depoPorosityPercentage,
                                            const std::vector<double>& porosityPercentageSamples,
                                            const std::vector<double> & permeabilitySamples)
{
   std::vector<double> porosityFractions = porosityPercentageSamples;

   for (unsigned i = 0; i < porosityFractions.size(); ++i)
      porosityFractions[i] /= 100.0;

   return Permeability(permAniso, new PermeabilityMultiPoint(depoPorosityPercentage/100.0, porosityFractions, permeabilitySamples));
}


std::vector<double> Permeability::parseCoefficientsFromString ( const std::string & string)
{
   std::istringstream stream(string);
   std::vector<double> result;

   while (stream)
   {
      double value = 0.0;

      stream >> value;

      if (!stream)
         break;

      result.push_back( value );
   }

   return result;
}

Permeability& Permeability::operator= (const Permeability& permeability){
   if (this != &permeability) {
      m_algorithm = permeability.m_algorithm;
   }
   return *this;
}

Permeability::Permeability( const Permeability& permeability )
   :m_algorithm( permeability.m_algorithm )
{}


}
