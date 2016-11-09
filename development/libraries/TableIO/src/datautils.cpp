#include "datautils.h"

#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>
using namespace std;

bool loadLine (istream & infile, string & line, bool checkForHeaders)
{
   string separators = " \t";
   while (true)
   {
      getline (infile, line, '\n');
      if ( infile.eof() )
      {
	 return false;
      }
      else
      {
         // check for CRLF EOL
         if ( !line.empty() && line[line.size() - 1] == '\r' )
         {
            line.erase( line.size() - 1 );
         }

	 if (checkForHeaders && line.find (";! ") == 0)
	 {
	    return true;
	 }

	 size_t firstNonSpace = line.find_first_not_of (separators, 0);

	 if (firstNonSpace == string::npos) // empty line
	    continue;
	 else if (line[firstNonSpace] == ';') // comment possibly with leading spaces
	    continue;
	 else
	    return true;
      }
   }
}

// searches from linePos, returns the 'word' in word and returns the linePos beyond 'word'
size_t loadWordFromLine (const string & line, size_t linePos, string & word)
{
   const string separators = " \t";
   const string quote = "\"";
   word = "";

   size_t wordStartPos = line.find_first_not_of (separators, linePos);
   if (wordStartPos == string::npos) return string::npos;

   if (line[wordStartPos] == quote[0])
   {
      // a string delimited by '"'
      size_t wordEndPos;
      while (true)
      {
	 wordEndPos = line.find_first_of (quote, wordStartPos + 1);
	 if (wordEndPos == string::npos) return string::npos;
	 if (line[wordEndPos - 1] != '\\') // escaped "
	 {
	    size_t wordLength = wordEndPos - (wordStartPos + 1);
	    word += line.substr (wordStartPos + 1, wordLength);
	    break;
	 }
	 else
	 {
	    size_t wordLength = (wordEndPos - 1) - (wordStartPos + 1);
	    word += line.substr (wordStartPos + 1, wordLength);
	    word += "\"";
	    wordStartPos = wordEndPos;
	 }
      }

      return wordEndPos + 1;
   }
   else
   {
      // not a string but a numerical value
      size_t wordEndPos = line.find_first_of (separators, wordStartPos + 1);

      size_t wordLength;
      if (wordEndPos == string::npos)
	 wordLength = string::npos;
      else
	 wordLength = wordEndPos - wordStartPos;

      word = line.substr (wordStartPos, wordLength);
      return wordEndPos + 1;
   }
}


bool saveStringToStream (ostream & ofile, const string & word, int & borrowed, int fieldWidth)
{
#ifdef linux
   // setw () does not work like it should on (this version of) linux
   int wordLength = std::max(fieldWidth - borrowed, (int) word.length ());
   borrowed = std::max(wordLength + borrowed - fieldWidth, 0);

   int spaceLength = wordLength - word.length ();
   while (spaceLength > 0)
   {
      ofile << " ";
      --spaceLength;
   }
   ofile << word;
#else
   int wordLength = std::max(fieldWidth - borrowed, (int) word.length ());
   borrowed = std::max(wordLength + borrowed - fieldWidth, 0);

   ofile << setw (wordLength) << word;
#endif
   return !ofile.fail ();
}

bool findAndRemoveDelimiters (string & line, const char delimiters[])
{
   if (line[0] != delimiters[0]) return false;

   size_t closingDelimPos = line.find (delimiters[1]);
   if (closingDelimPos == string::npos) return false;

   line.erase (closingDelimPos); // until end of line
   line.erase (0, 1);

   return true;
}

