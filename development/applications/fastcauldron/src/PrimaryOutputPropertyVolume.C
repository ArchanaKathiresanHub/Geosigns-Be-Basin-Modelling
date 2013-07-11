#include "PrimaryOutputPropertyVolume.h"
#include "FastcauldronSimulator.h"
#include "timefilter.h"

#include "Interface/RunParameters.h"

OutputPropertyMap* allocatePrimaryPropertyVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new PrimaryOutputPropertyVolume ( property, formation, snapshot );
}


PrimaryOutputPropertyVolume::PrimaryOutputPropertyVolume ( const PropertyList         propertyName,
                                                                 LayerProps*          formation,
                                                           const Interface::Snapshot* snapshot ) :
   OutputPropertyMap ( propertyName, formation, 0, snapshot ), m_isCalculated ( false ) {

   PropertyValue* localValues = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( propertyListName ( m_propertyName ),
                                                                                                                   snapshot, 0, m_formation,
                                                                                                                   m_formation->getMaximumNumberOfElements () + 1 ));

   m_values.push_back ( localValues );

   if ( localValues->getProperty () != 0 ) {
      m_properties.push_back ((const Property*)(localValues->getProperty ()));
   }

}

bool PrimaryOutputPropertyVolume::isCalculated () const {
   return m_isCalculated;
}

bool PrimaryOutputPropertyVolume::calculate () {

   if ( isCalculated ()) {
      return m_propertyMaps [ 0 ]->retrieved ();
   } 

   assert ( m_propertyMaps.size () == 1 );
   assert ( m_propertyMaps [ 0 ] != 0 );

   // The map has to be retrieved.
   m_propertyMaps [ 0 ]->retrieveGhostedData ();

   return m_propertyMaps [ 0 ]->retrieved ();
}

bool PrimaryOutputPropertyVolume::initialise () {

   unsigned int i;
   unsigned int j;
   unsigned int k;
   double value;
   Interface::GridMap* propertyMap;
   double ***propertyVector;

   propertyMap = m_values [ 0 ]->getGridMap ();
   propertyMap->retrieveData ();
   m_propertyMaps.push_back ( propertyMap );

   DMDAVecGetArray( m_formation->layerDA,
                    *m_formation->vectorList.VecArray [ m_propertyName ],
                    &propertyVector );

   // Chemical compaction is special in that it will not be output directly.
   bool propertyIsDefined = ( m_propertyName == CHEMICAL_COMPACTION ? m_formation->hasChemicalCompaction () and 
                                                                      FastcauldronSimulator::getInstance ().getRunParameters ()->getChemicalCompaction () : 
                                                                      true );

   for ( i = propertyMap->firstI (); i <= propertyMap->lastI (); ++i ) {

      for ( j = propertyMap->firstJ (); j <= propertyMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j ) and propertyIsDefined ) {

            for ( k = propertyMap->firstK (); k <= propertyMap->lastK (); ++k ) {
               propertyMap->setValue ( i, j, k, propertyVector [ k ][ j ][ i ] );
            }

         } else {

            for ( k = propertyMap->firstK (); k <= propertyMap->lastK (); ++k ) {
               propertyMap->setValue ( i, j, k, CAULDRONIBSNULLVALUE );
            }

         }

      }

   }

   // The map has to be restored in case of any communicating of data is required.
   propertyMap->restoreData ();

   DMDAVecRestoreArray ( m_formation->layerDA,
                         *m_formation->vectorList.VecArray [ m_propertyName ],
                         &propertyVector );

  return true;
}

void PrimaryOutputPropertyVolume::finalise () {
   assert ( m_propertyMaps.size () == 1 );
   assert ( m_propertyMaps [ 0 ] != 0 );

   if ( m_propertyMaps [ 0 ]->retrieved ()) {
      m_propertyMaps [ 0 ]->restoreData ( true, true );
   }

}

bool PrimaryOutputPropertyVolume::matches ( const LayerProps*          formation,
                                            const Interface::Surface*  surface,
                                            const Interface::Snapshot* snapshot,
                                            const std::string&         propertyName ) const {

   bool propertyMatches = false;
   unsigned int i;

   for ( i = 0; i < m_properties.size (); ++i ) {
      propertyMatches = propertyMatches or m_properties [ i ]->getName () == propertyName;
   }

   // Since Primary properties are scalar values and for surfaces, the formation is not included in the comparison.
   return propertyMatches and m_surface == surface and m_snapshot == snapshot;
}
