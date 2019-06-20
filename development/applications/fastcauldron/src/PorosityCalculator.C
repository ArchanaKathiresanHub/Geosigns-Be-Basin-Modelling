#include "PorosityCalculator.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"

#include "RunParameters.h"


OutputPropertyMap* allocatePorosityCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<PorosityCalculator>( property, formation, surface, snapshot );
}

OutputPropertyMap* allocatePorosityVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<PorosityVolumeCalculator>( property, formation, snapshot );
}

PorosityCalculator::PorosityCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_ves = 0;
   m_maxVes = 0;
   m_chemicalCompaction = 0;
   m_isCalculated = false;
   m_lithologies = 0;

   m_chemicalCompactionRequired = m_formation->hasChemicalCompaction () and 
                                  FastcauldronSimulator::getInstance ().getRunParameters ()->getChemicalCompaction ();

}

bool PorosityCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                            OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   double value;
   double undefinedValue;
   double chemicalCompactionValue;
   Interface::GridMap* porosityMap;

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

   porosityMap = propertyValues [ 0 ]->getGridMap ();
   porosityMap->retrieveData ();
   undefinedValue = porosityMap->getUndefinedValue ();

   for ( i = porosityMap->firstI (); i <= porosityMap->lastI (); ++i ) {

      for ( j = porosityMap->firstJ (); j <= porosityMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {

            if ( m_chemicalCompactionRequired ) {
               chemicalCompactionValue = (*m_chemicalCompaction)( i, j );
            } else {
               chemicalCompactionValue = 0.0;
            }

            value = 100.0 * (*m_lithologies)( i, j )->porosity ( (*m_ves)( i, j ), (*m_maxVes)( i, j ), m_chemicalCompactionRequired, chemicalCompactionValue );
            porosityMap->setValue ( i, j, value );
            
         } else {
            porosityMap->setValue ( i, j, undefinedValue );
         }

      }

   }

   porosityMap->restoreData ();
   m_isCalculated = true;
   return true;
}

void PorosityCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "PorosityVec2", 
                                                                                                         m_snapshot, 0, 
                                                                                                         m_formation,
                                                                                                         m_surface )));

}

bool PorosityCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

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
   }

   return m_ves != 0 and m_maxVes != 0 and m_lithologies != 0 and ( m_chemicalCompactionRequired ? m_chemicalCompaction != 0 : true );
}



PorosityVolumeCalculator::PorosityVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_ves = 0;
   m_maxVes = 0;
   m_chemicalCompaction = 0;
   m_isCalculated = false;
   m_lithologies = 0;

   m_chemicalCompactionRequired = m_formation->hasChemicalCompaction () and 
                                  FastcauldronSimulator::getInstance ().getRunParameters ()->getChemicalCompaction ();

}

bool PorosityVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                  OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   unsigned int k;
   double undefinedValue;
   double value;
   double chemicalCompactionValue;
   Interface::GridMap* porosityMap;

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

   porosityMap = propertyValues [ 0 ]->getGridMap ();
   porosityMap->retrieveData ();
   undefinedValue = porosityMap->getUndefinedValue ();

   for ( i = porosityMap->firstI (); i <= porosityMap->lastI (); ++i ) {

      for ( j = porosityMap->firstJ (); j <= porosityMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {

            for ( k = porosityMap->firstK (); k <= porosityMap->lastK (); ++k ) {

               if ( m_chemicalCompactionRequired ) {
                  chemicalCompactionValue = m_chemicalCompaction->getVolumeValue ( i, j, k );
               } else {
                  chemicalCompactionValue = 0.0;
               }

               value = 100.0 * (*m_lithologies)( i, j )->porosity ( m_ves->getVolumeValue ( i, j, k ),
                                                                    m_maxVes->getVolumeValue ( i, j, k ),
                                                                    m_chemicalCompactionRequired,
                                                                    chemicalCompactionValue );
               porosityMap->setValue ( i, j, k, value );
            }

         } else {

            for ( k = porosityMap->firstK (); k <= porosityMap->lastK (); ++k ) {
               porosityMap->setValue ( i, j, k, undefinedValue );
            }

         }

      }

   }

   porosityMap->restoreData ();
   m_isCalculated = true;
   return true;
}

void PorosityVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "Porosity", 
                                                                                                            m_snapshot, 0, 
                                                                                                            m_formation,
                                                                                                            m_formation->getMaximumNumberOfElements () + 1 )));

}

bool PorosityVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   m_ves = PropertyManager::getInstance().findOutputPropertyVolume ( "Ves", m_formation, m_snapshot );
   m_maxVes = PropertyManager::getInstance().findOutputPropertyVolume ( "MaxVes", m_formation, m_snapshot );

   if ( m_chemicalCompactionRequired ) {
      m_chemicalCompaction = PropertyManager::getInstance().findOutputPropertyVolume ( "ChemicalCompaction", m_formation, m_snapshot );
   } else {
      m_chemicalCompaction = 0;
   }

   m_lithologies = &m_formation->getCompoundLithologyArray ();
//    m_lithologies = &m_formation->Lithology;

   return m_ves != 0 and m_maxVes != 0 and m_lithologies != 0 and ( m_chemicalCompactionRequired ? m_chemicalCompaction != 0 : true );
}
