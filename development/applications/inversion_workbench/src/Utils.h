#ifndef INVERSIO_UTILS_H
#define INVERSIO_UTILS_H

#include <string>
#include "formattingexception.h"


struct ConversionException : formattingexception::BaseException< ConversionException > {};

template <typename T> T fromString( const std::string & x )
{
   T y;
   std::istringstream s(x);
   s >> y;

   if ( !s ) throw ConversionException() << "Error while converting std::string '" << x << "' to type '" << typeid(y).name() << "'" ;

   return y;
}


#endif
