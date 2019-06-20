//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MINING__DOMAIN_SURFACE_PROPERTY_H_
#define _MINING__DOMAIN_SURFACE_PROPERTY_H_

#include <map>

#include "AbstractSurface.h"

#include "ProjectHandle.h"
#include "Snapshot.h"
#include "GridMap.h"
#include "Surface.h"
#include "Property.h"

#include "FormationProperty.h"
#include "DerivedPropertyManager.h"

#include "PropertyInterpolator2D.h"
#include "ElementPosition.h"
#include "InterpolatedPropertyValues.h"

#include "DomainProperty.h"


namespace DataAccess {

   namespace Mining {

      class DomainSurfaceProperty : public DomainProperty {

         /// \typedef SurfaceToGridMapMapping
         typedef std::map < const DataModel::AbstractSurface*, AbstractDerivedProperties::SurfacePropertyPtr > SurfaceToPropertyValueMapping;

      public :

         DomainSurfaceProperty ( const DomainPropertyCollection*            collection,
                                 DerivedProperties::DerivedPropertyManager& propertyManager,
                                 const Interface::Snapshot*                 snapshot,
                                 const Interface::Property*                 property );

         ~DomainSurfaceProperty ();


         double compute ( const ElementPosition& position ) const;


      protected :

         PropertyInterpolator2D        interpolate2D;
         SurfaceToPropertyValueMapping m_values;

      };



      /// Functor class for allocating DomainSurfaceProperty objects.
      class DomainSurfacePropertyAllocator : public DomainPropertyAllocator {

      public :

         DomainProperty* allocate ( const DomainPropertyCollection*            collection,
                                    DerivedProperties::DerivedPropertyManager& propertyManager,
                                    const Interface::Snapshot*                 snapshot,
                                    const Interface::Property*                 property ) const;

      };

   }

}


#endif // _MINING__DOMAIN_SURFACE_PROPERTY_H_
