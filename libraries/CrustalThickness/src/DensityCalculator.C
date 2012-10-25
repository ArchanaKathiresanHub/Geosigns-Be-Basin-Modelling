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

#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>

const double GRAVITY = 9.81;

//------------------------------------------------------------//
//------------------------------------------------------------//
DensityCalculator::DensityCalculator() {

   m_depthBasementMap = 0; 
   m_depthWaterBottomMap = 0; 
   m_pressureBasementMap = 0; 
   m_pressureWaterBottomMap = 0;

   m_sedimentDensity = 0.0;
   m_waterBottomDepthValue = 0.0;
   m_sedimentThickness = 0.0;
   m_topBasementDepthValue = 0.0;
} 

//------------------------------------------------------------//
void DensityCalculator::loadData( Interface::ProjectHandle* projectHandle, const string & baseSurfaceName ) {

   bool debug = false;

   const Interface::Snapshot * zeroSnapshot = projectHandle->findSnapshot (0);
   
   const Interface::Property * depthProperty =  projectHandle->findProperty("Depth");
   if (!depthProperty) {
      string s = "Could not find property named Depth.";
      throw s;
   }
   const Interface::Property * pressureProperty = projectHandle->findProperty("LithoStaticPressure");
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

         cout << "Take surface " << bottomOfSedimentSurface->getName() << " as the base of syn-rift" << endl;
      }
    }

   // Find the pressure property of the bottom of sediment
   const Interface::Formation * bottomFormation = bottomOfSedimentSurface->getTopFormation ();

   Interface::PropertyValueList * propertyValues = projectHandle->getPropertyValues (SURFACE | FORMATION | FORMATIONSURFACE,
                                                                                     pressureProperty, zeroSnapshot, 0, bottomFormation, 0, VOLUME);
   if (propertyValues->size() != 1) {
      stringstream ss;
      ss << "Could not find property LithoStaticPressure for formation " << bottomFormation->getName() <<  " at age " << zeroSnapshot->getTime() << "." << endl;

      throw ss.str();
   }
   const Interface::PropertyValue * pressureBasement = (* propertyValues)[0];
   
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
   propertyValues = projectHandle->getPropertyValues (SURFACE | FORMATION | FORMATIONSURFACE, pressureProperty, zeroSnapshot, 0, formationWB, 0, VOLUME);

   if (propertyValues->size() != 1) {
      stringstream ss;
      ss << "Could not find property LithoStaticPressure for formation." << formationWB->getName() << " at age " << zeroSnapshot->getTime() << "."<< endl;

      throw ss.str();
   }
   const Interface::PropertyValue * pressureWaterBottom = (* propertyValues)[0];

   m_depthBasementMap    = const_cast<Interface::GridMap *>(bottomOfSedimentSurface->getInputDepthMap());
   m_depthWaterBottomMap = const_cast<Interface::GridMap *>(topOfSedimentSurface->getInputDepthMap());
   m_pressureBasementMap    = pressureBasement->getGridMap();
   m_pressureWaterBottomMap = pressureWaterBottom->getGridMap();

}
#if 0
//------------------------------------------------------------//
void DensityCalculator::setWaterBottomDepthMap( Interface::ProjectHandle* projectHandle ) {
   
   Interface::PaleoPropertyList * pList = projectHandle->getSurfaceDepthHistory();
   
   for( int i = 0; i < pList->size(); ++ i ) {
      if( pList[i]->getSnapshot()->getTime() == 0 ) {
         m_depthWaterBottomMap = pList[i]->getMap();
         break;
      }
   }
}
#endif
//------------------------------------------------------------//
void DensityCalculator::retrieveData() {

   m_depthBasementMap->retrieveData ();
   m_depthWaterBottomMap->retrieveData ();
   m_pressureBasementMap->retrieveData ();
   m_pressureWaterBottomMap->retrieveData ();
}

//------------------------------------------------------------//
void DensityCalculator::restoreData() {

   m_depthBasementMap->restoreData ();
   m_depthWaterBottomMap->restoreData ();
   m_pressureBasementMap->restoreData ();
   m_pressureWaterBottomMap->restoreData ();
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
   
   unsigned int maxK = m_pressureWaterBottomMap->getDepth() - 1 ;

   double pressureTopBasementValue = m_pressureBasementMap->getValue (i, j);
   double pressureWaterBottomValue = m_pressureWaterBottomMap->getValue (i, j, maxK);

   m_sedimentDensity = Interface::DefaultUndefinedMapValue;

   if((pressureTopBasementValue != m_pressureBasementMap->getUndefinedValue()) &&
      (pressureWaterBottomValue != m_pressureWaterBottomMap->getUndefinedValue()) &&
      (m_sedimentThickness != Interface::DefaultUndefinedMapValue)) {
   
      // Integrated sediment density calculated across grid using pressure at WaterBottom and TopBasement surfaces
      if( m_sedimentThickness != 0.0 ) {
         m_sedimentDensity = ((pressureTopBasementValue - pressureWaterBottomValue) * 1e6 ) / (GRAVITY * m_sedimentThickness);
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
