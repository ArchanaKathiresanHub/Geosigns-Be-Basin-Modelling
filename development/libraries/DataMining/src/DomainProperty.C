#include "DomainProperty.h"
#include "DomainPropertyCollection.h"

namespace DataAccess { namespace Mining
{

  DomainProperty::DomainProperty( const DomainPropertyCollection*            collection,
                                  DerivedProperties::DerivedPropertyManager& propertyManager,
                                  const Interface::Snapshot*                 snapshot,
                                  const Interface::Property*                 property ) :
     m_collection ( collection ),
     m_propertyManager ( propertyManager ),
     m_snapshot ( snapshot ),
     m_property ( property )
  {
  }

  DomainProperty::~DomainProperty() {}

  const Interface::ProjectHandle * DomainProperty::getProjectHandle() const { return m_collection->getProjectHandle(); }

   DerivedProperties::DerivedPropertyManager& DomainProperty::getPropertyManager () const {
      return m_propertyManager;
   }


  bool DomainProperty::initialise()
  {
     // Do nothing.
     return true;
  }

}} // namespace DataAccess::Mining
