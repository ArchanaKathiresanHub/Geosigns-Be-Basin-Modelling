#include "AllochthonousModellingFactory.h"
#include "GeoMorphAllochthonousLithology.h"
#include "GeoMorphAllochthonousLithologyDistribution.h"
#include "AllochthonousLithologySimulator.h"

#include "ProjectHandle.h"
#include "Formation.h"
#include "Surface.h"
#include "Snapshot.h"
#include "GridMap.h"
#include "Grid.h"
#include "AttributeValue.h"
#include "LithoType.h"
#include "Interface.h"

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
AllochMod::AllochthonousModellingFactory::produceProjectHandle ( database::ProjectFileHandlerPtr database,
                                                                 const std::string & name) const
{
  return new AllochthonousLithologySimulator ( database, name, this );
}

DataAccess::Interface::AllochthonousLithology *
AllochMod::AllochthonousModellingFactory::produceAllochthonousLithology (DataAccess::Interface::ProjectHandle& projectHandle, database::Record * record) const
{
   return new GeoMorphAllochthonousLithology (projectHandle, record);
}

DataAccess::Interface::AllochthonousLithologyDistribution *
AllochMod::AllochthonousModellingFactory::produceAllochthonousLithologyDistribution (DataAccess::Interface::ProjectHandle& projectHandle, database::Record * record) const
{
   return new GeoMorphAllochthonousLithologyDistribution (projectHandle, record);
}
