//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "SonicCalculator.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"

#include "RunParameters.h"

OutputPropertyMap* allocateSonicCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<SonicCalculator>( property, formation, surface, snapshot );
}

OutputPropertyMap* allocateSonicVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<SonicVolumeCalculator>( property, formation, snapshot );
}

SonicCalculator::SonicCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_velocity = 0;
   m_isCalculated = false;

}

bool SonicCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties,
                                         OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   Interface::GridMap* sonicMap;
   double undefinedValue;

   if ( not m_velocity->isCalculated ()) {

      if ( not m_velocity->calculate ()) {
         return false;
      }

   }

   sonicMap = propertyValues [ 0 ]->getGridMap ();
   sonicMap->retrieveData ();
   undefinedValue = sonicMap->getUndefinedValue ();

   for ( i = sonicMap->firstI (); i <= sonicMap->lastI (); ++i ) {

      for ( j = sonicMap->firstJ (); j <= sonicMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {
            sonicMap->setValue ( i, j, 1.0e6 / (*m_velocity)( i, j ));
         } else {
            sonicMap->setValue ( i, j, undefinedValue );
         }

      }

   }

   sonicMap->restoreData ();
   m_isCalculated = true;
   return true;
}

void SonicCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "SonicVec2",
                                                                                                         m_snapshot, 0,
                                                                                                         m_formation,
                                                                                                         m_surface )));

}

bool SonicCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   if (FastcauldronSimulator::getInstance().getCauldron()->no2Doutput()) {
      propertyValues[0]->allowOutput( false );
   }

   m_velocity = PropertyManager::getInstance().findOutputPropertyMap ( "Velocity", m_formation, m_surface, m_snapshot );

   return m_velocity != 0;
}


SonicVolumeCalculator::SonicVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_velocity = 0;
   m_isCalculated = false;

}

bool SonicVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties,
                                               OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   unsigned int k;
   double undefinedValue;
   Interface::GridMap* sonicMap;

   if ( not m_velocity->isCalculated ()) {

      if ( not m_velocity->calculate ()) {
         return false;
      }

   }

   sonicMap = propertyValues [ 0 ]->getGridMap ();
   sonicMap->retrieveData ();
   undefinedValue = sonicMap->getUndefinedValue ();

   for ( i = sonicMap->firstI (); i <= sonicMap->lastI (); ++i ) {

      for ( j = sonicMap->firstJ (); j <= sonicMap->lastJ (); ++j ) {

         for ( k = sonicMap->firstK (); k <= sonicMap->lastK (); ++k ) {

            if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {
               sonicMap->setValue ( i, j, k, 1.0e6 / m_velocity->getVolumeValue ( i, j, k ));
            } else {
               sonicMap->setValue ( i, j, k, undefinedValue );
            }

         }

      }

   }

   sonicMap->restoreData ();
   m_isCalculated = true;
   return true;
}

void SonicVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "SonicVec2",
                                                                                                            m_snapshot, 0,
                                                                                                            m_formation,
                                                                                                            m_formation->getMaximumNumberOfElements () + 1 )));

}

bool SonicVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   m_velocity = PropertyManager::getInstance().findOutputPropertyVolume ( "Velocity", m_formation, m_snapshot );

   return m_velocity != 0;
}
