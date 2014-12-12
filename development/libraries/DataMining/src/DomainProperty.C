#include "DomainProperty.h"
#include "DomainPropertyCollection.h"

namespace DataAccess { namespace Mining
{

  DomainProperty::DomainProperty( const DomainPropertyCollection*  collection,
                                  const Interface::Snapshot* snapshot,
                                  const Interface::Property* property
                                )
                                : m_collection ( collection )
                                , m_snapshot ( snapshot )
                                , m_property ( property )
  {
  }

  DomainProperty::~DomainProperty() {}

  const Interface::ProjectHandle * DomainProperty::getProjectHandle() const { return m_collection->getProjectHandle(); }

  bool DomainProperty::initialise()
  { 
     // Do nothing.
     return true;
  }

  void DomainProperty::compute( const ElementPosition      & position,
                                InterpolatedPropertyValues & evaluations ) const
  {
     evaluations.setValue( getProperty(), compute( position ) );
  }
}} // namespace DataAccess::Mining

