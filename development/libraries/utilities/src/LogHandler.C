//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include "LogHandler.h"

// Utilities
#include "FormattingException.h"

// Boost Log library
#include <boost/date_time/time_facet.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

// Initialise singleton token
bool LogHandler::m_logIsCreated;

LogHandler::LogHandler( const std::string & logName, const Verbosity_level verbosity, const int& mpiRank ){

   // C++11 const std::string mpiRankString = std::to_string( mpiRank );
   std::ostringstream mpiRankConverter;
   mpiRankConverter << mpiRank;
   std::string mpiRankString = mpiRankConverter.str();
   m_logName = logName + "_" + mpiRankString + ".log";

   if (!m_logIsCreated){

      ///1. Initiate log file name and format
      boost::log::add_file_log
         (
         // Write on the fly
         boost::log::keywords::auto_flush = true,
         boost::log::keywords::file_name = m_logName,
         // Time stamp format is "<Year-Month-Day Hour:Minute:SecondWithFraction | MPIRank>:" like in "<2005-10-15 13:12:11.000000 | 4>:"
         // More info @ http://www.boost.org/doc/libs/1_59_0/doc/html/date_time/date_time_io.html
         boost::log::keywords::format =
         (
         boost::log::expressions::stream
         << "<"
         << boost::log::expressions::format_date_time< boost::posix_time::ptime >( "TimeStamp", "%Y-%m-%d %H:%M:%S.%f" )
         << " | " << mpiRankString
         << ">: " << boost::log::expressions::smessage
         )
         );

      ///2. Add the console output if serial or only for the first rank if distributued
      if (mpiRank == 0)
      {
         boost::log::add_console_log
            ( std::cout
            , boost::log::keywords::format = "%Message%"
            );
      }

      ///3. Set log file verbosity
      switch (verbosity)
      {
      case DIAGNOSTIC:  boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::debug   );    break;
      case DETAILED:    boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::info    );    break;
      case NORMAL:      boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::warning );    break;
      case MINIMAL:     boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::error   );    break;
      case QUIET:       boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::fatal   );    break;
      default: throw formattingexception::GeneralException() << "Unknown verbosity level for logging file " << m_logName << "."; break;
      }

      boost::log::add_common_attributes();
      m_logIsCreated = true;
   }
   else{
      throw formattingexception::GeneralException() << "Log file " << m_logName << " already created.";
   }
}


void LogHandler::write( const Severity_level severity, const std::string & message ){

   if (m_logIsCreated) {
      switch (severity)
      {
      case DEBUG:     BOOST_LOG_TRIVIAL( debug )   << "MeSsAgE DEBUG    " << message; break;
      case INFO:      BOOST_LOG_TRIVIAL( info )                           << message; break;
      case WARNING:   BOOST_LOG_TRIVIAL( warning ) << "MeSsAgE WARNING  " << message; break;
      case ERROR:     BOOST_LOG_TRIVIAL( error )   << "MeSsAgE ERROR    " << message; break;
      case FATAL:     BOOST_LOG_TRIVIAL( fatal )   << "MeSsAgE FATAL    " << message; break;
      default: throw formattingexception::GeneralException() << "Unknwon severity level '" << severity << "'."; break;
      }
   }
   else {
      throw formattingexception::GeneralException() << "Cannot find log file for current application.";
   }
}
