#include "mpi.h"
#include "RequiredGenex5PropertyCalculator.h"


#include "Interface/PropertyValue.h"
#include "Interface/GridMap.h"

#include "SourceRock.h"

#include "DerivedOutputPropertyMap.h"


RequiredGenex5PropertyCalculator::RequiredGenex5PropertyCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_isCalculated = false;
   m_sourceRock = 0;

   int id;

   for ( id = 0; id < CBMGenerics::ComponentManager::NumberOfSpecies; ++id ){
      m_hasSpeciesMap [ id ] = false;
   }

}


bool RequiredGenex5PropertyCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                    OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   double undefinedValue;
   int id;
   int mapCount;


#if 0
   Interface::GridMap* c1Map;
   Interface::GridMap* c2Map;
   Interface::GridMap* c3Map;
   Interface::GridMap* c4Map;
   Interface::GridMap* c5Map;

   Interface::GridMap* c15AroMap;
   Interface::GridMap* c15SatMap;

   Interface::GridMap* c6AroMap;
   Interface::GridMap* c6SatMap;

   Interface::GridMap* asphaltenes;
   Interface::GridMap* resins;

   Interface::GridMap* cox;
   Interface::GridMap* n2;


   c1Map = propertyValues [ 0 ]->getGridMap ();
   c1Map->retrieveData ();

   c2Map = propertyValues [ 1 ]->getGridMap ();
   c2Map->retrieveData ();

   c3Map = propertyValues [ 2 ]->getGridMap ();
   c3Map->retrieveData ();

   c4Map = propertyValues [ 3 ]->getGridMap ();
   c4Map->retrieveData ();

   c5Map = propertyValues [ 4 ]->getGridMap ();
   c5Map->retrieveData ();

   c6AroMap = propertyValues [ 5 ]->getGridMap ();
   c6AroMap->retrieveData ();

   c6SatMap = propertyValues [ 6 ]->getGridMap ();
   c6SatMap->retrieveData ();

   c15AroMap = propertyValues [ 7 ]->getGridMap ();
   c15AroMap->retrieveData ();

   c15SatMap = propertyValues [ 8 ]->getGridMap ();
   c15SatMap->retrieveData ();

   asphaltenes = propertyValues [ 9 ]->getGridMap ();
   asphaltenes->retrieveData ();

   resins = propertyValues [ 10 ]->getGridMap ();
   resins->retrieveData ();

   cox = propertyValues [ 11 ]->getGridMap ();
   cox->retrieveData ();

   n2 = propertyValues [ 12 ]->getGridMap ();
   n2->retrieveData ();

   // The assumption here is that all the maps will have the same undefined-value.
   undefinedValue = c1Map->getUndefinedValue ();
#endif


   for ( id = 0, mapCount = 0; id < CBMGenerics::ComponentManager::NumberOfSpecies; ++id ) {

      if ( m_hasSpeciesMap [ id ]) {
         propertyValues [ mapCount ]->getGridMap ()->retrieveData ();
         m_sourceRock->fillResultMap ( CBMGenerics::ComponentManager::SpeciesNamesId ( id ), propertyValues [ mapCount ]->getGridMap ());
         ++mapCount;
      }

   }


#if 0
   // Should initialise the methane-map with null values first.

   m_sourceRock->fillResultMap ( CBMGenerics::ComponentManager::C1, c1Map );
   m_sourceRock->fillResultMap ( CBMGenerics::ComponentManager::C2, c2Map );
   m_sourceRock->fillResultMap ( CBMGenerics::ComponentManager::C3, c3Map );
   m_sourceRock->fillResultMap ( CBMGenerics::ComponentManager::C4, c4Map );
   m_sourceRock->fillResultMap ( CBMGenerics::ComponentManager::C5, c5Map );

   m_sourceRock->fillResultMap ( CBMGenerics::ComponentManager::C6Minus14Aro, c6AroMap );
   m_sourceRock->fillResultMap ( CBMGenerics::ComponentManager::C6Minus14Sat, c6SatMap );

   m_sourceRock->fillResultMap ( CBMGenerics::ComponentManager::C15PlusAro, c15AroMap );
   m_sourceRock->fillResultMap ( CBMGenerics::ComponentManager::C15PlusSat, c15SatMap );

   m_sourceRock->fillResultMap ( CBMGenerics::ComponentManager::asphaltene, asphaltenes );
   m_sourceRock->fillResultMap ( CBMGenerics::ComponentManager::resin, resins );

   m_sourceRock->fillResultMap ( CBMGenerics::ComponentManager::COx, cox );
   m_sourceRock->fillResultMap ( CBMGenerics::ComponentManager::N2, n2 );
#endif

   m_isCalculated = true;
   return true;
}


void RequiredGenex5PropertyCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   CBMGenerics::ComponentManager& manager = CBMGenerics::ComponentManager::getInstance ();

   bool sourceRockContainsSulphur = false; //dynamic_cast<const Genex6::SourceRock*>(m_formation->getSourceRock1 ())->isSulphur ();
   int id;
   PropertyValue* species;

   if ( m_formation->isSourceRock ()) {
      m_sourceRock = (GeoPhysics::GeoPhysicsSourceRock*) (m_formation->getSourceRock1 ());
   } else {
      m_sourceRock = 0;
   }

   if ( m_sourceRock != 0 ) {
      sourceRockContainsSulphur = m_sourceRock->isSulphur ();
   }

   for ( id = 0; id < CBMGenerics::ComponentManager::NumberOfSpecies; ++id ) {

      // If the source-rock contains sulphur then always add the species
      // If the source-rock does not contain sulphur then add only those species that do not contain sulphur.
      if ( sourceRockContainsSulphur or ( not sourceRockContainsSulphur and not manager.isSulphurComponent ( id ))) {
         species = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( manager.getSpeciesSourceRockExpelledByName ( id ), 
                                                                                                   m_snapshot, 0, 
                                                                                                   m_formation,
                                                                                                   0 ));

         properties.push_back ( species );
         m_hasSpeciesMap [ id ] = true;
      } else {
         m_hasSpeciesMap [ id ] = false;
         // Otherwise add a null value for the species.
      }

   }


#if 0
   PropertyValue* c1;
   PropertyValue* c2;
   PropertyValue* c3;
   PropertyValue* c4;
   PropertyValue* c5;

   PropertyValue* c15Aro;
   PropertyValue* c15Sat;
   PropertyValue* c6Aro;
   PropertyValue* c6Sat;

   PropertyValue* asphaltenes;
   PropertyValue* resins;

   PropertyValue* cox;
   PropertyValue* n2;



   c1 = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "C1ExpelledCumulative", 
                                                                                        m_snapshot, 0, 
                                                                                        m_formation,
                                                                                        0 ));

   c2 = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "C2ExpelledCumulative", 
                                                                                        m_snapshot, 0, 
                                                                                        m_formation,
                                                                                        0 ));

   c3 = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "C3ExpelledCumulative", 
                                                                                        m_snapshot, 0, 
                                                                                        m_formation,
                                                                                        0 ));

   c4 = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "C4ExpelledCumulative", 
                                                                                        m_snapshot, 0, 
                                                                                        m_formation,
                                                                                        0 ));

   c5 = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "C5ExpelledCumulative", 
                                                                                        m_snapshot, 0, 
                                                                                        m_formation,
                                                                                        0 ));

   c15Aro = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "C15+AroExpelledCumulative", 
                                                                                            m_snapshot, 0, 
                                                                                            m_formation,
                                                                                            0 ));

   c15Sat = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "C15+SatExpelledCumulative", 
                                                                                            m_snapshot, 0, 
                                                                                            m_formation,
                                                                                            0 ));

   c6Aro = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "C6-14AroExpelledCumulative", 
                                                                                            m_snapshot, 0, 
                                                                                            m_formation,
                                                                                            0 ));

   c6Sat = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "C6-14SatExpelledCumulative", 
                                                                                            m_snapshot, 0, 
                                                                                            m_formation,
                                                                                            0 ));

   asphaltenes = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "asphaltenesExpelledCumulative", 
                                                                                                 m_snapshot, 0, 
                                                                                                 m_formation,
                                                                                                 0 ));

   resins = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "resinsExpelledCumulative", 
                                                                                            m_snapshot, 0, 
                                                                                            m_formation,
                                                                                            0 ));

   cox = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "COxExpelledCumulative", 
                                                                                         m_snapshot, 0, 
                                                                                         m_formation,
                                                                                         0 ));

   n2 = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "N2ExpelledCumulative", 
                                                                                        m_snapshot, 0, 
                                                                                        m_formation,
                                                                                        0 ));


   properties.push_back ( c1 );
   properties.push_back ( c2 );
   properties.push_back ( c3 );
   properties.push_back ( c4 );
   properties.push_back ( c5 );
   properties.push_back ( c6Aro );
   properties.push_back ( c6Sat );
   properties.push_back ( c15Aro );
   properties.push_back ( c15Sat );
   properties.push_back ( asphaltenes );
   properties.push_back ( resins );
   properties.push_back ( cox );
   properties.push_back ( n2 );
#endif
}

bool RequiredGenex5PropertyCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   bool isActive = false;

   if ( m_formation->isSourceRock ()) {

      isActive = m_formation->getTopSurface ()->getSnapshot ()->getTime () >= m_snapshot->getTime ();

      if ( isActive ) {
         m_sourceRock = (GeoPhysics::GeoPhysicsSourceRock*) (m_formation->getSourceRock1 ());
      } else {
         m_sourceRock = 0;
      }

   } else {
      m_sourceRock = 0;
   }

   if ( m_sourceRock == 0 ) {

      OutputPropertyMap::PropertyValueList::iterator iter;

      for ( iter = propertyValues.begin (); iter != propertyValues.end (); ++iter ) {
         (*iter)->allowOutput ( false );
      }

   }

   return ( m_formation->isSourceRock () and ( m_sourceRock != 0 or not isActive )) or not m_formation->isSourceRock ();
}



OutputPropertyMap* allocateRequiredGenex5PropertyCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<RequiredGenex5PropertyCalculator>( property, formation, surface, snapshot );
}
