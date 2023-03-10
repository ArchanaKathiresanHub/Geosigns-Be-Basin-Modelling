#include <assert.h>

#include <iostream>
#include <sstream>
using namespace std;

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "ProjectHandle.h"
#include "ObjectFactory.h"
#include "GridMap.h"
#include "Surface.h"
#include "Snapshot.h"
#include "PaleoSurfaceProperty.h"

using namespace DataAccess;
using namespace Interface;



#if 0
PaleoSurfaceProperty::PaleoSurfaceProperty (ProjectHandle * projectHandle, Record * record, const string& initialName ) :
   PaleoProperty (projectHandle, record),
   surfaceName ( initialName )
{
   m_surface = (const Surface *) m_projectHandle.findSurface ( getSurfaceName ());
}
#endif

PaleoSurfaceProperty::PaleoSurfaceProperty (ProjectHandle& projectHandle, Record * record, const Surface* surface ) :
   PaleoProperty (projectHandle, record),
   surfaceName ( surface->getName ())
{
   m_surface = surface;
}

PaleoSurfaceProperty::~PaleoSurfaceProperty (void)
{
}

const std::string& PaleoSurfaceProperty::getSurfaceName (void) const
{
   return surfaceName;
}

const Surface * PaleoSurfaceProperty::getSurface (void) const
{
   return m_surface;
}

void PaleoSurfaceProperty::asString (string & str) const
{
   std::ostringstream buf;

   buf << "PaleoSurfaceProperty:";
   buf << " age = " << getSnapshot ()->getTime ();
   buf << ", surface name = " << getSurface ()->getName ();
   buf << endl;

   str = buf.str ();
}
