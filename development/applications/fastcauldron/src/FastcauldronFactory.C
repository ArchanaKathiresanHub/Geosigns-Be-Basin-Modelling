#include "mpi.h"
#include "FastcauldronFactory.h"

#include "ProjectHandle.h"
#include "Formation.h"
#include "Surface.h"
#include "Snapshot.h"
#include "GridMap.h"
#include "Grid.h"
#include "AttributeValue.h"
#include "LithoType.h"
#include "Interface.h"
#include "Property.h"

using namespace DataAccess;
using Interface::GridMap;
using Interface::Grid;
using Interface::Snapshot;
using Interface::Formation;
using Interface::Surface;
using Interface::AttributeValue;
using Interface::LithoType;

#include "FastcauldronSimulator.h"
#include "Property.h"
#include "CauldronPropertyValue.h"
#include "CauldronRelatedProject.h"
#include "CauldronRunParameters.h"
#include "layer.h"
#include "CauldronCrustFormation.h"
#include "CauldronMantleFormation.h"
#include "Lithology.h"

DataAccess::Interface::ProjectHandle *
FastcauldronFactory::produceProjectHandle ( database::ProjectFileHandlerPtr pfh,
                                            const string & name) const
{
   return new FastcauldronSimulator ( pfh, name, this );
}


DataAccess::Interface::RelatedProject* FastcauldronFactory::produceRelatedProject ( Interface::ProjectHandle& projectHandle,
                                                                                    database::Record* record ) const
{
   return new CauldronRelatedProject ( projectHandle, record );
}

DataAccess::Interface::RunParameters * FastcauldronFactory::produceRunParameters ( Interface::ProjectHandle& projectHandle,
                                                                                   database::Record * record ) const
{
   return new CauldronRunParameters ( projectHandle, record );
}


DataAccess::Interface::Property * FastcauldronFactory::produceProperty ( Interface::ProjectHandle& projectHandle,
                                                                         database::Record *              record,
                                                                         const string &                  userName,
                                                                         const string &                  cauldronName,
                                                                         const string &                  unit,
                                                                         Interface::PropertyType         type,
                                                                         const DataModel::PropertyAttribute attr,
                                                                         const DataModel::PropertyOutputAttribute attrOut) const
{
   return new Property (projectHandle, record, userName, cauldronName, unit, type, attr, attrOut);
}

Interface::PropertyValue * FastcauldronFactory::producePropertyValue ( Interface::ProjectHandle&    projectHandle,
                                                                       database::Record *           record,
                                                                       const std::string &          name,
                                                                       const Interface::Property *  property,
                                                                       const Interface::Snapshot *  snapshot,
                                                                       const Interface::Reservoir * reservoir,
                                                                       const Interface::Formation * formation,
                                                                       const Interface::Surface *   surface,
                                                                       Interface::PropertyStorage   storage,
                                                                       const std::string & /*fileName*/ ) const
{
   return new CauldronPropertyValue ( projectHandle, record, name, property, snapshot, reservoir, formation, surface, storage );
}


Interface::Formation* FastcauldronFactory::produceFormation ( Interface::ProjectHandle& projectHandle,
                                                              database::Record * record ) const
{
   return new LayerProps ( projectHandle, record );
}

Interface::CrustFormation* FastcauldronFactory::produceCrustFormation ( Interface::ProjectHandle& projectHandle,
                                                                        database::Record * record ) const
{
   return new CauldronCrustFormation ( projectHandle, record );
}

Interface::MantleFormation* FastcauldronFactory::produceMantleFormation ( Interface::ProjectHandle& projectHandle,
                                                                          database::Record * record ) const
{
   return new CauldronMantleFormation ( projectHandle, record );
}

GeoPhysics::CompoundLithology* FastcauldronFactory::produceCompoundLithology ( GeoPhysics::ProjectHandle& projectHandle ) const
{
   return new Lithology ( projectHandle );
}
