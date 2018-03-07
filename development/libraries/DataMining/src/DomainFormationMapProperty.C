//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "DomainFormationMapProperty.h"

#include "Interface/PropertyValue.h"

using namespace AbstractDerivedProperties;

namespace DataAccess { namespace Mining
{

   DomainFormationMapProperty::DomainFormationMapProperty( const DomainPropertyCollection *           collection,
                                                           DerivedProperties::DerivedPropertyManager& propertyManager,
                                                           const Interface::Snapshot      *           snapshot,
                                                           const Interface::Property      *           property
                                                         )
      : DomainProperty( collection, propertyManager, snapshot, property )
   {

      FormationMapPropertyList values = propertyManager.getFormationMapProperties ( getProperty (), getSnapshot (), true );

      for ( size_t i = 0; i < values.size (); ++i ) {
         m_values [ values [ i ]->getFormation ()] = values [ i ];
      }

   }


   DataAccess::Mining::DomainFormationMapProperty::~DomainFormationMapProperty()
   {
      m_values.clear ();
   }

   double DomainFormationMapProperty::compute( const ElementPosition & position ) const
   {
      if ( position.getFormation() != 0 )
      {
         FormationMapToPropertyValueMapping::const_iterator propIter = m_values.find( position.getFormation() );

         if ( propIter != m_values.end() )
         {
            FormationMapPropertyPtr grid = propIter->second;
            return interpolate2D ( position, grid );
         }
         else
         {
            return DataAccess::Interface::DefaultUndefinedMapValue;
         }
      }
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }


   DomainProperty* DomainFormationMapPropertyAllocator::allocate( const DomainPropertyCollection *           collection,
                                                                  DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                  const Interface::Snapshot      *           snapshot,
                                                                  const Interface::Property      *           property ) const
   {
      return new DomainFormationMapProperty( collection, propertyManager, snapshot, property );
   }
}} // namespace DataAccess::Mining
