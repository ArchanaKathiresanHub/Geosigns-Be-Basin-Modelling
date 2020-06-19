//
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file LogHandler.h
/// @brief This file is a loging utility (using Boost log) for all IBS applications and libraries

#ifndef UTILITIES_LOGHANDLER_H
#define UTILITIES_LOGHANDLER_H

/// std library
#include <string>
#include <sstream>

// utilities library
#include "FormattingException.h"

/// @brief Class LogHandler keeps handling of log files and log console output. It is a base class for
///        any other API class in IBS.
/// @detail This class should be used to create one log file per MPI rank.
class LogHandler
{

   typedef formattingexception::GeneralException LogHandlerException;

public:

   /// @name Types definitions
   /// @{
   /// @brief Set of verbosity levels for log filter
   /// @details The log will write only the messages with Verbosity_level<=Severity_level;
   enum VerbosityLevel{
      DIAGNOSTIC_LEVEL,
      DETAILED_LEVEL,
      NORMAL_LEVEL,
      MINIMAL_LEVEL,
      QUIET_LEVEL
   };

   /// @brief Set of severity levels for log messages
   /// @details The log will write only the messages with Verbosity_level<=Severity_level;
   enum SeverityLevel {
      DEBUG_SEVERITY,
      INFO_SEVERITY,
      WARNING_SEVERITY,
      ERROR_SEVERITY,
      FATAL_SEVERITY
   };
   /// @}

   /// @brief Set the style for log messages
   /// @details The log will apply the specified style to the provided message
   enum Style {
      /// @brief Uses the style of the provided message
      /// @details
      /// DEFAULT
      DEFAULT,
      /// @brief Adds a line of "_" characters and puts the text in the middle of next line between "_" characters
      /// @details
      /// _____________________________________________
      /// ____________________TITLE____________________
      TITLE,
      /// @brief Adds a line of "///" characters and puts the text in the next line after "/// " characters
      /// @details
      /// ///////////////
      /// /// SECTION
      SECTION,
      /// @brief Adds "/// " characters before the text
      /// @details
      /// /// SUBSECTION
      SUBSECTION,
      /// @brief Adds  "   -> " characters before the text
      /// @details
      ///    -> COMPUTATION_STEP
      COMPUTATION_STEP,
      /// @brief Adds  "      # " characters before the text
      /// @details
      ///       # COMPUTATION_SUBSTEP
      COMPUTATION_SUBSTEP,
      /// @brief Adds  "        " characters before the text
      /// @details
      ///         COMPUTATION_DETAILS
      COMPUTATION_DETAILS
   };
   /// @}

   /// @brief Constructor which inititates the boost log file
   /// @detail The name of the log file will be "logName_mpiRank.log".
   ///   The console output is only avalaible for mpi rank 0.
   ///   The log created is a global boost object.
   ///   This constructor can be called only once per application since we create on log file per application.
   ///   Once called this constructor switches the singleton token s_logIsCreated to true.
   /// @param[in] logName The main name of the log file (i.e. fastcauldron)
   /// @param[in] verbosity The level of verbosity used as a filter for the log file
   /// @param[in] mpiRank The MPI rank which will be added as a sufix of the log file name (i.e. "24"). Set to 0 by default.
   LogHandler( const std::string & logName, const VerbosityLevel& verbosity, int mpiRank = 0 );

   /// @brief Constructor which must be used to write into the boost log file
   /// @details Style is set to default
   /// @param[in] severity The severity level of the message
   explicit LogHandler( const SeverityLevel& severity );

   /// @brief Constructor which must be used to write into the boost log file
   /// @param[in] severity The severity level of the message
   /// @param[in] style The style of the message
   LogHandler( const SeverityLevel& severity, const Style& style );

   /// @brief Delete default assginment operator
   LogHandler & operator = ( const LogHandler & ) = delete;

   /// @brief Delete default move assginment operator
   LogHandler & operator = ( LogHandler && ) = delete;

   /// @brief Delete default copy constructor
   LogHandler( const LogHandler & ) = delete;

   /// @brief Delete default move copy constructor
   LogHandler( LogHandler && ) = delete;

   /// @brief Destructor which writes the stream (m_oss) in the boost log according to the current severity level
   ~LogHandler();

   /// @brief Logs the provided time in hours/minutes/secons
   /// @param[in] timeToDisplay The time to display in seconds
   /// @param[in] severity The severity level of the message
   /// @param[in] msgToDisplay The message to display
   static void displayTime( const SeverityLevel severity, const double timeToDisplay, const char * msgToDisplay );

   /// @brief Stream operator to write simple types into the log file
   /// @details Should be used like this:
   ///    -# LogHandler(DEBUG) << "This is my debug message with value" << 4 << " or " << m_value;
   ///    -# LogHandler(INFO)  << "This is my info message with value"  << 4 << " or " << m_value;
   ///    -# etc.
   template <typename T> LogHandler & operator << ( T val )
   {
      m_oss << val;
      return *this;
   }

   /// @brief Stream operator to write string vectors into the log file
   /// @details Should be used like this:
   ///    -# LogHandler(DEBUG) << "This is my debug message with values" << m_stringVector;
   ///    -# LogHandler(INFO)  << "This is my info message with values"  << m_stringVector;
   ///    -# etc.
   template <typename T> LogHandler & operator << ( std::vector<T> vectVal )
   {
      m_oss << "[";
      for ( size_t i = 0; i < vectVal.size(); i++ )
      {
         m_oss << vectVal[i];
         if ( i < vectVal.size() - 1 )
         {
            m_oss << ";";
         }
      }
      m_oss << "]";

      return *this;
   }

   /// @brief Get the name of the log file
   /// @details Used for unit tests only
   /// @return s_logName The name of the log file such as "fastcauldron_0.log"
   const std::string getName() const { return s_logName; }

private:

   /// @brief Apply the current style to the current osstream
   void applyStyle();

   static std::string s_logName;         ///< Full name of the log file (from constructor logName_mpiRank.log)
   static bool        s_logIsCreated;    ///< Singleton token
   SeverityLevel      m_severity;        ///< The current severity level
   Style              m_style;           ///< The current logging style
   std::ostringstream m_oss;             ///< The stream containing the message to be writen in the log file during destruction of the object
};
#endif
