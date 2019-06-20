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
using namespace std;

// utilities library
#include "mangle.h"

// TableIo library
#include "database.h"
#include "cauldronschemafuncs.h"
using namespace database;

// DataAccess library
#include "MantleFormation.h"
#include "Formation.h"
#include "Grid.h"
#include "GridMap.h"
#include "Snapshot.h"
#include "Surface.h"
#include "ObjectFactory.h"
#include "ProjectHandle.h"
using namespace DataAccess;
using namespace Interface;

MantleFormation::MantleFormation (ProjectHandle * projectHandle, database::Record* record) : Formation ( projectHandle, record ), Interface::BasementFormation ( projectHandle, record, MantleFormationName, projectHandle->getMantleLithoName() ) {
}


MantleFormation::~MantleFormation (void)
{
}

/// Return the present day, user-supplied Mantle thickness GridMap.
const GridMap * MantleFormation::getInputThicknessMap (void) const {

   const GridMap * thicknessMap;

   if (( thicknessMap = (const GridMap *) getChild (ThicknessMap)) != 0) {
      return thicknessMap;
   } else {
      // Fixed mantle thickness.

      double presentDayMantleThickness;

      if ( m_projectHandle->getBottomBoundaryConditions () == MANTLE_HEAT_FLOW ) {
         presentDayMantleThickness = database::getLithoMantleThickness (m_record);
      } else  if ( m_projectHandle->isALC() ) {
         presentDayMantleThickness = database::getInitialLithosphericMantleThickness ( m_record );
      } else {
         presentDayMantleThickness = database::getInitialLthMntThickns (m_record);
      }

      AddConstant add ( presentDayMantleThickness );

      const Grid * grid = m_projectHandle->getActivityOutputGrid();

      if ( grid == 0 ) {
         grid = (Grid *) m_projectHandle->getInputGrid ();
      }

      thicknessMap = m_projectHandle->getFactory ()->produceGridMap ( this, ThicknessMap, grid, presentDayMantleThickness );
      assert (thicknessMap == getChild (ThicknessMap));
   }

   return thicknessMap;
}
const GridMap * MantleFormation::getInitialThicknessMap (void) const {
   const GridMap * thicknessMap;

   if (( thicknessMap = (const GridMap *) getChild (ThicknessMap)) != 0) {
      return thicknessMap;
   } else {
      // Fixed mantle thickness.

      double initMantleThickness = database::getInitialLithosphericMantleThickness ( m_record );

      AddConstant add ( initMantleThickness );

      const Grid * grid = m_projectHandle->getActivityOutputGrid();

      if ( grid == 0 ) {
         grid = (Grid *) m_projectHandle->getInputGrid ();
      }

      thicknessMap = m_projectHandle->getFactory ()->produceGridMap ( this, ThicknessMap, grid, initMantleThickness );
      assert (thicknessMap == getChild (ThicknessMap));
   }

   return thicknessMap;
}

double MantleFormation::getPresentDayThickness () const {

   double thickness;

   if ( m_projectHandle->getBottomBoundaryConditions () == MANTLE_HEAT_FLOW ) {
      thickness = database::getLithoMantleThickness ( m_record );
   } else {
      // m_projectHandle->getBottomBoundaryConditions () == FIXED_BASEMENT_TEMPERATURE
      thickness = database::getInitialLthMntThickns ( m_record );
   }

   return thickness;
}

double MantleFormation::getInitialLithosphericMantleThickness () const {

   double thickness = 0;
   if ( m_projectHandle->isALC() )
   {
      thickness = database::getInitialLithosphericMantleThickness ( m_record );
   }
   return thickness;
}

PaleoFormationPropertyList * MantleFormation::getPaleoThicknessHistory () const {
   return m_projectHandle->getMantlePaleoThicknessHistory ();
}

int MantleFormation::getDepositionSequence () const {
   return -2;
}

void MantleFormation::asString (string & str) const
{
   ostringstream buf;

   buf << "MantleFormation";
   buf << ": name = "                << getName ();
   buf << ", top surface name = "    << getTopSurface ()->getName ();
   buf << ", bottom surface name = " << getBottomSurface ()->getName ();
   buf << endl;

   str = buf.str ();
}
