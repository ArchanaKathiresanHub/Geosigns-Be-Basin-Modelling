#include "DomainFormationMapProperty.h"

#include "Interface/PropertyValue.h"

namespace DataAccess { namespace Mining
{
    
   DomainFormationMapProperty::DomainFormationMapProperty( const DomainPropertyCollection * collection,
                                                           const Interface::Snapshot      * snapshot,
                                                           const Interface::Property      * property
                                                         )
      : DomainProperty( collection, snapshot, property )
   {

      Interface::PropertyValueList * values = getProjectHandle()->getPropertyValues( Interface::FORMATION, getProperty(), getSnapshot(), 0, 0, 0, MAP );

      for ( Interface::PropertyValueList::const_iterator valueIter = values->begin(); valueIter != values->end(); ++valueIter )
      {
         m_values[ (*valueIter)->getFormation() ] = *valueIter;
      }
      delete values;
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
            const Interface::GridMap * grid = propIter->second->getGridMap();
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
            const Interface::GridMap* grid = propIter->second->getGridMap ();
            return interpolate2D ( position, grid );
         }
         else
         {
            return DataAccess::Interface::DefaultUndefinedMapValue;
         }
      }
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }


   DomainProperty* DomainFormationMapPropertyAllocator::allocate( const DomainPropertyCollection * collection,
                                                                  const Interface::Snapshot      * snapshot,
                                                                  const Interface::Property      * property ) const
   {
      return new DomainFormationMapProperty( collection, snapshot, property );
   }
}} // namespace DataAccess::Mining

