#include "ThicknessErrorMapCalculator.h"
#include "timefilter.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"

OutputPropertyMap* allocateThicknessErrorMapCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<ThicknessErrorMapCalculator>( property, formation, surface, snapshot );
}

ThicknessErrorMapCalculator::ThicknessErrorMapCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_isCalculated = false;

}

bool ThicknessErrorMapCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties,
                                                     OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   Interface::GridMap* thicknessErrorMap;
   double undefinedValue;
   double **propertyVector;
   bool isSediment = m_formation->isSediment();

   if ( isSediment ) {
      DMDAVecGetArray ( *FastcauldronSimulator::getInstance ().getCauldron ()->mapDA,
                        m_formation->Thickness_Error,
                        &propertyVector );
   }

   thicknessErrorMap = propertyValues [ 0 ]->getGridMap ();
   thicknessErrorMap->retrieveData ();
   undefinedValue = thicknessErrorMap->getUndefinedValue ();

   for ( i = thicknessErrorMap->firstI (); i <= thicknessErrorMap->lastI (); ++i ) {

      for ( j = thicknessErrorMap->firstJ (); j <= thicknessErrorMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j ) and isSediment ) {
            thicknessErrorMap->setValue ( i, j, propertyVector [ j ][ i ] );
         } else {
            thicknessErrorMap->setValue ( i, j, undefinedValue );
         }

      }

   }

   thicknessErrorMap->restoreData ();

   if ( isSediment ) {
      DMDAVecRestoreArray ( *FastcauldronSimulator::getInstance ().getCauldron ()->mapDA,
                            m_formation->Thickness_Error,
                            &propertyVector );
   }

   m_isCalculated = true;
   return true;
}

void ThicknessErrorMapCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "ThicknessError",
                                                                                                         m_snapshot, 0,
                                                                                                         m_formation,
                                                                                                         0 )));

}

bool ThicknessErrorMapCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {
   return true;
}
