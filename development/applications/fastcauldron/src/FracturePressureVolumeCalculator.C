#include "FracturePressureVolumeCalculator.h"

#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"
#include "HydraulicFracturingManager.h"


OutputPropertyMap* allocateFracturePressureVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<FracturePressureVolumeCalculator>( property, formation, snapshot );
}



FracturePressureVolumeCalculator::FracturePressureVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) : 
   m_formation ( formation ),
   m_snapshot ( snapshot ),
   m_isCalculated ( false )
{

}

void FracturePressureVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "FracturePressure", 
                                                                                                            m_snapshot, 0, 
                                                                                                            m_formation,
                                                                                                            m_formation->getMaximumNumberOfElements () + 1 )));

}

bool FracturePressureVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   m_lithologies = &m_formation->getCompoundLithologyArray ();
   m_depth = PropertyManager::getInstance().findOutputPropertyVolume ( "Depth", m_formation, m_snapshot );
   m_hydrostaticPressure = PropertyManager::getInstance().findOutputPropertyVolume ( "HydroStaticPressure", m_formation, m_snapshot );
   m_lithostaticPressure = PropertyManager::getInstance().findOutputPropertyVolume ( "LithoStaticPressure", m_formation, m_snapshot );

   return m_lithologies != 0 and m_depth != 0 and m_hydrostaticPressure != 0 and m_lithostaticPressure != 0;
}


bool FracturePressureVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                          OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   unsigned int k;
   double undefinedValue;
   double value;
   double seaBottomDepth;
   double seaBottomTemperature;
   Interface::GridMap* fracturePressureMap;

   if ( not m_depth->isCalculated ()) {

      if ( not m_depth->calculate ()) {
         return false;
      }

   }

   if ( not m_hydrostaticPressure->isCalculated ()) {

      if ( not m_hydrostaticPressure->calculate ()) {
         return false;
      }

   }


   if ( not m_lithostaticPressure->isCalculated ()) {

      if ( not m_lithostaticPressure->calculate ()) {
         return false;
      }

   }

   fracturePressureMap = propertyValues [ 0 ]->getGridMap ();
   fracturePressureMap->retrieveData ();
   undefinedValue = fracturePressureMap->getUndefinedValue ();

   for ( i = fracturePressureMap->firstI (); i <= fracturePressureMap->lastI (); ++i ) {

      for ( j = fracturePressureMap->firstJ (); j <= fracturePressureMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {
            seaBottomDepth = FastcauldronSimulator::getInstance ().getSeaBottomDepth ( i, j, m_snapshot->getTime ());
            seaBottomTemperature = FastcauldronSimulator::getInstance ().getSeaBottomTemperature ( i, j, m_snapshot->getTime ());

            for ( k = fracturePressureMap->firstK (); k <= fracturePressureMap->lastK (); ++k ) {

               value = HydraulicFracturingManager::getInstance ().fracturePressure ( (*m_lithologies)( i, j ), m_formation->getFluid (),
                                                                                     seaBottomTemperature, seaBottomDepth,
                                                                                     m_depth->getVolumeValue ( i, j, k ),
                                                                                     m_hydrostaticPressure->getVolumeValue ( i, j, k ),
                                                                                     m_lithostaticPressure->getVolumeValue ( i, j, k ));

               fracturePressureMap->setValue ( i, j, k, value );
            }

         } else {

            for ( k = fracturePressureMap->firstK (); k <= fracturePressureMap->lastK (); ++k ) {
               fracturePressureMap->setValue ( i, j, k, undefinedValue );
            }

         }

      }

   }

   fracturePressureMap->restoreData ();
   m_isCalculated = true;
   return true;

}
