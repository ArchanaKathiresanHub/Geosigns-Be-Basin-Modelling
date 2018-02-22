#include "SurfaceOutputPropertyValue.h"

DerivedProperties::SurfaceOutputPropertyValue::SurfaceOutputPropertyValue ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                         const DataModel::AbstractProperty*          property,
                                                         const DataModel::AbstractSnapshot*          snapshot,
                                                         const DataModel::AbstractSurface*           surface ) : OutputPropertyValue ( property ) {

   if ( property != 0 and snapshot != 0 and surface != 0 ) {
      m_surfaceProperty = propertyManager.getSurfaceProperty ( property, snapshot, surface );
   }
   m_mapName = "";
}

DerivedProperties::SurfaceOutputPropertyValue::SurfaceOutputPropertyValue ( DerivedProperties::SurfacePropertyPtr& surfaceProperty ) :
   OutputPropertyValue ( surfaceProperty != 0 ?  surfaceProperty->getProperty() : 0 ) {

   m_mapName = "";
}

void DerivedProperties::SurfaceOutputPropertyValue::setMapName( const string& mapName ) {

   m_mapName = mapName;
}

const string & DerivedProperties::SurfaceOutputPropertyValue::getMapName() const {

   return m_mapName;
}

bool DerivedProperties::SurfaceOutputPropertyValue::isPrimary() const {

   if ( m_surfaceProperty != 0 ) {
      return m_surfaceProperty->isPrimary();
   } else {
      return false;
   }
}

double DerivedProperties::SurfaceOutputPropertyValue::getValue ( const double i, const double j, const double k ) const {

   if ( m_surfaceProperty != 0 ) {
      return m_surfaceProperty->interpolate ( i, j );
   } else {
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }
}

void  DerivedProperties::SurfaceOutputPropertyValue::restoreData () const {
   if ( m_surfaceProperty != 0 ) {
      m_surfaceProperty->restoreData();
   }
}

void  DerivedProperties::SurfaceOutputPropertyValue::retrieveData () const {
   if ( m_surfaceProperty != 0 ) {
      m_surfaceProperty->retrieveData();
   }
}

unsigned int DerivedProperties::SurfaceOutputPropertyValue::getDepth () const {

   if ( m_surfaceProperty != 0 ) {
      return 1;
   } else {
      return 0;
   }  
}


bool DerivedProperties::SurfaceOutputPropertyValue::hasMap() const { 

   if( m_surfaceProperty != 0 ) {

      if( m_surfaceProperty->isPrimary() and m_surfaceProperty->getGridMap() == 0 ) {
         return false;
      }
      return true;
   }
   return false;
}

const DataModel::AbstractGrid* DerivedProperties::SurfaceOutputPropertyValue::getGrid () const {

   if( m_surfaceProperty != 0 ) {
      return m_surfaceProperty->getGrid();
   }
   return false;
}

const string & DerivedProperties::SurfaceOutputPropertyValue::getName() const {
   
   if( m_surfaceProperty != 0 ) {
      return m_surfaceProperty->getProperty()->getName();
   } else {
      return s_nullString;
   }
}

const DataModel::AbstractSurface* DerivedProperties::SurfaceOutputPropertyValue::getSurface() const { 

   if( m_surfaceProperty != 0 ) {
      return  m_surfaceProperty->getSurface();
   } else {
      return 0;
   }
}
