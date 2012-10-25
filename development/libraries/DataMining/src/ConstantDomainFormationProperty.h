#ifndef _MINING__CONSTANT_DOMAIN_FORMATION_PROPERTY_H_
#define _MINING__CONSTANT_DOMAIN_FORMATION_PROPERTY_H_

#include <map>

#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"
#include "Interface/Formation.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"

#include "FiniteElementTypes.h"

#include "PropertyInterpolator2D.h"
#include "PropertyInterpolator3D.h"
#include "ElementPosition.h"
#include "InterpolatedPropertyValues.h"

#include "DomainProperty.h"

namespace DataAccess {

   namespace Mining {

      /// Objects contain the 3D grid-maps for the associated property.
      class ConstantDomainFormationProperty : public DomainProperty {

      public :

         using DomainProperty::compute;

         ConstantDomainFormationProperty ( const DomainPropertyCollection* collection,
                                           const Interface::Snapshot*      snapshot,
                                           const Interface::Property*      property );

         ~ConstantDomainFormationProperty ();

         /// \brief Interpolate the Cauldron property at the point specified.
         double compute ( const ElementPosition& position ) const;

         /// \brief Get the coefficients of the property at the position.
         ///
         /// If the formation is null or the grid map of the property-value is 
         /// null the the coefficients vector will be filled with null values.
         void extractCoefficients ( const ElementPosition& position,
                                          FiniteElementMethod::ElementVector& coefficients ) const;

      protected :

         /// \typedef FormationToGridMapMapping
         typedef std::map < const Interface::Formation*, const Interface::PropertyValue*> FormationToPropertyValueMapping;

         PropertyInterpolator2D           interpolate2D;
         PropertyInterpolator3D           interpolate3D;
         FormationToPropertyValueMapping  m_values;

      };


      /// Functor class for allocating DomainFormationProperty objects.
      class ConstantDomainFormationPropertyAllocator : public DomainPropertyAllocator {

      public :

         DomainProperty* allocate ( const DomainPropertyCollection* collection,
                                    const Interface::Snapshot*      snapshot,
                                    const Interface::Property*      property ) const;

      };

   }

}



#endif // _MINING__CONSTANT_DOMAIN_FORMATION_PROPERTY_H_
