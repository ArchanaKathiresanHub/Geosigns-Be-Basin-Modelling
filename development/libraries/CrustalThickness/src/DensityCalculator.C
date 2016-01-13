//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "DensityCalculator.h"

// DataAccess library
#include "Interface/CrustFormation.h"
#include "Interface/Formation.h"
#include "Interface/LithoType.h"
#include "Interface/ObjectFactory.h"
#include "Interface/Property.h"
#include "Interface/ProjectHandle.h"
#include "Interface/PropertyValue.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"

// std library
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>

// @todo must already be defined somewhere else
const double GRAVITY = 9.81;

//------------------------------------------------------------//
DensityCalculator::DensityCalculator() {

   m_waterBottomDepthValue = 0.0;
   m_sedimentThickness     = 0.0;
   m_topBasementDepthValue = 0.0;
   m_WLS                   = 0.0;
   m_backstrip             = 0.0;
   m_sedimentDensity       = 0.0;

   m_bottomOfSedimentSurface = 0;
   m_topOfSedimentSurface    = 0;

   m_densityTerm                = 1.0;
   m_backstrippingMantleDensity = 0.0;
   m_waterDensity               = 0.0;
}

void DensityCalculator::loadTopAndBottomOfSediments( GeoPhysics::ProjectHandle* projectHandle, const double snapshotAge, const string & baseSurfaceName ) {

   bool debug = false;

   const Interface::Snapshot * currentSnapshot = projectHandle->findSnapshot( snapshotAge, MINOR | MAJOR );

   //1. Find the bottom of the sediment
   if (baseSurfaceName == "") {

      const Interface::CrustFormation * formationCrust = dynamic_cast<const Interface::CrustFormation *>(projectHandle->getCrustFormation());

      if (!formationCrust) {
         stringstream ss;
         ss << "Could not find Crust formation at the age " << currentSnapshot->getTime();
         throw ss.str();
      }
      m_bottomOfSedimentSurface = formationCrust->getTopSurface();

      if (debug && projectHandle->getRank() == 0) {
         cout << "Crust formation: " << formationCrust->getName() << ", surface above " << m_bottomOfSedimentSurface->getName() << "." << endl;
      }
   }
   else {
      m_bottomOfSedimentSurface = projectHandle->findSurface( baseSurfaceName );

      if (!m_bottomOfSedimentSurface) {
         stringstream ss;
         ss << "Could not find user defined base surface of the rift event: " << baseSurfaceName;
         throw ss.str();
      }
      else {
         if (false && projectHandle->getRank() == 0) {
            printf( "Using surface %s as the base of syn-rift\n", m_bottomOfSedimentSurface->getName().c_str() );
         }
      }
   }

   //2. Find the top of the sediment
   Interface::FormationList * myFormations = projectHandle->getFormations( currentSnapshot, true );
   const Interface::Formation * formationWB = (*myFormations)[0]; // find Water bottom

   if (!formationWB) {
      stringstream ss;
      ss << "Could not find the Water bottom formation at the age " << currentSnapshot->getTime();
      throw ss.str();
   }

   m_topOfSedimentSurface = formationWB->getTopSurface();

   if (debug && projectHandle->getRank() == 0) {
      cout << "Top surface: " << m_topOfSedimentSurface->getName() << "; surface below " << m_bottomOfSedimentSurface->getName() << "." << endl;
   }
}

//------------------------------------------------------------//
const DataModel::AbstractProperty* DensityCalculator::loadDepthProperty( GeoPhysics::ProjectHandle* projectHandle, const double snapshotAge) {

   DerivedProperties::DerivedPropertyManager derivedManager( projectHandle );
   const Interface::Snapshot * currentSnapshot = projectHandle->findSnapshot( snapshotAge, MINOR | MAJOR );

   const DataModel::AbstractProperty* depthProperty = derivedManager.getProperty( "Depth" );

   if (!depthProperty) {
      string s = "Could not find property named Depth.";
      throw s;
   }

   return depthProperty;
}

//------------------------------------------------------------//
void DensityCalculator::loadDepthData( GeoPhysics::ProjectHandle* projectHandle, const DataModel::AbstractProperty* depthProperty, const double snapshotAge ) {

   DerivedProperties::DerivedPropertyManager derivedManager( projectHandle );
   const Interface::Snapshot * currentSnapshot = projectHandle->findSnapshot( snapshotAge, MINOR | MAJOR );

   // Find the depth property of the bottom of sediment
   m_depthBasement = derivedManager.getSurfaceProperty( depthProperty, currentSnapshot, m_bottomOfSedimentSurface );
   // Find the depth property of the top of sediment
   m_depthWaterBottom = derivedManager.getSurfaceProperty( depthProperty, currentSnapshot, m_topOfSedimentSurface );
}

//------------------------------------------------------------//
const DataModel::AbstractProperty* DensityCalculator::loadPressureProperty( GeoPhysics::ProjectHandle* projectHandle, const double snapshotAge ) {

   DerivedProperties::DerivedPropertyManager derivedManager( projectHandle );

   const Interface::Snapshot * zeroSnapshot = projectHandle->findSnapshot( 0 );
   const DataModel::AbstractProperty* pressureProperty = derivedManager.getProperty( "LithoStaticPressure" );

   if (!pressureProperty) {
      string s = "Could not find property named LithoStaticPressure.";
      throw s;
   }
   
   return pressureProperty;
}

//------------------------------------------------------------//
void DensityCalculator::loadPressureData( GeoPhysics::ProjectHandle* projectHandle, const DataModel::AbstractProperty* pressureProperty, const double snapshotAge ) {

   DerivedProperties::DerivedPropertyManager derivedManager( projectHandle );
   const Interface::Snapshot * currentSnapshot = projectHandle->findSnapshot (snapshotAge, MINOR | MAJOR);
 
   // Find the pressure property of the bottom of sediment
   m_pressureBasement = derivedManager.getSurfaceProperty( pressureProperty, currentSnapshot, m_bottomOfSedimentSurface );
   // Find the pressure property of the top of sediment
   m_pressureWaterBottom = derivedManager.getSurfaceProperty( pressureProperty, currentSnapshot, m_topOfSedimentSurface );

}

//------------------------------------------------------------//
void DensityCalculator::loadSnapshots( Interface::ProjectHandle* projectHandle ) {
   
   Interface::FormationList* formations = projectHandle->getFormations ( );
   Interface::FormationList::const_iterator formationIter;

   for ( formationIter = formations->begin (); formationIter != formations->end (); ++formationIter ) {
      const Interface::Formation * formation = ( *formationIter );

      if ( formation != 0 ) {
         const Interface::Surface * topSurface = formation->getBottomSurface();
         m_snapshots.push_back( topSurface->getSnapshot()->getTime() );
      }
   }
   m_snapshots.push_back( 0.0 ); // add present day
}

//------------------------------------------------------------//
void DensityCalculator::retrieveData() {

   m_depthBasement      ->retrieveData ();
   m_depthWaterBottom   ->retrieveData ();
   m_pressureBasement   ->retrieveData ();
   m_pressureWaterBottom->retrieveData ();
}

//------------------------------------------------------------//
void DensityCalculator::restoreData() {

   m_depthBasement      ->restoreData ();
   m_depthWaterBottom   ->restoreData ();
   m_pressureBasement   ->restoreData ();
   m_pressureWaterBottom->restoreData ();
}

//------------------------------------------------------------//
void DensityCalculator::computeNode( unsigned int i, unsigned int j ) {

 
   m_topBasementDepthValue = m_depthBasement->get (i, j) ;
   m_waterBottomDepthValue = m_depthWaterBottom->get (i, j);

   if((m_topBasementDepthValue != m_depthBasement->getUndefinedValue()) &&   
      (m_waterBottomDepthValue != m_depthWaterBottom->getUndefinedValue())) {
   
      m_sedimentThickness = m_topBasementDepthValue - m_waterBottomDepthValue;
   } else {
      m_sedimentThickness = Interface::DefaultUndefinedMapValue;
   }

   double pressureTopBasementValue = m_pressureBasement->get (i, j);
   double pressureWaterBottomValue = m_pressureWaterBottom->get (i, j);

   m_sedimentDensity = Interface::DefaultUndefinedMapValue;
   m_backstrip = Interface::DefaultUndefinedMapValue;

   if((pressureTopBasementValue != m_pressureBasement->getUndefinedValue()) &&
      (pressureWaterBottomValue != m_pressureWaterBottom->getUndefinedValue()) &&
      (m_sedimentThickness != Interface::DefaultUndefinedMapValue)) {
   
      m_backstrip =  m_sedimentThickness *  m_backstrippingMantleDensity * m_densityTerm - 
         ((( pressureTopBasementValue - pressureWaterBottomValue ) * 1e6 ) / GRAVITY ) * m_densityTerm;

      // Integrated sediment density calculated across grid using pressure at WaterBottom and TopBasement surfaces
      if( m_sedimentThickness != 0.0 ) {
         m_sedimentDensity = ((pressureTopBasementValue - pressureWaterBottomValue) * 1e6 ) / (GRAVITY * m_sedimentThickness);
      } 
   }
   m_WLS = Interface::DefaultUndefinedMapValue;

   if( m_waterBottomDepthValue != m_depthWaterBottom->getUndefinedValue() && m_backstrip != Interface::DefaultUndefinedMapValue ) {
      
      if( m_waterBottomDepthValue  >= 0.0 ) {
         m_WLS = m_waterBottomDepthValue + m_backstrip;
      } else {
         m_WLS = m_waterBottomDepthValue * m_backstrippingMantleDensity * m_densityTerm + m_backstrip;
      }
      if(m_WLS < 0 ) {
         m_WLS = 0.0;
      }
   } 
}

//------------------------------------------------------------//
bool DensityCalculator::setDensities( const double aMantleDensity, const double aWaterDensity ) {

   m_backstrippingMantleDensity = aMantleDensity;
   m_waterDensity = aWaterDensity;
   if(( m_waterDensity - m_backstrippingMantleDensity ) != 0.0 ) {
      m_densityTerm = 1.0 / ( m_backstrippingMantleDensity - m_waterDensity );
      return true;
   } 
   return false;
}