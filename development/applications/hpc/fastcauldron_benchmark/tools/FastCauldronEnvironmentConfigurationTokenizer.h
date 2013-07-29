#ifndef HPC_FCBENCH_FASTCAULDRONENVIRONMENTCONFIGURATIONTOKENIZER_H
#define HPC_FCBENCH_FASTCAULDRONENVIRONMENTCONFIGURATIONTOKENIZER_H

#include <string>

#include "formattingexception.h"

namespace hpc {


class FastCauldronEnvironmentConfigurationTokenizer
{
public:
   FastCauldronEnvironmentConfigurationTokenizer(const std::string & text);

   // returns the next token and next marker
   void next( std::string & token, std::string & marker);

   // returns true iff there are more tokens
   bool hasMore() const;

   struct Exception : formattingexception :: BaseException< Exception > {};

private:
   std::string m_text;
   std::string::size_type m_posLeft, m_posRight; // position of (left or right side) of a marker 
};


}

#endif

