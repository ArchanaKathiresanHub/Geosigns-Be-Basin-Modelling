//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include "MaxVesCalculator.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"

#include "Interface/RunParameters.h"

// utilities library
#include "ConstantsNumerical.h"
using Utilities::Numerical::CauldronNoDataValue;
using Utilities::Numerical::IbsNoDataValue;

OutputPropertyMap* allocateMaxVesCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<MaxVesCalculator>( property, formation, surface, snapshot );
}

OutputPropertyMap* allocateMaxVesVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<MaxVesVolumeCalculator>( property, formation, snapshot );
}

MaxVesCalculator::MaxVesCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_isCalculated = false;

}

bool MaxVesCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                    OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }
   
   bool useRealThickness = FastcauldronSimulator::getInstance ().getRunParameters ()->getNonGeometricLoop () and
      (FastcauldronSimulator::getInstance ().getCauldron ()->DoOverPressure or FastcauldronSimulator::getInstance ().getCauldron ()-> Do_Iteratively_Coupled );

   double age;
   unsigned int i;
   unsigned int j;
   int k;
   int usableK;
   double value;
   double undefinedValue;

   Interface::GridMap* maxVesMap;
   double ***maxVesVector;

   if ( FastcauldronSimulator::getInstance ().getCauldron()->no2Doutput()) {
      propertyValues [ 0 ]->allowOutput( false );
   }

   maxVesMap = propertyValues [ 0 ]->getGridMap ();
   maxVesMap->retrieveData ();
   undefinedValue = maxVesMap->getUndefinedValue ();

   DMDAVecGetArray( m_formation->layerDA,
                    *m_formation->vectorList.VecArray [ MAXVES ],
                    &maxVesVector );
                     
   if ( m_snapshot != 0 ) {
      age = m_snapshot->getTime ();
   } else {
      // If no snap-shot is defined then present-day MaxVes is saved.
      age = 0.0;
   }

   for ( i = maxVesMap->firstI (); i <= maxVesMap->lastI (); ++i ) {

      for ( j = maxVesMap->firstJ (); j <= maxVesMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {

            usableK = 0;        		
            if ( m_surface == m_formation->getTopSurface ()) {
               for ( k = int ( m_formation->getMaximumNumberOfElements ()) - 1; k >= 0; --k ) {
                  double thickness = ( useRealThickness ? m_formation->getRealThickness ( i, j, k, age ) :  m_formation->getSolidThickness ( i, j, k, age ));

                  if ( thickness != IbsNoDataValue and thickness > 0 ) {
                     usableK = k + 1;
                     break;
                  }
               }

            } else {
               usableK = 0;
            }
            value = maxVesVector [ usableK ][ j ][ i ];
            maxVesMap->setValue ( i, j, value );
            
         } else {
            maxVesMap->setValue ( i, j, undefinedValue );
         }
      }
   }

   maxVesMap->restoreData ();
   DMDAVecRestoreArray ( m_formation->layerDA,
                         *m_formation->vectorList.VecArray [ MAXVES ],
                         &maxVesVector );

   m_isCalculated = true;
   
   return true;
}

void MaxVesCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   if ( FastcauldronSimulator::getInstance ().getCalculationMode () == HYDROSTATIC_HIGH_RES_DECOMPACTION_MODE or
        FastcauldronSimulator::getInstance ().getCalculationMode () == COUPLED_HIGH_RES_DECOMPACTION_MODE ) {
      properties.push_back ((PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "MaxVesHighRes",
                                                                                                            m_snapshot, 0, 
                                                                                                            m_formation,
                                                                                                            m_surface )));
   } else {
      properties.push_back ((PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "MaxVesVec2",
                                                                                                            m_snapshot, 0, 
                                                                                                            m_formation,
                                                                                                            m_surface )));
   }

}

bool MaxVesCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   if ( m_formation->getTopSurface () == m_surface ) {
      // Number-of-nodes = number-of-elements + 1.
      // But C array indexing starts a 0, so 1 must be subtracted.
      m_kIndex = m_formation->getMaximumNumberOfElements ();
   } else if ( m_formation->getBottomSurface () == m_surface ) {
      m_kIndex = 0;
   } else {
      assert ( false );
      // Error
   }

   return true;
}


MaxVesVolumeCalculator::MaxVesVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_isCalculated = false;

}

bool MaxVesVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                          OutputPropertyMap::PropertyValueList&  propertyValues ) {

   unsigned int i;
   unsigned int j;
   unsigned int k;
   double value;
   Interface::GridMap* propertyMap;
   double ***propertyVector;

   propertyMap = propertyValues [ 0 ]->getGridMap ();
   propertyMap->retrieveData ();

   DMDAVecGetArray( m_formation->layerDA,
                    *m_formation->vectorList.VecArray [ MAXVES ],
                    &propertyVector );
                    
   for ( i = propertyMap->firstI (); i <= propertyMap->lastI (); ++i ) {

      for ( j = propertyMap->firstJ (); j <= propertyMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j ) ) {

            for ( k = propertyMap->firstK (); k <= propertyMap->lastK (); ++k ) {
               propertyMap->setValue ( i, j, k, propertyVector [ k ][ j ][ i ] );
            }

         } else {

            for ( k = propertyMap->firstK (); k <= propertyMap->lastK (); ++k ) {
               propertyMap->setValue ( i, j, k, CauldronNoDataValue );
            }

         }

      }

   }

   // The map has to be restored in case of any communicating of data is required.
   propertyMap->restoreData ();

   DMDAVecRestoreArray ( m_formation->layerDA,
                         *m_formation->vectorList.VecArray [ MAXVES ],
                         &propertyVector );

   return true;
}

void MaxVesVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "MaxVes", 
                                                                                                            m_snapshot, 0, 
                                                                                                            m_formation,
                                                                                                            m_formation->getMaximumNumberOfElements () + 1 )));

}

bool MaxVesVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {
   return true;
}






