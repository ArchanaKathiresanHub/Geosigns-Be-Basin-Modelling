//
// Copyright ( C ) 2013-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "typeDefinitions.h"

namespace DataExtraction
{

namespace ParseUtilities
{

bool splitString( char* str, char separator, char*& firstPart, char*& secondPart );
DoublePairVector parseCoordinates( char* coordinatesString );
DoubleVector parseAges( char* agesString );
StringVector parseStrings( char* stringsString );

} // namespace ParseUtilities

} // namespace DataExtraction
