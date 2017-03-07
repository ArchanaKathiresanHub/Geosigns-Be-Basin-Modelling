#include "mpi.h"
#include "FastcauldronFactory.h"

#include "Interface/ProjectHandle.h"
#include "Interface/Formation.h"
#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"
#include "Interface/Grid.h"
#include "Interface/AttributeValue.h"
#include "Interface/LithoType.h"
#include "Interface/Interface.h"
#include "Interface/Property.h"

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
#include "PropertyValue.h"
#include "RelatedProject.h"
#include "RunParameters.h"
#include "layer.h"
#include "CrustFormation.h"
#include "MantleFormation.h"
#include "Lithology.h"

DataAccess::Interface::ProjectHandle *
FastcauldronFactory::produceProjectHandle ( database::ProjectFileHandlerPtr pfh,
                                            const string & name,
                                            const string & accessMode) {
   return new FastcauldronSimulator ( pfh, name, accessMode, this );
}


DataAccess::Interface::RelatedProject* FastcauldronFactory::produceRelatedProject ( Interface::ProjectHandle* projectHandle,
                                                                                    database::Record*              record ) {
   return new RelatedProject ( projectHandle, record );
}

DataAccess::Interface::RunParameters * FastcauldronFactory::produceRunParameters ( Interface::ProjectHandle * projectHandle,
                                                                                   database::Record *              record ) {
   return new RunParameters ( projectHandle, record );
}


DataAccess::Interface::Property * FastcauldronFactory::produceProperty ( Interface::ProjectHandle * projectHandle,
                                                                         database::Record *              record,
                                                                         const string &                  userName,
                                                                         const string &                  cauldronName,
                                                                         const string &                  unit,
                                                                         Interface::PropertyType         type,
                                                                         const DataModel::PropertyAttribute attr ) {

   return new Property (projectHandle, record, userName, cauldronName, unit, type, attr );

}

Interface::PropertyValue * FastcauldronFactory::producePropertyValue ( Interface::ProjectHandle *   projectHandle,
                                                                            database::Record *                record,
                                                                            const string &                    name,
                                                                            const Interface::Property *  property,
                                                                            const Interface::Snapshot *  snapshot,
                                                                            const Interface::Reservoir * reservoir,
                                                                            const Interface::Formation * formation,
                                                                            const Interface::Surface *   surface,
                                                                            Interface::PropertyStorage   storage) {

   return new PropertyValue ( projectHandle, record, name, property, snapshot, reservoir, formation, surface, storage );
}


Interface::Formation* FastcauldronFactory::produceFormation ( Interface::ProjectHandle *   projectHandle,
                                                                   database::Record *                record ) {
   return new LayerProps ( projectHandle, record );
}

Interface::CrustFormation* FastcauldronFactory::produceCrustFormation ( Interface::ProjectHandle * projectHandle,
                                                                             database::Record *              record) {

   return new CrustFormation ( projectHandle, record );
}

Interface::MantleFormation* FastcauldronFactory::produceMantleFormation ( Interface::ProjectHandle * projectHandle,
                                                                               database::Record *              record) {
   return new MantleFormation ( projectHandle, record );
}


GeoPhysics::CompoundLithology* FastcauldronFactory::produceCompoundLithology ( GeoPhysics::ProjectHandle * projectHandle ) {
   return new Lithology ( projectHandle );
}
