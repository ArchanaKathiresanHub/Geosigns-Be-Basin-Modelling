#include "DomainProperty.h"

#include "DomainPropertyCollection.h"

DataAccess::Mining::DomainProperty::DomainProperty ( const DomainPropertyCollection*  collection,
                                                     const Interface::Snapshot* snapshot,
                                                     const Interface::Property* property ) : 
   m_collection ( collection ),
   m_snapshot ( snapshot ),
   m_property ( property )
{
}

DataAccess::Mining::DomainProperty::~DomainProperty () {
}

const DataAccess::Interface::Property* DataAccess::Mining::DomainProperty::getProperty () const {
   return m_property;
}

const DataAccess::Interface::Snapshot* DataAccess::Mining::DomainProperty::getSnapshot () const {
   return m_snapshot;
}

const DataAccess::Interface::ProjectHandle* DataAccess::Mining::DomainProperty::getProjectHandle () const {
   return m_collection->getProjectHandle ();
}

const DataAccess::Mining::DomainPropertyCollection* DataAccess::Mining::DomainProperty::getPropertyCollection () const {
   return m_collection;
}

bool DataAccess::Mining::DomainProperty::initialise () {
   // Do nothing.
   return true;
}

void DataAccess::Mining::DomainProperty::compute ( const ElementPosition&            position,
                                                         InterpolatedPropertyValues& evaluations ) const {
   evaluations.setValue ( getProperty (), compute ( position ));
}


DataAccess::Mining::DomainPropertyAllocator::DomainPropertyAllocator () {
}

DataAccess::Mining::DomainPropertyAllocator::~DomainPropertyAllocator () {
}
