//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ReservoirOutputPropertyValue.h"

DerivedProperties::ReservoirOutputPropertyValue::ReservoirOutputPropertyValue ( AbstractDerivedProperties::AbstractPropertyManager& propertyManager,
                                                                                const DataModel::AbstractProperty*                  property,
                                                                                const DataModel::AbstractSnapshot*                  snapshot,
                                                                                const DataModel::AbstractReservoir*                 reservoir ) : OutputPropertyValue ( property ) {

   if ( property != 0 and snapshot != 0 and reservoir != 0 ) {
      m_reservoirProperty = propertyManager.getReservoirProperty ( property, snapshot, reservoir );
   }

}

double DerivedProperties::ReservoirOutputPropertyValue::getValue ( const double i, const double j, const double k ) const 
{
   if ( m_reservoirProperty != 0 ) 
   {
       auto value = m_reservoirProperty->interpolate(i, j);
       auto isNaN = OutputPropertyValue::checkForNANPropertyValue(value);
       value = isNaN ? DataAccess::Interface::DefaultUndefinedMapValue : value;
       return value;
   } 
   else 
   {
       return DataAccess::Interface::DefaultUndefinedMapValue;
   }
}

void DerivedProperties::ReservoirOutputPropertyValue::restoreData () const {
  if ( m_reservoirProperty != 0 ) {
     m_reservoirProperty->restoreData();
  }
}

void DerivedProperties::ReservoirOutputPropertyValue::retrieveData () const {
  if ( m_reservoirProperty != 0 ) {
     m_reservoirProperty->retrieveData();
  }
}

unsigned int DerivedProperties::ReservoirOutputPropertyValue::getDepth () const {

   if ( m_reservoirProperty != 0 ) {
      return 1;
   } else {
      return 0;
   }
}

const std::string & DerivedProperties::ReservoirOutputPropertyValue::getName() const {

   if ( m_reservoirProperty != 0 ) {
      return m_reservoirProperty->getProperty()->getName();
   } else {
      return s_nullString;
   }
}

const DataModel::AbstractReservoir*  DerivedProperties::ReservoirOutputPropertyValue::getReservoir() const {

   if ( m_reservoirProperty != 0 ) {
      return m_reservoirProperty->getReservoir();
   }
   return nullptr;
}

const DataModel::AbstractGrid* DerivedProperties::ReservoirOutputPropertyValue::getGrid () const {

   if( m_reservoirProperty != 0 ) {
      return m_reservoirProperty->getGrid();
   }
   return nullptr;
}

bool DerivedProperties::ReservoirOutputPropertyValue::isPrimary() const {

   if( m_reservoirProperty != 0 ) {
      return m_reservoirProperty->isPrimary();
   } else {
      return false;
   }
}

bool DerivedProperties::ReservoirOutputPropertyValue::hasProperty() const {
   return m_reservoirProperty != nullptr;
}
