#include "SmectiteIlliteAdapter.h"
#include "timefilter.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"

OutputPropertyMap* allocateSmectiteIlliteAdapter ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<SmectiteIlliteAdapter>( property, formation, surface, snapshot );
}

OutputPropertyMap* allocateSmectiteIlliteVolumeAdapter ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<SmectiteIlliteVolumeAdapter>( property, formation, snapshot );
}

SmectiteIlliteAdapter::SmectiteIlliteAdapter ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
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

bool SmectiteIlliteAdapter::operator ()( const OutputPropertyMap::OutputPropertyList& properties,
                                                        OutputPropertyMap::PropertyValueList&  propertyValues ) {


   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   Interface::GridMap* smectiteIlliteMap;
   double undefinedValue;
   double ***propertyVector;
   bool isSediment = m_formation->isSediment();

   if ( isSediment ) {
      DMDAVecGetArray ( m_formation->layerDA,
                        m_formation->m_IlliteFraction,
                        &propertyVector );
   }

   smectiteIlliteMap = propertyValues [ 0 ]->getGridMap ();
   smectiteIlliteMap->retrieveData ();
   undefinedValue = smectiteIlliteMap->getUndefinedValue ();

   for ( i = smectiteIlliteMap->firstI (); i <= smectiteIlliteMap->lastI (); ++i ) {

      for ( j = smectiteIlliteMap->firstJ (); j <= smectiteIlliteMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j ) and isSediment ) {
            smectiteIlliteMap->setValue ( i, j, propertyVector [ m_kIndex ][ j ][ i ] );
         } else {
            smectiteIlliteMap->setValue ( i, j, undefinedValue );
         }

      }

   }

   smectiteIlliteMap->restoreData ();

   if ( isSediment ) {
      DMDAVecRestoreArray ( m_formation->layerDA,
                            m_formation->m_IlliteFraction,
                            &propertyVector );
   }

   m_isCalculated = true;
   return true;
}

void SmectiteIlliteAdapter::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "IlliteFraction",
                                                                                                         m_snapshot, 0,
                                                                                                         0,
                                                                                                         m_surface )));

}

bool SmectiteIlliteAdapter::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {
   return true;
}


SmectiteIlliteVolumeAdapter::SmectiteIlliteVolumeAdapter ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_isCalculated = false;

}

bool SmectiteIlliteVolumeAdapter::operator ()( const OutputPropertyMap::OutputPropertyList& properties,
                                                              OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   unsigned int k;
   Interface::GridMap* smectiteIlliteMap;
   double ***propertyVector;
   double undefinedValue;
   bool isSediment = m_formation->isSediment();

   if ( isSediment ) {
      DMDAVecGetArray ( m_formation->layerDA,
                        m_formation->m_IlliteFraction,
                        &propertyVector );
   }

   smectiteIlliteMap = propertyValues [ 0 ]->getGridMap ();
   smectiteIlliteMap->retrieveData ();
   undefinedValue = smectiteIlliteMap->getUndefinedValue ();

   for ( i = smectiteIlliteMap->firstI (); i <= smectiteIlliteMap->lastI (); ++i ) {

      for ( j = smectiteIlliteMap->firstJ (); j <= smectiteIlliteMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j ) and isSediment ) {

            for ( k = smectiteIlliteMap->firstK (); k <= smectiteIlliteMap->lastK (); ++k ) {
               smectiteIlliteMap->setValue ( i, j, k, propertyVector [ k ][ j ][ i ] );
            }

         } else {

            for ( k = smectiteIlliteMap->firstK (); k <= smectiteIlliteMap->lastK (); ++k ) {
               smectiteIlliteMap->setValue ( i, j, k, undefinedValue );
            }

         }

      }

   }

   smectiteIlliteMap->restoreData ();

   if ( isSediment ) {
      DMDAVecRestoreArray ( m_formation->layerDA,
                            m_formation->m_IlliteFraction,
                            &propertyVector );
   }

   m_isCalculated = true;
   return true;
}

void SmectiteIlliteVolumeAdapter::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "IlliteFraction",
                                                                                                            m_snapshot, 0,
                                                                                                            m_formation,
                                                                                                            m_formation->getMaximumNumberOfElements () + 1 )));

}

bool SmectiteIlliteVolumeAdapter::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {
   return true;
}
