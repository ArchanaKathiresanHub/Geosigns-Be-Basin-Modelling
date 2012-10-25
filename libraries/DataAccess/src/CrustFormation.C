#include <assert.h>
#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
      #include <sstream>
      using namespace std;
      #define USESTANDARD
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
      #include<strstream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   #include <sstream>
   using namespace std;
   #define USESTANDARD
#endif // sgi

#include "mangle.h"

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Interface/Interface.h"

#include "Interface/CrustFormation.h"
#include "Interface/Formation.h"
#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"
#include "Interface/ObjectFactory.h"
#include "Interface/ProjectHandle.h"
#include "Interface/PaleoFormationProperty.h"

using namespace DataAccess;
using namespace Interface;


CrustFormation::CrustFormation (ProjectHandle * projectHandle, database::Record* record ) : Formation ( projectHandle, record ), BasementFormation ( projectHandle, record, CrustFormationName, CrustLithologyName ) {

   // This pointer does NOT need to be deallocated, since the map it will point to will be deallocated
   // elsewhere.
   //
   // 1. If the bottom-boundary-conditions are mantle-heat-flow, the the map will be deleted
   //    by the 'parent' objects destructor;
   //
   // 2. Otherwise, the map will be deleted by the project handle when it destorys the crust
   //    thickness history sequence.
   //
   m_inputThicknessMap = 0;
   m_initialThicknessMap = 0;
   m_initialCrustalThickness = 0;
}


CrustFormation::~CrustFormation (void)
{
}

/// Return the present day, user-supplied Crust thickness GridMap.
const GridMap * CrustFormation::getInputThicknessMap (void) const {

   if ( m_inputThicknessMap != 0 ) {
      // Map has already been retrieved.
      return m_inputThicknessMap;
   } else if ( m_projectHandle->getBottomBoundaryConditions () == MANTLE_HEAT_FLOW ) {
      // Fixed crust thickness.

      const Grid * grid = m_projectHandle->getActivityOutputGrid();

      if (!grid) {
         grid = (Grid *) m_projectHandle->getInputGrid ();
      }

      m_inputThicknessMap  = m_projectHandle->getFactory ()->produceGridMap (this, ThicknessMap, grid, database::getFixedCrustThickness (m_record));
      
   } else {
      // Crust thinning.
      // So must return map for present-day, or nearest.

      // No map will [should?] be older than this.
      // As long as there is a map younger than this age the actual value does not matter.
      double closestToPresentDay = 5.0e9;

      PaleoFormationPropertyList * thicknessMaps = getPaleoThicknessHistory ();
      PaleoFormationPropertyList::iterator thicknessIter;

      for ( thicknessIter = thicknessMaps->begin (); thicknessIter != thicknessMaps->end (); ++thicknessIter ) {

         // Find map with youngest age.
         if ((*thicknessIter)->getSnapshot()->getTime () < closestToPresentDay ) {
            closestToPresentDay = (*thicknessIter)->getSnapshot()->getTime ();
            m_inputThicknessMap = (*thicknessIter)->getMap (CrustThinningHistoryInstanceThicknessMap);
         }

      }

      assert ( m_inputThicknessMap != 0 );
      delete thicknessMaps;
   }

   return m_inputThicknessMap;
}
// in ALC mode the initial CrustalThickness is a scalar value 
const GridMap * CrustFormation::getInitialThicknessMap (void) const {
   if ( m_initialThicknessMap != 0 ) {
      return m_initialThicknessMap;
   } else {

      double closestToAgeOfBasin = -1.0;
      PaleoFormationPropertyList * thicknessMaps = getPaleoThicknessHistory ();
      PaleoFormationPropertyList::iterator thicknessIter;

      for ( thicknessIter = thicknessMaps->begin (); thicknessIter != thicknessMaps->end (); ++thicknessIter ) {

         // Find map with older age.
         if ((*thicknessIter)->getSnapshot()->getTime () > closestToAgeOfBasin ) {
            closestToAgeOfBasin = (*thicknessIter)->getSnapshot()->getTime ();
            m_initialThicknessMap = (*thicknessIter)->getMap (CrustThinningHistoryInstanceThicknessMap);
         }

      }

      assert ( m_initialThicknessMap != 0 );
      delete thicknessMaps;
   }

   return m_initialThicknessMap;
}

GridMap * CrustFormation::loadCrustHeatProductionMap () const {

   double crustHeatProduction;
   GridMap * gridMap = 0;

   const std::string & heatProductionMapName = getTopCrustHeatProdGrid (m_record);

   if (heatProductionMapName.length () != 0)
   {
      gridMap = m_projectHandle->loadInputMap ("BasementIoTbl", heatProductionMapName);
   }
   else if (( crustHeatProduction = getTopCrustHeatProd (m_record)) != RecordValueUndefined )
   {

      const Grid * grid = m_projectHandle->getActivityOutputGrid();

      if ( grid == 0 ) {
         grid = (Grid *) m_projectHandle->getInputGrid ();
      }

      gridMap = m_projectHandle->getFactory ()->produceGridMap (this, CrustHeatProductionMap, grid, crustHeatProduction );

      assert (gridMap == getChild (CrustHeatProductionMap));
   }

   return gridMap;
}

const GridMap * CrustFormation::getCrustHeatProductionMap () const {

   const GridMap * gridMap;

   if ((gridMap = (const GridMap *) getChild (CrustHeatProductionMap)) != 0) {
      return gridMap;
   } else 
   if ((gridMap = loadCrustHeatProductionMap ()) != 0) {
      return gridMap;
   } else {
      return 0;
   }
  
}

GridMap * CrustFormation::loadCrustThicknessMeltOnsetMap () const {

   double crustMeltOnset;
   GridMap * gridMap = 0;

   database::Table* basaltThicknessIoTbl =  m_projectHandle->getTable ("BasaltThicknessIoTbl");
   if( basaltThicknessIoTbl == 0 ) {
      return 0;
   }
   Record *firstRecord = basaltThicknessIoTbl->getRecord (0);
   if( firstRecord == 0 ) {
      return 0;
   }

   const std::string & crustMeltOnsetMapName = getCrustThicknessMeltOnsetGrid (firstRecord);
   if (crustMeltOnsetMapName.length () != 0)
   {
      gridMap = m_projectHandle->loadInputMap ("BasaltThicknessIoTbl", crustMeltOnsetMapName);
   }
   else if (( crustMeltOnset = getCrustThicknessMeltOnset (firstRecord)) != RecordValueUndefined )
   {
      const Grid * grid = m_projectHandle->getActivityOutputGrid();
      if ( grid == 0 ) {
         grid = (Grid *) m_projectHandle->getInputGrid ();
      }
      gridMap = m_projectHandle->getFactory ()->produceGridMap (this, CrustThicknessMeltOnsetMap, grid, crustMeltOnset );
      assert (gridMap == getChild (CrustThicknessMeltOnsetMap));
   }
   return gridMap;
}

const GridMap * CrustFormation::getCrustThicknessMeltOnsetMap () const {
   const GridMap * gridMap;
   if ((gridMap = (const GridMap *) getChild (CrustThicknessMeltOnsetMap)) != 0) {
      return gridMap;
   } else 
   if ((gridMap = loadCrustThicknessMeltOnsetMap ()) != 0) {
      return gridMap;
   } else {
      return 0;
   }
}

GridMap * CrustFormation::loadBasaltThicknessMap () const {
   double basaltThicknessValue;
   GridMap * gridMap = 0;
   database::Table* basaltThicknessIoTbl =  m_projectHandle->getTable ("BasaltThicknessIoTbl");
   if( basaltThicknessIoTbl == 0 ) {
      return 0;
   }
   Record *firstRecord = basaltThicknessIoTbl->getRecord (0);
   if( firstRecord == 0 ) {
      return 0;
   }
   const std::string & basaltThicknessMapName = getBasaltThicknessGrid (firstRecord);
   if (basaltThicknessMapName.length () != 0)
   {
      gridMap = m_projectHandle->loadInputMap ("BasaltThicknessIoTbl", basaltThicknessMapName);
   }
   else if (( basaltThicknessValue = getBasaltThickness (firstRecord)) != RecordValueUndefined )
   {
      const Grid * grid = m_projectHandle->getActivityOutputGrid();
      if ( grid == 0 ) {
         grid = (Grid *) m_projectHandle->getInputGrid ();
      }
      gridMap = m_projectHandle->getFactory ()->produceGridMap (this, BasaltThicknessMap, grid, basaltThicknessValue );
      assert (gridMap == getChild (BasaltThicknessMap));
   }
   return gridMap;
}

const GridMap * CrustFormation::getBasaltThicknessMap () const {
   const GridMap * gridMap;
   if ((gridMap = (const GridMap *) getChild (BasaltThicknessMap)) != 0) {
      return gridMap;
   } else 
   if ((gridMap = loadBasaltThicknessMap ()) != 0) {
      return gridMap;
   } else {
      return 0;
   }
  
}

PaleoFormationPropertyList * CrustFormation::getPaleoThicknessHistory () const {
   return m_projectHandle->getCrustPaleoThicknessHistory ();
}

double CrustFormation::getHeatProductionDecayConstant () const {
   return database::getCrustHeatPDecayConst ( m_record );
}

int CrustFormation::getDepositionSequence () const {
   return -1;
}

void CrustFormation::asString (string & str) const
{
#ifdef USESTANDARD
   ostringstream buf;
#else
   strstream buf;
#endif

   buf << "CrustFormation";
   buf << ": name = " << BasementFormation::getName ();
   buf << ", heat production decay constant = " << getHeatProductionDecayConstant ();
   buf << ", top surface name = " << Formation::getTopSurface ()->getName ();
   buf << ", bottom surface name = " << Formation::getBottomSurface ()->getName ();
   buf << endl;

   str = buf.str ();
#ifndef USESTANDARD
   buf.rdbuf ()->freeze (0);
#endif
}
