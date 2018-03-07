//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FormationSurfaceOutputPropertyValue.h"

using namespace AbstractDerivedProperties;

DerivedProperties::FormationSurfaceOutputPropertyValue::FormationSurfaceOutputPropertyValue (       AbstractPropertyManager&      propertyManager,
                                                                                              const DataModel::AbstractProperty*  property,
                                                                                              const DataModel::AbstractSnapshot*  snapshot,
                                                                                              const DataModel::AbstractFormation* formation,
                                                                                              const DataModel::AbstractSurface*   surface )  : OutputPropertyValue ( property ){

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

bool DerivedProperties::FormationSurfaceOutputPropertyValue::hasProperty() const {
   return m_formationSurfaceProperty != nullptr;
}
