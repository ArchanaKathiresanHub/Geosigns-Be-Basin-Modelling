#ifndef __FASTCAULDRONOBJECTFACTORY_HH_
#define __FASTCAULDRONOBJECTFACTORY_HH_

#include "GeoPhysicsObjectFactory.h"
// #include "Interface/ObjectFactory.h"

#include "PiecewiseInterpolator.h"

#include "CompoundLithology.h"

namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;  
   }
}

namespace database
{
   class Record;
   class Database;
}


/** @addtogroup Fastcauldron
 *
 * @{
 */

using namespace DataAccess;

/// \file FastcauldronFactory.h
/// \brief An object factory creating objects necessary for allochthonous modelling.

/// \brief An object factory creating objects necessary for allochthonous modelling.
class FastcauldronFactory : public GeoPhysics::ObjectFactory {

public :


   /// Returns the project-handle.
   Interface::ProjectHandle * produceProjectHandle ( database::Database * database,
                                                          const string &       name,
                                                          const string &       accessMode);

   /// Allocate a fastcauldron related-project.
   Interface::RelatedProject* produceRelatedProject ( Interface::ProjectHandle* projectHandle,
                                                           database::Record*              record );

   Interface::RunParameters * produceRunParameters ( Interface::ProjectHandle * projectHandle, 
                                                          database::Record *              record);

   /// Allocate a fastcauldron property.
   Interface::Property * produceProperty ( Interface::ProjectHandle * projectHandle,
                                                database::Record *              record,
                                                const string &                  userName,
                                                const string &                  cauldronName,
                                                const string &                  unit, 
                                                Interface::PropertyType         type);

   /// Allocate a fastcauldron property-value.
   Interface::PropertyValue * producePropertyValue ( Interface::ProjectHandle *   projectHandle,
                                                          database::Record *                record,
                                                          const string &                    name,
                                                          const Interface::Property *  property,
                                                          const Interface::Snapshot *  snapshot,
                                                          const Interface::Reservoir * reservoir,
                                                          const Interface::Formation * formation,
                                                          const Interface::Surface *   surface,
                                                          Interface::PropertyStorage   storage);



   Interface::Formation* produceFormation ( Interface::ProjectHandle *   projectHandle,
                                                 database::Record *                record );

   Interface::CrustFormation* produceCrustFormation ( Interface::ProjectHandle * projectHandle,
                                                           database::Record *              record);

   Interface::MantleFormation* produceMantleFormation ( Interface::ProjectHandle * projectHandle,
                                                             database::Record *              record);


   GeoPhysics::CompoundLithology* produceCompoundLithology ( GeoPhysics::ProjectHandle * projectHandle );

};

/** @} */


#endif // __FASTCAULDRONOBJECTFACTORY_HH_
