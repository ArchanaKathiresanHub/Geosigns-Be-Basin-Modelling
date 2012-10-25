#include "stdafx.h"
//
// Overloading << and >> operators for 
// stream functionality with FixedStrings
//

#include "h5_struct_types.h"
#include <string.h>
FixedString& FixedString::operator= (const char *s)
{
   safeCopy (str, s);
   return *this;
}

bool FixedString::stringOkay (const char *s)
{
   return ( s && (strlen (s) < MAX_STRING_SIZE) );
}

void FixedString::safeCopy (char *s1, const char *s2)
{
   if ( stringOkay (s2) )
   {
      strcpy (s1, s2);
   }
   else
   {
      strError (cout);
   }
}

void FixedString::strError (ostream &os)
{
   os << endl << "Error in FixedString::copy"
      << endl << "string is null or greather than MAX size" 
      << endl;
}   
