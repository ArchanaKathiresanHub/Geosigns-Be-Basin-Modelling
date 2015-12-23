//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include "FormattingException.h"

namespace formattingexception
{

GeneralException 
   :: GeneralException()
   : m_message()
   , m_formatted()
{}

GeneralException
   :: GeneralException(const GeneralException & other)
   : std::exception(other)
   , m_message(other.m_message.str())
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
     return "Unknown exception";
   else
     return m_formatted.c_str();
}

}
