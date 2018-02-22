#include "FormationSurfaceOutputPropertyValue.h"

DerivedProperties::FormationSurfaceOutputPropertyValue::FormationSurfaceOutputPropertyValue ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                                              const DataModel::AbstractProperty*          property,
                                                                                              const DataModel::AbstractSnapshot*          snapshot,
                                                                                              const DataModel::AbstractFormation*         formation,
                                                                                              const DataModel::AbstractSurface*           surface )  : OutputPropertyValue ( property ){

   if ( property != 0 and snapshot != 0 and formation != 0 and surface != 0 ) {
      m_formationSurfaceProperty = propertyManager.getFormationSurfaceProperty ( property, snapshot, formation, surface );
   }

   m_mapName = "";
}

void DerivedProperties:: FormationSurfaceOutputPropertyValue::restoreData () const {
   if ( m_formationSurfaceProperty != 0 ) {
      m_formationSurfaceProperty->restoreData();
   }
}

void DerivedProperties::FormationSurfaceOutputPropertyValue::retrieveData () const {
   if ( m_formationSurfaceProperty != 0 ) {
      m_formationSurfaceProperty->retrieveData();
   }
}

bool DerivedProperties::FormationSurfaceOutputPropertyValue::hasMap() const { 

   if( m_formationSurfaceProperty != 0 ) {

      if( m_formationSurfaceProperty->isPrimary() and m_formationSurfaceProperty->getGridMap() == 0 ) {
         return false;
      }
      return true;
   }
   return false;
}

const DataModel::AbstractGrid* DerivedProperties::FormationSurfaceOutputPropertyValue::getGrid () const {

   if( m_formationSurfaceProperty != 0 ) {
      return m_formationSurfaceProperty->getGrid();
   }
   return false;
}

double DerivedProperties::FormationSurfaceOutputPropertyValue::getValue ( const double i, const double j, const double k ) const {

   if ( m_formationSurfaceProperty != 0 ) {
      return m_formationSurfaceProperty->interpolate ( i, j );
   } else {
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }
}

unsigned int DerivedProperties::FormationSurfaceOutputPropertyValue::getDepth () const {

   if ( m_formationSurfaceProperty != 0 ) {
      return 1;
   } else {
      return 0;
   }
   
}

void DerivedProperties::FormationSurfaceOutputPropertyValue::setMapName( const string& mapName ) {

   m_mapName = mapName;
}

const string & DerivedProperties::FormationSurfaceOutputPropertyValue::getMapName() const {

   return m_mapName;
}

bool DerivedProperties::FormationSurfaceOutputPropertyValue::isPrimary() const {

   if ( m_formationSurfaceProperty != 0 ) {
      return m_formationSurfaceProperty->isPrimary();
   } else {
      return false;
   }
}

const string &  DerivedProperties::FormationSurfaceOutputPropertyValue::getName() const {
   
   if ( m_formationSurfaceProperty != 0 ) {
      return  m_formationSurfaceProperty->getProperty()->getName(); 
   } else {
      return s_nullString;
   }
}

const DataModel::AbstractSurface* DerivedProperties::FormationSurfaceOutputPropertyValue::getSurface() const { 

   if( m_formationSurfaceProperty != 0 ) {
      return  m_formationSurfaceProperty->getSurface();
   } else {
      return 0;
   }
}
