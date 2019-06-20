#include "FracturePressureMapCalculator.h"

#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"
#include "HydraulicFracturingManager.h"


OutputPropertyMap* allocateFracturePressureMapCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<FracturePressureMapCalculator>( property, formation, surface, snapshot );
}


FracturePressureMapCalculator::FracturePressureMapCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) : 
   m_formation ( formation ),
   m_surface ( surface ),
   m_snapshot ( snapshot ),
   m_isCalculated ( false )
{

}

void FracturePressureMapCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "FracturePressure", 
                                                                                                         m_snapshot, 0, 
                                                                                                         m_formation,
                                                                                                         m_surface )));

}

bool FracturePressureMapCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   m_lithologies = &m_formation->getCompoundLithologyArray ();
   m_depth = PropertyManager::getInstance().findOutputPropertyMap ( "Depth", m_formation, m_surface, m_snapshot );
   m_hydrostaticPressure = PropertyManager::getInstance().findOutputPropertyMap ( "HydroStaticPressure", m_formation, m_surface, m_snapshot );
   m_lithostaticPressure = PropertyManager::getInstance().findOutputPropertyMap ( "LithoStaticPressure", m_formation, m_surface, m_snapshot );

   if ( FastcauldronSimulator::getInstance ().getCauldron()->no2Doutput()) {
      propertyValues [ 0 ]->allowOutput( false );
   }
   return m_lithologies != 0 and m_depth != 0 and m_hydrostaticPressure != 0 and m_lithostaticPressure != 0;
}


bool FracturePressureMapCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                          OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
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

            value = HydraulicFracturingManager::getInstance ().fracturePressure ( (*m_lithologies)( i, j ), m_formation->getFluid (),
                                                                                  seaBottomTemperature, seaBottomDepth,
                                                                                  (*m_depth)( i, j ),
                                                                                  (*m_hydrostaticPressure)( i, j ),
                                                                                  (*m_lithostaticPressure)( i, j ));

            fracturePressureMap->setValue ( i, j, value );

         } else {
            fracturePressureMap->setValue ( i, j, undefinedValue );
         }

      }

   }

   fracturePressureMap->restoreData ();
   m_isCalculated = true;
   return true;

}
