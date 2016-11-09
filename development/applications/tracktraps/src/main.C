#include <stdlib.h>

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif


#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>

#include <string.h>
#include <string>
#include <iostream>

using namespace std;

#define Min(a,b)        (a < b ? a : b)
#define Max(a,b)        (a > b ? a : b)

#include "PersistentTrapFactory.h"

#include "ProjectHandle.h"

using namespace PersistentTraps;

static char * argv0 = 0;
static void showUsage (char * message = 0);

bool verbose = false;

int main (int argc, char ** argv)
{
   string projectFileName;
   string outputFileName;
   string directoryExtension;

   // the root object
   if ((argv0 = strrchr (argv[0], '/')) != 0)
   {
      ++argv0;
   }
   else
   {
      argv0 = argv[0];
   }

   int i;

   for (i = 1; i < argc; i++)
   {
      if (strncmp (argv[i], "-project", Max (2, strlen (argv[i]))) == 0 && projectFileName.size () == 0)
      {
         if (i + 1 >= argc)
         {
            showUsage ("Argument for '-project' is missing");
            return -1;
         }
         projectFileName = argv[++i];
      }
      else if (strncmp (argv[i], "-output", Max (2, strlen (argv[i]))) == 0)
      {
         if (i + 1 >= argc)
         {
            showUsage ("Argument for '-output' is missing");
            return -1;
         }
         outputFileName = argv[++i];
      }
      else if (strncmp (argv[i], "-verbose", Max (2, strlen (argv[i]))) == 0)
      {
         verbose = true;
      }
      else if (strncmp (argv[i], "-", Max (1, strlen (argv[i]))) != 0 && projectFileName.size () == 0)
      {
         projectFileName = argv[i];
      }
      else
      {
         showUsage ();
         return -1;
      }
   }

   if (projectFileName.size () == 0)
   {
      showUsage ("No project file specified");
      return -1;
   }

   if (outputFileName.size () == 0)
   {
      outputFileName = projectFileName;
   }

   PersistentTrapFactory* factory = new PersistentTrapFactory;
   ProjectHandle * projectHandle = (ProjectHandle *) Interface::OpenCauldronProject (projectFileName, "rw", factory);

   if (!projectHandle)
   {
      cerr << "Failed to read project file " << projectFileName << endl;
      return -1;
   }

   if (verbose)
   {
      cerr << "Finished reading project " << projectFileName << endl;
   }

   if (projectHandle->createPersistentTraps ())
   {
      projectHandle->saveProject (outputFileName);
   }

   delete projectHandle;
   delete factory;

   return 0;
}

void showUsage (char * message)
{
   cerr << endl;
   if (message)
   {
      cerr << argv0 << ": " << message << endl;
   }

   cerr << "Usage: " << argv0
      << " [-project] projectfile <-output filename> [-verbose]"
      << endl;
   exit (-1);
}
