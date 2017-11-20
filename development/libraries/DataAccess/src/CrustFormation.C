//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// std library
#include <assert.h>
#include <iostream>
#include <sstream>
#include <limits>
using namespace std;

// utilities library
#include "mangle.h"

// TableIo library
#include "database.h"
#include "cauldronschemafuncs.h"
using namespace database;

// DataAccess library
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

//------------------------------------------------------------//

CrustFormation::CrustFormation (ProjectHandle * projectHandle, database::Record* record ) : Formation ( projectHandle, record ), BasementFormation ( projectHandle, record, CrustFormationName, projectHandle->getCrustLithoName() ) {

   // This pointer does NOT need to be deallocated, since the map it will point to will be deallocated
   // elsewhere.
   //
   // 1. If the bottom-boundary-conditions are mantle-heat-flow, the the map will be deleted
   //    by the 'parent' objects destructor;
   //
   // 2. Otherwise, the map will be deleted by the project handle when it destorys the crust
   //    thickness history sequence.
   //
   m_inputThicknessMap       = nullptr;
   m_initialThicknessMap     = nullptr;
   basaltThickness           = nullptr;
   crustalThicknessMeltOnset = nullptr;

   m_initialCrustalThickness = 0;
}

//------------------------------------------------------------//

CrustFormation::~CrustFormation (void)
{
}

//------------------------------------------------------------//

const GridMap * CrustFormation::getInputThicknessMap (void) const {

   if ( m_inputThicknessMap != nullptr ) {
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

      double closestToPresentDay = std::numeric_limits<double>::infinity();

      PaleoFormationPropertyList * thicknessMaps = getPaleoThicknessHistory ();
      PaleoFormationPropertyList::iterator thicknessIter;

      for ( thicknessIter = thicknessMaps->begin (); thicknessIter != thicknessMaps->end (); ++thicknessIter ) {

         // Find map with youngest age.
         if ((*thicknessIter)->getSnapshot()->getTime () < closestToPresentDay ) {
            closestToPresentDay = (*thicknessIter)->getSnapshot()->getTime ();
            m_inputThicknessMap = (*thicknessIter)->getMap (CrustThinningHistoryInstanceThicknessMap);
         }

      }

      assert ( m_inputThicknessMap != nullptr );
      delete thicknessMaps;
   }

   return m_inputThicknessMap;
}

//------------------------------------------------------------//

// in ALC mode the initial CrustalThickness is a scalar value 
const GridMap * CrustFormation::getInitialThicknessMap () const {
   if ( m_initialThicknessMap != nullptr ) {
      // Map has already been retrieved
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

      assert ( m_initialThicknessMap != nullptr );
      delete thicknessMaps;
   }

   return m_initialThicknessMap;
}

//------------------------------------------------------------//

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

//------------------------------------------------------------//

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

//------------------------------------------------------------//

PaleoFormationPropertyList * CrustFormation::getPaleoThicknessHistory () const {
   return m_projectHandle->getCrustPaleoThicknessHistory ();
}

//------------------------------------------------------------//

double CrustFormation::getHeatProductionDecayConstant () const {
   return database::getCrustHeatPDecayConst ( m_record );
}

//------------------------------------------------------------//

int CrustFormation::getDepositionSequence () const {
   return CRUST_DEPOSITION;
}

//------------------------------------------------------------//

void CrustFormation::asString (string & str) const
{
   ostringstream buf;

   buf << "CrustFormation";
   buf << ": name = "                           << BasementFormation::getName ();
   buf << ", heat production decay constant = " << getHeatProductionDecayConstant ();
   buf << ", top surface name = "               << Formation::getTopSurface ()->getName ();
   buf << ", bottom surface name = "            << Formation::getBottomSurface ()->getName ();
   buf << endl;

   str = buf.str ();
}
