//
// Copyright ( C ) 2013-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "parseUtilities.h"

#include <cstring>

namespace DataExtraction
{

namespace ParseUtilities
{

bool splitString( char* str, char separator, char*& firstPart, char*& secondPart )
{
  if ( !str || std::strlen( str ) == 0 ) return false;
  secondPart = std::strchr( str, separator );
  if ( secondPart != 0 )
  {
    *secondPart = '\0';
    ++secondPart;
    if ( *secondPart == '\0' ) secondPart = 0;
  }

  firstPart = str;
  return std::strlen( firstPart ) != 0;
}

DoublePairVector parseCoordinates( char* coordinatesString )
{
  DoublePairVector coordinatePairs;
  char* strPtr = coordinatesString;
  char* sectionX;
  char* sectionY;
  while ( splitString( strPtr, ',', sectionX, strPtr ) &&
          splitString( strPtr, ',', sectionY, strPtr ) )
  {
    double numberX, numberY;

    numberX = std::atof( sectionX );
    numberY = std::atof( sectionY );

    coordinatePairs.push_back( DoublePair( numberX, numberY ) );
  }
  return coordinatePairs;
}

StringVector parseStrings( char* stringsString )
{
  StringVector strings;
  char* strPtr = stringsString;
  char* section;
  while ( splitString( strPtr, ',', section, strPtr ) )
  {
    strings.push_back( std::string( section ) );
  }
  return strings;
}

DoubleVector parseAges( char* agesString )
{
  DoubleVector ages;
  char* strPtr = agesString;
  char* commasection;
  char* section;
  while ( splitString( strPtr, ',', commasection, strPtr ) )
  {
    while ( splitString( commasection, '-', section, commasection ) )
    {
      double number = atof( section );
      ages.push_back( number );
    }
    ages.push_back( -1 ); // separator
  }
  ages.push_back( -1 ); // separator
  return ages;
}

} // namespace ParseUtilities

} // namespace DataExtraction
