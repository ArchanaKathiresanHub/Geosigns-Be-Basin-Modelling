#include "Utilities.h"
#include <string.h>
#include <algorithm>
#include <iomanip>

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

void outputToFile( std::ofstream &outfile, const double value, const int width ) 
{
   outfile << ",";
   if( width > 0 ) {
      outfile << std::setw(width);
   }
   if( value != 0.0 ) {
      outfile << value;
   } else {
      outfile << " ";
   }
}
void outputToStringWithDot( std::stringstream &str, const double value ) 
{
   // output double value as "6." if the fraction part is 0
   if( value == 0.0 ) {
      str << "";
   } else {
      double intpart;
      if( std::modf( value, &intpart ) == 0.0 ) {
         str << value << ".";
      } else {
         str << value;
      } 
   }
}

}
