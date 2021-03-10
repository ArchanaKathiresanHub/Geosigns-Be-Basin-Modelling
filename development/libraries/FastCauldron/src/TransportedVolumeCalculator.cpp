#include "TransportedVolumeCalculator.h"

#include "EosPack.h"
#include "PetscBlockVector.h"
#include "ElementVolumeGrid.h"
#include "ComponentManager.h"
#include "PVTCalculator.h"

#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"

#include "LayerElement.h"
#include "ElementContributions.h"



OutputPropertyMap* allocateTransportedVolumeCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<TransportedVolumeCalculator>( property, formation, snapshot );
}


TransportedVolumeCalculator::TransportedVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_isCalculated = false;

}

void TransportedVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   CauldronPropertyValue* component;

   component = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "TransportedMass",
                                                                                                  m_snapshot, 0,
                                                                                                  m_formation,
                                                                                                  m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( component );

}

bool TransportedVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {
   return true;
}

bool TransportedVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                     OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   double temperature;
   double pressure;

   unsigned int i;
   unsigned int j;
   unsigned int k;
   unsigned int c;

   const ElementVolumeGrid& grid = m_formation->getVolumeGrid ( 1 );

   Interface::GridMap* transportedMassesMap;

   PetscBlockVector<double> layerTransportedMasses;
   double elementVolumeTransported;

   layerTransportedMasses.setVector ( grid, m_formation->getTransportedMassesVec (), INSERT_VALUES );


   transportedMassesMap = propertyValues [ 0 ]->getGridMap ();
   transportedMassesMap->retrieveData ();


   for ( i = grid.firstI (); i <= grid.lastI (); ++i ) {

      for ( j = grid.firstJ (); j <= grid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = grid.firstK (); k <= grid.lastK (); ++k ) {

               elementVolumeTransported = layerTransportedMasses ( k, j, i );

               transportedMassesMap->setValue ( i, j, k, elementVolumeTransported );

               // If last element in row or column or ... then copy value to fill array.
               // Since arrays are the same size as the number of nodes.
               if ( i == grid.getNumberOfXElements () - 1 ) {
                  transportedMassesMap->setValue ( i + 1, j, k, elementVolumeTransported );
               }

               if ( j == grid.getNumberOfYElements () - 1 ) {
                  transportedMassesMap->setValue ( i, j + 1, k, elementVolumeTransported );
               }

               if ( k == grid.getNumberOfZElements () - 1 ) {
                  transportedMassesMap->setValue ( i, j, k + 1, elementVolumeTransported );
               }

               if ( i == grid.getNumberOfXElements () - 1 and j == grid.getNumberOfYElements () - 1 ) {
                  transportedMassesMap->setValue ( i + 1, j + 1, k, elementVolumeTransported );
               }

               if ( i == grid.getNumberOfXElements () - 1 and k == grid.getNumberOfZElements () - 1 ) {
                  transportedMassesMap->setValue ( i + 1, j, k + 1, elementVolumeTransported );
               }

               if ( j == grid.getNumberOfYElements () - 1 and k == grid.getNumberOfZElements () - 1 ) {
                  transportedMassesMap->setValue ( i, j + 1, k + 1, elementVolumeTransported );
               }

               if ( i == grid.getNumberOfXElements () - 1 and j == grid.getNumberOfYElements () - 1 and k == grid.getNumberOfZElements () - 1 ) {
                  transportedMassesMap->setValue ( i + 1, j + 1, k + 1, elementVolumeTransported );
               }

            }

         }

      }

   }

   transportedMassesMap->restoreData ();

   m_isCalculated = true;
   return true;
}



