#include "FormationOutputPropertyValue.h"

DerivedProperties::FormationOutputPropertyValue::FormationOutputPropertyValue ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                                const DataModel::AbstractProperty*          property,
                                                                                const DataModel::AbstractSnapshot*          snapshot,
                                                                                const DataModel::AbstractFormation*         formation ) :
   OutputPropertyValue ( property ) {

   if ( property != 0 and snapshot != 0 and formation != 0 ) {
      m_formationProperty = propertyManager.getFormationProperty ( property, snapshot, formation );
   }

}

DerivedProperties::FormationOutputPropertyValue::FormationOutputPropertyValue ( DerivedProperties::FormationPropertyPtr& formationProperty ) : 
   OutputPropertyValue ( formationProperty->getProperty ()), m_formationProperty ( formationProperty ) {
}

void DerivedProperties::FormationOutputPropertyValue::retrieveData () const {

   if ( m_formationProperty != 0 ) {
      m_formationProperty->retrieveData();
   }
}

const string & DerivedProperties::FormationOutputPropertyValue::getName() const { 

   if( m_formationProperty != 0 ) {
      return  m_formationProperty->getProperty()->getName(); 
   } else {
      return s_nullString;
   }
}

void DerivedProperties::FormationOutputPropertyValue::restoreData () const {

   if ( m_formationProperty != 0 ) {
      m_formationProperty->restoreData();
   }
}


double DerivedProperties::FormationOutputPropertyValue::getValue ( const double i, const double j, const double k ) const {

   if ( m_formationProperty != 0 ) {
      return m_formationProperty->interpolate ( i, j, k );
   } else {
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }

}

unsigned int DerivedProperties::FormationOutputPropertyValue::getDepth () const {

   if ( m_formationProperty != 0 ) {
      return m_formationProperty->lengthK ();
   } else {
      return 0;
   }
   
}

bool DerivedProperties::FormationOutputPropertyValue::isPrimary() const {
   if ( m_formationProperty != 0 ) {
      return m_formationProperty->isPrimary ();
   } else {
      return false;
   }
}
   
bool DerivedProperties::FormationOutputPropertyValue::hasMap() const { 

   if( m_formationProperty != 0 ) {

      if( m_formationProperty->isPrimary() and m_formationProperty->getGridMap() == 0 ) {
         return false;
      }
      return true;
   }
   return false;
}

const DataModel::AbstractGrid* DerivedProperties::FormationOutputPropertyValue::getGrid () const {

   if( m_formationProperty != 0 ) {
      return m_formationProperty->getGrid();
   }
   return false;
}
