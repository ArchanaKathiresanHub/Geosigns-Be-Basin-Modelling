//
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MINING__DOMAIN_PROPERTY_FACTORY_H_
#define _MINING__DOMAIN_PROPERTY_FACTORY_H_

#include <map>

#include "ProjectFileHandler.h"
#include "ProjectHandle.h"
#include "ObjectFactory.h"
#include "Snapshot.h"
#include "Property.h"

#include "GeoPhysicsObjectFactory.h"

#include "DerivedPropertyManager.h"

#include "DomainProperty.h"
#include "CauldronDomain.h"

namespace DataAccess
{

   namespace Mining
   {
      /// Class for allocating domain-property-derived objects.
      class ObjectFactory : public GeoPhysics::ObjectFactory
      {

         typedef std::map < const Interface::Property*, DomainPropertyAllocator* > PropertyToDomainPropertyAllocator;

      public :
         virtual ~ObjectFactory ();

         /// Allocates a data-mining project-handle.
         Interface::ProjectHandle* produceProjectHandle( database::ProjectFileHandlerPtr pfh,
                                                         const string & name) const;

         /// Allocate a new domain-property.
         virtual DomainProperty* allocate ( const DomainPropertyCollection*            collection,
                                            DerivedProperties::DerivedPropertyManager& propertyManager,
                                            const Interface::Snapshot*                 snapshot,
                                            const Interface::Property*                 property ) const;

         /// Determine is the property has an associated allocator.
         virtual bool containsAllocator( const Interface::Property* property ) const;

         /// Add a new allocator.
         ///
         /// If an allocator exists for the property then it is first deleted
         /// and the new one added to the map.
         virtual void addAllocator( const Interface::Property * property,
                                    DomainPropertyAllocator   * allocator );

         /// Allocate a DomainFormationPropertyAllocator.
         virtual DomainPropertyAllocator* produceFormationPropertyAllocator( const Interface::ProjectHandle * projectHandle,
                                                                             const Interface::Property      * property ) const;
         /// Allocate a DomainFormationMapPropertyAllocator.
         virtual DomainPropertyAllocator* produceFormationMapPropertyAllocator( const Interface::ProjectHandle * projectHandle,
                                                                                const Interface::Property      * property ) const;

         /// Allocate a ConstantDomainFormationPropertyAllocator.
         virtual DomainPropertyAllocator* produceFormationConstantPropertyAllocator( const Interface::ProjectHandle * projectHandle,
                                                                                     const Interface::Property      * property ) const;

         /// Allocate a DomainSurfacePropertyAllocator.
         virtual DomainPropertyAllocator* produceSurfacePropertyAllocator( const Interface::ProjectHandle * projectHandle,
                                                                           const Interface::Property      * property ) const;

         /// Allocate a DomainReservoirPropertyAllocator.
         virtual DomainPropertyAllocator* produceReservoirPropertyAllocator( const Interface::ProjectHandle * projectHandle,
                                                                             const Interface::Property      * property ) const;

         /// Allocate a domain-property-collection.
         virtual DomainPropertyCollection* produceDomainPropertyCollection( Interface::ProjectHandle* projectHandle ) const;

         virtual CauldronDomain* produceCauldronDomain( Interface::ProjectHandle * projectHandle ) const;

         /// Initialises the domain-property allocators.
         ///
         /// When a project-handle is produced, this function MUST be called.
         /// It should be called once during the life-time of the application.
         /// Since only a single project-handle will be allocated.
         void initialiseObjectFactory( Interface::ProjectHandle * handle );

      protected :

         Interface::ProjectHandle        * m_projectHandle;
         PropertyToDomainPropertyAllocator m_allocators;
      };
   }
}

#endif // _MINING__DOMAIN_PROPERTY_FACTORY_H_
