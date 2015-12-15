#include "GlobalDefs.h"

#include <cstdlib>

const std::string& unitImage ( const DistanceUnit unit ) {

   static std::string images [ 3 ] = { "Metre", "Foot", "Unknown" };

   if ( unit == METRE ) {
      return images [ 0 ];
   } else if ( unit == FOOT ) {
      return images [ 1 ];
   } else {
      return images [ 2 ];
   }

}

const std::string& unitShortIdentifier ( const DistanceUnit unit ) {

   static std::string images [ 3 ] = { "m", "ft", "Unknown" };

   if ( unit == METRE ) {
      return images [ 0 ];
   } else if ( unit == FOOT ) {
      return images [ 1 ];
   } else {
      return images [ 2 ];
   }

}

std::string stripSpaces ( const std::string& str ) {

   std::string result;
   size_t i;
   size_t startPosition = 0;
   size_t endPosition = str.length ();

   // Find first non-space character.
   for ( startPosition = 0; startPosition < str.length (); ++startPosition ) {

      if ( str [ startPosition ] != ' ' ) {
         break;
      }

   }

   // Find last non-space character.
   for ( i = str.length () - 1; i >= startPosition; --i ) {

      if ( str [ i ] != ' ' ) {
         endPosition = i;
         break;
      }

   }

   // Get sub-string.
   result = str.substr ( startPosition, endPosition - startPosition + 1 );

   return result;
}

std::string addQuotes ( const std::string& str ) {

   return '"' + str + '"';

}


std::string addExtension ( const std::string& str,
                           const std::string& ext ) {

   if ( str.find ( '.' + ext ) == std::string::npos ) {
      // Add the extension.
      return str + '.' + ext;
   } else {
      // The extension is already there.
      return str;
   }

}


DistanceUnit convertToUnit ( const std::string & str ) {

   std::string lowerCaseStr = toLower ( str );

   if ( str == "foot" or str == "feet" ) {
      return FOOT;
   } else if ( str == "metre" or str == "meter" ) {
      return METRE;
   }

   // Error
   return METRE;
}

double convertToDouble ( const std::string & str ) {

  double result;
  char* strEnd;
  const char* buffer = str.c_str ();

  result = std::strtod ( buffer, &strEnd );

  if ( strEnd == buffer ) {
     // Error!
     // ThrowException ( Exceptions::ConstraintError,
     //                  "String '" << str << "' cannot be converted to double" );
  } // end if

  return result;
}

std::string toLower ( const std::string& str ) {

   std::string copy ( str );
   size_t i;
   char diff = 'A' - 'a';

   for ( i = 0; i < copy.length (); ++i ) {

      if ( copy [ i ] >= 'A' and copy [ i ] <= 'Z' ) {
         copy [ i ] -= diff;
      }

   }

   return copy;
}

double convertUnitValue ( const double value,
                          const DistanceUnit from, 
                          const DistanceUnit to ) {

   static double conversions [ 2 ][ 2 ] = {{ 1.0, 3.280839895 }, { 0.3048, 1.0 }};


   return value * conversions [ from ][ to ];
}

