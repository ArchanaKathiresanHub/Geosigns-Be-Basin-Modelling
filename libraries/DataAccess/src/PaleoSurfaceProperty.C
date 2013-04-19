#include <assert.h>

#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
      #include <sstream>
      using namespace std;
      #define USESTANDARD
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
      #include<strstream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   #include <sstream>
   using namespace std;
   #define USESTANDARD
#endif // sgi

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Interface/ProjectHandle.h"
#include "Interface/ObjectFactory.h"
#include "Interface/GridMap.h"
#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/PaleoSurfaceProperty.h"

using namespace DataAccess;
using namespace Interface;



#if 0
PaleoSurfaceProperty::PaleoSurfaceProperty (ProjectHandle * projectHandle, Record * record, const string& initialName ) : 
   PaleoProperty (projectHandle, record), 
   surfaceName ( initialName )
{
   m_surface = (const Surface *) m_projectHandle->findSurface ( getSurfaceName ());
}
#endif

PaleoSurfaceProperty::PaleoSurfaceProperty (ProjectHandle * projectHandle, Record * record, const Surface* surface ) : 
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
#ifdef USESTANDARD
   ostringstream buf;
#else
   strstream buf;
#endif

   buf << "PaleoSurfaceProperty:";
   buf << " age = " << getSnapshot ()->getTime ();
   buf << ", surface name = " << getSurface ()->getName ();
   buf << endl;

   str = buf.str ();
#ifndef USESTANDARD
   buf.rdbuf ()->freeze (0);
#endif
}
