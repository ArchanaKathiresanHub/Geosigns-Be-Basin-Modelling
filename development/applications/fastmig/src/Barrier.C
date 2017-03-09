//
// Copyright (C) 2016-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Barrier.h"
#include "Formation.h"
#include "migration.h"
#include "array.h"

#include "Interface/Snapshot.h"
#include "Interface/Grid.h"
#include "Interface/GridMap.h"

#include "FormationProperty.h"

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
   m_blockingPermeability = m_reservoir->getBlockingPerm ();
   m_blockingPorosity = m_reservoir->getBlockingPoro();
}

Barrier::~Barrier (void)
{
   Array<bool>::delete2d (m_values);
}


/// Use permeability properties of given formation to update the barrier's properties
void Barrier::updateBlocking (const migration::Formation * formation,
                              const DataAccess::Interface::Snapshot * snapshot)
{
   updateBlockingPermeability(formation, snapshot);
   updateBlockingPorosity(formation, snapshot);
}

void Barrier::updateBlockingPermeability (const migration::Formation * formation,
                                          const DataAccess::Interface::Snapshot * snapshot)
{
#if 0
   cerr << "Update blocking of reservoir " << m_reservoir->getName () << " with formation " << formation->getName () << endl;
#endif

   DerivedProperties::FormationPropertyPtr gridMap = m_reservoir->getVolumeProperty (formation, "Permeability", snapshot);

   if (!gridMap) return;
   gridMap->retrieveData ();
   unsigned int depth = gridMap->lengthK ();

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
            if ((value = gridMap->get (i, j, k)) != undefined && value < m_blockingPermeability)
            {
               m_values[i - m_firstI][j - m_firstJ] = true;
               break;
            }
         }
      }
   }
   gridMap->restoreData ();

}
   
void Barrier::updateBlockingPorosity (const migration::Formation * formation,
                                      const DataAccess::Interface::Snapshot * snapshot)
{
#if 0
   cerr << "Update blocking of reservoir " << m_reservoir->getName () << " with formation " << formation->getName () << endl;
#endif
   DerivedProperties::FormationPropertyPtr gridMap = m_reservoir->getVolumeProperty(formation, "Porosity", snapshot);
   if (!gridMap) return;
   gridMap->retrieveData ();
   unsigned int depth = gridMap->lengthK();

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
            if ((value = gridMap->get (i, j, k)) != undefined && value < m_blockingPorosity)
            {
               m_values[i - m_firstI][j - m_firstJ] = true;
               break;
            }
         }
      }
   }
   gridMap->restoreData ();
   
      }


/// return whether the barrier is blocking for (i, j)
bool Barrier::isBlocking (unsigned int i, unsigned int j)
{
   return m_values[i - m_firstI][j - m_firstJ];
}


