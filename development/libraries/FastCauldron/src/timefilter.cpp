//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include   "timefilter.h"

#include <petsc.h>

#include<iostream>
#include<iomanip>

#include<assert.h>
using namespace std;

TimeFilter::TimeFilter(){

   int PLCount;

   for ( PLCount=0; PLCount<ENDPROPERTYLIST; ++PLCount) {
      PropertyOutputOption[PLCount] = NOOUTPUT;
   }

   for ( PLCount=0; PLCount<ENDPROPERTYLIST; ++PLCount) {
      PropertyOutputOption[PLCount] = NOOUTPUT;
      m_string2PropertyName[PropertyName[PLCount]] = static_cast<PropertyIdentifier>(PLCount);
   }

   if ( sizeof ( PropertyName ) / sizeof ( std::string ) != ENDPROPERTYLIST + 1 ) {
      PetscPrintf ( PETSC_COMM_WORLD, "Basin_Error: The number of items in the PropertyName list does not match the number of enumerations in the PropertyName enumeration type.\n" );
      assert ( sizeof ( PropertyName ) / sizeof ( std::string ) == ENDPROPERTYLIST + 1 );
   }

}

TimeFilter::~TimeFilter(){
}

PropertyIdentifier& operator++(PropertyIdentifier& pl){
  pl = PropertyIdentifier(int(pl) + 1);
  return pl;
}

PropertyIdentifier operator++(PropertyIdentifier& pl, int i){
  PropertyIdentifier Old = pl;
  pl = PropertyIdentifier(int(pl) + 1);
  return Old;
}

PropertyIdentifier getPropertyList ( const std::string& name ) {

   int property;

   for ( property = 0; property != ENDPROPERTYLIST; ++property ) {

      if ( PropertyName [ property ] == name ) {
         return static_cast<PropertyIdentifier>(property);
      }

   }

   return ENDPROPERTYLIST;
}


void TimeFilter::setFilter(const string & propertyName, const string & outputOption){

  int PLCount;
  bool propertyFound = false;

  for ( PLCount=0; PLCount<ENDPROPERTYLIST; ++PLCount) {

    if (PropertyName[PLCount] == propertyName) {
      PropertyOutputOption[PLCount] = getOutputRange(outputOption);
      propertyFound = true;

      break;
    }
  }

  if ( ! propertyFound ) {
    // Error, property not found.
    return;
  }

  /*
  The migration module needs Depth maps at snapshots for ALL its reservoir layers
  No output or export for source rocks only is not sufficient
  */
  if ( (PLCount == DEPTH || PLCount == POROSITYVEC || PLCount == PERMEABILITYVEC ||
     PLCount == VES || PLCount == MAXVES || PLCount == TEMPERATURE ||
     PLCount == VR || PLCount == OVERPRESSURE || PLCount == PRESSURE ||
        PLCount == HYDROSTATICPRESSURE || PLCount == LITHOSTATICPRESSURE) && PropertyOutputOption[PLCount] < SEDIMENTSONLY )
  {

    if ( PropertyOutputOption[PLCount] < SEDIMENTSONLY ) {
      PropertyOutputOption[PLCount] = SEDIMENTSONLY;
    }

  }

  PropertyOutputOption [ PERMEABILITYHVEC ] = PropertyOutputOption [ PERMEABILITYVEC ];

  PropertyOutputOption[HEAT_FLOWY] = PropertyOutputOption[HEAT_FLOW];
  PropertyOutputOption[HEAT_FLOWZ] = PropertyOutputOption[HEAT_FLOW];

  PropertyOutputOption [ FLUID_VELOCITY_Y ] = PropertyOutputOption [ FLUID_VELOCITY ];
  PropertyOutputOption [ FLUID_VELOCITY_Z ] = PropertyOutputOption [ FLUID_VELOCITY ];

  PropertyOutputOption [ CHEMICAL_COMPACTION ] = NOOUTPUT;
}

void TimeFilter::setFilter(const string& propertyName, const DataAccess::Interface::PropertyOutputOption outputOption) {

   // This function is TEMPORARY.
   using namespace DataAccess;

   string outputOptionStr;

   switch ( outputOption ) {
      case Interface::NO_OUTPUT                     : outputOptionStr = "None"; break;
      case Interface::SOURCE_ROCK_ONLY_OUTPUT       : outputOptionStr = "SourceRockOnly"; break;
      case Interface::SEDIMENTS_ONLY_OUTPUT         : outputOptionStr = "SedimentsOnly"; break;
      case Interface::SEDIMENTS_AND_BASEMENT_OUTPUT : outputOptionStr = "SedimentsPlusBasement"; break;
      default: outputOptionStr = "None";
   }

   setFilter ( propertyName, outputOptionStr );
}

OutputOption TimeFilter::getOutputRange(const string & outputOption){

  int RangeCount;
  bool optionFound = false;

  for (RangeCount=0; RangeCount<4; RangeCount++) {
    if (OutputOptionName[RangeCount] == outputOption) {
      optionFound = true;
      break;
    }
  }

  if ( ! optionFound ) {
    RangeCount = 0;
  }

  return OutputOption(RangeCount);
}


bool TimeFilter::propertyIsSelected ( const PropertyIdentifier propertyId ) const {
  return PropertyOutputOption [ propertyId ] != NOOUTPUT;
}

const std::string& propertyListName ( const PropertyIdentifier property ) {

   if ( property >= 0 and property < ENDPROPERTYLIST ) {
      return PropertyName [ property ];
   } else {
      return PropertyName [ ENDPROPERTYLIST ];
   }

}

OutputOption TimeFilter::getPropertyOutputOption ( const std::string& propertyName ) const
{
   return PropertyOutputOption[ getPropertylist( propertyName ) ];
}

PropertyIdentifier TimeFilter::getPropertylist ( const std::string& propertyName ) const {

   std::map <std::string, PropertyIdentifier>::const_iterator propertyIter = m_string2PropertyName.find ( propertyName );

   if ( propertyIter != m_string2PropertyName.end ()) {
      return propertyIter->second;
   } else {
      return ENDPROPERTYLIST;
   }

}
