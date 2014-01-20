#include "InvasionCapillaryEntryPressureCalculator.h"
#include "CompoundLithology.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"
#include "MultiComponentFlowHandler.h"
#include "element_contributions.h"
#include "FiniteElementTypes.h"

#include "LayerElement.h"
#include "PetscBlockVector.h"
#include "Saturation.h"
#include "ElementContributions.h"
#include "Lithology.h"

#include "Interface/RunParameters.h"
#include "Interface/Interface.h"
#include "consts.h"

#include "timefilter.h"
#include "BrooksCorey.h"

#define DEBUG

using namespace FiniteElementMethod;
using namespace pvtFlash;


OutputPropertyMap* allocateInvasionCapillaryEntryPressureVolumeCalculator ( const ::PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<InvasionCapillaryEntryPressureVolumeCalculator>( property, formation, snapshot );
}


//volume calculator 
//constructor
InvasionCapillaryEntryPressureVolumeCalculator::InvasionCapillaryEntryPressureVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {
   m_isCalculated = false;
}

InvasionCapillaryEntryPressureVolumeCalculator::~InvasionCapillaryEntryPressureVolumeCalculator() {
}

void InvasionCapillaryEntryPressureVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {


   PropertyValue* phase;

   phase = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "InvasionVapourCapillaryEntryPressureCift", 
                                                                                              m_snapshot, 0,
                                                                                              m_formation,
                                                                                              m_formation->getMaximumNumberOfElements () + 1 )); 
   properties.push_back ( phase );

   phase = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "InvasionLiquidCapillaryEntryPressureCift", 
                                                                                              m_snapshot, 0,
                                                                                              m_formation,
                                                                                              m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( phase );

   phase = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "InvasionVapourCapillaryEntryPressure", 
                                                                                              m_snapshot, 0,
                                                                                              m_formation,
                                                                                              m_formation->getMaximumNumberOfElements () + 1 )); 
   properties.push_back ( phase );

   phase = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "InvasionLiquidCapillaryEntryPressure", 
                                                                                              m_snapshot, 0,
                                                                                              m_formation,
                                                                                              m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( phase );


}

bool InvasionCapillaryEntryPressureVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   if ( not FastcauldronSimulator::getInstance ().getMcfHandler ().saveEntryPressureAtInvasion () or
        not FastcauldronSimulator::getInstance ().getMcfHandler ().containsFormation ( m_formation )) {

      OutputPropertyMap::PropertyValueList::iterator iter;

      for ( iter = propertyValues.begin (); iter != propertyValues.end (); ++iter ) {
         (*iter)->allowOutput ( false );
      }

   }

   return true;
}

bool InvasionCapillaryEntryPressureVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                                  OutputPropertyMap::PropertyValueList&  propertyValues )
{

   const int HcVapourDensityIndex = 0;
   const int HcLiquidDensityIndex = 1;
  
   using namespace CBMGenerics;

   if (m_isCalculated)
   {
      return true;
   }

   unsigned int i;
   unsigned int j;
   unsigned int k;

   // Vapour invasion entry pressure with constant interfacial tension.
   double viepcift;
   
   // Liquid invasion entry pressure with constant interfacial tension.
   double liepcift;
   
   // Vapour invasion entry pressure with computed interfacial tension.
   double viep;
   
   // Liquid invasion entry pressure with coputed interfacial tension.
   double liep;

   double undefinedValue;

   Interface::GridMap * liquidInvasionEntryPressureCift;
   Interface::GridMap * vapourInvasionEntryPressureCift;

   Interface::GridMap * liquidInvasionEntryPressure;
   Interface::GridMap * vapourInvasionEntryPressure;

   const ElementVolumeGrid& grid = m_formation->getVolumeGrid ( pvtFlash::N_PHASES );
 
   //get vapour phase invasion entry pressure
   vapourInvasionEntryPressureCift = propertyValues[0]->getGridMap ();
   vapourInvasionEntryPressureCift->retrieveData ();

   //get liquid phase invasion entry pressure
   liquidInvasionEntryPressureCift = propertyValues[1]->getGridMap ();
   liquidInvasionEntryPressureCift->retrieveData ();
   
   //get vapour phase invasion entry pressure
   vapourInvasionEntryPressure = propertyValues[2]->getGridMap ();
   vapourInvasionEntryPressure->retrieveData ();

   //get liquid phase invasion entry pressure
   liquidInvasionEntryPressure = propertyValues[3]->getGridMap ();
   liquidInvasionEntryPressure->retrieveData ();
   
   undefinedValue = vapourInvasionEntryPressureCift->getUndefinedValue ();
   
   PetscBlockVector<PVTPhaseValues> invasionEntryPressureCift;
   PetscBlockVector<PVTPhaseValues> invasionEntryPressure;

   invasionEntryPressureCift.setVector ( grid, m_formation->getEntryPressureBeforeInvasionVec (), INSERT_VALUES );
   invasionEntryPressure.setVector    ( grid, m_formation->getEntryPressureAfterInvasionVec  (), INSERT_VALUES );

   for ( i = grid.firstI (); i <= grid.lastI (); ++i ) {

      for ( j = grid.firstJ (); j <= grid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = grid.firstK (); k <= grid.lastK (); ++k ) {

               viepcift = invasionEntryPressureCift ( k, j, i )( pvtFlash::VAPOUR_PHASE );
               liepcift = invasionEntryPressureCift ( k, j, i )( pvtFlash::LIQUID_PHASE );

               viep = invasionEntryPressure ( k, j, i )( pvtFlash::VAPOUR_PHASE );
               liep = invasionEntryPressure ( k, j, i )( pvtFlash::LIQUID_PHASE );

               liquidInvasionEntryPressureCift->setValue ( i, j, k, liepcift );
               vapourInvasionEntryPressureCift->setValue ( i, j, k, viepcift );

               liquidInvasionEntryPressure->setValue ( i, j, k, liep );
               vapourInvasionEntryPressure->setValue ( i, j, k, viep );


               // adjust for element and node based difference
               // If last element in row or column or ... then copy value to fill array.
               // Since arrays are the same size as the number of nodes.
               if ( i == grid.getNumberOfXElements () - 1 ) {
                  liquidInvasionEntryPressureCift->setValue ( i + 1, j, k, liepcift );
                  vapourInvasionEntryPressureCift->setValue ( i + 1, j, k, viepcift );

                  liquidInvasionEntryPressure->setValue ( i + 1, j, k, liep );
                  vapourInvasionEntryPressure->setValue ( i + 1, j, k, viep );
               }

               if ( j == grid.getNumberOfYElements () - 1 ) {
                  liquidInvasionEntryPressureCift->setValue ( i, j + 1, k, liepcift );
                  vapourInvasionEntryPressureCift->setValue ( i, j + 1, k, viepcift );

                  liquidInvasionEntryPressure->setValue ( i, j + 1, k, liep );
                  vapourInvasionEntryPressure->setValue ( i, j + 1, k, viep );
               }

               if ( k == grid.getNumberOfZElements () - 1 ) {
                  liquidInvasionEntryPressureCift->setValue ( i, j, k + 1, liepcift );
                  vapourInvasionEntryPressureCift->setValue ( i, j, k + 1, viepcift );

                  liquidInvasionEntryPressure->setValue ( i, j, k + 1, liep );
                  vapourInvasionEntryPressure->setValue ( i, j, k + 1, viep );
               }

               if ( i == grid.getNumberOfXElements () - 1 and j == grid.getNumberOfYElements () - 1 ) {
                  liquidInvasionEntryPressureCift->setValue ( i + 1, j + 1, k, liepcift );
                  vapourInvasionEntryPressureCift->setValue ( i + 1, j + 1, k, viepcift );

                  liquidInvasionEntryPressure->setValue ( i + 1, j + 1, k, liep );
                  vapourInvasionEntryPressure->setValue ( i + 1, j + 1, k, viep );
               }

               if ( i == grid.getNumberOfXElements () - 1 and k == grid.getNumberOfZElements () - 1 ) {
                  liquidInvasionEntryPressureCift->setValue ( i + 1, j, k + 1, liepcift );
                  vapourInvasionEntryPressureCift->setValue ( i + 1, j, k + 1, viepcift );

                  liquidInvasionEntryPressure->setValue ( i + 1, j, k + 1, liep );
                  vapourInvasionEntryPressure->setValue ( i + 1, j, k + 1, viep );
               }

               if ( j == grid.getNumberOfYElements () - 1 and k == grid.getNumberOfZElements () - 1 ) {
                  liquidInvasionEntryPressureCift->setValue ( i, j + 1, k + 1, liepcift );
                  vapourInvasionEntryPressureCift->setValue ( i, j + 1, k + 1, viepcift );

                  liquidInvasionEntryPressure->setValue ( i, j + 1, k + 1, liep );
                  vapourInvasionEntryPressure->setValue ( i, j + 1, k + 1, viep );
               }

               if ( i == grid.getNumberOfXElements () - 1 and j == grid.getNumberOfYElements () - 1 and k == grid.getNumberOfZElements () - 1 ) {
                  liquidInvasionEntryPressureCift->setValue ( i + 1, j + 1, k + 1, liepcift );
                  vapourInvasionEntryPressureCift->setValue ( i + 1, j + 1, k + 1, viepcift );

                  liquidInvasionEntryPressure->setValue ( i + 1, j + 1, k + 1, liep );
                  vapourInvasionEntryPressure->setValue ( i + 1, j + 1, k + 1, viep );
               }

            }

         } else {

            for ( k = liquidInvasionEntryPressure->firstK (); k <= liquidInvasionEntryPressure->lastK (); ++k ) {
               liquidInvasionEntryPressureCift->setValue(i,j,k,undefinedValue);
               vapourInvasionEntryPressureCift->setValue(i,j,k,undefinedValue);

               liquidInvasionEntryPressure->setValue(i,j,k,undefinedValue);
               vapourInvasionEntryPressure->setValue(i,j,k,undefinedValue);
            }

         }

      }

   }


   liquidInvasionEntryPressureCift->restoreData ();
   vapourInvasionEntryPressureCift->restoreData ();

   liquidInvasionEntryPressure->restoreData ();
   vapourInvasionEntryPressure->restoreData ();

   m_isCalculated = true;

   return true;
}
