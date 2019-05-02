// 
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef UTILITIES_H
#define UTILITIES_H


#include <string>
#include <vector>
#include <math.h>
#include <fstream>
#include <sstream>

namespace Genex6
{
   void ParseLine(const std::string &theString, const std::string &theDelimiter, std::vector<std::string> &theTokens);
   void TransformStringToUpper(std::string &theString);
   
   bool fabsEqualDouble(const double val1, const double val2, const double EPS = 1E-05);
   bool EqualDouble(const double val1, const double val2, const double EPS = 1E-05);
   std::string toLower ( const std::string& str ); 
   void outputToFile( std::ofstream &outfile, const double value, const int width = 0 );
   void outputToStringWithDot( std::stringstream &str, const double value ) ;
 
}

#endif

