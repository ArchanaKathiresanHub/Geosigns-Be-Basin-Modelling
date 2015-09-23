#include "RequestHandling.h"
#include "Formation.h"
#include "Surface.h"
#include "rankings.h"
#include "migration.h"

#include "Interface/ProjectHandle.h"


#include <assert.h>
#include <math.h>

#include <iostream>
#include <vector>
using namespace std;

using namespace CBMGenerics;

using namespace DataAccess;

namespace migration
{

   Surface::Surface (Interface::ProjectHandle * projectHandle, Migrator * const migrator, database::Record * record)
      : Interface::Surface (projectHandle, record),
        m_migrator (migrator)
   {
   }

   Surface::~Surface (void)
   {
   }

   const Surface * Surface::getTopSurface () const
   {
      const Interface::Formation * formation = getTopFormation ();
      
      return (const Surface *) (formation ? formation->getTopSurface () : 0);
   }

   const Surface * Surface::getBottomSurface () const
   {
      const Interface::Formation * formation = getBottomFormation ();
      
      return (const Surface *) (formation ? formation->getBottomSurface () : 0);
   }

}
