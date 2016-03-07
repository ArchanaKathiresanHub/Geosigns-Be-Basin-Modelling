#include "FormationOutputPropertyValue.h"

FormationOutputPropertyValue::FormationOutputPropertyValue ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                             const DataModel::AbstractProperty*          property,
                                                             const DataModel::AbstractSnapshot*          snapshot,
                                                             const DataModel::AbstractFormation*         formation ) {

   if ( property != 0 and snapshot != 0 and formation != 0 ) {
      m_formationProperty = propertyManager.getFormationProperty ( property, snapshot, formation );
   }

}

FormationOutputPropertyValue::FormationOutputPropertyValue ( DerivedProperties::FormationPropertyPtr& formationProperty ) : m_formationProperty ( formationProperty )
{
}


double FormationOutputPropertyValue::getValue ( const double i, const double j, const double k ) const {

   if ( m_formationProperty != 0 ) {
      return m_formationProperty->interpolate ( i, j, k, not isPrimary() );
   } else {
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }

}

unsigned int FormationOutputPropertyValue::getDepth () const {

   if ( m_formationProperty != 0 ) {
      return m_formationProperty->lengthK ();
   } else {
      return 0;
   }
   
}

bool FormationOutputPropertyValue::isPrimary() const {
   
   if ( m_formationProperty != 0 ) {
      return m_formationProperty->isPrimary();
   } else {
      return false;
   }
}
