#ifndef HPC_FCBENCH_PARSER_H
#define HPC_FCBENCH_PARSER_H

#include <string>

#include "formattingexception.h"

namespace hpc
{

 
struct ParseException : formattingexception::BaseException< ParseException > {};
class Parser
{
public:
   Parser(const std::string & text);

   bool hasNextToken();
   bool eof() const;
   
   void skipws();
   void expect(const std::string & string);

   std::string nextToken() ;

   std::string remaining() const;

private:
   bool m_ignoreWhiteSpace;
   std::string m_text;
   std::string::size_type m_pos;
} ;

}

#endif
