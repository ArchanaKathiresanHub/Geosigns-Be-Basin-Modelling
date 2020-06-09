//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "mpi.h"
#include "RequiredGenex5PropertyCalculator.h"


#include "PropertyValue.h"
#include "GridMap.h"

#include "SourceRock.h"

#include "DerivedOutputPropertyMap.h"


RequiredGenex5PropertyCalculator::RequiredGenex5PropertyCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_isCalculated = false;
   m_sourceRock = 0;

   int id;

   for ( id = 0; id < ComponentId::NUMBER_OF_SPECIES; ++id ){
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

   for ( id = 0, mapCount = 0; id < ComponentId::NUMBER_OF_SPECIES; ++id ) {

      if ( m_hasSpeciesMap [ id ]) {
         propertyValues [ mapCount ]->getGridMap ()->retrieveData ();
         m_sourceRock->fillResultMap ( CBMGenerics::ComponentManager::SpeciesNamesId ( id ), propertyValues [ mapCount ]->getGridMap ());
         ++mapCount;
      }

   }
   
   m_isCalculated = true;
   return true;
}


void RequiredGenex5PropertyCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   CBMGenerics::ComponentManager& manager = CBMGenerics::ComponentManager::getInstance ();

   bool sourceRockContainsSulphur = false; //dynamic_cast<const Genex6::SourceRock*>(m_formation->getSourceRock1 ())->isSulphur ();
   int id;
   CauldronPropertyValue* species;

   if ( m_formation->isSourceRock ()) {
      m_sourceRock = (GeoPhysics::GeoPhysicsSourceRock*) (m_formation->getSourceRock1 ());
   } else {
      m_sourceRock = 0;
   }

   if ( m_sourceRock != 0 ) {
      sourceRockContainsSulphur = m_sourceRock->isSulphur ();
   }

   for ( id = 0; id < ComponentId::NUMBER_OF_SPECIES; ++id ) {

      // If the source-rock contains sulphur then always add the species
      // If the source-rock does not contain sulphur then add only those species that do not contain sulphur.
      if ( sourceRockContainsSulphur or ( not sourceRockContainsSulphur and not manager.isSulphurComponent ( id ))) {
         species = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( manager.getSpeciesSourceRockExpelledByName ( id ), 
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



OutputPropertyMap* allocateRequiredGenex5PropertyCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<RequiredGenex5PropertyCalculator>( property, formation, surface, snapshot );
}
