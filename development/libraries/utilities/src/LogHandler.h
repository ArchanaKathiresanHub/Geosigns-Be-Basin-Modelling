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

#include <string>

/// @brief Class LogHandler keeps handling of log files and log console output. It is a base class for 
///        any other API class in IBS.
/// @detail This class should be used to create one log file per MPI rank. This class is a singleton.
class LogHandler
{
public:
   /// @name Types definitions
   /// @{
   /// @brief Set of verbosity levels for log filter
   enum Verbosity_level{
      DIAGNOSTIC,
      DETAILED,
      NORMAL,
      MINIMAL,
      QUIET
   };

   /// @brief Set of severity levels for severity levels
   enum Severity_level {
      DEBUG,
      INFO,
      WARNING,
      ERROR,
      FATAL
   };
   /// @}

   /// @brief Constructor which inititates the log file
   /// @detail The name of the log file will be "logName_mpiRank.log".
   ///   The console output is only avalaible for mpi rank 0.
   /// @param logName The main name of the log file (i.e. fastcauldron)
   /// @param verbosity The level of verbosity used as a filter for the log file
   /// @param mpiRank The MPI rank which will be added as a sufix of the log file name (i.e. "24"). Set to 0 by default.
   LogHandler( const std::string & logName, const Verbosity_level verbosity, const int& mpiRank = 0 );

   /// @brief Simple destructor switching back the singleton token to false
   ~LogHandler() { m_logIsCreated = false; };

   /// @brief Write the message in the log according to the severity level
   /// @param severity The severity level of the message
   /// @param message The message given to the log
   static void write( const Severity_level severity, const std::string & message );

   /// @brief Get the name of the log file
   /// @return m_logName The name of the log file such as "logName_mpiRank.log"
   const std::string getName() const { return m_logName; };

private:
   /// @ brief Overwrite default assginment operator
   LogHandler& operator= (const LogHandler&){};
   /// @ brief Overwrite default copy constructor
   LogHandler( const LogHandler& ){};

   std::string m_logName;         ///< Full name of the log file

protected:
   static bool m_logIsCreated;    ///< Singleton token (protected for unit test purposes only!)

};
#endif