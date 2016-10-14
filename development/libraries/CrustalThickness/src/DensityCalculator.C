//                                                                      
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "DensityCalculator.h"

#include "Interface/Snapshot.h"
#include "Interface/Surface.h"
#include "Interface/Formation.h"
#include "Interface/CrustFormation.h"
#include "Interface/LithoType.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"
#include "Interface/ProjectHandle.h"
#include "Interface/ObjectFactory.h"

#include "LithostaticPressureFormationCalculator.h"

// std library
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>

// utilities library
#include "ConstantsPhysics.h"
using Utilities::Physics::AccelerationDueToGravity;

//------------------------------------------------------------//
DensityCalculator::DensityCalculator() {

   m_depthBasementMap = 0; 
   m_depthWaterBottomMap = 0; 

   m_sedimentDensity = 0.0;
   m_waterBottomDepthValue = 0.0;

   m_sedimentThickness = 0.0;
   m_topBasementDepthValue = 0.0;
} 

//------------------------------------------------------------//
void DensityCalculator::loadData( GeoPhysics::ProjectHandle* projectHandle, const string & baseSurfaceName ) {

   bool debug = false;

   PropertyManager derivedManager (projectHandle);

   const Interface::Snapshot * zeroSnapshot = projectHandle->findSnapshot (0);
   const DataModel::AbstractProperty* pressureProperty = derivedManager.getProperty ( "LithoStaticPressure" );
  
   if (!pressureProperty) {
      string s = "Could not find property named LithoStaticPressure.";
      throw s;
   }

   // Find the depth of the bottom of sediment
   const Interface::Surface * bottomOfSedimentSurface;

   if( baseSurfaceName == "" ) {

      const Interface::CrustFormation * formationCrust = dynamic_cast<const Interface::CrustFormation *>(projectHandle->getCrustFormation ());
      
      if (!formationCrust) {
         string s = "Could not find Crust Formation.";
         throw s;
      }
      bottomOfSedimentSurface = formationCrust->getTopSurface();
      
      if(debug && projectHandle->getRank() == 0) {
         cout << "Crust formation: " << formationCrust->getName() << ", surface above " << bottomOfSedimentSurface->getName() << "." << endl;
      }
      //cout << "Take the default one : " << bottomOfSedimentSurface->getName()  << endl;
   } else {
      bottomOfSedimentSurface = projectHandle->findSurface ( baseSurfaceName );

      if (!bottomOfSedimentSurface) {
         stringstream ss;
         ss << "Could not find user defined base surface of the rift event: " << baseSurfaceName;
         throw ss.str();
      } else {
         if( projectHandle->getRank() == 0 ) {
            cout << "Take surface " << bottomOfSedimentSurface->getName() << " as the base of syn-rift" << endl;
         }
      }
    }

   // Find the pressure property of the bottom of sediment
   m_pressureBasement = derivedManager.getSurfaceProperty ( pressureProperty, zeroSnapshot, bottomOfSedimentSurface );
    
   // Find the depth of the top of sediment
   Interface::FormationList * myFormations = projectHandle->getFormations (zeroSnapshot, true);
   const Interface::Formation * formationWB = (*myFormations)[0]; // find Water bottom
   
   if (!formationWB) {
      string s = "Could not find topformation.";
      throw s;
   } 
   
   const Interface::Surface * topOfSedimentSurface = formationWB->getTopSurface();
   
   if(debug && projectHandle->getRank() == 0) {
      cout << "Top formation: " << formationWB->getName() << "; surface below " << topOfSedimentSurface->getName() << "." << endl;
   }
   
   // Find the pressure property of the top of sediment
   m_pressureWaterBottom = derivedManager.getSurfaceProperty ( pressureProperty, zeroSnapshot, topOfSedimentSurface );
 
   m_depthBasementMap    = const_cast<Interface::GridMap *>(bottomOfSedimentSurface->getInputDepthMap());
   m_depthWaterBottomMap = const_cast<Interface::GridMap *>(topOfSedimentSurface->getInputDepthMap());

   delete myFormations;
}
//------------------------------------------------------------//
void DensityCalculator::retrieveData() {

   m_depthBasementMap->retrieveData ();
   m_depthWaterBottomMap->retrieveData ();
   m_pressureBasement->retrieveData ();
   m_pressureWaterBottom->retrieveData ();
}

//------------------------------------------------------------//
void DensityCalculator::restoreData() {

   m_depthBasementMap->restoreData ();
   m_depthWaterBottomMap->restoreData ();
   m_pressureBasement->restoreData ();
   m_pressureWaterBottom->restoreData ();
}
//------------------------------------------------------------//
void DensityCalculator::computeNode( unsigned int i, unsigned int j ) {


   m_topBasementDepthValue = m_depthBasementMap->getValue (i, j) ;
   m_waterBottomDepthValue = m_depthWaterBottomMap->getValue (i, j);

   if((m_topBasementDepthValue != m_depthBasementMap->getUndefinedValue()) &&   
      (m_waterBottomDepthValue != m_depthWaterBottomMap->getUndefinedValue())) {

      m_sedimentThickness = m_topBasementDepthValue - m_waterBottomDepthValue;
   } else {
      m_sedimentThickness = Interface::DefaultUndefinedMapValue; // or 0.0?  
   }

   double pressureTopBasementValue = m_pressureBasement->get (i, j);
   double pressureWaterBottomValue = m_pressureWaterBottom->get (i, j);

   m_sedimentDensity = Interface::DefaultUndefinedMapValue;

   if((pressureTopBasementValue != m_pressureBasement->getUndefinedValue()) &&
      (pressureWaterBottomValue != m_pressureWaterBottom->getUndefinedValue()) &&
      (m_sedimentThickness != Interface::DefaultUndefinedMapValue)) {
   
      // Integrated sediment density calculated across grid using pressure at WaterBottom and TopBasement surfaces
      if( m_sedimentThickness != 0.0 ) {
         m_sedimentDensity = ((pressureTopBasementValue - pressureWaterBottomValue) * 1e6 ) / (AccelerationDueToGravity * m_sedimentThickness);
      }
   }
}
//------------------------------------------------------------//
double DensityCalculator::getTopBasementDepthValue() const {
   if( m_topBasementDepthValue != m_depthBasementMap->getUndefinedValue() ) {
      return m_topBasementDepthValue;
   } else {
      return Interface::DefaultUndefinedMapValue;
   }
}
//------------------------------------------------------------//
double DensityCalculator::getSedimentDensity() const {
   return m_sedimentDensity;
}

//------------------------------------------------------------//
double DensityCalculator::getWLS( const double backstrippingMantleDensity, const double densityDiff ) const {

   double WLS = Interface::DefaultUndefinedMapValue;

   if( m_waterBottomDepthValue != m_depthWaterBottomMap->getUndefinedValue() && m_sedimentThickness != Interface::DefaultUndefinedMapValue ) {
      if( m_sedimentDensity != Interface::DefaultUndefinedMapValue ) {
         WLS = m_waterBottomDepthValue + m_sedimentThickness * (backstrippingMantleDensity - m_sedimentDensity) * densityDiff;
      } else {
         if(( m_sedimentThickness == 0 ) && ( m_sedimentDensity == Interface::DefaultUndefinedMapValue )) {
            WLS = m_waterBottomDepthValue;
         }
      }
       
      if(WLS < 0 ) WLS = 0;
   } 
   return WLS;
   // if( m_waterBottomDepthValue != m_depthWaterBottomMap->getUndefinedValue() && m_sedimentThickness != Interface::DefaultUndefinedMapValue ) {
   //    if( m_sedimentDensity != Interface::DefaultUndefinedMapValue ) {
   //       WLS = m_waterBottomDepthValue + m_sedimentThickness * (backstrippingMantleDensity - m_sedimentDensity) * densityDiff;
   //    } else {
   //       // WLS = m_waterBottomDepthValue;
   //    }
       
   //    if(WLS < 0 ) WLS = 0;
   // } 
   // return WLS;
}
//------------------------------------------------------------//
PropertyManager::PropertyManager ( GeoPhysics::ProjectHandle* projectHandle ) :
   DerivedProperties::DerivedPropertyManager ( projectHandle ) {
      

}  
