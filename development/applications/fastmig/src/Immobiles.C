//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifdef USEOTGC
#include <assert.h>
#include "EosPack.h"

#ifdef sgi
#ifdef _STANDARD_C_PLUS_PLUS
#include <iostream>
#else // !_STANDARD_C_PLUS_PLUS
#include<iostream.h>
#endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
#include <iostream>
#endif // sgi

#include <vector>

using namespace std;


#include "Immobiles.h"
#include "migration.h"
#include "rankings.h"

using namespace migration;


Immobiles::Immobiles (void)
{
   reset ();
}

Immobiles::~Immobiles ()
{
}

void Immobiles::reset (void)
{
   for (unsigned int immobilesId = 0; immobilesId < NumImmobiles; ++immobilesId)
   {
      reset ((ImmobilesId)immobilesId);
   }
}

void Immobiles::set (const Immobiles & immobiles)
{
   for (unsigned int immobilesId = 0; immobilesId < NumImmobiles; ++immobilesId)
   {
      set ((ImmobilesId)immobilesId, immobiles.getWeight ((ImmobilesId)immobilesId));
   }
}

void Immobiles::add (const Immobiles & immobiles)
{
   for (unsigned int immobilesId = 0; immobilesId < NumImmobiles; ++immobilesId)
   {
      add ((ImmobilesId)immobilesId, immobiles.getWeight ((ImmobilesId)immobilesId));
   }
}

void Immobiles::subtract (const Immobiles & immobiles)
{
   for (unsigned int immobilesId = 0; immobilesId < NumImmobiles; ++immobilesId)
   {
      subtract ((ImmobilesId)immobilesId, immobiles.getWeight ((ImmobilesId)immobilesId));
   }
}

void Immobiles::addFraction (const Immobiles & immobiles, double fraction)
{
   for (unsigned int immobilesId = 0; immobilesId < NumImmobiles; ++immobilesId)
   {
      add ((ImmobilesId)immobilesId, fraction * immobiles.getWeight ((ImmobilesId)immobilesId));
   }
}

void Immobiles::subtractFraction (const Immobiles & immobiles, double fraction)
{
   for (unsigned int immobilesId = 0; immobilesId < NumImmobiles; ++immobilesId)
   {
      subtract ((ImmobilesId)immobilesId, fraction * immobiles.getWeight ((ImmobilesId)immobilesId));
   }
}

bool Immobiles::isEmpty (void) const
{
   for (unsigned int immobilesId = 0; immobilesId < NumImmobiles; ++immobilesId)
   {
      if (!isEmpty ((ImmobilesId)immobilesId)) return false;
   }
   return true;
}

double Immobiles::getWeight (void) const
{
   double total = 0;
   for (unsigned int immobilesId = 0; immobilesId < NumImmobiles; ++immobilesId)
   {
      total += getWeight ((ImmobilesId)immobilesId);
   }

   return total;
}

void Immobiles::setWeight (const double& weight)
{
   double correction = weight / getWeight ();
   for (int immobilesId = 0; immobilesId < NumImmobiles; ++immobilesId)
   {
      set ((ImmobilesId)immobilesId, getWeight ((ImmobilesId)immobilesId) * correction);
   }
}

Immobiles & Immobiles::operator= (const Immobiles & original)
{
   if (this != &original)
   {
      set (original);
   }

   return *this;
}

ostream & migration::operator<< (ostream & stream, Immobiles &immobiles)
{
   return stream << &immobiles;
}

ostream & migration::operator<< (ostream & stream, Immobiles * immobiles)
{
   if (!immobiles)
   {
      stream << "null";
   }
   else
   {
      for (int immobile = 0; immobile < NumImmobiles; ++immobile)
      {
         stream << immobiles->getWeight ((ImmobilesId)immobile) << ", ";
      }
   }
   return stream;
}

double Immobiles::getVolume (void)  const
{
   double total = 0.0;
   for (int immobilesId = 0; immobilesId < NumImmobiles; ++immobilesId)
   {
      total += getVolume ((ImmobilesId)immobilesId);
   }
   return total;
}

#endif
