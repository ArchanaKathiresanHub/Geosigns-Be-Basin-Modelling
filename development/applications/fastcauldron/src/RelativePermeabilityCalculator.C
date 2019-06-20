#include "RelativePermeabilityCalculator.h"

#include "PetscBlockVector.h"
#include "PVTCalculator.h"

#include "ElementVolumeGrid.h"
#include "DerivedOutputPropertyMap.h"
#include "LayerElement.h"
#include "ElementContributions.h"
#include "Lithology.h"

#include "FastcauldronSimulator.h"
#include "MultiComponentFlowHandler.h"

OutputPropertyMap* allocateRelativePermeabilityCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<RelativePermeabilityCalculator>( property, formation, snapshot );
}


RelativePermeabilityCalculator::RelativePermeabilityCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_isCalculated = false;

}

void RelativePermeabilityCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   CauldronPropertyValue* component;

   component = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "BrineRelativePermeability",
                                                                                                  m_snapshot, 0,
                                                                                                  m_formation,
                                                                                                  m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( component );

   component = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HcLiquidRelativePermeability",
                                                                                                  m_snapshot, 0,
                                                                                                  m_formation,
                                                                                                  m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( component );

   component = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HcVapourRelativePermeability",
                                                                                                  m_snapshot, 0,
                                                                                                  m_formation,
                                                                                                  m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( component );

}

bool RelativePermeabilityCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   if ( not FastcauldronSimulator::getInstance ().getMcfHandler ().containsFormation ( m_formation )) {

      OutputPropertyMap::PropertyValueList::iterator iter;

      for ( iter = propertyValues.begin (); iter != propertyValues.end (); ++iter ) {
         (*iter)->allowOutput ( false );
      }

   }

   return true;
}

bool RelativePermeabilityCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                  OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   unsigned int k;

   const ElementVolumeGrid& grid = m_formation->getVolumeGrid ( Saturation::NumberOfPhases );


   Interface::GridMap* waterRelPermMap;
   Interface::GridMap* liquidRelPermMap;
   Interface::GridMap* vapourRelPermMap;
   double undefinedValue;

   PetscBlockVector<Saturation> layerSaturations;

   waterRelPermMap = propertyValues [ 0 ]->getGridMap ();
   waterRelPermMap->retrieveData ();

   liquidRelPermMap = propertyValues [ 1 ]->getGridMap ();
   liquidRelPermMap->retrieveData ();

   vapourRelPermMap = propertyValues [ 2 ]->getGridMap ();
   vapourRelPermMap->retrieveData ();

   undefinedValue = waterRelPermMap->getUndefinedValue ();

   layerSaturations.setVector ( grid, m_formation->getPhaseSaturationVec (), INSERT_VALUES );

   for ( i = grid.firstI (); i <= grid.lastI (); ++i ) {

      for ( j = grid.firstJ (); j <= grid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = grid.firstK (); k <= grid.lastK (); ++k ) {

               const LayerElement& element = m_formation->getLayerElement ( i, j, k );

               if ( element.isActive ()) {
                  waterRelPermMap->setValue  ( i, j, k, element.getLithology ()->relativePermeability ( Saturation::WATER,  layerSaturations ( k, j, i )));
                  liquidRelPermMap->setValue ( i, j, k, element.getLithology ()->relativePermeability ( Saturation::LIQUID, layerSaturations ( k, j, i )));
                  vapourRelPermMap->setValue ( i, j, k, element.getLithology ()->relativePermeability ( Saturation::VAPOUR, layerSaturations ( k, j, i )));
                  // waterRelPermMap->setValue  ( i, j, k, element.getLithology ()->relativePermeability ( Saturation::WATER,  layerSaturations ( k, j, i )( Saturation::WATER )));
                  // liquidRelPermMap->setValue ( i, j, k, element.getLithology ()->relativePermeability ( Saturation::LIQUID, layerSaturations ( k, j, i )( Saturation::LIQUID )));
                  // vapourRelPermMap->setValue ( i, j, k, element.getLithology ()->relativePermeability ( Saturation::VAPOUR, layerSaturations ( k, j, i )( Saturation::VAPOUR )));
               } else {
                  waterRelPermMap->setValue ( i, j, k, undefinedValue );
                  liquidRelPermMap->setValue ( i, j, k, undefinedValue );
                  vapourRelPermMap->setValue ( i, j, k, undefinedValue );
               }

               // If last element in row or column or ... then copy value to fill array.
               // Since arrays are the same size as the number of nodes.
               if ( i == grid.getNumberOfXElements () - 1 ) {
                  waterRelPermMap->setValue  ( i + 1, j, k, waterRelPermMap->getValue  ( i, j, k ));
                  liquidRelPermMap->setValue ( i + 1, j, k, liquidRelPermMap->getValue ( i, j, k ));
                  vapourRelPermMap->setValue ( i + 1, j, k, vapourRelPermMap->getValue ( i, j, k ));
               }

               if ( j == grid.getNumberOfYElements () - 1 ) {
                  waterRelPermMap->setValue  ( i, j + 1, k, waterRelPermMap->getValue  ( i, j, k ));
                  liquidRelPermMap->setValue ( i, j + 1, k, liquidRelPermMap->getValue ( i, j, k ));
                  vapourRelPermMap->setValue ( i, j + 1, k, vapourRelPermMap->getValue ( i, j, k ));
               }

               if ( k == grid.getNumberOfZElements () - 1 ) {
                  waterRelPermMap->setValue  ( i, j, k + 1, waterRelPermMap->getValue  ( i, j, k ));
                  liquidRelPermMap->setValue ( i, j, k + 1, liquidRelPermMap->getValue ( i, j, k ));
                  vapourRelPermMap->setValue ( i, j, k + 1, vapourRelPermMap->getValue ( i, j, k ));
               }

               if ( i == grid.getNumberOfXElements () - 1 and j == grid.getNumberOfYElements () - 1 ) {
                  waterRelPermMap->setValue  ( i + 1, j + 1, k, waterRelPermMap->getValue  ( i, j, k ));
                  liquidRelPermMap->setValue ( i + 1, j + 1, k, liquidRelPermMap->getValue ( i, j, k ));
                  vapourRelPermMap->setValue ( i + 1, j + 1, k, vapourRelPermMap->getValue ( i, j, k ));
               }

               if ( i == grid.getNumberOfXElements () - 1 and k == grid.getNumberOfZElements () - 1 ) {
                  waterRelPermMap->setValue  ( i + 1, j, k + 1, waterRelPermMap->getValue  ( i, j, k ));
                  liquidRelPermMap->setValue ( i + 1, j, k + 1, liquidRelPermMap->getValue ( i, j, k ));
                  vapourRelPermMap->setValue ( i + 1, j, k + 1, vapourRelPermMap->getValue ( i, j, k ));
               }

               if ( j == grid.getNumberOfYElements () - 1 and k == grid.getNumberOfZElements () - 1 ) {
                  waterRelPermMap->setValue  ( i, j + 1, k + 1, waterRelPermMap->getValue  ( i, j, k ));
                  liquidRelPermMap->setValue ( i, j + 1, k + 1, liquidRelPermMap->getValue ( i, j, k ));
                  vapourRelPermMap->setValue ( i, j + 1, k + 1, vapourRelPermMap->getValue ( i, j, k ));
               }

               if ( i == grid.getNumberOfXElements () - 1 and j == grid.getNumberOfYElements () - 1 and k == grid.getNumberOfZElements () - 1 ) {
                  waterRelPermMap->setValue  ( i + 1, j + 1, k + 1, waterRelPermMap->getValue  ( i, j, k ));
                  liquidRelPermMap->setValue ( i + 1, j + 1, k + 1, liquidRelPermMap->getValue ( i, j, k ));
                  vapourRelPermMap->setValue ( i + 1, j + 1, k + 1, vapourRelPermMap->getValue ( i, j, k ));
               }

            }

         } else {

            for ( k = grid.firstK (); k <= grid.lastK (); ++k ) {
               waterRelPermMap->setValue ( i, j, k, undefinedValue );
               liquidRelPermMap->setValue ( i, j, k, undefinedValue );
               vapourRelPermMap->setValue ( i, j, k, undefinedValue );
            }

         }

      }

   }

   waterRelPermMap->restoreData ();
   liquidRelPermMap->restoreData ();
   vapourRelPermMap->restoreData ();

   m_isCalculated = true;
   // layerSaturations.restoreVector ( UPDATE_EXCLUDING_GHOSTS );

   return true;
}
