#ifndef _MINING__DOMAIN_FORMATION_MAP_PROPERTY_H_
#define _MINING__DOMAIN_FORMATION_MAP_PROPERTY_H_

#include <map>

#include "Interface/ProjectHandle.h"
#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"
#include "Interface/Surface.h"
#include "Interface/Property.h"

#include "PropertyInterpolator2D.h"
#include "ElementPosition.h"
#include "InterpolatedPropertyValues.h"

#include "DomainProperty.h"

namespace DataAccess
{
   namespace Mining
   {
      class DomainFormationMapProperty : public DomainProperty
      {
         /// \typedef SurfaceToGridMapMapping
         typedef std::map < const Interface::Formation *, const Interface::PropertyValue * > FormationMapToPropertyValueMapping;

      public :

         DomainFormationMapProperty( const DomainPropertyCollection * collection,
                                     const Interface::Snapshot      * snapshot,
                                     const Interface::Property      * property );

         virtual ~DomainFormationMapProperty();


         /// Add the property, evaluated at the position, to the evaluations mapping.
         ///
         /// If the formation is null then a null-value will be added.
         void compute( const ElementPosition&            position,
                             InterpolatedPropertyValues& evaluations );

         double compute( const ElementPosition& position ) const;

      protected :
         PropertyInterpolator2D             interpolate2D;
         FormationMapToPropertyValueMapping m_values;
      };


      /// Functor class for allocating DomainFormationMapProperty objects.
      class DomainFormationMapPropertyAllocator : public DomainPropertyAllocator
      {
      public :
         DomainProperty* allocate ( const DomainPropertyCollection * collection,
                                    const Interface::Snapshot      * snapshot,
                                    const Interface::Property      * property ) const;
      };
   }
}


#endif // _MINING__DOMAIN_SURFACE_PROPERTY_H_
