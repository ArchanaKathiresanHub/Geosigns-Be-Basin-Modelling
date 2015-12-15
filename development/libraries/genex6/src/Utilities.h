
#ifndef UTILITIES_H
#define UTILITIES_H


#include <string>
#include <vector>
#include <math.h>

namespace Genex6
{
  void ParseLine(const std::string &theString, const std::string &theDelimiter, std::vector<std::string> &theTokens);
  void TransformStringToUpper(std::string &theString);
  
  bool fabsEqualDouble(const double val1, const double val2, const double EPS = 1E-05);
  bool EqualDouble(const double val1, const double val2, const double EPS = 1E-05);
  std::string toLower ( const std::string& str ); 
}

#endif

