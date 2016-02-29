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
      m_string2PropertyName[PropertyName[PLCount]] = static_cast<PropertyList>(PLCount);
   }

   if ( sizeof ( PropertyName ) / sizeof ( std::string ) != ENDPROPERTYLIST + 1 ) {
      PetscPrintf ( PETSC_COMM_WORLD, "MeSsAgE ERROR The number of items in the PropertyName list does not match the number of enumerations in the PropertyName enumeration type.\n" );
      assert ( sizeof ( PropertyName ) / sizeof ( std::string ) == ENDPROPERTYLIST + 1 );
   }

}

TimeFilter::~TimeFilter(){
};

PropertyList& operator++(PropertyList& pl){
  pl = PropertyList(int(pl) + 1);
  return pl;
};

PropertyList operator++(PropertyList& pl, int i){
  PropertyList Old = pl;
  pl = PropertyList(int(pl) + 1);
  return Old;
}

PropertyList getPropertyList ( const std::string& name ) {

   int property;

   for ( property = 0; property != ENDPROPERTYLIST; ++property ) {

      if ( PropertyName [ property ] == name ) {
         return static_cast<PropertyList>(property);
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

  PropertyOutputOption[STERANEAROMATISATION] = PropertyOutputOption[BIOMARKERS];
  PropertyOutputOption[STERANEISOMERISATION] = PropertyOutputOption[BIOMARKERS];
  PropertyOutputOption[HOPANEISOMERISATION] = PropertyOutputOption[BIOMARKERS];

  PropertyOutputOption [ CHEMICAL_COMPACTION ] = NOOUTPUT;
};

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


bool TimeFilter::propertyIsSelected ( const PropertyList propertyId ) const {
  return PropertyOutputOption [ propertyId ] != NOOUTPUT; 
}

const std::string& propertyListName ( const PropertyList property ) {

   if ( property >= 0 and property < ENDPROPERTYLIST ) {
      return PropertyName [ property ];
   } else {
      return PropertyName [ ENDPROPERTYLIST ];
   }

}

PropertyList TimeFilter::getPropertylist ( const std::string& propertyName ) const {

   std::map <std::string, PropertyList>::const_iterator propertyIter = m_string2PropertyName.find ( propertyName );

   if ( propertyIter != m_string2PropertyName.end ()) {
      return propertyIter->second;
   } else {
      return ENDPROPERTYLIST;
   }

}

#if 0

Interface::PropertyOutputOption MajorProperty::getMaximumOutputOption ( const PropertyList    property,
                                                                        const CalculationMode calculation ) {

   static const Interface::PropertyOutputOption NONE = Interface::NO_OUTPUT;
   static const Interface::PropertyOutputOption SEDS = Interface::SEDIMENTS_ONLY_OUTPUT;
   static const Interface::PropertyOutputOption SEBA = Interface::SEDIMENTS_AND_BASEMENT_OUTPUT;

   static const Interface::PropertyOutputOption maximumOutputOptions [ NumberOfMajorProperties ][ NumberOfCalculationModes ] = 
      {{ NONE, NONE, SEBA, NONE, SEBA, NONE, SEBA, NONE }, /* DIFFUSIVITYVEC */
       { SEBA, NONE, SEBA, SEDS, SEBA, NONE, SEBA, NONE }, /* POROSITYVEC */
       { NONE, NONE, SEBA, NONE, SEBA, NONE, SEBA, NONE }, /* VELOCITYVEC */
       { NONE, NONE, SEBA, NONE, SEDS, NONE, SEBA, NONE }, /* REFLECTIVITYVEC */
       {, NONE }, /* SONICVEC */
       { NONE, NONE, SEBA, SEDS, NONE, NONE, SEBA, NONE }, /* BULKDENSITYVEC */
       { NONE, NONE, SEBA, NONE, SEBA, NONE, SEBA, NONE }, /* THCONDVEC */
       { NONE, NONE, SEBA, SEDS, SEBA, NONE, SEBA, NONE }, /* PERMEABILITYVEC */
       { SEBA, SEDS, SEBA, SEDS, SEDS, SEDS, SEBA, NONE }, /* DEPTH */
       { NONE, NONE, SEBA, NONE, SEBA, NONE, SEBA, NONE }, /* HEAT_FLOW */
       { NONE, NONE, NONE, SEDS, NONE, NONE, SEDS, NONE }, /* FLUID_VELOCITY */
//        {, NONE }, /* ISOSTATICMASS */ 
//        {, NONE }, /* ISOSTATICDEFLECTION */
//        {, NONE }, /* ISOSTATICWB */
//        {, NONE }, /* MASSFLUX */
       { SEBA, SEDS, SEBA, SEDS, SEBA, SEDS, SEBA, NONE }, /* MAXVES */
       { NONE, NONE, SEBA, NONE, SEBA, NONE, SEBA, NONE }, /* TEMPERATURE */ 
       { NONE, NONE, SEDS, SEDS, SEDS, NONE, SEDS, NONE }, /* PRESSURE */
       { NONE, NONE, SEDS, NEDS, SEDS, NONE, SEDS, NONE }, /* HYDROSTATICPRESSURE */
       { NONE, NONE, SEBA, SEDS, SEBA, NONE, SEBA, NONE }, /* LITHOSTATICPRESSURE */
       { NONE, NONE, SEDS, SEDS, NONE, NONE, SEDS, NONE }, /* OVERPRESSURE */
       { SEBA, SEDS, SEBA, SEDS, SEBA, SEDS, SEBA, NONE }, /* VES */
       { NONE, NONE, SEDS, NONE, SEDS, NONE, SEDS, NONE }, /* VR */
       {, NONE }, /* BIOMARKERS */
       {, NONE }, /* ILLITEFRACTION */
       { NONE, NONE, NONE, SEDS, NONE, NONE, SEDS, NONE }, /* ALLOCHTHONOUS_LITHOLOGY */
       { NONE, NONE, SEDS, SEDS, SEDS, NONE, SEDS, NONE }, /* EROSIONFACTOR */ 
       { NONE, NONE, NONE, SEDS, NONE, NONE, SEDS, NONE }, /* FAULTELEMENTS */
       { NONE, NONE, NONE, SEDS, NONE, NONE, SEDS, NONE }, /* FCTCORRECTION */
       { SEBA, SEDS, SEBA, SEDS, SEBA, SEDS, SEBA, NONE }, /* THICKNESS */
       { NONE, NONE, NONE, SEDS, NONE, NONE, SEDS, NONE }, /* THICKNESSERROR */
       { NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE }, /* CHEMICAL_COMPACTION */
       {, NONE }}; /* LITHOLOGY */

   return maximumOutputOptions [ property ][ calcualtion ];

}

Interface::PropertyOutputOption MajorProperty::getMinimumOutputOption ( const PropertyList    property,
                                                                        const CalculationMode calculation ) {
}


Interface::PropertyOutputOption MajorProperty::getMaximumOutputOption ( const std::string&      property,
                                                                        const CalculationMode   calculation ) {
   return getMaximumOutputOption ( MajorPropertyNameValue ( property ), calculation );
}

Interface::PropertyOutputOption MajorProperty::getMinimumOutputOption ( const std::string&      property,
                                                                        const CalculationMode   calculation ) {
   return getMinimumOutputOption ( MajorPropertyNameValue ( property ), calculation );
}

#endif
