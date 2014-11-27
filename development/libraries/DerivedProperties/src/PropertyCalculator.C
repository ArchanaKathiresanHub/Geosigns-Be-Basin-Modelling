#include "PropertyCalculator.h"

DerivedProperties::PropertyCalculator::PropertyCalculator ( const GeoPhysics::ProjectHandle* projectHandle ) : m_projectHandle ( projectHandle ) {
}

DerivedProperties::PropertyCalculator::~PropertyCalculator () {
}

bool DerivedProperties::PropertyCalculator::getNodeIsValid ( const unsigned int i, const unsigned int j ) const {
   return m_projectHandle->getNodeIsValid ( i, j );
}

const GeoPhysics::ProjectHandle* DerivedProperties::PropertyCalculator::getProjectHandle () const {
   return m_projectHandle;
}
      
