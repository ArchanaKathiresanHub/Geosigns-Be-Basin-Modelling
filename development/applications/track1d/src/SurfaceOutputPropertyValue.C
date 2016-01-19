#include "SurfaceOutputPropertyValue.h"

SurfaceOutputPropertyValue::SurfaceOutputPropertyValue ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                         const DataModel::AbstractProperty*          property,
                                                         const DataModel::AbstractSnapshot*          snapshot,
                                                         const DataModel::AbstractSurface*           surface ) {

   if ( property != 0 and snapshot != 0 and surface != 0 ) {
      m_surfaceProperty = propertyManager.getSurfaceProperty ( property, snapshot, surface );
   }

}

double SurfaceOutputPropertyValue::getValue ( const double i, const double j, const double k ) const {

   if ( m_surfaceProperty != 0 ) {
      return m_surfaceProperty->interpolate ( i, j );
   } else {
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }

}


unsigned int SurfaceOutputPropertyValue::getDepth () const {

   if ( m_surfaceProperty != 0 ) {
      return 1;
   } else {
      return 0;
   }
   
}

bool SurfaceOutputPropertyValue::isPrimary() const {

   if ( m_surfaceProperty != 0 ) {
      return m_surfaceProperty->isPrimary();
   } else {
      return false;
   }
}
