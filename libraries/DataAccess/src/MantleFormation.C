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

#include "Interface/MantleFormation.h"
#include "Interface/Formation.h"
#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"
#include "Interface/ObjectFactory.h"
#include "Interface/ProjectHandle.h"

using namespace DataAccess;
using namespace Interface;

MantleFormation::MantleFormation (ProjectHandle * projectHandle, database::Record* record) : Formation ( projectHandle, record ), Interface::BasementFormation ( projectHandle, record, MantleFormationName, MantleLithologyName ) {
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
      } else  if ( m_projectHandle->getBottomBoundaryConditions () == ADVANCED_LITHOSPHERE_CALCULATOR ) {
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
   if ( m_projectHandle->getBottomBoundaryConditions () == ADVANCED_LITHOSPHERE_CALCULATOR ) {
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
#ifdef USESTANDARD
   ostringstream buf;
#else
   strstream buf;
#endif

   buf << "MantleFormation";
   buf << ": name = " << getName ();
   buf << ", top surface name = " << getTopSurface ()->getName ();
   buf << ", bottom surface name = " << getBottomSurface ()->getName ();
   buf << endl;

   str = buf.str ();
#ifndef USESTANDARD
   buf.rdbuf ()->freeze (0);
#endif
}
