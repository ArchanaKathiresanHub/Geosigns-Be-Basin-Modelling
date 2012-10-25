#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>

#ifdef sgi
#include <string.h>
#include <iostream.h>
#else
#include <string>
#include <iostream>
#endif

#define Min(a,b)        (a < b ? a : b)
#define Max(a,b)        (a > b ? a : b)

#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"
#include <vector>
#include <map>

using namespace std;
using namespace database;

DataSchema * createReducedCauldronSchema (bool outputs);

bool stringSmaller (const string & lhs, const string & rhs)
{
   // cerr << lhs << ((lhs < rhs) ? " < " : " >= ") << rhs << endl;
   return lhs < rhs;
}


typedef vector < string > StringList;
typedef map < string, string, less < string > >StringToStringMap;

int main (int argc, char ** argv)
{
   bool outputs = false;
   bool nosnapshots = false;
   bool verbose = false;
   char * projectFileName;
   char * projectPath;

   string outputDir;

   StringList gridMaps;

   if (argc < 2)
   {
      cerr << "Usage: " << argv[0] << " [-nosnapshots] [-output] [-verbose] projectfile" << endl;
      return -1;
   }

   int i;
   for (i = 1; i < argc; i++)
   {
      if (strncmp (argv[i], "-nosnapshots", Max (2, strlen (argv[i]))) == 0)
      {
	 nosnapshots = true;
      }
      else if (strncmp (argv[i], "-output", Max (2, strlen (argv[i]))) == 0)
      {
	 outputs = true;
      }
      else if (strncmp (argv[i], "-verbose", Max (2, strlen (argv[i]))) == 0)
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

   if (verbose) cout << "--- project file ---" << endl;
   cout << projectFileName << endl;

   Table * gridMapTbl = projBase->getTable ("GridMapIoTbl");

   if (gridMapTbl)
   {
      for (i = 0; i < gridMapTbl->size (); i++)
      {
	 Record * gridMapRcrd = gridMapTbl->getRecord (i);
	 assert (gridMapRcrd);
	 string mapFileName = getMapFileName (gridMapRcrd);
	 gridMaps.push_back (mapFileName);
      }

      sort (gridMaps.begin (), gridMaps.end (), stringSmaller);

      StringList::iterator new_end = unique (gridMaps.begin (), gridMaps.end ());

      if (verbose) cout << "--- input maps ---" << endl;
      StringList::iterator iter;
      for (iter = gridMaps.begin (); iter != new_end; ++iter)
      {
	 cout << * iter << endl;
      }
   }

   Table * relatedProjectsTbl = projBase->getTable ("RelatedProjectsIoTbl");
   if (relatedProjectsTbl)
   {
      if (verbose) cout << "--- related projects ---" << endl;
      for (i = 0; i < relatedProjectsTbl->size (); i++)
      {
	 Record * relatedProjectsRcrd = relatedProjectsTbl->getRecord (i);
	 const string relatedProject = getFilename (relatedProjectsRcrd);
	 if (relatedProject[0] != '*')
	 {
	    cout << relatedProject << endl;
	 }
      }
   }

   if (!outputs)
   {
      delete projBase;
      return 0;
   }

   Table * runStatusTbl = projBase->getTable ("RunStatusIoTbl");
   if (runStatusTbl == 0 || runStatusTbl->size() == 0)
   {
      delete projBase;
      return 0;
   }

   Record * runStatusRcrd = runStatusTbl->getRecord (0);
   assert (runStatusRcrd);

   Table * ioOptionsTbl = projBase->getTable ("IoOptionsIoTbl");
   if (ioOptionsTbl == 0 || ioOptionsTbl->size() == 0)
   {
      delete projBase;
      return 0;
   }

   Record * ioOptionsRcrd = ioOptionsTbl->getRecord (0);
   assert (ioOptionsRcrd);

   const string & mapType = getMapType (ioOptionsRcrd);

   outputDir = getOutputDirOfLastRun (runStatusRcrd);
   int dirLength = outputDir.length ();
   if (dirLength > 0 && outputDir[dirLength - 1] == '/')
      outputDir.erase (dirLength - 1, string::npos);

   Table * timeTbl = projBase->getTable ("TimeIoTbl");
   if (timeTbl)
   {
      StringList gridMaps;

      for (i = 0; i < timeTbl->size (); i++)
      {
	 Record * timeRcrd = timeTbl->getRecord (i);
	 assert (timeRcrd);
	 gridMaps.push_back (getMapFileName (timeRcrd));
      }

      sort (gridMaps.begin (), gridMaps.end (), stringSmaller);

      StringList::iterator new_end = unique (gridMaps.begin (), gridMaps.end ());

      if (verbose) cout << "--- output maps ---" << endl;
      StringList::iterator iter;
      for (iter = gridMaps.begin (); iter != new_end; ++iter)
      {
	 cout << outputDir << "/" << * iter << endl;
      }
   }

   if (!nosnapshots)
   {
      Table * snapshotTbl = projBase->getTable ("SnapshotIoTbl");
      if (snapshotTbl)
      {
	 if (verbose) cout << "--- snapshots ---" << endl;

	 struct stat statbuf;
	 for (i = 0; i < snapshotTbl->size (); i++)
	 {
	    Record * snapshotRcrd = snapshotTbl->getRecord (i);
	    assert (snapshotRcrd);
	    string filePath = getSnapshotFileName (snapshotRcrd);
	    if (filePath.length () == 0) continue;
	    filePath.insert (0, "/");
	    filePath.insert (0, outputDir);
	    if (stat (filePath.c_str (), & statbuf) == -1)
	    {
	       filePath += ".Z";
	    }
	    if (stat (filePath.c_str (), & statbuf) != -1)
	       cout << filePath << endl;
	 }
      }
   }

   delete projBase;
   return 0;
}


DataSchema * createReducedCauldronSchema (bool outputs)
{
   DataSchema * dataSchema = new DataSchema;

   createGridMapIoTblDefinition (dataSchema);
   createRelatedProjectsIoTblDefinition (dataSchema);
   createRunStatusIoTblDefinition (dataSchema);
   createIoOptionsIoTblDefinition (dataSchema);
   if (outputs)
   {
      createTimeIoTblDefinition (dataSchema);
      createSnapshotIoTblDefinition (dataSchema);
   }

   return dataSchema;
}
