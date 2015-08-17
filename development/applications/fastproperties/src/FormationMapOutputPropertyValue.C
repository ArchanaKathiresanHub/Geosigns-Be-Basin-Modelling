#include "FormationMapOutputPropertyValue.h"

FormationMapOutputPropertyValue::FormationMapOutputPropertyValue ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                   const DataModel::AbstractProperty*          property,
                                                                   const DataModel::AbstractSnapshot*          snapshot,
                                                                   const DataModel::AbstractFormation*         formation ) : OutputPropertyValue ( property ) {

   if ( property != 0 and snapshot != 0 and formation != 0 ) {
      m_formationMapProperty = propertyManager.getFormationMapProperty ( property, snapshot, formation );
   }

}

double FormationMapOutputPropertyValue::getValue ( const double i, const double j, const double k ) const {

   if ( m_formationMapProperty != 0 ) {
      return m_formationMapProperty->interpolate ( i, j );
   } else {
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }

}

unsigned int FormationMapOutputPropertyValue::getDepth () const {

   if ( m_formationMapProperty != 0 ) {
      return 1;
   } else {
      return 0;
   }
   
}

bool FormationMapOutputPropertyValue::hasMap() const { 

   if( m_formationMapProperty != 0 ) {

      if( m_formationMapProperty->isPrimary() and m_formationMapProperty->getGridMap() == 0 ) {
         return false;
      }
      return true;
   }
   return false;
}
