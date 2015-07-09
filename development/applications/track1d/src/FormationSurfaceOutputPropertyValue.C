#include "FormationSurfaceOutputPropertyValue.h"

FormationSurfaceOutputPropertyValue::FormationSurfaceOutputPropertyValue ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                           const DataModel::AbstractProperty*          property,
                                                                           const DataModel::AbstractSnapshot*          snapshot,
                                                                           const DataModel::AbstractFormation*         formation,
                                                                           const DataModel::AbstractSurface*           surface ) {

   if ( property != 0 and snapshot != 0 and formation != 0 and surface != 0 ) {
      m_formationSurfaceProperty = propertyManager.getFormationSurfaceProperty ( property, snapshot, formation, surface );
   }

}

double FormationSurfaceOutputPropertyValue::getValue ( const double i, const double j, const double k ) const {

   if ( m_formationSurfaceProperty != 0 ) {
      return m_formationSurfaceProperty->interpolate ( i, j );
   } else {
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }

}


unsigned int FormationSurfaceOutputPropertyValue::getDepth () const {

   if ( m_formationSurfaceProperty != 0 ) {
      return 1;
   } else {
      return 0;
   }
   
}


