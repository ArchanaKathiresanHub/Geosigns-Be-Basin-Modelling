//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
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

   typedef formattingexception::GeneralException logHandlerException;

public:
   /// @name Types definitions
   /// @{
   /// @brief Set of verbosity levels for log filter
   /// @details The log will write only the messages with Verbosity_level<=Severity_level;
   enum VerbosityLevel{
      DIAGNOSTIC,
      DETAILED,
      NORMAL,
      MINIMAL,
      QUIET
   };

   /// @brief Set of severity levels for log messages
   /// @details The log will write only the messages with Verbosity_level<=Severity_level;
   enum SeverityLevel {
      DEBUG,
      INFO,
      WARNING,
      ERROR,
      FATAL
   };
   /// @}

   /// @brief Constructor which inititates the boost log file 
   /// @detail The name of the log file will be "logName_mpiRank.log".
   ///   The console output is only avalaible for mpi rank 0.
   ///   The log created is a global boost object.
   ///   This constructor can be called only once per application since we create on log file per application.
   ///   Once called this constructor switches the singleton token s_logIsCreated to true.
   /// @param logName The main name of the log file (i.e. fastcauldron)
   /// @param verbosity The level of verbosity used as a filter for the log file
   /// @param mpiRank The MPI rank which will be added as a sufix of the log file name (i.e. "24"). Set to 0 by default.
   LogHandler( const std::string & logName, const VerbosityLevel verbosity, const int& mpiRank = 0 );

   /// @brief Constructor which must be used to write into the boost log file
   /// @param severity The severity level of the message
   LogHandler( const SeverityLevel severity );

   /// @brief Destructor which writes the stream (m_oss) in the boost log according to the current severity level
   ~LogHandler();

   /// @brief Stream operator to write into the log file
   /// @details Should be used like this:
   ///    -# LogHandler(DEBUG) << "This is my debug message with value" << 4 << " or " << m_value;
   ///    -# LogHandler(INFO)  << "This is my info message with value"  << 4 << " or " << m_value;
   ///    -# etc.
   template <typename T> LogHandler & operator << (T val){
      m_oss << val;
      return *this;
   };

   /// @brief Get the name of the log file
   /// @details Used for unit tests only
   /// @return s_logName The name of the log file such as "fastcauldron_0.log"
   const std::string getName() const { return s_logName; };

private:
   /// @ brief Overwrite default assginment operator
   LogHandler& operator= (const LogHandler&);
   /// @ brief Overwrite default copy constructor
   LogHandler( const LogHandler& );

   static std::string s_logName;         ///< Full name of the log file (from constructor logName_mpiRank.log)
   static bool        s_logIsCreated;    ///< Singleton token
   SeverityLevel      m_severity;        ///< The current severity level
   std::ostringstream m_oss;             ///< The stream containing the message to be writen in the log file during destruction of the object

};
#endif