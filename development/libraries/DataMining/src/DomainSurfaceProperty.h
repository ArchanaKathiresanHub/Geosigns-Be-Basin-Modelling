#ifndef _MINING__DOMAIN_SURFACE_PROPERTY_H_
#define _MINING__DOMAIN_SURFACE_PROPERTY_H_

#include <map>

#include "AbstractSurface.h"

#include "Interface/ProjectHandle.h"
#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"
#include "Interface/Surface.h"
#include "Interface/Property.h"

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
         typedef std::map < const DataModel::AbstractSurface*, DerivedProperties::SurfacePropertyPtr > SurfaceToPropertyValueMapping;

      public :

         DomainSurfaceProperty ( const DomainPropertyCollection*            collection,
                                 DerivedProperties::DerivedPropertyManager& propertyManager,
                                 const Interface::Snapshot*                 snapshot,
                                 const Interface::Property*                 property );

         ~DomainSurfaceProperty ();


         /// Add the property, evaluated at the position, to the evaluations mapping.
         ///
         /// If the formation is null then a null-value will be added.
         void compute ( const ElementPosition&            position,
                              InterpolatedPropertyValues& evaluations ) const;

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
