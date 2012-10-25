#ifndef TOKENIZE_HH
#define TOKENIZE_HH
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

//! the function is used for tokenizing the input file
//! it reads line by line and split into strings and store in a vector
/*!
  \param str it is the line of the file in string format using getline function
  \param tokens vector of strings where the splitted strings stored
  \param delimiters delimiters like space or dot or etc.
 */
void tokenize(const std::string& str,
			  std::vector<std::string>& tokens,
			  const std::string& delimiters = " ")
{

  tokens.clear();
    // Skip delimiters at beginning.
  std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
  std::string::size_type pos     = str.find_first_of(delimiters, lastPos);
	
  while (std::string::npos != pos || std::string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}
#endif
