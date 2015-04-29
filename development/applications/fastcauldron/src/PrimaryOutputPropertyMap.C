#include "PrimaryOutputPropertyMap.h"
#include "FastcauldronSimulator.h"
#include "timefilter.h"
#include "propinterface.h"

#include "Interface/RunParameters.h"

PrimaryOutputPropertyMap::PrimaryOutputPropertyMap ( const PropertyList         propertyName,
                                                           LayerProps*          formation,
                                                     const Interface::Surface*  surface,
                                                     const Interface::Snapshot* snapshot ) :
   OutputPropertyMap ( propertyName, formation, surface, snapshot ), m_isCalculated ( false ) {

   string outputPropertyName;

   // The property-name is extended here to include high-res decompaction.
   // Since hr-decompaction only outputs primary properties then the addition is required only here.
   // If at some future point further output (derived properties) is required then this must be re-visited.
   if ( FastcauldronSimulator::getInstance ().getCalculationMode () == HYDROSTATIC_HIGH_RES_DECOMPACTION_MODE or
        FastcauldronSimulator::getInstance ().getCalculationMode () == COUPLED_HIGH_RES_DECOMPACTION_MODE ) {
      outputPropertyName = propertyListName ( m_propertyName ) + "HighRes";
   } else {
      outputPropertyName = propertyListName ( m_propertyName );
   }

   LayerProps* localFormation = ( m_propertyName == CHEMICAL_COMPACTION ? formation : 0 );

   PropertyValue* localValues = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( outputPropertyName,
                                                                                                                snapshot, 0, localFormation,
                                                                                                                surface ));
   if( m_propertyName == CHEMICAL_COMPACTION && ! m_formation->hasChemicalCompaction () ) {
      localValues->allowOutput( false );
   }

   if ( FastcauldronSimulator::getInstance ().getCauldron()->no2Doutput()) {
      localValues->allowOutput( false );
   }

   m_values.push_back ( localValues );

   if ( localValues->getProperty () != 0 ) {
      m_properties.push_back ((const Property*)(localValues->getProperty ()));
   }

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

}

bool PrimaryOutputPropertyMap::isCalculated () const {
   return m_isCalculated;
}

bool PrimaryOutputPropertyMap::calculate () {

   if ( isCalculated ()) {
      return m_propertyMaps [ 0 ]->retrieved ();
   } 

   assert ( m_propertyMaps.size () == 1 );
   assert ( m_propertyMaps [ 0 ] != 0 );

   // The map has to be retrieved.
   m_propertyMaps [ 0 ]->retrieveGhostedData ();

   m_isCalculated = true;
   return m_propertyMaps [ 0 ]->retrieved ();
}

bool PrimaryOutputPropertyMap::initialise () {

   unsigned int i;
   unsigned int j;
   double value;
   Interface::GridMap* propertyMap;
   double ***propertyVector;

   propertyMap = m_values [ 0 ]->getGridMap ();
   propertyMap->retrieveData ();
   m_propertyMaps.push_back ( propertyMap );

   DMDAVecGetArray( m_formation->layerDA,
                    *m_formation->vectorList.VecArray [ m_propertyName ],
                    &propertyVector );

   // Chemical compaction is an exception, in that it will not be output directly.
   bool propertyIsDefined = ( m_propertyName == CHEMICAL_COMPACTION ? m_formation->hasChemicalCompaction () and 
                                                                      FastcauldronSimulator::getInstance ().getRunParameters ()->getChemicalCompaction () : 
                                                                      true );

   for ( i = FastcauldronSimulator::getInstance ().firstI (); i <= FastcauldronSimulator::getInstance ().lastI (); ++i ) {

      for ( j = FastcauldronSimulator::getInstance ().firstJ (); j <= FastcauldronSimulator::getInstance ().lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j ) and propertyIsDefined ) {
            value = propertyVector [ m_kIndex ][ j ][ i ];
         } else {
            value = CAULDRONIBSNULLVALUE;
         }

         propertyMap->setValue ( i, j, value );
      }

   }

   // The map has to be restored in case of any communicating of data is required.
   propertyMap->restoreData ();

   DMDAVecRestoreArray ( m_formation->layerDA,
                         *m_formation->vectorList.VecArray [ m_propertyName ],
                         &propertyVector );

  return true;
}

void PrimaryOutputPropertyMap::finalise () {
   assert ( m_propertyMaps.size () == 1 );
   assert ( m_propertyMaps [ 0 ] != 0 );

   if ( m_propertyMaps [ 0 ]->retrieved ()) {
      m_propertyMaps [ 0 ]->restoreData ( true, true );
   }

}

OutputPropertyMap* allocatePrimaryPropertyCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new PrimaryOutputPropertyMap ( property, formation, surface, snapshot );
}
