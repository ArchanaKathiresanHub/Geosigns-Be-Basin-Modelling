#include "utils.h"

#include "Interface/ProjectHandle.h"

using namespace DataAccess;
using namespace Interface;

#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"
using namespace database;

/// Check whether migration was successfully performed
bool migrationCompleted (ProjectHandle * projectHandle)
{
   Table *runStatusIoTbl = projectHandle->getTable ("RunStatusIoTbl");

   assert (runStatusIoTbl);

   if (runStatusIoTbl->size () == 0)
   {
      return false;
   }

   Record *runStatusIoRecord = runStatusIoTbl->getRecord (0);

   assert (runStatusIoRecord);
   string runStatus = getMCStatusOfLastRun (runStatusIoRecord);

   if (runStatus != "XmigCalculated" && runStatus != "FastXmigCalculated")
   {
      return false;
   }

   return true;
}
