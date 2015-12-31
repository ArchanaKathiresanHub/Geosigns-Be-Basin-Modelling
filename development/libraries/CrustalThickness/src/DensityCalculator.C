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

   m_depthBasementMap = 0; 
   m_depthWaterBottomMap = 0; 
   m_pressureBasementMap = 0; 
   m_pressureWaterBottomMap = 0;

   m_waterBottomDepthValue = 0.0;
   m_sedimentThickness = 0.0;
   m_topBasementDepthValue = 0.0;
   m_WLS = 0.0;
   m_backstrip = 0.0;
   m_sedimentDensity = 0.0;

   m_bottomOfSedimentSurface = 0;
   m_topOfSedimentSurface = 0;

   m_densityTerm = 1.0;
   m_backstrippingMantleDensity = 0.0;
   m_waterDensity = 0.0;
} 

//------------------------------------------------------------//
const Interface::Property * DensityCalculator::loadDepthProperty( Interface::ProjectHandle* projectHandle, const double snapshotAge, const string & baseSurfaceName ) {

   bool debug = false;
   const Interface::Snapshot * currentSnapshot = projectHandle->findSnapshot( snapshotAge, MINOR | MAJOR );

   const Interface::Property * depthProperty = projectHandle->findProperty( "Depth" );

   if (!depthProperty) {
      string s = "Could not find property named Depth.";
      throw s;
   }

   // Find the depth of the bottom of the sediment
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

   // Find the depth of the top of the sediment
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

   return depthProperty;
}
//------------------------------------------------------------//
void DensityCalculator::loadDepthData( Interface::ProjectHandle* projectHandle, const Interface::Property * depthProperty, const double snapshotAge ) {

   bool debug = false;

   const Interface::Snapshot * currentSnapshot = projectHandle->findSnapshot (snapshotAge, MINOR | MAJOR);

   Interface::PropertyValueList * propertyValues = projectHandle->getPropertyValues (SURFACE | FORMATION | FORMATIONSURFACE,
                                                                                     depthProperty, currentSnapshot, 0, 0, m_bottomOfSedimentSurface, MAP);


   if (propertyValues->size() != 1) {
      stringstream ss;
      if( snapshotAge != 0.0 ) {
         ss << "WARNING: ";
      }
      ss << "Could not find property Depth (" << propertyValues->size() << ") for bottom of sediment surface " << 
         m_bottomOfSedimentSurface->getName() <<  " at age " << currentSnapshot->getTime() << "." << endl;

      throw ss.str();
   }

   const Interface::PropertyValue * depthBasement = (* propertyValues)[0];
   delete propertyValues;

 
   // Find the pressure property of the top of sediment
   propertyValues = projectHandle->getPropertyValues (SURFACE | FORMATION | FORMATIONSURFACE, depthProperty, currentSnapshot, 0, 0, m_topOfSedimentSurface, MAP);

   if (propertyValues->size() != 1) {
      stringstream ss;
      if( snapshotAge != 0.0 ) {  ss << "WARNING: ";  }

      ss << "Could not find property Depth (" << propertyValues->size() << ") for top of sediment surface." << 
         m_topOfSedimentSurface->getName() << " at age " << currentSnapshot->getTime() << "."<< endl;

      throw ss.str();
   }
   const Interface::PropertyValue * depthWaterBottom = (* propertyValues)[0];
   delete propertyValues;

   m_depthBasementMap    = depthBasement->getGridMap();
   m_depthWaterBottomMap = depthWaterBottom->getGridMap();

   if( m_depthBasementMap == 0 || m_depthWaterBottomMap == 0 ) {
      stringstream ss;
      if( snapshotAge != 0.0 ) {  ss << "WARNING: ";  }

      ss << "Could not find property Depth at the age " << currentSnapshot->getTime() << "."<< endl;

      throw ss.str();
   }
}

//------------------------------------------------------------//
const Interface::Property * DensityCalculator::loadPressureProperty( Interface::ProjectHandle* projectHandle, const double snapshotAge ) {

   const Interface::Snapshot * currentSnapshot = projectHandle->findSnapshot (snapshotAge, MINOR | MAJOR);
   
   const Interface::Property * pressureProperty = projectHandle->findProperty("LithoStaticPressure");

   if (!pressureProperty) {
      string s = "Could not find property named LithoStaticPressure."; 
      throw s;
   }
   return pressureProperty;
   
}

//------------------------------------------------------------//
void DensityCalculator::loadPressureData( Interface::ProjectHandle* projectHandle, const Interface::Property * pressureProperty, const double snapshotAge ) {

   bool debug = false;

   const Interface::Snapshot * currentSnapshot = projectHandle->findSnapshot (snapshotAge, MINOR | MAJOR);
 
   // Find the pressure property of the bottom of sediment
   const Interface::Formation * bottomFormation = m_bottomOfSedimentSurface->getTopFormation ();

   Interface::PropertyValueList * propertyValues = projectHandle->getPropertyValues (SURFACE | FORMATION | FORMATIONSURFACE,
                                                                                     pressureProperty, currentSnapshot, 0, 0, m_bottomOfSedimentSurface, MAP);


   if (propertyValues->size() != 1) {
      stringstream ss;
      if( snapshotAge != 0.0 ) { ss << "WARNING: ";  }
      ss << "Could not find property LithoStaticPressure (" << propertyValues->size() << ") for bottom of sediment surface " << m_bottomOfSedimentSurface->getName() 
         <<  " at age " << currentSnapshot->getTime() << "." << endl;

      throw ss.str();
   }
   const Interface::PropertyValue * pressureBasement = (* propertyValues)[0];
   delete propertyValues;

   const Interface::Formation * formationWB = m_topOfSedimentSurface->getBottomFormation ();
   
   // Find the pressure property of the top of sediment
   propertyValues = projectHandle->getPropertyValues (SURFACE | FORMATION | FORMATIONSURFACE, pressureProperty, currentSnapshot, 0, 0, m_topOfSedimentSurface, MAP);

   if (propertyValues->size() != 1) {
      stringstream ss;
      if( snapshotAge != 0.0 ) { ss << "WARNING: ";  }

      ss << "Could not find property LithoStaticPressure (" << propertyValues->size() << ") for top of sediment surface." << m_topOfSedimentSurface->getName() 
         << " at age " << currentSnapshot->getTime() << "."<< endl;

      throw ss.str();
   }
   const Interface::PropertyValue * pressureWaterBottom = (* propertyValues)[0];
   delete propertyValues;

   m_pressureBasementMap    = pressureBasement->getGridMap();
   m_pressureWaterBottomMap = pressureWaterBottom->getGridMap();

   if( m_pressureBasementMap == 0 || m_pressureWaterBottomMap == 0 ) {
      stringstream ss;
      if( snapshotAge != 0.0 ) {  ss << "WARNING: ";  }

      ss << "Could not find property LithostaticPressure at the age " << currentSnapshot->getTime() << "."<< endl;

      throw ss.str();
}
}

//------------------------------------------------------------//
void DensityCalculator::loadSnapshots( Interface::ProjectHandle* projectHandle ) {
   
   
   Interface::FormationList* formations = projectHandle->getFormations ( ); //0, true );
   Interface::FormationList::const_iterator formationIter;

   for ( formationIter = formations->begin (); formationIter != formations->end (); ++formationIter ) {
      const Interface::Formation * formation = ( *formationIter );

      if ( formation != 0 ) {
         const Interface::Surface * topSurface = formation->getBottomSurface(); //TopSurface();
         m_snapshots.push_back( topSurface->getSnapshot()->getTime() );
      }
   }
   m_snapshots.push_back( 0.0 ); // add present day
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
   m_backstrip = Interface::DefaultUndefinedMapValue;

   if((pressureTopBasementValue != m_pressureBasementMap->getUndefinedValue()) &&
      (pressureWaterBottomValue != m_pressureWaterBottomMap->getUndefinedValue()) &&
      (m_sedimentThickness != Interface::DefaultUndefinedMapValue)) {
   
      m_backstrip =  m_sedimentThickness *  m_backstrippingMantleDensity * m_densityTerm - 
         ((( pressureTopBasementValue - pressureWaterBottomValue ) * 1e6 ) / GRAVITY ) * m_densityTerm;

      // Integrated sediment density calculated across grid using pressure at WaterBottom and TopBasement surfaces
      if( m_sedimentThickness != 0.0 ) {
         m_sedimentDensity = ((pressureTopBasementValue - pressureWaterBottomValue) * 1e6 ) / (GRAVITY * m_sedimentThickness);
      } 
   }
   m_WLS = Interface::DefaultUndefinedMapValue;

   if( m_waterBottomDepthValue != m_depthWaterBottomMap->getUndefinedValue() && m_backstrip != Interface::DefaultUndefinedMapValue ) {
      
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
double DensityCalculator::getTopBasementDepthValue() const {
   if( m_topBasementDepthValue != m_depthBasementMap->getUndefinedValue() ) {
      return m_topBasementDepthValue;
   } else {
      return Interface::DefaultUndefinedMapValue;
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


/*
//------------------------------------------------------------//
double DensityCalculator::calculateWLS( const double backstrippingMantleDensity, const double densityDiff ) {

   m_WLS = Interface::DefaultUndefinedMapValue;
   m_backstrip = Interface::DefaultUndefinedMapValue;

   if( m_waterBottomDepthValue != m_depthWaterBottomMap->getUndefinedValue() && m_sedimentThickness != Interface::DefaultUndefinedMapValue ) {
      if( m_sedimentDensity != Interface::DefaultUndefinedMapValue ) {
         m_backstrip =  m_sedimentThickness * (backstrippingMantleDensity - m_sedimentDensity) * densityDiff;
         m_WLS = m_waterBottomDepthValue + m_backstrip;
      } else {
         if(( m_sedimentThickness == 0 ) && ( m_sedimentDensity == Interface::DefaultUndefinedMapValue )) {
            m_WLS = m_waterBottomDepthValue;
            m_backstrip = 0.0;
         }
      }
       
      if(m_WLS < 0 ) m_WLS = 0;
   } 
   return m_WLS;
}
   return WLS;
   // if( m_waterBottomDepthValue != m_depthWaterBottomMap->getUndefinedValue() && m_sedimentThickness != Interface::DefaultUndefinedMapValue ) {
   //    if( m_sedimentDensity != Interface::DefaultUndefinedMapValue ) {
   //       WLS = m_waterBottomDepthValue + m_sedimentThickness * (backstrippingMantleDensity - m_sedimentDensity) * densityDiff;
   //    } else {
   //       // WLS = m_waterBottomDepthValue;
   //    }

*/

#if 0
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
#endif
