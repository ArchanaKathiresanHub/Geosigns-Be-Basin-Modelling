//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// std library
#include <cassert>
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
#include "Interface.h"
#include "CrustFormation.h"
#include "Formation.h"
#include "Grid.h"
#include "GridMap.h"
#include "Snapshot.h"
#include "Surface.h"
#include "ObjectFactory.h"
#include "ProjectHandle.h"
#include "PaleoFormationProperty.h"
using namespace DataAccess;
using namespace Interface;

//------------------------------------------------------------//

CrustFormation::CrustFormation (ProjectHandle& projectHandle, database::Record* record ) :
   Formation ( projectHandle, record ), BasementFormation ( projectHandle, record, CrustFormationName, projectHandle.getCrustLithoName() ) {

   // This pointer does NOT need to be deallocated, since the map it will point to will be deallocated
   // elsewhere.
   //
   // 1. If the bottom-boundary-conditions are mantle-heat-flow, the the map will be deleted
   //    by the 'parent' objects destructor;
   //
   // 2. Otherwise, the map will be deleted by the project handle when it destorys the crust
   //    thickness history sequence.
   //
   m_inputThicknessMap         = nullptr;
   m_initialThicknessMap       = nullptr;
   m_basaltThickness           = nullptr;
   m_crustalThicknessMeltOnset = nullptr;

   m_initialCrustalThickness = 0;
}

//------------------------------------------------------------//

const GridMap * CrustFormation::getInputThicknessMap (void) const {

   if ( m_inputThicknessMap != nullptr ) {
      // Map has already been retrieved.
      return m_inputThicknessMap;
   } else if ( getProjectHandle().getBottomBoundaryConditions () == MANTLE_HEAT_FLOW ) {
      // Fixed crust thickness.

      const Grid * grid = getProjectHandle().getActivityOutputGrid();

      if (!grid) {
         grid = getProjectHandle().getInputGrid();
      }

      m_inputThicknessMap  = getProjectHandle().getFactory ()->produceGridMap (this, ThicknessMap, grid, database::getFixedCrustThickness (m_record));

   } else {
      // Crust thinning.
      // So must return map for present-day, or nearest.

      double closestToPresentDay = std::numeric_limits<double>::infinity();

      PaleoFormationPropertyList * thicknessMaps = getPaleoThicknessHistory ();

      for ( PaleoFormationPropertyList::iterator thicknessIter = thicknessMaps->begin (); thicknessIter != thicknessMaps->end (); ++thicknessIter ) {

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

      for ( PaleoFormationPropertyList::iterator thicknessIter = thicknessMaps->begin (); thicknessIter != thicknessMaps->end (); ++thicknessIter ) {

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
   GridMap * gridMap = nullptr;

   const std::string & heatProductionMapName = getTopCrustHeatProdGrid (m_record);

   if (heatProductionMapName.length () != 0)
   {
      gridMap = getProjectHandle().loadInputMap ("BasementIoTbl", heatProductionMapName);
   }
   else if (( crustHeatProduction = getTopCrustHeatProd (m_record)) != RecordValueUndefined )
   {

      const Grid * grid = getProjectHandle().getActivityOutputGrid();

      if ( grid == nullptr ) {
         grid = const_cast<Grid *>(getProjectHandle().getInputGrid());
      }

      gridMap = getProjectHandle().getFactory ()->produceGridMap (this, CrustHeatProductionMap, grid, crustHeatProduction );

      assert (gridMap == getChild (CrustHeatProductionMap));
   }

   return gridMap;
}

//------------------------------------------------------------//

const GridMap * CrustFormation::getCrustHeatProductionMap () const {

   const GridMap * gridMap;

   if ((gridMap = dynamic_cast<const GridMap *>(getChild(CrustHeatProductionMap))) != nullptr) {
      return gridMap;
   } else
   if ((gridMap = loadCrustHeatProductionMap ()) != nullptr) {
      return gridMap;
   } else {
      return nullptr;
   }

}

//------------------------------------------------------------//

PaleoFormationPropertyList * CrustFormation::getPaleoThicknessHistory () const {
   return getProjectHandle().getCrustPaleoThicknessHistory ();
}

//------------------------------------------------------------//
PaleoFormationPropertyList* CrustFormation::getOceaPaleoThicknessHistory() const {
	return getProjectHandle().getOceaCrustPaleoThicknessHistory();
}
//-----------------------------------------------------------//

double CrustFormation::getHeatProductionDecayConstant () const {
   return database::getCrustHeatPDecayConst ( m_record );
}

//------------------------------------------------------------//

int CrustFormation::getDepositionSequence () const {
   return CRUST_DEPOSITION;
}

//------------------------------------------------------------//

GridMap * CrustFormation::loadCrustThicknessMeltOnsetMap() const {

    double crustMeltOnset;
    GridMap * gridMap = nullptr;

    database::Table* basaltThicknessIoTbl = getProjectHandle().getTable( "BasaltThicknessIoTbl" );
    if (basaltThicknessIoTbl == nullptr) {
        return nullptr;
    }
    Record *firstRecord = basaltThicknessIoTbl->getRecord( 0 );
    if (firstRecord == nullptr) {
        return nullptr;
    }

    const std::string & crustMeltOnsetMapName = getCrustThicknessMeltOnsetGrid( firstRecord );
    if (crustMeltOnsetMapName.length() != 0) {
        gridMap = getProjectHandle().loadInputMap( "BasaltThicknessIoTbl", crustMeltOnsetMapName );
    }
    else if ((crustMeltOnset = getCrustThicknessMeltOnset( firstRecord )) != RecordValueUndefined) {
        const Grid * grid = getProjectHandle().getActivityOutputGrid();
        if (grid == nullptr) {
            grid = const_cast<Grid *>(getProjectHandle().getInputGrid());
        }
        gridMap = getProjectHandle().getFactory()->produceGridMap( this, CrustThicknessMeltOnsetMap, grid, crustMeltOnset );
        assert( gridMap == getChild( CrustThicknessMeltOnsetMap ) );
    }
    return gridMap;
}

//------------------------------------------------------------//

const GridMap * CrustFormation::getCrustThicknessMeltOnsetMap() const {
    const GridMap * gridMap;
    if ((gridMap = dynamic_cast<const GridMap *>(getChild(CrustThicknessMeltOnsetMap))) != nullptr) {
        return gridMap;
    }
    else
        if ((gridMap = loadCrustThicknessMeltOnsetMap()) != nullptr) {
            return gridMap;
        }
        else {
            return nullptr;
        }
}

//------------------------------------------------------------//

GridMap * CrustFormation::loadBasaltThicknessMap() const {
    double basaltThicknessValue;
    GridMap * gridMap = nullptr;
    database::Table* basaltThicknessIoTbl = getProjectHandle().getTable( "BasaltThicknessIoTbl" );
    if (basaltThicknessIoTbl == nullptr) {
        return nullptr;
    }
    Record *firstRecord = basaltThicknessIoTbl->getRecord( 0 );
    if (firstRecord == nullptr) {
        return nullptr;
    }
    const std::string & basaltThicknessMapName = getBasaltThicknessGrid( firstRecord );
    if (basaltThicknessMapName.length() != 0) {
        gridMap = getProjectHandle().loadInputMap( "BasaltThicknessIoTbl", basaltThicknessMapName );
    }
    else if ((basaltThicknessValue = getBasaltThickness( firstRecord )) != RecordValueUndefined) {
        const Grid * grid = getProjectHandle().getActivityOutputGrid();
        if (grid == nullptr) {
            grid = const_cast<Grid *>(getProjectHandle().getInputGrid());
        }
        gridMap = getProjectHandle().getFactory()->produceGridMap( this, BasaltThicknessMap, grid, basaltThicknessValue );
        assert( gridMap == getChild( BasaltThicknessMap ) );
    }
    return gridMap;
}

//------------------------------------------------------------//

const GridMap * CrustFormation::getBasaltThicknessMap() const {
    const GridMap * gridMap;
    if ((gridMap = dynamic_cast<const GridMap *>(getChild(BasaltThicknessMap))) != nullptr) {
        return gridMap;
    }
    else
        if ((gridMap = loadBasaltThicknessMap()) != nullptr) {
            return gridMap;
        }
        else {
            return nullptr;
        }

}

//------------------------------------------------------------//

void CrustFormation::asString (string & str) const
{
   std::ostringstream buf;

   buf << "CrustFormation";
   buf << ": name = "                           << BasementFormation::getName ();
   buf << ", heat production decay constant = " << getHeatProductionDecayConstant ();
   buf << ", top surface name = "               << Formation::getTopSurface ()->getName ();
   buf << ", bottom surface name = "            << Formation::getBottomSurface ()->getName ();
   buf << endl;

   str = buf.str ();
}
