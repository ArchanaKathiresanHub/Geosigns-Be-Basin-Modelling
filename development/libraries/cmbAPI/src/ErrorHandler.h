//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ErrorHandler.h 
/// @brief This file keeps API declaration for errors handling for set of Cauldron APIs

#ifndef ERROR_HANDLER_API
#define ERROR_HANDLER_API

#include "FormattingException.h"
#include <string>

/// @brief Class ErrorHandler keeps handling of error codes, error messages and is a base class for 
///        any other API class
class ErrorHandler
{
public:
   /// @name Types definitions
   /// @{
   /// @brief Set of return codes for API functions
   typedef enum
   {
      NoError = 0,           ///< Call was successful
      IoError,               ///< Error related to IO operations
      MemAllocError,         ///< Error with memory allocation
      NonexistingID,         ///< Unknown ID of object is given
      UndefinedValue,        ///< The requested value not defined (for uninitialized values)
      OutOfRangeValue,       ///< The given value is out of allowed range 
      NotImplementedAPI,     ///< For not implemented API methods
      AlreadyDefined,        ///< The value was already defined, and can't be redefined again
      MutationError,         ///< Error happened during mutation of the base case
      ValidationError,       ///< Error happened during validation of mutated case
      MonteCarloSolverError, ///< Error happened in MC solver
      RSProxyError,          ///< Error happened in response surface builder
      SUMLibException,       ///< Exception was caught during SUMLib call
      RunManagerError,       ///< Error happened in RunManager
      RunManagerAborted,     ///< Execution of run manager was aborted by user
      LSFLibError,           ///< Error happened in RunManager when it interracts with LSF
      WrongPath,             ///< Wrong path given
      SerializationError,    ///< Error happens during serialization process
      DeserializationError,  ///< Error happens during deserialization process
      UnknownError           ///< Unknown error
   } ReturnCode;
   /// @}

   class Exception : public formattingexception::BaseException<Exception>
   {
   public:
      Exception( ReturnCode           errCode ) : m_errCode( errCode ) { ; }
      Exception( const ErrorHandler & eh      ) : m_errCode( eh.errorCode() ) { m_message << eh.errorMessage(); }

      ReturnCode errorCode() const { return m_errCode; }

   protected:
      ReturnCode m_errCode;
   };

   /// @name Error handling functions
   /// @{
   /// @brief If any error is happened during interface call this function will return error message
   /// @return error message which is valid till next interface call.
   std::string errorMessage() const { return m_lastErrorMsg; }

   /// @brief If any error is happened during interface call this function will return error code
   /// @return error code
   ReturnCode  errorCode() const { return m_retCode; }
   /// @}

   /// @brief Report error and setup message and error code. Also used by an API objects which has no its own ErrorHandler
   /// @param rc Error code
   /// @param msg Error message with error description
   /// @return Error code
   ReturnCode reportError( ReturnCode rc, const std::string & msg )  { m_retCode = rc; m_lastErrorMsg = msg; return rc; }

   /// @brief Clean error code and error message
   void resetError() { m_retCode = NoError; m_lastErrorMsg.clear(); }

   /// @brief Move error message and error code from the given ErrorHandler to the current
   /// @param otherErrHandler other error handler from which error must be moved to the current
   /// @return error code
   ReturnCode moveError( ErrorHandler & otherErrHandler )
   {
      m_lastErrorMsg = otherErrHandler.errorMessage();
      m_retCode = otherErrHandler.errorCode();
      otherErrHandler.resetError();
      return m_retCode;
   }

protected:
   /// @name Constructors/destructor
   /// @{
   /// @brief Constructor which creates empty model
   ErrorHandler() : m_retCode( NoError ) {;}
   /// @brief Destructor, no any actual work is needed here, all is done in implementation part
   ~ErrorHandler() {;}
   /// @}

   /// @name Error handling interface for API functions
   /// @{
   /// @brief Set error message and error code
   /// @param rc Error code
   /// @param msg Error message which usually comes with exception
   void setErrorMessage( ReturnCode rc, const std::string & msg ) { m_retCode = rc; m_lastErrorMsg = msg; }
   /// @}

private:
   /// @name Implementation part
   /// @{
   std::string m_lastErrorMsg; ///< keeps error message
   ReturnCode  m_retCode;      ///< keeps error code
   /// @}
};

#endif // ERROR_HANDLER_API
