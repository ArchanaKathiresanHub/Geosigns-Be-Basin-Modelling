#include "Utilities.h"
#include <string.h>
#include <algorithm>

// Should be "#ifdef linux"
#ifndef sun 
#ifndef sgi
#include <cctype>
#endif
#endif

namespace Genex6
{
std::string toLower ( const std::string& str ) {

    std::string strCpy( str );

    std::transform ( strCpy.begin(), strCpy.end(), strCpy.begin(), ::tolower );
    return strCpy;
}
 
void ParseLine(const std::string &theString, const std::string &theDelimiter, std::vector<std::string> &theTokens)
{
   std::string::size_type startPos = 0;
   std::string::size_type endPos = 0;

   std::string::size_type increment = 0;
   std::string Token;

   if(theString.empty() || theDelimiter.empty()) {
      return;
   }
   while(endPos!=std::string::npos) {
      endPos = theString.find_first_of(theDelimiter,startPos);
      increment = endPos - startPos;

      Token = theString.substr(startPos,increment);
      if(Token.size() != 0) {
         theTokens.push_back(Token);
      }
      startPos += increment + 1;
   }
}
void TransformStringToUpper(std::string &theString)
{
   std::string::iterator it;
   for(it = theString.begin(); it != theString.end(); ++ it) {
      (*it) = toupper(*(it));
   }

}
bool fabsEqualDouble(const double val1, const double val2, const double EPS )
{
   return (fabs((fabs(val1) - fabs(val2))) < EPS );
}
bool EqualDouble(const double val1, const double val2, const double EPS )
{
   return (fabs(val1 - val2) < EPS);
}

}
