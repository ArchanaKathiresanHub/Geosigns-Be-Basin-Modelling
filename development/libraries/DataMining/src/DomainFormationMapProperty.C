#include "DomainFormationMapProperty.h"

#include "Interface/PropertyValue.h"

namespace DataAccess { namespace Mining
{
    
   DomainFormationMapProperty::DomainFormationMapProperty( const DomainPropertyCollection *           collection,
                                                           DerivedProperties::DerivedPropertyManager& propertyManager,
                                                           const Interface::Snapshot      *           snapshot,
                                                           const Interface::Property      *           property
                                                         )
      : DomainProperty( collection, propertyManager, snapshot, property )
   {

      DerivedProperties::FormationMapPropertyList values = propertyManager.getFormationMapProperties ( getProperty (), getSnapshot (), true );

      for ( size_t i = 0; i < values.size (); ++i ) {
         m_values [ values [ i ]->getFormation ()] = values [ i ];
      }

   }


   DataAccess::Mining::DomainFormationMapProperty::~DomainFormationMapProperty()
   {
      m_values.clear ();
   }


   void DomainFormationMapProperty::compute( const ElementPosition      & position,
                                             InterpolatedPropertyValues & evaluations )
   {

      if ( position.getFormation() != 0 )
      {
         FormationMapToPropertyValueMapping::const_iterator propIter = m_values.find( position.getFormation() );

         if ( propIter != m_values.end() )
         {
            DerivedProperties::FormationMapPropertyPtr grid = propIter->second;
            evaluations.setValue( getProperty(), interpolate2D( position, grid ) );
         }
         else
         {
            evaluations.setValue( getProperty(), DataAccess::Interface::DefaultUndefinedMapValue );
         }
      }
      else if ( not evaluations.contains( getProperty() ) )
      {
         // What kind of error is this?
         evaluations.setValue( getProperty(), DataAccess::Interface::DefaultUndefinedMapValue );
      }
   }


   double DomainFormationMapProperty::compute( const ElementPosition & position ) const
   {
      if ( position.getFormation() != 0 )
      {
         FormationMapToPropertyValueMapping::const_iterator propIter = m_values.find( position.getFormation() );

         if ( propIter != m_values.end() )
         {
            DerivedProperties::FormationMapPropertyPtr grid = propIter->second;
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

