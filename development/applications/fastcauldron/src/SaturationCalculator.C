#include "SaturationCalculator.h"

#include "EosPack.h"
#include "PetscBlockVector.h"
#include "ElementVolumeGrid.h"
#include "PVTCalculator.h"
#include "Saturation.h"

#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"
#include "MultiComponentFlowHandler.h"


OutputPropertyMap* allocateAverageSaturationCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<AverageSaturationCalculator>( property, formation, surface, snapshot );
}

OutputPropertyMap* allocateAverageSaturationVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<AverageSaturationVolumeCalculator>( property, formation, snapshot );
}

OutputPropertyMap* allocateSaturationCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<SaturationCalculator>( property, formation, surface, snapshot );
}

OutputPropertyMap* allocateSaturationVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<SaturationVolumeCalculator>( property, formation, snapshot );
}


SaturationVolumeCalculator::SaturationVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_isCalculated = false;

}

void SaturationVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   CauldronPropertyValue* phase;

   phase = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "BrineSaturation", 
                                                                                              m_snapshot, 0,
                                                                                              m_formation,
                                                                                              m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( phase );

   phase = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HcLiquidSaturation", 
                                                                                              m_snapshot, 0,
                                                                                              m_formation,
                                                                                              m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( phase );

   phase = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HcVapourSaturation", 
                                                                                              m_snapshot, 0,
                                                                                              m_formation,
                                                                                              m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( phase );

   phase = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "ImmobileSaturation", 
                                                                                              m_snapshot, 0,
                                                                                              m_formation,
                                                                                              m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( phase );

}

bool SaturationVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {


   if ( not FastcauldronSimulator::getInstance ().getMcfHandler ().containsFormation ( m_formation )) {

      OutputPropertyMap::PropertyValueList::iterator iter;

      for ( iter = propertyValues.begin (); iter != propertyValues.end (); ++iter ) {
         (*iter)->allowOutput ( false );
      }

   }


   return true;
}

bool SaturationVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                    OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   unsigned int k;
   unsigned int p;
   double value;

   const ElementVolumeGrid& grid = m_formation->getSaturationGrid ();

   Interface::GridMap* phaseSaturationMaps [ Saturation::NumberOfPhases ];
   PetscBlockVector<Saturation> saturations;

   saturations.setVector ( grid, m_formation->getPhaseSaturationVec (), INSERT_VALUES );

   for ( p = 0; p < Saturation::NumberOfPhases; ++p ) {
      phaseSaturationMaps [ p ] = propertyValues [ p ]->getGridMap ();
      phaseSaturationMaps [ p ]->retrieveData ();
   }

   
   for ( p = 0; p < Saturation::NumberOfPhases; ++p ) {
      Saturation::Phase phaseSaturation = Saturation::Phase ( p );

      for ( i = grid.firstI (); i <= grid.lastI (); ++i ) {

         for ( j = grid.firstJ (); j <= grid.lastJ (); ++j ) {

            if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

               for ( k = grid.firstK (); k <= grid.lastK (); ++k ) {

                  value = saturations ( k, j, i )( phaseSaturation );

                  phaseSaturationMaps [ p ]->setValue ( i, j, k, value );

                  // Fill other heat flow nodes if current (i,j) position is at end of array
                  if ( i == grid.getNumberOfXElements () - 1 ) {
                     phaseSaturationMaps [ p ]->setValue ( i + 1, j, k, value );
                  }

                  if ( j == grid.getNumberOfYElements () - 1 ) {
                     phaseSaturationMaps [ p ]->setValue ( i, j + 1, k, value );
                  }

                  if ( k == grid.getNumberOfZElements () - 1 ) {
                     phaseSaturationMaps [ p ]->setValue ( i, j, k + 1, value );
                  }

                  if ( i == grid.getNumberOfXElements () - 1  and j == grid.getNumberOfYElements () - 1 ) {
                     phaseSaturationMaps [ p ]->setValue ( i + 1, j + 1, k, value );
                  }

                  if ( i == grid.getNumberOfXElements () - 1  and k == grid.getNumberOfZElements () - 1 ) {
                     phaseSaturationMaps [ p ]->setValue ( i + 1, j, k + 1, value );
                  }

                  if ( j == grid.getNumberOfYElements () - 1  and k == grid.getNumberOfZElements () - 1 ) {
                     phaseSaturationMaps [ p ]->setValue ( i, j + 1, k + 1, value );
                  }

                  if ( i == grid.getNumberOfXElements () - 1  and j == grid.getNumberOfYElements () - 1  and k == grid.getNumberOfZElements () - 1 ) {
                     phaseSaturationMaps [ p ]->setValue ( i + 1, j + 1, k + 1, value );
                  }

               }

            }

         }

      }

   }

   for ( p = 0; p < Saturation::NumberOfPhases; ++p ) {
      phaseSaturationMaps [ p ]->restoreData ();
   }

   m_isCalculated = true;
   return true;
}




SaturationCalculator::SaturationCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_isCalculated = false;

}

void SaturationCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   CauldronPropertyValue* phase;

   phase = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "BrineSaturation", 
                                                                                           m_snapshot, 0,
                                                                                           m_formation,
                                                                                           0 ));
   properties.push_back ( phase );

   phase = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "HcLiquidSaturation", 
                                                                                           m_snapshot, 0,
                                                                                           m_formation,
                                                                                           0 ));
   properties.push_back ( phase );

   phase = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "HcVapourSaturation", 
                                                                                           m_snapshot, 0,
                                                                                           m_formation,
                                                                                           0 ));
   properties.push_back ( phase );

   phase = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "ImmobileSaturation", 
                                                                                           m_snapshot, 0,
                                                                                           m_formation,
                                                                                           0 ));
   properties.push_back ( phase );

}

bool SaturationCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {
   return true;
}

bool SaturationCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                              OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   unsigned int k;
   unsigned int p;

   const ElementVolumeGrid& grid = m_formation->getSaturationGrid ();

   Interface::GridMap* phaseSaturationMaps [ Saturation::NumberOfPhases ];

   PetscBlockVector<Saturation> saturations;
   double value;

   saturations.setVector ( grid, m_formation->getPhaseSaturationVec (), INSERT_VALUES );

   for ( p = 0; p < Saturation::NumberOfPhases; ++p ) {
      phaseSaturationMaps [ p ] = propertyValues [ p ]->getGridMap ();
      phaseSaturationMaps [ p ]->retrieveData ();
   }

   
   for ( p = 0; p < Saturation::NumberOfPhases; ++p ) {
      Saturation::Phase phaseSaturation = Saturation::Phase ( p );

      for ( i = grid.firstI (); i <= grid.lastI (); ++i ) {

         for ( j = grid.firstJ (); j <= grid.lastJ (); ++j ) {

            if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {
               value = 0.0;

               for ( k = grid.firstK (); k <= grid.lastK (); ++k ) {
                  value += saturations ( k, j, i )( phaseSaturation );
               }

               // Now divide by the number of elements in z-direciton
               value /= double ( grid.lastK () + 1 );

               value = saturations ( 0, j, i )( phaseSaturation );

               phaseSaturationMaps [ p ]->setValue ( i, j, value );

               // Fill other heat flow nodes if current (i,j) position is at end of array
               if ( i == grid.getNumberOfXElements () - 1 ) {
                  phaseSaturationMaps [ p ]->setValue ( i + 1, j, value );
               }
            
               if ( j == grid.getNumberOfYElements () - 1 ) {
                  phaseSaturationMaps [ p ]->setValue ( i, j + 1, value );
               }

               if ( i == grid.getNumberOfXElements () - 1  and j == grid.getNumberOfYElements () - 1 ) {
                  phaseSaturationMaps [ p ]->setValue ( i + 1, j + 1, value );
               }

            }

         }

      }

   }

   for ( p = 0; p < Saturation::NumberOfPhases; ++p ) {
      phaseSaturationMaps [ p ]->restoreData ();
   }

   m_isCalculated = true;
   return true;
}


//Average calculation

AverageSaturationVolumeCalculator::AverageSaturationVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_isCalculated = false;

}

void AverageSaturationVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   CauldronPropertyValue* phase;

   phase = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "AverageBrineSaturation", 
                                                                                              m_snapshot, 0,
                                                                                              m_formation,
                                                                                              m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( phase );

   phase = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "AverageHcLiquidSaturation", 
                                                                                              m_snapshot, 0,
                                                                                              m_formation,
                                                                                              m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( phase );

   phase = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "AverageHcVapourSaturation", 
                                                                                              m_snapshot, 0,
                                                                                              m_formation,
                                                                                              m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( phase );

   phase = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "AverageImmobileSaturation", 
                                                                                              m_snapshot, 0,
                                                                                              m_formation,
                                                                                              m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( phase );

}

bool AverageSaturationVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {
   return true;
}

bool AverageSaturationVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                    OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   unsigned int k;
   unsigned int p;
   double value;

   const NodalVolumeGrid& grid = m_formation->getNodalVolumeGrid(Saturation::NumberOfPhases);

   Interface::GridMap* phaseSaturationMaps [ Saturation::NumberOfPhases ];
   PetscBlockVector<Saturation> saturations;

   saturations.setVector ( grid, m_formation->getAveragedSaturations (), INSERT_VALUES );

   for ( p = 0; p < Saturation::NumberOfPhases; ++p ) {
      phaseSaturationMaps [ p ] = propertyValues [ p ]->getGridMap ();
      phaseSaturationMaps [ p ]->retrieveData ();
   }

   
   for ( p = 0; p < Saturation::NumberOfPhases; ++p ) {
      Saturation::Phase phaseSaturation = Saturation::Phase ( p );

      for ( i = grid.firstI (); i <= grid.lastI (); ++i ) {

         for ( j = grid.firstJ (); j <= grid.lastJ (); ++j ) {

		   if ( FastcauldronSimulator::getInstance ().nodeIsDefined( i, j )) {

               for ( k = grid.firstK (); k <= grid.lastK (); ++k ) {

                  value = saturations ( k, j, i )( phaseSaturation );

                  phaseSaturationMaps [ p ]->setValue ( i, j, k, value );
                  
               }

            }

         }

      }

   }

   for ( p = 0; p < Saturation::NumberOfPhases; ++p ) {
      phaseSaturationMaps [ p ]->restoreData ();
   }

   m_isCalculated = true;
   return true;
}



//for map
AverageSaturationCalculator::AverageSaturationCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_isCalculated = false;

}

void AverageSaturationCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   CauldronPropertyValue* phase;

   phase = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "AverageBrineSaturation", 
                                                                                           m_snapshot, 0,
                                                                                           m_formation,
                                                                                           0 ));
   properties.push_back ( phase );

   phase = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "AverageHcLiquidSaturation", 
                                                                                           m_snapshot, 0,
                                                                                           m_formation,
                                                                                           0 ));
   properties.push_back ( phase );

   phase = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "AverageHcVapourSaturation", 
                                                                                           m_snapshot, 0,
                                                                                           m_formation,
                                                                                           0 ));
   properties.push_back ( phase );

   phase = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "AverageImmobileSaturation", 
                                                                                           m_snapshot, 0,
                                                                                           m_formation,
                                                                                           0 ));
   properties.push_back ( phase );

}

bool AverageSaturationCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {
   return true;
}

bool AverageSaturationCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                              OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   unsigned int k;
   unsigned int p;

   const NodalVolumeGrid& grid = m_formation->getNodalVolumeGrid (Saturation::NumberOfPhases);

   Interface::GridMap* phaseSaturationMaps [ Saturation::NumberOfPhases ];

   PetscBlockVector<Saturation> saturations;
   double value;

   saturations.setVector ( grid, m_formation->getAveragedSaturations(), INSERT_VALUES );

   for ( p = 0; p < Saturation::NumberOfPhases; ++p ) {
      phaseSaturationMaps [ p ] = propertyValues [ p ]->getGridMap ();
      phaseSaturationMaps [ p ]->retrieveData ();
   }

   
   for ( p = 0; p < Saturation::NumberOfPhases; ++p ) {
      Saturation::Phase phaseSaturation = Saturation::Phase ( p );

      for ( i = grid.firstI (); i <= grid.lastI (); ++i ) {

         for ( j = grid.firstJ (); j <= grid.lastJ (); ++j ) {

            if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {
               value = 0.0;

               for ( k = grid.firstK (); k <= grid.lastK (); ++k ) {
                  value += saturations ( k, j, i )( phaseSaturation );
               }

               // Now divide by the number of elements in z-direciton
               value /= double ( grid.lastK () + 1 );

               value = saturations ( 0, j, i )( phaseSaturation );

               phaseSaturationMaps [ p ]->setValue ( i, j, value );

            }

         }

      }

   }

   for ( p = 0; p < Saturation::NumberOfPhases; ++p ) {
      phaseSaturationMaps [ p ]->restoreData ();
   }

   m_isCalculated = true;
   return true;
}
