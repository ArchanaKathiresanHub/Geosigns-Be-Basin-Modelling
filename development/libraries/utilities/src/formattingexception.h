//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#ifndef UTILITIES_GENERALEXCEPTION_H
#define UTILITIES_GENERALEXCEPTION_H

#include <exception>
#include <sstream>
#include <vector>

namespace formattingexception {

class GeneralException : public std::exception
{
public:
   GeneralException();
   GeneralException(const GeneralException & other);

   GeneralException & operator=( const GeneralException & other);

   // Overriding std::exception functions
   virtual ~GeneralException() throw () ;
   virtual const char * what() const throw ()  ;

   // Allow nice notation
   template <typename T> GeneralException & operator << (const T & x )
   {
      m_message << x;
      return *this;
   }

   template <typename T> GeneralException & operator << (const std::vector<T> & x )
   {
      for ( size_t i = 0; i < x.size(); ++i ) m_message << x[i] << " ";
      return *this;
   }

private:
   std::ostringstream m_message;
   mutable std::string m_formatted;
};

template <typename T>
struct BaseException : GeneralException
{
   template <typename U> T & operator<< (const U & x) 
   {
      return static_cast<T &>( GeneralException::operator<<( x ) );
   }
};

}

#endif
