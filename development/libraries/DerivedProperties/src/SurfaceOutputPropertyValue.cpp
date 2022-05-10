//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SurfaceOutputPropertyValue.h"

using namespace AbstractDerivedProperties;

DerivedProperties::SurfaceOutputPropertyValue::SurfaceOutputPropertyValue (       AbstractPropertyManager&     propertyManager,
                                                                            const DataModel::AbstractProperty* property,
                                                                            const DataModel::AbstractSnapshot* snapshot,
                                                                            const DataModel::AbstractSurface*  surface ) : OutputPropertyValue ( property ) {

   if ( property != 0 and snapshot != 0 and surface != 0 ) {
      m_surfaceProperty = propertyManager.getSurfaceProperty ( property, snapshot, surface );
   }
   m_mapName = "";
}

DerivedProperties::SurfaceOutputPropertyValue::SurfaceOutputPropertyValue ( SurfacePropertyPtr& surfaceProperty ) :
   OutputPropertyValue ( surfaceProperty != 0 ?  surfaceProperty->getProperty() : 0 ) {

   m_mapName = "";
}

void DerivedProperties::SurfaceOutputPropertyValue::setMapName( const std::string& mapName ) {

   m_mapName = mapName;
}

const std::string & DerivedProperties::SurfaceOutputPropertyValue::getMapName() const {

   return m_mapName;
}

bool DerivedProperties::SurfaceOutputPropertyValue::isPrimary() const {

   if ( m_surfaceProperty != 0 ) {
      return m_surfaceProperty->isPrimary();
   } else {
      return false;
   }
}

double DerivedProperties::SurfaceOutputPropertyValue::getValue ( const double i, const double j, const double k ) const 
{
   if ( m_surfaceProperty != 0 ) 
   {
       auto value = m_surfaceProperty->interpolate(i, j);
       auto isNaN = OutputPropertyValue::checkForNANPropertyValue(value);
       value = isNaN ? DataAccess::Interface::DefaultUndefinedMapValue : value;
       return value;
   } 
   else 
   {
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

const DataModel::AbstractGrid* DerivedProperties::SurfaceOutputPropertyValue::getGrid () const {

   if( m_surfaceProperty != 0 ) {
      return m_surfaceProperty->getGrid();
   }
   return nullptr;
}

const std::string & DerivedProperties::SurfaceOutputPropertyValue::getName() const {

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
      return nullptr;
   }
}

bool DerivedProperties::SurfaceOutputPropertyValue::hasProperty() const {
   return m_surfaceProperty != nullptr;
}
