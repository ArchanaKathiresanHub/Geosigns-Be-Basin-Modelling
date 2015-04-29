#include "VelocityCalculator.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"


#include "Interface/RunParameters.h"

OutputPropertyMap* allocateVelocityCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<VelocityCalculator>( property, formation, surface, snapshot );
}

OutputPropertyMap* allocateVelocityVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<VelocityVolumeCalculator>( property, formation, snapshot );
}

VelocityCalculator::VelocityCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_porosity = 0;
   m_pressure = 0;
   m_temperature = 0;
   m_bulkDensity = 0;
   m_isCalculated = false;
   m_lithologies = 0;
   m_fluid = 0;

}

bool VelocityCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                            OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   const std::string& projectVelocityName = FastcauldronSimulator::getInstance ().getRunParameters ()->getVelAlgorithm ();

   unsigned int i;
   unsigned int j;
   double value;
   Interface::GridMap* velocityMap;
   double undefinedValue;

   string velocityAlgorithmName = ( projectVelocityName == "" ? GeoPhysics::DefaultVelocityAlgorithm : projectVelocityName );

   VelocityAlgorithm velocityAlgorithm = velocityAlgorithmValue ( velocityAlgorithmName );

   if ( not m_porosity->isCalculated ()) {

      if ( not m_porosity->calculate ()) {
         return false;
      }

   }

   if ( not m_pressure->isCalculated ()) {

      if ( not m_pressure->calculate ()) {
         return false;
      } 

   }

   if ( not m_temperature->isCalculated ()) {

      if ( not m_temperature->calculate ()) {
         return false;
      } 

   }

   if ( not m_bulkDensity->isCalculated ()) {

      if ( not m_bulkDensity->calculate ()) {
         return false;
      } 

   }

   velocityMap = propertyValues [ 0 ]->getGridMap ();
   velocityMap->retrieveData ();
   undefinedValue = velocityMap->getUndefinedValue ();

   for ( i = velocityMap->firstI (); i <= velocityMap->lastI (); ++i ) {

      for ( j = velocityMap->firstJ (); j <= velocityMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {
            (*m_lithologies)( i, j )->calcVelocity ( m_fluid,
                                                     velocityAlgorithm,
                                                     0.01 * (*m_porosity)( i, j ),
                                                     (*m_bulkDensity)( i, j ),
                                                     (*m_pressure)( i, j ),
                                                     (*m_temperature)( i, j ),
                                                     value );
            velocityMap->setValue ( i, j, value );
         } else {
            velocityMap->setValue ( i, j, undefinedValue );
         }

      }

   }

   velocityMap->restoreData ();
   m_isCalculated = true;
   return true;
}

void VelocityCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "VelocityVec2", 
                                                                                                         m_snapshot, 0, 
                                                                                                         m_formation,
                                                                                                         m_surface )));

}

bool VelocityCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   m_porosity = PropertyManager::getInstance().findOutputPropertyMap ( "Porosity", m_formation, m_surface, m_snapshot );
   m_pressure = PropertyManager::getInstance().findOutputPropertyMap ( "Pressure", m_formation, m_surface, m_snapshot );
   m_temperature = PropertyManager::getInstance().findOutputPropertyMap ( "Temperature", m_formation, m_surface, m_snapshot );
   m_bulkDensity = PropertyManager::getInstance().findOutputPropertyMap ( "BulkDensity", m_formation, m_surface, m_snapshot );

   m_lithologies = &m_formation->getCompoundLithologyArray ();
   m_fluid = m_formation->fluid;

   if ( FastcauldronSimulator::getInstance ().getCauldron()->no2Doutput()) {
      propertyValues [ 0 ]->allowOutput ( false );
   }

   return m_porosity != 0 and m_pressure != 0 and m_temperature != 0 and m_bulkDensity != 0 and m_lithologies != 0 and m_fluid != 0;
}


VelocityVolumeCalculator::VelocityVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_porosity = 0;
   m_pressure = 0;
   m_temperature = 0;
   m_bulkDensity = 0;
   m_isCalculated = false;
   m_lithologies = 0;
   m_fluid = 0;

}

bool VelocityVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                  OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   const std::string& projectVelocityName = FastcauldronSimulator::getInstance ().getRunParameters ()->getVelAlgorithm ();

   string velocityAlgorithmName = ( projectVelocityName == "" ? GeoPhysics::DefaultVelocityAlgorithm : projectVelocityName );

   VelocityAlgorithm velocityAlgorithm = velocityAlgorithmValue ( velocityAlgorithmName );

   unsigned int i;
   unsigned int j;
   unsigned int k;
   double value;
   double undefinedValue;
   Interface::GridMap* velocityMap;

   if ( not m_porosity->isCalculated ()) {

      if ( not m_porosity->calculate ()) {
         return false;
      }

   }

   if ( not m_pressure->isCalculated ()) {

      if ( not m_pressure->calculate ()) {
         return false;
      } 

   }

   if ( not m_temperature->isCalculated ()) {

      if ( not m_temperature->calculate ()) {
         return false;
      } 

   }

   if ( not m_bulkDensity->isCalculated ()) {

      if ( not m_bulkDensity->calculate ()) {
         return false;
      } 

   }

   velocityMap = propertyValues [ 0 ]->getGridMap ();
   velocityMap->retrieveData ();
   undefinedValue = velocityMap->getUndefinedValue ();

   for ( i = velocityMap->firstI (); i <= velocityMap->lastI (); ++i ) {

      for ( j = velocityMap->firstJ (); j <= velocityMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {
            
            for ( k = velocityMap->firstK (); k <= velocityMap->lastK (); ++k ) {
               (*m_lithologies)( i, j )->calcVelocity ( m_fluid,
                                                        velocityAlgorithm,
                                                        0.01 * m_porosity->getVolumeValue ( i, j, k ),
                                                        m_bulkDensity->getVolumeValue ( i, j, k ),
                                                        m_pressure->getVolumeValue ( i, j, k ),
                                                        m_temperature->getVolumeValue ( i, j, k ),
                                                        value );
               velocityMap->setValue ( i, j, k, value );
            }

         } else {

            for ( k = velocityMap->firstK (); k <= velocityMap->lastK (); ++k ) {
               velocityMap->setValue ( i, j, k, undefinedValue );
            }

         }

      }

   }

   velocityMap->restoreData ();
   m_isCalculated = true;
   return true;
}

void VelocityVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "Velocity",
                                                                                                            m_snapshot, 0,
                                                                                                            m_formation,
                                                                                                            m_formation->getMaximumNumberOfElements () + 1 )));

}

bool VelocityVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   m_porosity = PropertyManager::getInstance().findOutputPropertyVolume ( "Porosity", m_formation, m_snapshot );
   m_pressure = PropertyManager::getInstance().findOutputPropertyVolume ( "Pressure", m_formation, m_snapshot );
   m_temperature = PropertyManager::getInstance().findOutputPropertyVolume ( "Temperature", m_formation, m_snapshot );
   m_bulkDensity = PropertyManager::getInstance().findOutputPropertyVolume ( "BulkDensity", m_formation, m_snapshot );

   m_lithologies = &m_formation->getCompoundLithologyArray ();
   m_fluid = m_formation->fluid;

   return m_porosity != 0 and m_pressure != 0 and m_temperature != 0 and m_bulkDensity != 0 and m_lithologies != 0 and m_fluid != 0;
}
