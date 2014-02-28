// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_EXCEPTION_H
#define SUMLIB_EXCEPTION_H

#include <string>
#include <stdexcept>
#include <sstream>

/**
 * This defines an exception throw where there are no additional messages
 * for the user other than the one defined by the exception class itself.
 *
 * @param type The name of the class to throw. The class to throw can be
 *             defined using the EXCEPTION macro.
 */
#undef THROW
#define THROW(type) \
   throw type(__FILE__, __LINE__)

/**
 * This defines an exception throw where additional messages for the user can
 * be provided.
 *
 * @param type The name of the class to throw
 * @param txt  The additional message to pass to the exception class. The type
 *             of @c txt @e MUST be of std::string!
 */
#define THROW2(type, txt) \
   throw type(__FILE__, __LINE__, (txt))

/**
 * This is the base class from which any exception defined in the SUM application
 * inherits.
 *
 * It is basically a std::runtime_error exception with an extra boolean to
 * indicate if it has been caught previously. This makes it possible to avoid
 * for example double actions.
 *
 * @param file The source file in which the exception occurred.
 * @param line The line at which the exception occurred.
 * @param msg  The message for the exception type.
 */
class Exception : public std::runtime_error
{
public:
   bool caught;
   Exception( const std::string& file,
              unsigned int line,
              const std::string& msg ):
   std::runtime_error( "File: " + file + ",Line: " + to_string( line ) + ", Message: " + msg ),
   caught( false )
   {
        // empty
   }
private:
    static std::string to_string( unsigned int val )
    {
        std::ostringstream o;
        o << val;
        return o.str();
    }
};

/**
 * This defines an exception class with a standard message.
 *
 * The defined class is a derivative of std::runtime_error, so this exception
 * and possible parents (std::exception) can also be caught. Since this class
 * is a derivative, it can be interrogated using what().
 *
 * @param klass The name of the class to throw as an exception. The name
 *              @c klass is on purpose to avoid problems with compiling.
 * @param msg   The standard message for the exception class.
 */
#define EXCEPTION( exception_class, msg )               \
   class exception_class : public Exception             \
   {                                                    \
   public:                                              \
      exception_class(                                  \
             const std::string& f,                      \
             unsigned int l,                            \
             const std::string& m = "")                 \
         : Exception( f, l,                             \
               m.empty() ? std::string(msg)             \
                       : std::string(msg":" ) + (m) )   \
      {                                                 \
      }                                                 \
   }

// Widely used exceptions
//

EXCEPTION( ModificationError, "The value has already been set." );
EXCEPTION( InvalidValue, "The parameter value(s) are not allowed." );
EXCEPTION( InvalidState, "The instance is in an invalid state" );
EXCEPTION( IndexOutOfBounds, "An index value is out of bounds" );
EXCEPTION( ValueOutOfBounds, "A value is out of bounds" );
EXCEPTION( DimensionMismatch, "Array dimensions do not match" );
EXCEPTION( DivisionByZero, "Denominator of division is zero" );
EXCEPTION( DimensionOutOfBounds, "Array dimensions are out of bounds" );
EXCEPTION( EmptyString, "String is empty" );
EXCEPTION( ItemNotFound, "The item specified was not found" );
EXCEPTION( DuplicateItem, "Duplicate items are not allowed" );
EXCEPTION( UnequalSize, "The sizes do not match." );
EXCEPTION( CalculationError, "Calculation error." );

#endif // SUMLIB_EXCEPTION_H
