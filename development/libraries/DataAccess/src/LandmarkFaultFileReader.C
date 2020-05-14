//
// Copyright (C) 2015-2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <stdlib.h>
#include <string.h>

#include "LandmarkFaultFileReader.h"

using namespace DataAccess;
using namespace Interface;


//------------------------------------------------------------//

LandmarkFaultFileReader::LandmarkFaultFileReader () {
}

//------------------------------------------------------------//

LandmarkFaultFileReader::~LandmarkFaultFileReader () {
}

//------------------------------------------------------------//

void LandmarkFaultFileReader::preParseFaults () {

  if ( ! m_isOpen )
  {
    fprintf ( stderr,
              "****************    ERROR LandmarkFaultFileReader::readFault   fault file is not open   ****************\n");
    return;
  }


  char buffer [ FaultFileLineLength ];


  Point         faultPoint;
  PointSequence newFaultLine;
  std::string   newFaultName;
  bool          newFault;
  DistanceUnit  unit;

  readBuffer ( buffer );

  while ( m_faultFile.good ()) {
    newFaultName = getFaultName ( buffer );

    unit = getDistanceUnit ( buffer );
    newFault = false;
    newFaultLine.clear ();

    while ( m_faultFile.good () && ! newFault ) {
      faultPoint [ X_COORD ] = getXCoordinate ( buffer );
      faultPoint [ Y_COORD ] = getYCoordinate ( buffer );
      faultPoint [ Z_COORD ] = 0.0;

      convertDistanceUnits ( unit, METRE, faultPoint [ X_COORD ]);
      convertDistanceUnits ( unit, METRE, faultPoint [ Y_COORD ]);

      newFaultLine.push_back ( faultPoint );

      readBuffer ( buffer );

      if ( m_faultFile.good ()) {
        newFault = (newFaultName != getFaultName ( buffer ));
      }

    }

    addFault ( newFaultName, newFaultLine );
  }
}

//------------------------------------------------------------//

void LandmarkFaultFileReader::readBuffer ( char* buffer ) {

  do {
    buffer [ 0 ] = 0;
    m_faultFile.getline ( buffer, FaultFileLineLength );

    ///
    /// 156 here is the location in the string where the domain units is specified.
    ///
  } while ( m_faultFile.good () && ( strlen ( buffer ) < 156 || isComment ( buffer )));

}

//------------------------------------------------------------//

bool LandmarkFaultFileReader::isComment ( const char* ) const {
  //
  // What are the landmark fault file comments characters?
  //
  return false;
}

//------------------------------------------------------------//

double LandmarkFaultFileReader::getDouble ( const char* buffer,
                                            const int   numberStart,
                                            const int   numberLength ) const {

  ///
  /// The number is not going to be longer than the file line.
  ///
  char number [ LandmarkFaultFileLineLength ];
  int  I;

  for ( I = 0; I < LandmarkFaultFileLineLength; I++ ) {
    number [ I ] = 0;
  }

  strncpy ( number, &buffer [ numberStart ], numberLength );
  return atof ( number );
}

//------------------------------------------------------------//

int LandmarkFaultFileReader::getInteger ( const char* buffer,
                                          const int   numberStart,
                                          const int   numberLength ) const {

  ///
  /// The number is not going to be longer than the file line.
  ///
  char number [ LandmarkFaultFileLineLength ];
  int  I;

  for ( I = 0; I < LandmarkFaultFileLineLength; I++ ) {
    number [ I ] = 0;
  }

  strncpy ( number, &buffer [ numberStart ], numberLength );
  return atoi ( number );
}

//------------------------------------------------------------//

std::string LandmarkFaultFileReader::getString ( const char* buffer,
                                                 const int   stringStart,
                                                 const int   stringLength ) const {

  ///
  /// The number is not going to be longer than the file line.
  ///
  char subString [ LandmarkFaultFileLineLength ];

  for ( size_t I = 0; I < LandmarkFaultFileLineLength; I++ ) {
    subString [ I ] = 0;
  }

  strncpy ( subString, &buffer [ stringStart ], stringLength );

  ///
  /// Now, remove all trailing ' ' (space) characters.
  ///
  for ( size_t I = strlen ( subString ); I > 0; I-- ) {

    if ( subString [ I - 1 ] != ' ' ) {
      subString [ I ] = 0;
      break;
    }

  }


  return std::string ( subString );
}

//------------------------------------------------------------//

double LandmarkFaultFileReader::getXCoordinate ( const char* buffer ) const {

  return getDouble ( buffer, FAULT_X_START, FAULT_X_LENGTH );
}

//------------------------------------------------------------//

double LandmarkFaultFileReader::getYCoordinate ( const char* buffer ) const  {

  return getDouble ( buffer, FAULT_Y_START, FAULT_Y_LENGTH );
}

//------------------------------------------------------------//

double LandmarkFaultFileReader::getZCoordinate ( const char* buffer ) const  {

  return getDouble ( buffer, FAULT_Z_START, FAULT_Z_LENGTH );
}

//------------------------------------------------------------//

int LandmarkFaultFileReader::getPointType  ( const char* buffer ) const {
  return getInteger ( buffer, FAULT_POINT_TYPE_START, FAULT_POINT_TYPE_LENGTH );
}

//------------------------------------------------------------//

std::string LandmarkFaultFileReader::getFaultName ( const char* buffer ) const  {
  return getString ( buffer, FAULT_NAME_START, FAULT_NAME_LENGTH );
}

//------------------------------------------------------------//

DistanceUnit LandmarkFaultFileReader::getDistanceUnit ( const char* buffer ) const  {

  std::string unit = getString ( buffer, FAULT_DISTANCE_UNIT_START, FAULT_DISTANCE_UNIT_LENGTH );

  if ( unit == "feet" ) {
    return FOOT;
  } else { // if ( unit == "metre" || unit == "meter" ) {
    return METRE;
  }

}

//------------------------------------------------------------//
