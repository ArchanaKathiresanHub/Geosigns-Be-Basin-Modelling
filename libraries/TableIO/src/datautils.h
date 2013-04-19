#ifndef _DATAUTILS_H
#define _DATAUTILS_H

#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include<fstream>
      using std::istream;
      using std::ofstream;
      using std::ostream;
   #else // !_STANDARD_C_PLUS_PLUS
      #include<fstream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <fstream>
   using std::istream;
   using std::ofstream;
   using std::ostream;
#endif // sgi


#include <string>

#ifdef Min
#undef Min
#endif
template < class Type >
inline Type Min (const Type & a, const Type & b)
{
   return a < b ? a : b;
}

#ifdef Max
#undef Max
#endif
template < class Type >
inline Type Max (const Type & a, const Type & b)
{
   return a > b ? a : b;
}

extern bool loadLine (istream & infile, std::string & line, bool checkForHeaders = false);
extern size_t loadWordFromLine (std::string & line, size_t linePos, std::string & word);
extern bool findAndRemoveDelimiters (std::string & line, const char delimiters[]);

extern bool saveStringToStream (ostream & ofile, const std::string & word,
      int & borrowed, int fieldWidth = 15);

#endif // _DATAUTILS_H
