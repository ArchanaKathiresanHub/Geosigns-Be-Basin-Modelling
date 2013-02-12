#include "parser.h"

namespace hpc
{

Parser
   :: Parser(const std::string & text)
      : m_text(text)
      , m_pos(0)
      , m_ignoreWhiteSpace(true)
{}


void
Parser
   :: skipws()
{
   while (m_pos < m_text.size() && std::isspace(m_text[m_pos]) )
   {
      ++m_pos;
   }
}

void 
Parser
   :: expect(const std::string & string)
{
   if (m_ignoreWhiteSpace)
      skipws();

   if (string.empty())
      return;

   if ( m_text.find(string, m_pos) == m_pos)
   {
      m_pos += string.size()-1;
      ++m_pos;
   }
   else
   {
      throw ParseException() << "Expected '" << string << "' at position " << m_pos;
   }
}

bool 
Parser
   :: hasNextToken()
{
   skipws();
   return !eof();
}

std::string 
Parser
   :: nextToken() 
{
   if (!hasNextToken())
      throw ParseException() << "Unexpected end of input";
            
   if (m_text[m_pos] == '"')
   {
      ++m_pos; // skip the start quote character

      std::string token;
      while ( !eof() && m_text[m_pos] != '"')
      {
        token.push_back(m_text[m_pos]);
        ++m_pos;
      }
      
      if (!eof())
         ++m_pos; // skip the end quote 

      return token;
   }
   else if (std::isalnum(m_text[m_pos]) || m_text[m_pos] == '_')
   {
      std::string token;
      while ( !eof() && std::isalnum(m_text[m_pos]))
      {
         token.push_back(m_text[m_pos]);
         ++m_pos;
      }
      
      return token;
   }
   else 
   {
      return std::string(m_text, m_pos++, 1);
   }
}

bool 
Parser
   :: eof() const
{ 
   return m_pos == m_text.size(); 
}

std::string
Parser
   :: remaining() const
{
   return std::string(m_text, m_pos, std::string::npos);
}

}
