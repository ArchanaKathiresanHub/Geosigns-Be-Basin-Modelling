#include "TwoWayTimeCalculator.h"

#include "DerivedOutputPropertyMap.h"
#include "FastcauldronSimulator.h"
#include "PropertyManager.h"

OutputPropertyMap* allocateTwoWayTimeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<TwoWayTimeCalculator>( property, formation, surface, snapshot );
}

OutputPropertyMap* allocateTwoWayTimeVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<TwoWayTimeVolumeCalculator>( property, formation, snapshot );
}

OutputPropertyMap* allocateTwoWayTimeResidualCalculator( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ){
   return new DerivedOutputPropertyMap<TwoWayTimeResidualCalculator>( property, formation, surface, snapshot );
}


//--------------------------------------------------------------------------------------------------------------------------------------------
//2D CALCULATOR
//--------------------------------------------------------------------------------------------------------------------------------------------


TwoWayTimeCalculator::TwoWayTimeCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
m_formation( formation ), m_surface( surface ), m_snapshot( snapshot ),
m_topFormation( dynamic_cast<LayerProps const * const>( surface->getTopFormation( )) ),
m_bottomFormation( dynamic_cast<LayerProps const * const>(surface->getBottomFormation( )) )
{
   m_depth           = 0;
   m_pressure        = 0;
   m_seismicVelocity = 0;
   m_temperature     = 0;
   m_twoWayTimeTop   = 0;
   m_isCalculated    = false;
}

bool TwoWayTimeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                            OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   unsigned int k;

   double distance;
   double seaBottomDepth;
   double seismicVelocityBulk;
   double seismciVelocityFluid;
   const double time = m_snapshot->getTime( );

   double value;
   double undefinedValue;
   Interface::GridMap* TwoWayTimeMap;

   if (not m_pressure->isCalculated( )) {

      if (not m_pressure->calculate( )) {
         return false;
      }

   }

   if (not m_temperature->isCalculated( )) {

      if (not m_temperature->calculate( )) {
         return false;
      }

   }

   if (m_depth != 0 and not m_depth->isCalculated( )) {

      if (not m_depth->calculate( )) {
         return false;
      }

   }

   if (m_seismicVelocity != 0 and not m_seismicVelocity->isCalculated( )) {

      if (not m_seismicVelocity->calculate( )) {
         return false;
      }

   }

   if (m_twoWayTimeTop != 0 and not m_twoWayTimeTop->isCalculated( )) {

      if (not m_twoWayTimeTop->calculate( )) {
         return false;
      }

   }

   TwoWayTimeMap = propertyValues [ 0 ]->getGridMap ();
   TwoWayTimeMap->retrieveData ();
   undefinedValue = TwoWayTimeMap->getUndefinedValue ();

   for ( i = TwoWayTimeMap->firstI (); i <= TwoWayTimeMap->lastI (); ++i ) {

      for ( j = TwoWayTimeMap->firstJ (); j <= TwoWayTimeMap->lastJ (); ++j ) {

         if (FastcauldronSimulator::getInstance().nodeIsDefined( i, j )) {

            ///1.a If we are not at the top, add the twoWayTime from the surface above (top of the formation above)
            if ( m_twoWayTimeTop != 0 ){
               value = m_twoWayTimeTop->getMapValue( i, j );
            
               ///2. For all the nodes in the formation above (m_topFormation) add the twoWayTime
               for (k = 0; k < m_topFormation->getMaximumNumberOfElements( ); ++k) {

                  ///2.1 Compute the distance between the nodes
                  distance = m_depth->getVolumeValue( i, j, k ) - m_depth->getVolumeValue( i, j, k + 1 );
                  ///2.2 Compute the harmonic mean of the velocity between the nodes
                  seismicVelocityBulk = 2 * m_seismicVelocity->getVolumeValue( i, j, k ) * m_seismicVelocity->getVolumeValue( i, j, k + 1 );
                  if ( seismicVelocityBulk != 0 ){
                     seismicVelocityBulk /= m_seismicVelocity->getVolumeValue( i, j, k ) + m_seismicVelocity->getVolumeValue( i, j, k + 1 );
                     ///2.3 Compute the twoWayTime between the nodes
                     value += 2 * distance / seismicVelocityBulk;
                  }
                  else {
                     // In case we have basalt or other lithologies which are blocking the seismic waves (Vp=0)
                     value = undefinedValue;
                  }
               }

            }

            ///1.b If we are at the top, check for water TwoWayTime
            else{
               seaBottomDepth = FastcauldronSimulator::getInstance( ).getSeaBottomDepth( i, j, time );
               ///2. If there is water above the first formation, then the initial TwoWayTime is the one from the water above
               if (seaBottomDepth > 0) {
                  // If there is no sediment yet, there is no TwoWayTime value
                  if (m_bottomFormation->isBasement()) {
                     value = undefinedValue;
                  }
                  // If there is some sediments , the TwoWayTime value is computed using the fluid of the last sediment deposited
                  else {
                     assert( m_bottomFormation->fluid != 0 );
                     seismciVelocityFluid = m_bottomFormation->fluid->seismicVelocity( (*m_temperature)(i, j), (*m_pressure)(i, j) );
                     assert( seismciVelocityFluid != 0 );
                     value = 2 * seaBottomDepth / seismciVelocityFluid;
                  }
               }
               ///3. If no water above, then the initial TwoWayTime is 0
               else {
                  value = 0;
               }
            }

            TwoWayTimeMap->setValue ( i, j, value );

         } else {
            TwoWayTimeMap->setValue ( i, j, undefinedValue );
         }

      }

   }

   TwoWayTimeMap->restoreData ();
   m_isCalculated = true;
   return true;
}

void TwoWayTimeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "TwoWayTime", 
                                                                                                         m_snapshot, 0, 
                                                                                                         0,
                                                                                                         m_surface )));

}

bool TwoWayTimeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   // If we are not at the present day snapshot (t=0Ma): we do not ouput/compute the property.
   // !(*m_snapshot == *presentDaySnapshot) is a temporary coding --> cannot be fixed until derived property library is implemented
   const Interface::Snapshot* presentDaySnapshot = FastcauldronSimulator::getInstance( ).findOrCreateSnapshot( 0.0 );
   assert( ("presentDaySnapshot must be created", presentDaySnapshot != 0) );
   if (FastcauldronSimulator::getInstance( ).getCauldron( )->no2Doutput( ) or !(*m_snapshot == *presentDaySnapshot)) {
      propertyValues[0]->allowOutput( false );
   }

   m_pressure        = PropertyManager::getInstance( ).findOutputPropertyMap( "Pressure", m_formation, m_surface, m_snapshot );
   m_temperature     = PropertyManager::getInstance( ).findOutputPropertyMap( "Temperature", m_formation, m_surface, m_snapshot );
   
   // Store the TwoWayTime map of the surface above m_formation if we are not at the top of the model
   if (m_topFormation != 0){
      m_depth           = PropertyManager::getInstance( ).findOutputPropertyVolume( "Depth", m_topFormation, m_snapshot );
      m_seismicVelocity = PropertyManager::getInstance( ).findOutputPropertyVolume( "Velocity", m_topFormation, m_snapshot );
      m_twoWayTimeTop   = PropertyManager::getInstance( ).findOutputPropertyMap( "TwoWayTime", 0, m_topFormation->getTopSurface( ), m_snapshot );

      return m_pressure != 0 and m_temperature != 0 and m_topFormation!=0 and m_depth != 0 and m_seismicVelocity != 0 and m_twoWayTimeTop != 0;
   }
   else{
      m_depth           = 0;
      m_seismicVelocity = 0;
      m_twoWayTimeTop   = 0;

      return m_pressure != 0 and m_temperature != 0;
   }

}


//--------------------------------------------------------------------------------------------------------------------------------------------
//3D CALCULATOR
//--------------------------------------------------------------------------------------------------------------------------------------------


TwoWayTimeVolumeCalculator::TwoWayTimeVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_depth           = 0;
   m_seismicVelocity = 0;
   m_twoWayTimeTop   = 0;
   m_isCalculated    = false;
}

bool TwoWayTimeVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                  OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if (m_isCalculated) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   int k;

   double distance;
   double seismicVelocityBulk;

   double value;
   double undefinedValue;
   Interface::GridMap* TwoWayTimeMap;

   if (not m_depth->isCalculated( )) {

      if (not m_depth->calculate( )) {
         return false;
      }

   }

   if (not m_seismicVelocity->isCalculated( )) {

      if (not m_seismicVelocity->calculate( )) {
         return false;
      }

   }

   if (not m_twoWayTimeTop->isCalculated( )) {

      if (not m_twoWayTimeTop->calculate( )) {
         return false;
      }

   }

   TwoWayTimeMap = propertyValues [ 0 ]->getGridMap ();
   TwoWayTimeMap->retrieveData ();
   undefinedValue = TwoWayTimeMap->getUndefinedValue ();

   for ( i = TwoWayTimeMap->firstI (); i <= TwoWayTimeMap->lastI (); ++i ) {

      for ( j = TwoWayTimeMap->firstJ (); j <= TwoWayTimeMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {

            ///1. Add the twoWayTime from the top
            value = m_twoWayTimeTop->getMapValue( i, j );
            // The top of the formation as the same twoWayTime value than the top surface
            TwoWayTimeMap->setValue( i, j, TwoWayTimeMap->lastK( ), value );

            ///2. For all the nodes in the formation compute the twoWayTime
            for ( k = (TwoWayTimeMap->lastK( )-1); k >= 0; --k ) {
 
               ///2.1 Compute the distance between the nodes
               distance = m_depth->getVolumeValue( i, j, k ) - m_depth->getVolumeValue( i, j, k+1 );
               ///2.2 Compute the harmonic mean of the velocity between the nodes
               seismicVelocityBulk = 2* m_seismicVelocity->getVolumeValue( i, j, k ) * m_seismicVelocity->getVolumeValue( i, j, k+1 );
               seismicVelocityBulk /= (m_seismicVelocity->getVolumeValue( i, j, k ) + m_seismicVelocity->getVolumeValue( i, j, k+1 ));
               ///2.3 Compute the twoWayTime between the nodes
               value += 2 * distance / seismicVelocityBulk;

               TwoWayTimeMap->setValue ( i, j, k, value );
            }

         } else {

            for ( k = TwoWayTimeMap->firstK (); k <= TwoWayTimeMap->lastK (); ++k ) {
               TwoWayTimeMap->setValue ( i, j, k, undefinedValue );
            }

         }

      }

   }

   TwoWayTimeMap->restoreData ();
   m_isCalculated = true;
   return true;
}

void TwoWayTimeVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "TwoWayTime",
                                                                                                            m_snapshot, 0,
                                                                                                            m_formation,
                                                                                                            m_formation->getMaximumNumberOfElements () + 1 )));

}

bool TwoWayTimeVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   m_depth           = PropertyManager::getInstance( ).findOutputPropertyVolume( "Depth", m_formation, m_snapshot );
   m_seismicVelocity = PropertyManager::getInstance( ).findOutputPropertyVolume( "Velocity", m_formation, m_snapshot );
   // Store the TwoWayTime map of the surface at the top of the formation
   m_twoWayTimeTop   = PropertyManager::getInstance( ).findOutputPropertyMap( "TwoWayTime", 0, m_formation->getTopSurface( ), m_snapshot );

   // If we are not at the present day snapshot (t=0Ma): we do not ouput/compute the property.
   // !(*m_snapshot == *presentDaySnapshot) is a temporary coding --> cannot be fixed until derived property library is implemented
   const Interface::Snapshot* presentDaySnapshot = FastcauldronSimulator::getInstance( ).findOrCreateSnapshot( 0.0 );
   assert(( "presentDaySnapshot must be created", presentDaySnapshot != 0 ));
   if (!(*m_snapshot == *presentDaySnapshot)) {
      propertyValues[0]->allowOutput( false );
   }

   return m_depth != 0 and m_seismicVelocity != 0 and m_twoWayTimeTop != 0;
}


//--------------------------------------------------------------------------------------------------------------------------------------------
//RESIDUAL CALCULATOR
//--------------------------------------------------------------------------------------------------------------------------------------------


TwoWayTimeResidualCalculator::TwoWayTimeResidualCalculator( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
m_formation( formation ), m_surface( surface ), m_snapshot( snapshot )
{
   m_isCalculated = false;
}

bool TwoWayTimeResidualCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties,
   OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if (m_isCalculated) {
      return true;
   }

   unsigned int i;
   unsigned int j;

   double value;
   double undefinedValue;
   Interface::GridMap* TwoWayTimeResidualMap;


   if (not m_twoWayTimeCauldron->isCalculated( )) {

      if (not m_twoWayTimeCauldron->calculate( )) {
         return false;
      }

   }

   TwoWayTimeResidualMap = propertyValues[0]->getGridMap( );
   TwoWayTimeResidualMap->retrieveData( );
   // access to data if there is a map
   if (m_twoWayTimeInitial != 0) {
      m_twoWayTimeInitial->retrieveData();
   }
   undefinedValue = TwoWayTimeResidualMap->getUndefinedValue( );

   for (i = TwoWayTimeResidualMap->firstI( ); i <= TwoWayTimeResidualMap->lastI( ); ++i) {

      for (j = TwoWayTimeResidualMap->firstJ( ); j <= TwoWayTimeResidualMap->lastJ( ); ++j) {

         // if the surface is linked to an intial two way time
         if (m_twoWayTimeInitial != 0) {
            if (FastcauldronSimulator::getInstance().nodeIsDefined( i, j )) {

               value = m_twoWayTimeCauldron->getMapValue( i, j ) - m_twoWayTimeInitial->getValue( i, j );
               TwoWayTimeResidualMap->setValue( i, j, value );

            }
            else {
               TwoWayTimeResidualMap->setValue( i, j, undefinedValue );
            }
         }
         else {
            TwoWayTimeResidualMap->setValue( i, j, undefinedValue );
         }

      }

   }

   if (m_twoWayTimeInitial != 0) {
      m_twoWayTimeInitial->restoreData();
   }
   TwoWayTimeResidualMap->restoreData();
   m_isCalculated = true;
   return true;
}

void TwoWayTimeResidualCalculator::allocatePropertyValues( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back( (PropertyValue*)(FastcauldronSimulator::getInstance( ).createMapPropertyValue( "TwoWayTimeResidual",
      m_snapshot, 0,
      0,
      m_surface )) );

}

bool TwoWayTimeResidualCalculator::initialise( OutputPropertyMap::PropertyValueList& propertyValues ) {


   m_twoWayTimeCauldron = PropertyManager::getInstance( ).findOutputPropertyMap( "TwoWayTime", m_formation, m_surface, m_snapshot );
   m_twoWayTimeInitial = m_surface->getInputTwoWayTimeMap( );

   // If there is no 2D outputs, or no initial Two Way Time map linked to the stratigraphic surface,
   // or if we are not at the present day snapshot (t=0Ma): we do not ouput/compute the property.
   // !(*m_snapshot == *presentDaySnapshot) is a temporary coding --> cannot be fixed until derived property library is implemented
   const Interface::Snapshot* presentDaySnapshot = FastcauldronSimulator::getInstance( ).findOrCreateSnapshot( 0.0 );
   assert( ("presentDaySnapshot must be created", presentDaySnapshot != 0) );
   if (FastcauldronSimulator::getInstance( ).getCauldron( )->no2Doutput( ) or !m_twoWayTimeInitial or !(*m_snapshot == *presentDaySnapshot) ) {
      propertyValues[0]->allowOutput( false );
   }

   return m_twoWayTimeCauldron != 0;
}