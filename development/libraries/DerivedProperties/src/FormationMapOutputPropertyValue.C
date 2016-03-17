#include "FormationMapOutputPropertyValue.h"

DerivedProperties::FormationMapOutputPropertyValue::FormationMapOutputPropertyValue ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                                      const DataModel::AbstractProperty*          property,
                                                                                      const DataModel::AbstractSnapshot*          snapshot,
                                                                                      const DataModel::AbstractFormation*         formation ) : OutputPropertyValue ( property ) {

   if ( property != 0 and snapshot != 0 and formation != 0 ) {
      m_formationMapProperty = propertyManager.getFormationMapProperty ( property, snapshot, formation );
   }

}

double DerivedProperties::FormationMapOutputPropertyValue::getValue ( const double i, const double j, const double k ) const {

   if ( m_formationMapProperty != 0 ) {
      return m_formationMapProperty->interpolate ( i, j );
   } else {
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }

}

void DerivedProperties::FormationMapOutputPropertyValue::restoreData () const {
  if ( m_formationMapProperty != 0 ) {
     m_formationMapProperty->restoreData();
  }
}

void DerivedProperties::FormationMapOutputPropertyValue::retrieveData () const {
  if ( m_formationMapProperty != 0 ) {
     m_formationMapProperty->retrieveData();
  }
}

unsigned int DerivedProperties::FormationMapOutputPropertyValue::getDepth () const {

   if ( m_formationMapProperty != 0 ) {
      return 1;
   } else {
      return 0;
   }
}

const string & DerivedProperties::FormationMapOutputPropertyValue::getName() const {
   
   if ( m_formationMapProperty != 0 ) {
      return m_formationMapProperty->getProperty()->getName(); 
   } else {
      return s_nullString;
   }
}

bool DerivedProperties::FormationMapOutputPropertyValue::hasMap() const { 

   if( m_formationMapProperty != 0 ) {

      if( m_formationMapProperty->isPrimary() and m_formationMapProperty->getGridMap() == 0 ) {
         return false;
      }
      return true;
   }
   return false;
}

bool DerivedProperties::FormationMapOutputPropertyValue::isPrimary() const { 

   if( m_formationMapProperty != 0 ) {
      return m_formationMapProperty->isPrimary();
   } else {
      return false;
   }
}
