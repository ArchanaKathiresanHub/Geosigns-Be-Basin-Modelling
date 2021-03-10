#include "BasementPropertyCalculator.h"

#include "PropertyManager.h"
#include "FastcauldronSimulator.h"


BasementPropertyCalculator::BasementPropertyCalculator ( const PropertyIdentifier propertyName,
                                                         LayerProps* formation,
                                                         const Interface::Surface* surface,
                                                         const Interface::Snapshot* snapshot ) :
   OutputPropertyMap ( propertyName, formation, surface, snapshot ),
   m_formation ( formation ), m_snapshot ( snapshot ), m_isCalculated ( false ) {

   if ( m_formation->isBasement() &&
        (( m_formation->isMantle () && ( propertyName == ALC_ORIGINAL_MANTLE || propertyName == ALC_MAX_MANTLE_DEPTH )) ||
         ( m_formation->isCrust() && propertyName != ALC_ORIGINAL_MANTLE && propertyName != ALC_MAX_MANTLE_DEPTH ))) {
      string outputPropertyName = propertyListName ( propertyName );

      CauldronPropertyValue* localValues = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( outputPropertyName,
                                                                                                                   snapshot, 0, formation, 0 ));
      if ( FastcauldronSimulator::getInstance ().isALC () && ( propertyName == TOP_BASALT_ALC || propertyName == MOHO_ALC )) {
         localValues->allowOutput ( true );
      }
      m_values.push_back ( localValues );

      if ( localValues->getProperty () != 0 ) {
         m_properties.push_back ((const Property*)(localValues->getProperty ()));
      }
      if ( m_formation->getTopSurface () == surface ) {
         // Number-of-nodes = number-of-elements + 1.
         // But C array indexing starts a 0, so 1 must be subtracted.
         m_kIndex = m_formation->getMaximumNumberOfElements();
      } else if ( m_formation->getBottomSurface () == surface ) {
         m_kIndex = 0;
      } else {
         assert ( false );
         // Error
      }
   }
}

bool BasementPropertyCalculator::calculate ()
{

   if ( m_formation->isBasement() ) {

      if ( not isCalculated () ) {
         m_isCalculated = calculateProperty();

         // if (not allowOutput)
         //    m_values [ 0 ] -> allowOutput(false);

         m_propertyMaps.push_back ( m_values [ 0 ]->getGridMap ());
      }

      if ( not m_propertyMaps [ 0 ]->retrieved ()) {
         m_propertyMaps [ 0 ]->retrieveGhostedData ();
      }

      return m_isCalculated;
   } else {
      return true;
   }
}

bool BasementPropertyCalculator::calculateProperty () {

   if ( m_formation->isBasement()  &&
        (( m_formation->isMantle () && ( m_propertyName == ALC_ORIGINAL_MANTLE || m_propertyName == ALC_MAX_MANTLE_DEPTH )) ||
         ( m_formation->isCrust() && m_propertyName != ALC_ORIGINAL_MANTLE && m_propertyName != ALC_MAX_MANTLE_DEPTH ))) {

      if ( m_isCalculated ) {
         return true;
      }

      if( !m_BasinModel->isALC()) {
         // error
         return true;
      }

      unsigned int i;
      unsigned int j;
      double value;
      double undefinedValue;
      double **propertyVector;

      Interface::GridMap* theMap = m_values [ 0 ]->getGridMap ();
      theMap->retrieveData ();

      // Gets the values of Petsc vector from layer class for property 'm_propertyName'.
      DMDAVecGetArray( *m_BasinModel->mapDA,
                       *m_formation->vectorList.VecArray [ m_propertyName ],
                       (void*) &propertyVector );

      undefinedValue = theMap->getUndefinedValue ();

      for ( i = theMap->firstI (); i <= theMap->lastI (); ++i ) {

         for ( j = theMap->firstJ (); j <= theMap->lastJ (); ++j ) {

            if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {
               value = propertyVector [j][i];
               theMap->setValue ( i, j, value );
            } else {
               theMap->setValue ( i, j, undefinedValue );
            }
         }
      }

      theMap->restoreData ();

      DMDAVecRestoreArray( *m_BasinModel->mapDA,
                           *m_formation->vectorList.VecArray [ m_propertyName ],
                           (void*) &propertyVector );

      m_isCalculated = true;
      return true;
   } else {
      return true;
   }
}

bool BasementPropertyCalculator::initialise ( ) {

   if ( m_formation->isBasement()  &&
        (( m_formation->isMantle () && ( m_propertyName == ALC_ORIGINAL_MANTLE || m_propertyName == ALC_MAX_MANTLE_DEPTH )) ||
         ( m_formation->isCrust() && m_propertyName != ALC_ORIGINAL_MANTLE && m_propertyName != ALC_MAX_MANTLE_DEPTH ))) {
      m_BasinModel = const_cast<AppCtx*>(FastcauldronSimulator::getInstance().getCauldron());
      // m_propertyName = m_values[0]->getName();

      return m_formation->isBasement();
   } else {
      return true;
   }
}

void BasementPropertyCalculator::finalise () {
   if ( m_formation->isBasement() &&
        (( m_formation->isMantle () && ( m_propertyName == ALC_ORIGINAL_MANTLE || m_propertyName == ALC_MAX_MANTLE_DEPTH )) ||
         ( m_formation->isCrust() && m_propertyName != ALC_ORIGINAL_MANTLE && m_propertyName != ALC_MAX_MANTLE_DEPTH ))) {
      unsigned int i;

      for ( i = 0; i < m_propertyMaps.size (); ++i ) {
         m_propertyMaps [ i ]->restoreData ( true, true );
      }
   }
}

bool BasementPropertyCalculator::isCalculated () const {
   return m_isCalculated;
}


OutputPropertyMap* allocateBasementPropertyCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new BasementPropertyCalculator ( property, formation, surface, snapshot );
}
