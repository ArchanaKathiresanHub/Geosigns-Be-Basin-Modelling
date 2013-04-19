#include "generalexception.h"

namespace hpc 
{

GeneralException 
  :: GeneralException()
  : m_message()
  , m_formatted()
{}

GeneralException
  :: GeneralException(const GeneralException & other)
  : m_message(other.m_message.str())
  , m_formatted()
{}

GeneralException & 
GeneralException
  :: operator=( const GeneralException & other)
{
  m_message.str( other.m_message.str() );
  m_formatted.clear();
  return *this;
}

GeneralException
 :: ~GeneralException() throw ()
{
}

const char * 
GeneralException
 :: what() const throw () 
{
  m_formatted = m_message.str();

  if (m_formatted.empty())
    return "probe exception";
  else
    return m_formatted.c_str();
}

}
