#include "ThicknessCalculator.h"

#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"



OutputPropertyMap* allocateThicknessCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<ThicknessCalculator>( property, formation, surface, snapshot );
}

ThicknessCalculator::ThicknessCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_depthAbove = 0;
   m_depthBelow = 0;
   m_isCalculated = false;

}

bool ThicknessCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                             OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   double value;
   Interface::GridMap* thicknessMap;
   double undefinedValue;

   if ( not m_depthAbove->isCalculated ()) {

      if ( not m_depthAbove->calculate ()) {
         return false;
      }

   }

   if ( not m_depthBelow->isCalculated ()) {

      if ( not m_depthBelow->calculate ()) {
         return false;
      }

   }

   thicknessMap = propertyValues [ 0 ]->getGridMap ();
   thicknessMap->retrieveData ();
   undefinedValue = thicknessMap->getUndefinedValue ();

   for ( i = thicknessMap->firstI (); i <= thicknessMap->lastI (); ++i ) {

      for ( j = thicknessMap->firstJ (); j <= thicknessMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {
            value = (*m_depthBelow)( i, j ) - (*m_depthAbove)( i, j );
            thicknessMap->setValue ( i, j, value );
         } else {
            thicknessMap->setValue ( i, j, undefinedValue );
         }

      }

   }

   thicknessMap->restoreData ();
   m_isCalculated = true;
   return true;
}

void ThicknessCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   if ( FastcauldronSimulator::getInstance ().getCalculationMode () == HYDROSTATIC_HIGH_RES_DECOMPACTION_MODE or
        FastcauldronSimulator::getInstance ().getCalculationMode () == COUPLED_HIGH_RES_DECOMPACTION_MODE ) {
      properties.push_back ((CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "ThicknessHighRes", 
                                                                                                            m_snapshot, 0, 
                                                                                                            m_formation,
                                                                                                            0 )));
   } else {
      properties.push_back ((CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "Thickness", 
                                                                                                            m_snapshot, 0, 
                                                                                                            m_formation,
                                                                                                            0 )));
   }

}

bool ThicknessCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   const Interface::Surface* surfaceBelow;
   const Interface::Surface* surfaceAbove;

   surfaceAbove = m_formation->getTopSurface ();
   surfaceBelow = m_formation->getBottomSurface ();

   if ( FastcauldronSimulator::getInstance ().getCalculationMode () == HYDROSTATIC_HIGH_RES_DECOMPACTION_MODE or
        FastcauldronSimulator::getInstance ().getCalculationMode () == COUPLED_HIGH_RES_DECOMPACTION_MODE ) {
      m_depthAbove = PropertyManager::getInstance().findOutputPropertyMap ( "DepthHighRes", m_formation, surfaceAbove, m_snapshot );
      m_depthBelow = PropertyManager::getInstance().findOutputPropertyMap ( "DepthHighRes", m_formation, surfaceBelow, m_snapshot );
   } else {
      m_depthAbove = PropertyManager::getInstance().findOutputPropertyMap ( "Depth", m_formation, surfaceAbove, m_snapshot );
      m_depthBelow = PropertyManager::getInstance().findOutputPropertyMap ( "Depth", m_formation, surfaceBelow, m_snapshot );
   }
   if ( FastcauldronSimulator::getInstance ().getCauldron()->no2Doutput()) {
      propertyValues [ 0 ]->allowOutput( false );
   }

   return m_depthAbove != 0 and m_depthBelow != 0;
}
