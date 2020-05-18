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

std::string tail(std::string const& source, size_t const length) {
    if (length >= source.size()) { return source; }
    return source.substr(source.size() - length);
} // tail

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
