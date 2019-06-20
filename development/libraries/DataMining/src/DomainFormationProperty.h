//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MINING__DOMAIN_FORMATION_PROPERTY_H_
#define _MINING__DOMAIN_FORMATION_PROPERTY_H_

#include <map>

#include "Snapshot.h"
#include "GridMap.h"
#include "Formation.h"
#include "Property.h"
#include "PropertyValue.h"

#include "DerivedPropertyManager.h"

#include "FiniteElementTypes.h"

#include "PropertyInterpolator2D.h"
#include "PropertyInterpolator3D.h"
#include "ElementPosition.h"
#include "InterpolatedPropertyValues.h"

#include "DomainProperty.h"

namespace DataAccess {

   namespace Mining {

      /// Objects contain the 3D grid-maps for the associated property.
      class DomainFormationProperty : public DomainProperty {

      public :

         DomainFormationProperty ( const DomainPropertyCollection*            collection,
                                   DerivedProperties::DerivedPropertyManager& propertyManager,
                                   const Interface::Snapshot*                 snapshot,
                                   const Interface::Property*                 property );

         ~DomainFormationProperty ();


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
         typedef std::map < const DataModel::AbstractFormation*, AbstractDerivedProperties::FormationPropertyPtr > FormationToPropertyValueMapping;

         PropertyInterpolator2D           interpolate2D;
         PropertyInterpolator3D           interpolate3D;
         FormationToPropertyValueMapping  m_values;

      };


      /// Functor class for allocating DomainFormationProperty objects.
      class DomainFormationPropertyAllocator : public DomainPropertyAllocator {

      public :

         DomainProperty* allocate ( const DomainPropertyCollection*            collection,
                                    DerivedProperties::DerivedPropertyManager& propertyManager,
                                    const Interface::Snapshot*                 snapshot,
                                    const Interface::Property*                 property ) const;

      };

   }

}



#endif // _MINING__DOMAIN_FORMATION_PROPERTY_H_
