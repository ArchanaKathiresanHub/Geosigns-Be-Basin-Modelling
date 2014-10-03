// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_EXCEPTION_H
#define SUMLIB_EXCEPTION_H

#include <cassert>
#include <string>
#include <stdexcept>
#include <sstream>
#include <vector>

namespace SUMlib
{

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
   throw type( txt )

/**
 * This is the base class from which any exception defined in the SUM application
 * inherits.
 *
 * It is basically a std::runtime_error exception with an extra boolean to
 * indicate if it has been caught previously. This makes it possible to avoid
 * for example double actions.
 *
 * @param msg  The message for the exception type.
 */
class Exception : public std::runtime_error
{

   public:
      bool caught;
      Exception( const std::string& msg ) :
         std::runtime_error( msg ),
         caught( false )
      {}

   private:
       static std::string to_string( unsigned int val )
       {
           std::ostringstream o;
           o << val;
           return o.str();
       }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class InvalidTransforms
/// @brief Exception that is raised during propxy building when not all transformations are valid.
///
/// Note: The parameter indices that are reported should be corrected for fixed parameters.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class InvalidTransforms : public Exception
{
   public:
      /// Build exception.
      /// @param[in] observableIndices       : SUMlib indices for the observables that have invalid transforms.
      /// @param[in] parameterIndices        : SUMlib indices for the parameters that have invalid transforms (Important: fixed parameters included in index).
      /// @param[in] reasons                 : The reason why the transformation is invalid.
      InvalidTransforms( const std::vector< size_t >& observableIndices, const std::vector< size_t >& parameterIndices, const std::vector< std::string>& reasons ) :
         Exception( "" ),
         m_observableIndices( observableIndices ),
         m_parameterIndices( parameterIndices ),
         m_reasons( reasons )
      {
         assert( m_observableIndices.size() == m_parameterIndices.size() );
         assert( m_parameterIndices.size() == m_reasons.size() );
      }

      virtual ~InvalidTransforms() throw() {}

      /// Return the number of invalid transforms. This can be used to loop over and query the observable, parameter
      /// combinations and reasons.
      size_t getNumInvalidTransforms() const
      {
         return m_observableIndices.size();
      }

      /// Get the observable index for the invalid transform @param invalidTransformIndex.
      size_t getObservableIndex( size_t invalidTransformIndex ) const
      {
         return m_observableIndices[ invalidTransformIndex ];
      }

      /// Get the parameter index (for parameter vectors including fixed parameters) for the invalid transform @param invalidTransformIndex.
      size_t getParameterIndex( size_t invalidTransformIndex ) const
      {
         return m_parameterIndices[ invalidTransformIndex ];
      }

      /// Get the reason why the transform is invalid for invalid transform @param invalidTransformIndex.
      const std::string& getReason( size_t invalidTransformIndex ) const
      {
         return m_reasons[ invalidTransformIndex ];
      }

   private:
      std::vector< size_t >         m_observableIndices;
      std::vector< size_t >         m_parameterIndices;
      std::vector< std::string >    m_reasons;

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
             const std::string& m = "")                 \
         : Exception(                                   \
               m.empty() ? std::string(msg)             \
                       : std::string(msg": " ) + (m) )   \
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
EXCEPTION( ProxyEvaluateError, "Attempt to evaluate the proxy in an invalid region" );

} /// namespace SUMlib

#endif // SUMLIB_EXCEPTION_H
