#ifndef __PROPERTIESIOTBL_H__
#define __PROPERTIESIOTBL_H__

#include "propinterface.h"

namespace BasementIoTbl
{
   bool writeToContext (AppCtx* Basin_Model);
}

namespace RunOptionsIoTbl
{
   bool writeToContext (AppCtx* Basin_Model);
}

namespace StratIoTbl
{
   bool writeToContext (AppCtx* Basin_Model);
}

namespace FilterTimeIoTbl
{
   bool writeToContext (AppCtx* Basin_Model);
}

namespace SnapshotIoTbl
{
   bool writeToContext (AppCtx* Basin_Model);
}

namespace RelatedProjectsIoTbl
{
   bool writeToContext (AppCtx* Basin_Model);
}

#endif                          // __PROPERTIESIOTBL_H__
