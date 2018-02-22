#include "ReservoirOutputPropertyValue.h"

DerivedProperties::ReservoirOutputPropertyValue::ReservoirOutputPropertyValue ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                                const DataModel::AbstractProperty*          property,
                                                                                const DataModel::AbstractSnapshot*          snapshot,
                                                                                const DataModel::AbstractReservoir*         reservoir ) : OutputPropertyValue ( property ) {
   
   if ( property != 0 and snapshot != 0 and reservoir != 0 ) {
      m_reservoirProperty = propertyManager.getReservoirProperty ( property, snapshot, reservoir );
   }

}

double DerivedProperties::ReservoirOutputPropertyValue::getValue ( const double i, const double j, const double k ) const {

   if ( m_reservoirProperty != 0 ) {
      return m_reservoirProperty->interpolate ( i, j );
   } else {
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

const string & DerivedProperties::ReservoirOutputPropertyValue::getName() const {
   
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
   return 0;
}

bool DerivedProperties::ReservoirOutputPropertyValue::hasMap() const { 

   if( m_reservoirProperty != 0 ) {

      if( m_reservoirProperty->isPrimary() and m_reservoirProperty->getGridMap() == 0 ) {
         return false;
      }
      return true;
   }
   return false;
}

const DataModel::AbstractGrid* DerivedProperties::ReservoirOutputPropertyValue::getGrid () const {

   if( m_reservoirProperty != 0 ) {
      return m_reservoirProperty->getGrid();
   }
   return false;
}

bool DerivedProperties::ReservoirOutputPropertyValue::isPrimary() const { 

   if( m_reservoirProperty != 0 ) {
      return m_reservoirProperty->isPrimary();
   } else {
      return false;
   }
}
