#ifndef __FASTCAULDRONOBJECTFACTORY_HH_
#define __FASTCAULDRONOBJECTFACTORY_HH_

#include "ProjectFileHandler.h"

#include "PropertyAttribute.h"

#include "GeoPhysicsObjectFactory.h"
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
   Interface::ProjectHandle * produceProjectHandle ( database::ProjectFileHandlerPtr pfh,
                                                     const string&                   name,
                                                     const string&                   accessMode ) const;

   /// Allocate a fastcauldron related-project.
   Interface::RelatedProject* produceRelatedProject ( Interface::ProjectHandle* projectHandle,
                                                      database::Record*         record ) const;

   Interface::RunParameters * produceRunParameters ( Interface::ProjectHandle * projectHandle,
                                                     database::Record *         record) const;

   /// Allocate a fastcauldron property.
   Interface::Property * produceProperty ( Interface::ProjectHandle * projectHandle,
                                           database::Record *         record,
                                           const string &             userName,
                                           const string &             cauldronName,
                                           const string &             unit,
                                           Interface::PropertyType    type,
                                           const DataModel::PropertyAttribute       attr,
                                           const DataModel::PropertyOutputAttribute attrOut) const;

   /// Allocate a fastcauldron property-value.
   Interface::PropertyValue * producePropertyValue ( Interface::ProjectHandle *   projectHandle,
                                                     database::Record *                record,
                                                     const string &                    name,
                                                     const Interface::Property *  property,
                                                     const Interface::Snapshot *  snapshot,
                                                     const Interface::Reservoir * reservoir,
                                                     const Interface::Formation * formation,
                                                     const Interface::Surface *   surface,
                                                     Interface::PropertyStorage   storage,
                                                     const std::string & fileName = "") const;


   Interface::Formation* produceFormation ( Interface::ProjectHandle *   projectHandle,
                                            database::Record *           record ) const;

   Interface::CrustFormation* produceCrustFormation ( Interface::ProjectHandle * projectHandle,
                                                      database::Record *         record) const;

   Interface::MantleFormation* produceMantleFormation ( Interface::ProjectHandle * projectHandle,
                                                        database::Record *         record) const;


   GeoPhysics::CompoundLithology* produceCompoundLithology ( GeoPhysics::ProjectHandle * projectHandle ) const;

};

/** @} */


#endif // __FASTCAULDRONOBJECTFACTORY_HH_
