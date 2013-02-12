#include "projectdependencies.h"
#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"

#include <cassert>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;
using namespace database;

ProjectDependencies getProjectDependencies( database::Database * projBase )
{
   typedef vector < string > StringList;

   ProjectDependencies dependencies;

   // save input maps
   Table * gridMapTbl = projBase->getTable ("GridMapIoTbl");

   if (gridMapTbl)
   {
      StringList gridMaps;
      for (unsigned i = 0; i < gridMapTbl->size (); i++)
      {
         Record * gridMapRcrd = gridMapTbl->getRecord (i);
         assert (gridMapRcrd);
         string mapFileName = getMapFileName (gridMapRcrd);
         gridMaps.push_back (mapFileName);
      }

      sort (gridMaps.begin (), gridMaps.end ());

      StringList::iterator new_end = unique (gridMaps.begin (), gridMaps.end ());
      dependencies.inputMaps.insert( dependencies.inputMaps.end(), gridMaps.begin(), new_end);
   }

   Table * relatedProjectsTbl = projBase->getTable ("RelatedProjectsIoTbl");
   if (relatedProjectsTbl)
   {
      for (unsigned i = 0; i < relatedProjectsTbl->size (); i++)
      {
	 Record * relatedProjectsRcrd = relatedProjectsTbl->getRecord (i);
	 const string relatedProject = getFilename (relatedProjectsRcrd);
	 if (relatedProject[0] != '*')
	 {
	    dependencies.related.push_back(relatedProject);;
	 }
      }
   }

   Table * runStatusTbl = projBase->getTable ("RunStatusIoTbl");
   if (runStatusTbl == 0 || runStatusTbl->size() == 0)
   {
      return dependencies;
   }

   Record * runStatusRcrd = runStatusTbl->getRecord (0);
   assert (runStatusRcrd);

   Table * ioOptionsTbl = projBase->getTable ("IoOptionsIoTbl");
   if (ioOptionsTbl == 0 || ioOptionsTbl->size() == 0)
   {
      return dependencies;
   }

   Record * ioOptionsRcrd = ioOptionsTbl->getRecord (0);
   assert (ioOptionsRcrd);

   const string & mapType = getMapType (ioOptionsRcrd);

   std::string outputDir = getOutputDirOfLastRun (runStatusRcrd);
   int dirLength = outputDir.length ();
   if (dirLength > 0 && outputDir[dirLength - 1] == '/')
      outputDir.erase (dirLength - 1, string::npos);

   Table * timeTbl = projBase->getTable ("TimeIoTbl");
   if (timeTbl)
   {
      StringList gridMaps;

      for (unsigned i = 0; i < timeTbl->size (); i++)
      {
	 Record * timeRcrd = timeTbl->getRecord (i);
	 assert (timeRcrd);
	 gridMaps.push_back (getMapFileName (timeRcrd));
      }

      sort (gridMaps.begin (), gridMaps.end ());

      StringList::iterator new_end = unique (gridMaps.begin (), gridMaps.end ());

      StringList::iterator iter;
      for (iter = gridMaps.begin (); iter != new_end; ++iter)
      {
         dependencies.outputMaps.push_back( outputDir + "/" + *iter ) ;
      }
   }

   Table * snapshotTbl = projBase->getTable ("SnapshotIoTbl");
   if (snapshotTbl)
   {
      struct stat statbuf;
      for (unsigned i = 0; i < snapshotTbl->size (); i++)
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
            dependencies.snapshots.push_back( filePath );
      }
   }

   return dependencies;
}



