#include "Barrier.h"
#include "Formation.h"
#include "migration.h"
#include "array.h"

#include "Interface/Snapshot.h"
#include "Interface/Grid.h"
#include "Interface/GridMap.h"

#include <assert.h>

#include <iostream>
#include <vector>
using namespace std;

using DataAccess::Interface::GridMap;

using namespace migration;

Barrier::Barrier (Reservoir * reservoir) : m_reservoir (reservoir)
{
   m_firstI = m_reservoir->getGrid ()->firstI ();
   m_firstJ = m_reservoir->getGrid ()->firstJ ();

   m_values = Array<bool>::create2d (m_reservoir->getGrid ()->numI (),  m_reservoir->getGrid ()->numJ (), false);
   m_blockingPermeability = m_reservoir->getBlockingPermeability ();
}

Barrier::~Barrier (void)
{
   Array<bool>::delete2d (m_values);
}


/// Use permeability properties of given formation to update the barrier's properties
void Barrier::updateBlocking (const Formation * formation,
      const DataAccess::Interface::Snapshot * snapshot)
{
#if 0
   cerr << "Update blocking of reservoir " << m_reservoir->getName () << " with formation " << formation->getName () << endl;
#endif
   const GridMap * gridMap = m_reservoir->getVolumePropertyGridMap (formation, "Permeability", snapshot);
   if (!gridMap) return;
   gridMap->retrieveData ();
   unsigned int depth = gridMap->getDepth ();

   unsigned int lastI = m_reservoir->getGrid ()->lastI ();
   unsigned int lastJ = m_reservoir->getGrid ()->lastJ ();
   double undefined = gridMap->getUndefinedValue ();

   double value;
   for (unsigned int i = m_firstI; i <= lastI; ++i)
   {
      for (unsigned int j = m_firstJ; j <= lastJ; ++j)
      {
         if (m_values[i - m_firstI][j - m_firstJ]) continue; // already blocking
         for (unsigned int k = 0; k < depth; ++k)
         {
            if ((value = gridMap->getValue (i, j, k)) != undefined && value < m_blockingPermeability)
            {
               m_values[i - m_firstI][j - m_firstJ] = true;
               break;
            }
         }
      }
   }
   gridMap->restoreData ();
   // delete gridMap;
}


/// return whether the barrier is blocking for (i, j)
bool Barrier::isBlocking (unsigned int i, unsigned int j)
{
   return m_values[i - m_firstI][j - m_firstJ];
}


