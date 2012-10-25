#include <stdlib.h>
#include <unistd.h>

#include <dlfcn.h>

#include <assert.h>

#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include <iostream>
      using namespace std;
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   using namespace std;
#endif // sgi

#include "Interface/ProjectHandle.h"
#include "Interface/Property.h"

#include <string>
using namespace std;

using namespace DataAccess;
using namespace Interface;

typedef ProjectHandle * (* OpenCauldronProjectFunc) (const string &, const string &);
typedef void (* CloseCauldronProjectFunc) (ProjectHandle *);

using namespace std;

static char * argv0 = 0;
static void showUsage (const char * message = 0);

#define Max(a,b)        (a > b ? a : b)

enum ListMode { NOLISTMODE = 0, FORMATIONS, SURFACES, FORMATIONSURFACES, RESERVOIRS, SOURCEROCKS };

const string ListModeNames[] =
{
   "NoMode",
   "Formation",
   "Surface",
   "FormationSurface",
   "Reservoir",
   "SourceRock"
};

int main (int argc, char ** argv)
{
   string projectFileName;
   string libraryName;

   ListMode listMode = NOLISTMODE;

   if ((argv0 = strrchr (argv[0], '/')) != 0)
   {
      ++argv0;
   }
   else
   {
      argv0 = argv[0];
   }

   for (int arg = 1; arg < argc; ++arg)
   {
      if (strncmp (argv[arg], "-formations", Max (2, strlen (argv[arg]))) == 0)
      {
	 if (listMode != SURFACES)
	    listMode = FORMATIONS;
	 else
	    listMode = FORMATIONSURFACES;
      }
      else if (strncmp (argv[arg], "-surfaces", Max (3, strlen (argv[arg]))) == 0)
      {
	 if (listMode != FORMATIONS)
	    listMode = SURFACES;
	 else
	    listMode = FORMATIONSURFACES;
      }
      else if (strncmp (argv[arg], "-reservoirs", Max (2, strlen (argv[arg]))) == 0)
      {
         listMode = RESERVOIRS;
      }
      else if (strncmp (argv[arg], "-sourcerocks", Max (3, strlen (argv[arg]))) == 0)
      {
         listMode = SOURCEROCKS;
      }
      else if (strncmp (argv[arg], "-library", Max (2, strlen (argv[arg]))) == 0)
      {
         libraryName = argv[++arg];
      }
      else if (strncmp (argv[arg], "-", 1) != 0)
      {
         projectFileName = argv[arg];
      }
   }

   if (projectFileName == "")
   {
      showUsage ("No project file specified");
      return -1;
   }

   if (libraryName == "")
   {
      showUsage ("No library file specified");
      return -1;
   }

   if (listMode == NOLISTMODE)
   {
      showUsage ("No list mode specified");
      return -1;
   }

   void *dlhandle;

   dlhandle = dlopen (libraryName.c_str (), RTLD_LAZY | RTLD_LOCAL);
   if (dlhandle == 0)
   {
      /* couldn't open DSO */
      fprintf (stderr, "Error in dlopen: %s\n", dlerror ());
      return -1;
   }
   OpenCauldronProjectFunc openProjectPtr;
   CloseCauldronProjectFunc closeProjectPtr;

   openProjectPtr = (OpenCauldronProjectFunc) dlsym (dlhandle, "OpenCauldronProject");
   if (openProjectPtr == 0)
   {
      // Could not find the function
      fprintf (stderr, "Error in dlsym: %s\n", dlerror ());
      return -1;
   }

   closeProjectPtr = (CloseCauldronProjectFunc) dlsym (dlhandle, "CloseCauldronProject");
   if (closeProjectPtr == 0)
   {
      // Could not find the function
      fprintf (stderr, "Error in dlsym: %s\n", dlerror ());
      return -1;
   }

   ProjectHandle *projectHandle = (*openProjectPtr) (projectFileName.c_str (), "r");

   if (!projectHandle)
   {
      cerr << "Could not open project file '" << projectFileName << "'" << endl;
      return -1;
   }

   PropertyList *propertyList = 0;

   if (listMode == FORMATIONS)
   {
      propertyList = projectHandle->getProperties (false, FORMATION, 0, 0, 0, 0, VOLUME);
   }
   else if (listMode == SURFACES)
   {
      propertyList = projectHandle->getProperties (false, SURFACE, 0, 0, 0, 0, SURFACE);
   }
   else if (listMode == FORMATIONSURFACES)
   {
      propertyList = projectHandle->getProperties (false, FORMATIONSURFACE, 0, 0, 0, 0, SURFACE);
   }
   else if (listMode == RESERVOIRS)
   {
      propertyList = projectHandle->getProperties (false, RESERVOIR, 0, 0, 0, 0, SURFACE);
   }
   else if (listMode == SOURCEROCKS)
   {
      propertyList = projectHandle->getProperties (false, FORMATION, 0, 0, 0, 0, SURFACE);
   }

   PropertyList::iterator propertyIter;

   if (!propertyList)
   {
      cerr << "No properties present: " << endl;
   }
   else
   {
      cerr << propertyList->size () << " " << ListModeNames[listMode] << " Properties present: " << endl;

      for (propertyIter = propertyList->begin (); propertyIter != propertyList->end (); ++propertyIter)
      {
         const Property *property = *propertyIter;

         property->printOn (cerr);
      }
   }

   // delete projectHandle;
   (*closeProjectPtr) (projectHandle);
   cerr << "Project closed" << endl;
#if sun
   // Crashes on exit on IRIX
   dlclose (dlhandle);
#endif

   return 0;
}

void showUsage (const char * message)
{
   cerr << endl;
   if (message)
   {
      cerr << argv0 << ": " << message << endl;
   }

   cerr << "Usage (options may be abbreviated): " << argv0
      << " <-formations | -surfaces | -reservoirs | -sourcerocks | -formations - surfaces> <-library accessLibrary>  <projectfile>"
      << endl;
   exit (-1);
}

