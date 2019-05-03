//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "VitriniteReflectanceCalculator.h"
#include "timefilter.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"

// utilities library
#include "ConstantsNumerical.h"
using Utilities::Numerical::IbsNoDataValue;

OutputPropertyMap* allocateVitriniteReflectanceCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<VitriniteReflectanceCalculator>( property, formation, surface, snapshot );
}

OutputPropertyMap* allocateVitriniteReflectanceVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<VitriniteReflectanceVolumeCalculator>( property, formation, snapshot );
}

VitriniteReflectanceCalculator::VitriniteReflectanceCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   if ( m_formation->getTopSurface () == surface ) {
      // Number-of-nodes = number-of-elements + 1.
      // But C array indexing starts a 0, so 1 must be subtracted.
      m_kIndex = m_formation->getMaximumNumberOfElements ();
   } else if ( m_formation->getBottomSurface () == surface ) {
      m_kIndex = 0;
   } else {
      assert ( false );
      // Error
   }

   m_isCalculated = false;

}

bool VitriniteReflectanceCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties,
                                                        OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   double age;
   double solidThickness;
   unsigned int i;
   unsigned int j;
   int k;
   int usableK;
   Interface::GridMap* vitriniteReflectanceMap;
   double undefinedValue;
   double ***propertyVector;
   bool isSediment = m_formation->isSediment();

   if ( m_snapshot != 0 ) {
      age = m_snapshot->getTime ();
   } else {
      // If no snap-shot is defined then present-day vr is saved.
      age = 0.0;
   }

   if ( isSediment ) {
      DMDAVecGetArray ( m_formation->layerDA,
                        m_formation->Vre,
                        &propertyVector );
   }

   vitriniteReflectanceMap = propertyValues [ 0 ]->getGridMap ();
   vitriniteReflectanceMap->retrieveData ();
   undefinedValue = vitriniteReflectanceMap->getUndefinedValue ();

   for ( i = vitriniteReflectanceMap->firstI (); i <= vitriniteReflectanceMap->lastI (); ++i ) {

      for ( j = vitriniteReflectanceMap->firstJ (); j <= vitriniteReflectanceMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j ) and isSediment ) {
            usableK = 0;

            if ( m_formation->getTopSurface () == m_surface ) {

               // Find top-most active segment.
               for ( k = int ( m_formation->getMaximumNumberOfElements ()) - 1; k >= 0; --k ) {
                 solidThickness = m_formation->getSolidThickness ( i, j, k, age );

                  if ( solidThickness != IbsNoDataValue and solidThickness > 0 ) {

                     // The node at the top of the top-most active segment is sought.
                     usableK = k + 1;
                     break;
                  }

               }

            }
            vitriniteReflectanceMap->setValue ( i, j, propertyVector [ usableK ][ j ][ i ] );
         } else {
            vitriniteReflectanceMap->setValue ( i, j, undefinedValue );
         }

      }

   }

   vitriniteReflectanceMap->restoreData ();

   if ( isSediment ) {
      DMDAVecRestoreArray ( m_formation->layerDA,
                            m_formation->Vre,
                            &propertyVector );
   }

   m_isCalculated = true;
   return true;
}

void VitriniteReflectanceCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "VrVec2",
                                                                                                         m_snapshot, 0,
                                                                                                         m_formation,
                                                                                                         m_surface )));

}

bool VitriniteReflectanceCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   if ( FastcauldronSimulator::getInstance ().getCauldron()->no2Doutput()) {
      propertyValues [ 0 ]->allowOutput ( false );
   }

   return true;
}


VitriniteReflectanceVolumeCalculator::VitriniteReflectanceVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_isCalculated = false;

}

bool VitriniteReflectanceVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties,
                                                              OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }
   if( !m_formation->Vre ) {
      return false;
   }

   unsigned int i;
   unsigned int j;
   unsigned int k;
   Interface::GridMap* vitriniteReflectanceMap;
   double ***propertyVector;
   double undefinedValue;
   bool isSediment = m_formation->isSediment();

   if ( isSediment ) {
      DMDAVecGetArray ( m_formation->layerDA,
                        m_formation->Vre,
                        &propertyVector );
   }

   vitriniteReflectanceMap = propertyValues [ 0 ]->getGridMap ();
   vitriniteReflectanceMap->retrieveData ();
   undefinedValue = vitriniteReflectanceMap->getUndefinedValue ();

   for ( i = vitriniteReflectanceMap->firstI (); i <= vitriniteReflectanceMap->lastI (); ++i ) {

      for ( j = vitriniteReflectanceMap->firstJ (); j <= vitriniteReflectanceMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j ) and isSediment ) {

            for ( k = vitriniteReflectanceMap->firstK (); k <= vitriniteReflectanceMap->lastK (); ++k ) {
               vitriniteReflectanceMap->setValue ( i, j, k, propertyVector [ k ][ j ][ i ] );
            }

         } else {

            for ( k = vitriniteReflectanceMap->firstK (); k <= vitriniteReflectanceMap->lastK (); ++k ) {
               vitriniteReflectanceMap->setValue ( i, j, k, undefinedValue );
            }

         }

      }

   }

   vitriniteReflectanceMap->restoreData ();

   if ( isSediment ) {
      DMDAVecRestoreArray ( m_formation->layerDA,
                            m_formation->Vre,
                            &propertyVector );
   }

   m_isCalculated = true;
   return true;
}

void VitriniteReflectanceVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "Vr",
                                                                                                            m_snapshot, 0,
                                                                                                            m_formation,
                                                                                                            m_formation->getMaximumNumberOfElements () + 1 )));

}

bool VitriniteReflectanceVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {
   return true;
}
