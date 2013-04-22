#include "FCTCorrectionCalculator.h"
#include "timefilter.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"

OutputPropertyMap* allocateFCTCorrectionCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<FCTCorrectionCalculator>( property, formation, surface, snapshot );
}

FCTCorrectionCalculator::FCTCorrectionCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_isCalculated = false;

}

bool FCTCorrectionCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                 OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   Interface::GridMap* fctCorrectionMap;
   double undefinedValue;
   double **propertyVector;
   bool isSediment = m_formation->kind () == Interface::SEDIMENT_FORMATION;

   if ( isSediment ) {
      DMDAVecGetArray ( *FastcauldronSimulator::getInstance ().getCauldron ()->mapDA,
                        m_formation->FCTCorrection,
                        &propertyVector );
   }

   fctCorrectionMap = propertyValues [ 0 ]->getGridMap ();
   fctCorrectionMap->retrieveData ();
   undefinedValue = fctCorrectionMap->getUndefinedValue ();

   for ( i = fctCorrectionMap->firstI (); i <= fctCorrectionMap->lastI (); ++i ) {

      for ( j = fctCorrectionMap->firstJ (); j <= fctCorrectionMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j ) and isSediment ) {
            fctCorrectionMap->setValue ( i, j, propertyVector [ j ][ i ] );
         } else {
            fctCorrectionMap->setValue ( i, j, undefinedValue );
         }

      }

   }

   fctCorrectionMap->restoreData ();

   if ( isSediment ) {
      DMDAVecRestoreArray ( *FastcauldronSimulator::getInstance ().getCauldron ()->mapDA,
                            m_formation->FCTCorrection,
                            &propertyVector );
   }

   m_isCalculated = true;
   return true;
}

void FCTCorrectionCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "FCTCorrection", 
                                                                                                         m_snapshot, 0, 
                                                                                                         m_formation,
                                                                                                         0 )));

}

bool FCTCorrectionCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {
   return true;
}
