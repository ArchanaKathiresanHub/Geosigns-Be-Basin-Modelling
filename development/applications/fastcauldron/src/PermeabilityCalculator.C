//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PermeabilityCalculator.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"

#include "RunParameters.h"

// utilities library
#include "ConstantsMathematics.h"
using Utilities::Maths::MilliDarcyToM2;

OutputPropertyMap* allocatePermeabilityCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<PermeabilityCalculator>( property, formation, surface, snapshot );
}

OutputPropertyMap* allocatePermeabilityVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<PermeabilityVolumeCalculator>( property, formation, snapshot );
}

PermeabilityCalculator::PermeabilityCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_ves = 0;
   m_maxVes = 0;
   m_chemicalCompaction = 0;

   m_isCalculated = false;
   m_lithologies = 0;

   m_chemicalCompactionRequired = m_formation->hasChemicalCompaction () and 
                                  FastcauldronSimulator::getInstance ().getRunParameters ()->getChemicalCompaction ();

}

bool PermeabilityCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;

   double undefinedValue;
   double permNorm;
   double permPlane;
   double chemicalCompactionValue;
   GeoPhysics::CompoundProperty porosity;

   Interface::GridMap* verticalPermeabilityMap;
   Interface::GridMap* horizontalPermeabilityMap;

   if ( not m_ves->isCalculated ()) {

      if ( not m_ves->calculate ()) {
         return false;
      }

   }

   if ( not m_maxVes->isCalculated ()) {

      if ( not m_maxVes->calculate ()) {
         return false;
      } 

   }

   if ( m_chemicalCompactionRequired and not m_chemicalCompaction->isCalculated ()) {

      if ( not m_chemicalCompaction->calculate ()) {
         return false;
      } 

   }

   verticalPermeabilityMap = propertyValues [ 0 ]->getGridMap ();
   verticalPermeabilityMap->retrieveData ();

   // The assumption here is that all the maps will have the same undefined-value.
   undefinedValue = verticalPermeabilityMap->getUndefinedValue ();

   horizontalPermeabilityMap = propertyValues [ 1 ]->getGridMap ();
   horizontalPermeabilityMap->retrieveData ();

   for ( i = verticalPermeabilityMap->firstI (); i <= verticalPermeabilityMap->lastI (); ++i ) {

      for ( j = verticalPermeabilityMap->firstJ (); j <= verticalPermeabilityMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {

            if ( m_chemicalCompactionRequired ) {
               chemicalCompactionValue = (*m_chemicalCompaction)( i, j );
            } else {
               chemicalCompactionValue = 0.0;
            }

            m_formation->getLithology ( i, j )->getPorosity ((*m_ves)( i, j ), (*m_maxVes)( i, j ), m_chemicalCompactionRequired, chemicalCompactionValue, porosity );
            m_formation->getLithology ( i, j )->calcBulkPermeabilityNP ((*m_ves)( i, j ), (*m_maxVes)( i, j ), porosity, permNorm, permPlane );
            verticalPermeabilityMap->setValue ( i, j, permNorm / MilliDarcyToM2 );
            horizontalPermeabilityMap->setValue ( i, j, permPlane / MilliDarcyToM2 );
         } else {
            verticalPermeabilityMap->setValue ( i, j, undefinedValue );
            horizontalPermeabilityMap->setValue ( i, j, undefinedValue );
         }

      }

   }

   verticalPermeabilityMap->restoreData ();
   horizontalPermeabilityMap->restoreData ();

   m_isCalculated = true;
   return true;
}

void PermeabilityCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   CauldronPropertyValue* verticalPermeability;
   CauldronPropertyValue* horizontalPermeability;

   verticalPermeability = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "PermeabilityVec2", 
                                                                                                          m_snapshot, 0, 
                                                                                                          m_formation,
                                                                                                          m_surface ));


   horizontalPermeability = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "PermeabilityHVec2", 
                                                                                                            m_snapshot, 0, 
                                                                                                            m_formation,
                                                                                                            m_surface ));

   properties.push_back ( verticalPermeability );
   properties.push_back ( horizontalPermeability );

   if ( FastcauldronSimulator::getInstance ().getModellingMode () == Interface::MODE1D ) {
      horizontalPermeability->allowOutput ( false );
   }


}

bool PermeabilityCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   m_ves = PropertyManager::getInstance().findOutputPropertyMap ( "Ves", m_formation, m_surface, m_snapshot );
   m_maxVes = PropertyManager::getInstance().findOutputPropertyMap ( "MaxVes", m_formation, m_surface, m_snapshot );

   if ( m_chemicalCompactionRequired ) {
      m_chemicalCompaction = PropertyManager::getInstance().findOutputPropertyMap ( "ChemicalCompaction", m_formation, m_surface, m_snapshot );
   } else {
      m_chemicalCompaction = 0;
   }

   m_lithologies = &m_formation->getCompoundLithologyArray ();
//    m_lithologies = &m_formation->Lithology;

   if ( FastcauldronSimulator::getInstance ().getCauldron()->no2Doutput()) {
      propertyValues [ 0 ]->allowOutput ( false );
      propertyValues [ 1 ]->allowOutput ( false );
   }

   // Also check foc chemical compaction.
   return m_ves != 0 and m_maxVes != 0 and m_lithologies != 0 and ( m_chemicalCompactionRequired ? m_chemicalCompaction != 0 : true );
}


PermeabilityVolumeCalculator::PermeabilityVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_ves = 0;
   m_maxVes = 0;
   m_chemicalCompaction = 0;

   m_isCalculated = false;
   m_lithologies = 0;

   m_chemicalCompactionRequired = m_formation->hasChemicalCompaction () and 
                                  FastcauldronSimulator::getInstance ().getRunParameters ()->getChemicalCompaction ();

}

bool PermeabilityVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                      OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   unsigned int k;
   double undefinedValue;
   double permNorm;
   double permPlane;
   double chemicalCompactionValue;
   GeoPhysics::CompoundProperty porosity;

   Interface::GridMap* verticalPermeabilityMap;
   Interface::GridMap* horizontalPermeabilityMap;

   if ( not m_ves->isCalculated ()) {

      if ( not m_ves->calculate ()) {
         return false;
      }

   }

   if ( not m_maxVes->isCalculated ()) {

      if ( not m_maxVes->calculate ()) {
         return false;
      } 

   }

   if ( m_chemicalCompactionRequired and not m_chemicalCompaction->isCalculated ()) {

      if ( not m_chemicalCompaction->calculate ()) {
         return false;
      } 

   }

   verticalPermeabilityMap = propertyValues [ 0 ]->getGridMap ();
   verticalPermeabilityMap->retrieveData ();

   // The assumption here is that all the maps will have the same undefined-value.
   undefinedValue = verticalPermeabilityMap->getUndefinedValue ();

   horizontalPermeabilityMap = propertyValues [ 1 ]->getGridMap ();
   horizontalPermeabilityMap->retrieveData ();

   for ( i = verticalPermeabilityMap->firstI (); i <= verticalPermeabilityMap->lastI (); ++i ) {

      for ( j = verticalPermeabilityMap->firstJ (); j <= verticalPermeabilityMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {

            for ( k = verticalPermeabilityMap->firstK (); k <= verticalPermeabilityMap->lastK (); ++k ) {

               if ( m_chemicalCompactionRequired ) {
                  chemicalCompactionValue = m_chemicalCompaction->getVolumeValue ( i, j, k );
               } else {
                  chemicalCompactionValue = 0.0;
               }

               m_formation->getLithology ( i, j )->getPorosity ( m_ves->getVolumeValue ( i, j, k ),
                                                                 m_maxVes->getVolumeValue ( i, j, k ),
                                                                 m_chemicalCompactionRequired,
                                                                 chemicalCompactionValue,
                                                                 porosity );

               m_formation->getLithology ( i, j )->calcBulkPermeabilityNP ( m_ves->getVolumeValue ( i, j, k ),
                                                                            m_maxVes->getVolumeValue ( i, j, k ),
                                                                            porosity,
                                                                            permNorm,
                                                                            permPlane );
               verticalPermeabilityMap->setValue ( i, j, k, permNorm / MilliDarcyToM2 );
               horizontalPermeabilityMap->setValue ( i, j, k, permPlane / MilliDarcyToM2 );
            }

         } else {

            for ( k = verticalPermeabilityMap->firstK (); k <= verticalPermeabilityMap->lastK (); ++k ) {
               verticalPermeabilityMap->setValue ( i, j, k, undefinedValue );
               horizontalPermeabilityMap->setValue ( i, j, k, undefinedValue );
            }

         }

      }

   }

   verticalPermeabilityMap->restoreData ();
   horizontalPermeabilityMap->restoreData ();

   m_isCalculated = true;
   return true;
}

void PermeabilityVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   CauldronPropertyValue* verticalPermeability;
   CauldronPropertyValue* horizontalPermeability;

   verticalPermeability = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "Permeability", 
                                                                                                             m_snapshot, 0, 
                                                                                                             m_formation,
                                                                                                             m_formation->getMaximumNumberOfElements () + 1 ));


   horizontalPermeability = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HorizontalPermeability", 
                                                                                                               m_snapshot, 0, 
                                                                                                               m_formation,
                                                                                                               m_formation->getMaximumNumberOfElements () + 1 ));

   properties.push_back ( verticalPermeability );
   properties.push_back ( horizontalPermeability );

   if ( FastcauldronSimulator::getInstance ().getModellingMode () == Interface::MODE1D ) {
      horizontalPermeability->allowOutput ( false );
   }


}

bool PermeabilityVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   m_ves = PropertyManager::getInstance().findOutputPropertyVolume ( "Ves", m_formation, m_snapshot );
   m_maxVes = PropertyManager::getInstance().findOutputPropertyVolume ( "MaxVes", m_formation, m_snapshot );

   if ( m_chemicalCompactionRequired ) {
      m_chemicalCompaction = PropertyManager::getInstance().findOutputPropertyVolume ( "ChemicalCompaction", m_formation, m_snapshot );
   } else {
      m_chemicalCompaction = 0;
   }

   m_lithologies = &m_formation->getCompoundLithologyArray ();
//    m_lithologies = &m_formation->Lithology;

   // Also check foc chemical compaction.
   return m_ves != 0 and m_maxVes != 0 and m_lithologies != 0 and ( m_chemicalCompactionRequired ? m_chemicalCompaction != 0 : true );
}

