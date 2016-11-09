#include <cassert>
#include <unistd.h>

#include <string>
#include <string.h>
#include <iostream>
#include <vector>
#include <cstdlib>

#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"
#include "projectdependencies.h"


using namespace std;
using namespace database;

DataSchema * createReducedCauldronSchema (bool outputs);


int main (int argc, char ** argv)
{
   bool outputs = false;
   bool nosnapshots = false;
   bool verbose = false;
   char * projectFileName;
   char * projectPath;

   if (argc < 2)
   {
      cerr << "Usage: " << argv[0] << " [-nosnapshots] [-output] [-verbose] projectfile" << endl;
      return -1;
   }

   for (int i = 1; i < argc; i++)
   {
      if (strncmp (argv[i], "-nosnapshots", max<size_t>(2u, strlen (argv[i]))) == 0)
      {
	 nosnapshots = true;
      }
      else if (strncmp (argv[i], "-output", max<size_t>(2u, strlen (argv[i]))) == 0)
      {
	 outputs = true;
      }
      else if (strncmp (argv[i], "-verbose", max<size_t>(2u, strlen (argv[i]))) == 0)
      {
	 verbose = true;
      }
      else if (strncmp (argv[i], "-", 1) != 0)
      {
	 projectPath = argv[i];
      }
      else
      {
	 cerr << "Usage: " << argv[0] << " [-nosnapshots] [-output] [-verbose] projectfile" << endl;
	 return -1;
      }
   }

   char * endOfDir = strrchr (projectPath, '/');

   if (endOfDir)
   {
      * endOfDir = '\0';

      if (chdir (projectPath) != 0)
      {
	 perror (projectPath);
	 exit (-1);
      }

      projectFileName = endOfDir + 1;
   }
   else
      projectFileName = projectPath;

   DataSchema * projSchema = createReducedCauldronSchema (outputs);
   Database * projBase = Database::CreateFromFile (projectFileName, * projSchema);
   delete projSchema;

   if (!projBase)
   {
      cerr << "Failed to read project file " << projectFileName << endl;
      return -1;
   }

   ProjectDependencies dependencies = getProjectDependencies( projBase);
   delete projBase;

   if (verbose) cout << "--- project file ---" << endl;
   cout << projectFileName << endl;

   
   if (verbose) cout << "--- input maps ---" << endl;
   for (unsigned i = 0 ; i < dependencies.inputMaps.size(); ++i)
   {
      cout << dependencies.inputMaps[i] << endl;
   }

   if (verbose) cout << "--- related projects ---" << endl;
   for (unsigned i = 0; i < dependencies.related.size (); i++)
   {
      cout << dependencies.related[i] << endl;
   }

   if (!outputs)
   {
      return 0;
   }

   if (verbose) cout << "--- output maps ---" << endl;
   for (unsigned i = 0; i < dependencies.outputMaps.size(); ++i)
   {
      cout << dependencies.outputMaps[i] << endl;
   }

   if (!nosnapshots)
   {
      if (verbose) cout << "--- snapshots ---" << endl;
      for (unsigned i = 0; i < dependencies.snapshots.size(); ++i)
         cout << dependencies.snapshots[i] << endl;
   }

   return 0;
}

DataSchema * createReducedCauldronSchema (bool outputs)
{
   DataSchema * dataSchema = new DataSchema;

   createGridMapIoTblDefinition (dataSchema);
   createRelatedProjectsIoTblDefinition (dataSchema);
   createIoOptionsIoTblDefinition (dataSchema);
   if (outputs)
   {
      createTimeIoTblDefinition (dataSchema);
      createSnapshotIoTblDefinition (dataSchema);
   }

   return dataSchema;
}

