#ifndef _DATAUTILS_H
#define _DATAUTILS_H

#include <iosfwd>
#include <string>

extern bool loadLine (std::istream & infile, std::string & line, bool checkForHeaders = false);
extern size_t loadWordFromLine (const std::string & line, size_t linePos, std::string & word);

extern bool saveStringToStream (std::ostream & ofile, const std::string & word,
      int & borrowed, int fieldWidth = 15);

extern bool findAndRemoveDelimiters (std::string & line, const char delimiters[]);

#endif // _DATAUTILS_H
