#include "MaxVesCalculator.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"

#include "Interface/RunParameters.h"


OutputPropertyMap* allocateMaxVesCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<MaxVesCalculator>( property, formation, surface, snapshot );
}

MaxVesCalculator::MaxVesCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_isCalculated = false;

}

bool MaxVesCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                          OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   double value;
   double undefinedValue;

   Interface::GridMap* maxVesMap;
   double ***maxVesVector;

   maxVesMap = propertyValues [ 0 ]->getGridMap ();
   maxVesMap->retrieveData ();
   undefinedValue = maxVesMap->getUndefinedValue ();

   DAVecGetArray( m_formation->layerDA,
                  *m_formation->vectorList.VecArray [ MAXVES ],
		  &maxVesVector );

   for ( i = maxVesMap->firstI (); i <= maxVesMap->lastI (); ++i ) {

      for ( j = maxVesMap->firstJ (); j <= maxVesMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {
            value = maxVesVector [ m_kIndex ][ j ][ i ];
            maxVesMap->setValue ( i, j, value );
         } else {
            maxVesMap->setValue ( i, j, undefinedValue );
         }

      }

   }

   maxVesMap->restoreData ();

   DAVecRestoreArray ( m_formation->layerDA,
                       *m_formation->vectorList.VecArray [ MAXVES ],
                       &maxVesVector );

   m_isCalculated = true;


   return true;
}

void MaxVesCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "MaxVes",
                                                                                                         m_snapshot, 0, 
                                                                                                         m_formation,
                                                                                                         m_surface )));

}

bool MaxVesCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   if ( m_formation->getTopSurface () == m_surface ) {
      // Number-of-nodes = number-of-elements + 1.
      // But C array indexing starts a 0, so 1 must be subtracted.
      m_kIndex = m_formation->getMaximumNumberOfElements ();
   } else if ( m_formation->getBottomSurface () == m_surface ) {
      m_kIndex = 0;
   } else {
      assert ( false );
      // Error
   }

   return true;
}

