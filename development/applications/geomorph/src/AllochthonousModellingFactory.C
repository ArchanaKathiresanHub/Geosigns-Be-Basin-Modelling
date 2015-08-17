#include "AllochthonousModellingFactory.h"
#include "AllochthonousLithology.h"
#include "AllochthonousLithologyDistribution.h"
#include "AllochthonousLithologySimulator.h"

#include "Interface/ProjectHandle.h"
#include "Interface/Formation.h"
#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"
#include "Interface/Grid.h"
#include "Interface/AttributeValue.h"
#include "Interface/LithoType.h"
#include "Interface/Interface.h"
#include "Interface/AllochthonousLithology.h"
//#include "Interface/AllochthonousLithologyDistribution.h"


using namespace Numerics;
using namespace DataAccess;
using Interface::GridMap;
using Interface::Grid;
using Interface::Snapshot;
using Interface::Formation;
using Interface::Surface;
using Interface::AttributeValue;
using Interface::LithoType;



DataAccess::Interface::ProjectHandle *
AllochMod::AllochthonousModellingFactory::produceProjectHandle ( database::Database * database,
                                                                 const string & name,
                                                                 const string & accessMode) {
//   std::cout << "AllochthonousModellingFactory::produceProjectHandle" << std::endl;
  return new AllochthonousLithologySimulator ( database, name, accessMode, this );
}

DataAccess::Interface::AllochthonousLithology *
AllochMod::AllochthonousModellingFactory::produceAllochthonousLithology (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record)
{
   return new AllochthonousLithology (projectHandle, record);
}

DataAccess::Interface::AllochthonousLithologyDistribution *
AllochMod::AllochthonousModellingFactory::produceAllochthonousLithologyDistribution (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record)
{
   return new AllochthonousLithologyDistribution (projectHandle, record);
}

