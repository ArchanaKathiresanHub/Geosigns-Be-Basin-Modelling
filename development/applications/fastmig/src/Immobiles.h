#ifndef _MIGRATION_IMMOBILES_H_
#define _MIGRATION_IMMOBILES_H_
#ifdef USEOTGC

#ifdef sgi
#ifdef _STANDARD_C_PLUS_PLUS
#include <iostream>
#else // !_STANDARD_C_PLUS_PLUS
#include<iostream.h>
#endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
#include <iostream>
#endif // sgi

#include <math.h>
#include <stdlib.h>
#include <assert.h>


#include <string>
#include <map>
#include <vector>

#include "migration.h"

using namespace std;

/// Immobiles Class
namespace migration
{
   class Immobiles
   {
   public:
      /// Constructor
      Immobiles (void);

      /// Destructor
      virtual ~Immobiles (void);

      inline void reset (ImmobilesId immobilesId);
      inline void set (ImmobilesId immobilesId, double weight);
      inline void add (ImmobilesId immobilesId, double weight);
      inline void subtract (ImmobilesId immobilesId, double weight);
      inline void checkWeight (ImmobilesId immobilesId);
      inline bool isEmpty (ImmobilesId immobilesId) const;
	 
      inline double getWeight (ImmobilesId immobilesId) const;

      double getVolume (void) const;
      inline double getVolume (ImmobilesId immobilesId) const;

      inline vector<double> getWeights() const;

      void reset (void);
      void set (const Immobiles & immobiles);
      void add (const Immobiles & immobiles);
      void subtract (const Immobiles & immobiles);
      bool isEmpty (void) const;
      double getWeight (void) const;
      void setWeight(const double& weight);

      void addFraction (const Immobiles & immobiles, double fraction);
      void subtractFraction (const Immobiles & immobiles, double fraction);

      Immobiles & operator= (const Immobiles & original);

   private:

      double m_weights[NumImmobiles];
   };

   ostream & operator<< (ostream & stream, Immobiles & immobiles);
   ostream & operator<< (ostream & stream, Immobiles * immobiles);
}

void migration::Immobiles::reset (ImmobilesId immobilesId)
{
   assert ((unsigned int) immobilesId < NumImmobiles);
   m_weights[(int) immobilesId] = 0;
}

void migration::Immobiles::checkWeight (ImmobilesId immobilesId)
{
   if (m_weights[immobilesId] < 0)
   {
      cerr << "weight of " << ImmobileNames[immobilesId] << " (" << m_weights[immobilesId] << ") < 0, resetting to 0" << endl;
      m_weights[immobilesId] = 0;
   }
}

void migration::Immobiles::set (ImmobilesId immobilesId, double weight)
{
   assert ((unsigned int) immobilesId < NumImmobiles);
   m_weights[(int) immobilesId] = weight;
   checkWeight (immobilesId);
}

void migration::Immobiles::add (ImmobilesId immobilesId, double weight)
{
   assert ((unsigned int) immobilesId < NumImmobiles);
   m_weights[(int) immobilesId] += weight;

   checkWeight (immobilesId);
}

void migration::Immobiles::subtract (ImmobilesId immobilesId, double weight)
{
   assert ((unsigned int) immobilesId < NumImmobiles);
   m_weights[(int) immobilesId] -= weight;
   checkWeight (immobilesId);
}

bool migration::Immobiles::isEmpty (ImmobilesId immobilesId) const
{
   assert ((unsigned int) immobilesId < NumImmobiles);
   return (m_weights[(int) immobilesId] <= 0);
}

double migration::Immobiles::getWeight (ImmobilesId immobilesId) const
{
   assert ((unsigned int) immobilesId < NumImmobiles);
   return m_weights[(int) immobilesId];
}

double migration::Immobiles::getVolume (ImmobilesId immobilesId) const
{
   assert ((unsigned int) immobilesId < NumImmobiles);
   return getWeight (immobilesId) / ImmobileDensities[(int) immobilesId ];
}

vector<double> migration::Immobiles::getWeights () const
{
   vector<double> weights(NumImmobiles);
   for (unsigned int immobilesId = 0; immobilesId < NumImmobiles; ++immobilesId)
      weights[immobilesId] = m_weights[immobilesId];
   return weights;
}


#endif
#endif // _MIGRATION_IMMOBILES_H_

