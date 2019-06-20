#include "ErosionFactorCalculator.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"

#include "RunParameters.h"

OutputPropertyMap* allocateErosionFactorCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<ErosionFactorCalculator>( property, formation, surface, snapshot );
}

ErosionFactorCalculator::ErosionFactorCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_isCalculated = false;

}

bool ErosionFactorCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& , 
                                                 OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   double currentAge = m_snapshot->getTime ();
   double depoAge = m_formation->depoage;

   bool isNonGeometricLoop = ( FastcauldronSimulator::getInstance ().getCalculationMode () == OVERPRESSURE_MODE or
                               FastcauldronSimulator::getInstance ().getCalculationMode () == OVERPRESSURED_TEMPERATURE_MODE or 
                               FastcauldronSimulator::getInstance ().getCalculationMode () == PRESSURE_AND_TEMPERATURE_MODE or 
                               FastcauldronSimulator::getInstance ().getCalculationMode () == COUPLED_DARCY_MODE ) and                               
                              FastcauldronSimulator::getInstance ().getRunParameters ()->getNonGeometricLoop ();
   unsigned int i;
   unsigned int j;
   int k;
   int zStart;
   int zCount;
   double undefinedValue;
   double value;
   double depositionThickness;
   double currentThickness;
   Interface::GridMap* erosionFactorMap;

   erosionFactorMap = propertyValues [ 0 ]->getGridMap ();
   erosionFactorMap->retrieveData ();
   undefinedValue = erosionFactorMap->getUndefinedValue ();

   DMDAGetCorners ( m_formation->layerDA, PETSC_NULL, PETSC_NULL, &zStart, PETSC_NULL, PETSC_NULL, &zCount );

   for ( i = erosionFactorMap->firstI (); i <= erosionFactorMap->lastI (); ++i ) {

      for ( j = erosionFactorMap->firstJ (); j <= erosionFactorMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {

            if ( currentAge < depoAge ) {
               depositionThickness = 0.0;
               currentThickness = 0.0;

               if ( isNonGeometricLoop ) {

                  for ( k = zStart; k < zStart + zCount - 1; ++k ) {
                     depositionThickness += m_formation->getRealThickness ( i, j, k, depoAge );
                     currentThickness += m_formation->getRealThickness ( i, j, k, currentAge );
                  }

               } else {

                  for ( k = zStart; k < zStart + zCount - 1; ++k ) {
                     depositionThickness += m_formation->getSolidThickness ( i, j, k, depoAge );
                     currentThickness += m_formation->getSolidThickness ( i, j, k, currentAge );
                  }

               }

               if ( depositionThickness == 0.0 ) {
                  value = 1.0;
               } else {
                  value = currentThickness / depositionThickness;
               } 

            } else {
               depositionThickness = 1.0;
               currentThickness = 1.0;
               value = 1.0;
            } 


            erosionFactorMap->setValue ( i, j, value );
         } else {
            erosionFactorMap->setValue ( i, j, undefinedValue );
         }

      }

   }

   erosionFactorMap->restoreData ();
   m_isCalculated = true;
   return true;
}

void ErosionFactorCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "ErosionFactor", 
                                                                                                         m_snapshot, 0, 
                                                                                                         m_formation,
                                                                                                         0 )));

}

bool ErosionFactorCalculator::initialise ( OutputPropertyMap::PropertyValueList& ) {
   return true;
}
