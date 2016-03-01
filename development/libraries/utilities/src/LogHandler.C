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
bool LogHandler::s_logIsCreated = false;
// Initialise log file name
std::string LogHandler::s_logName;

LogHandler::LogHandler( const std::string & logName, const VerbosityLevel verbosity, int mpiRank ){

   // C++11 const std::string mpiRankString = std::to_string( mpiRank );
   std::ostringstream mpiRankConverter;
   mpiRankConverter << mpiRank;
   std::string mpiRankString = mpiRankConverter.str();
   std::string fullLogName = logName + "_" + mpiRankString + ".log";

   m_severity = INFO_SEVERITY;
   if (!s_logIsCreated){
      s_logName = fullLogName;

      ///1. Initiate log file name and format
      boost::log::add_file_log
         (
         // Write on the fly
         boost::log::keywords::auto_flush = true,
         boost::log::keywords::file_name = s_logName,
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
      case LogHandler::DIAGNOSTIC_LEVEL:  boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::debug   );  break;
      case LogHandler::DETAILED_LEVEL:    boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::info );     break;
      case LogHandler::NORMAL_LEVEL:      boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::warning );  break;
      case LogHandler::MINIMAL_LEVEL:     boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::error );    break;
      case LogHandler::QUIET_LEVEL:       boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::fatal );    break;
      default: throw LogHandlerException() << "Unknown verbosity level for logging file " << s_logName << "."; break;
      }

      boost::log::add_common_attributes();
      s_logIsCreated = true;
   }
   else{
      throw LogHandlerException() << "Log file '" << s_logName << "' already created, cannot create new log file '" << fullLogName << "'.";
   }
}

LogHandler::LogHandler( const SeverityLevel severity ){
   m_severity = severity;
}

LogHandler::~LogHandler(){
   if (!m_oss.str().empty()){
      try{
         if (s_logIsCreated) {
            switch (m_severity)
            {
            case LogHandler::DEBUG_SEVERITY:     BOOST_LOG_TRIVIAL( debug )   << "MeSsAgE DEBUG    " << m_oss.str(); break;
            case LogHandler::INFO_SEVERITY:      BOOST_LOG_TRIVIAL( info )                           << m_oss.str(); break;
            case LogHandler::WARNING_SEVERITY:   BOOST_LOG_TRIVIAL( warning ) << "MeSsAgE WARNING  " << m_oss.str(); break;
            case LogHandler::ERROR_SEVERITY:     BOOST_LOG_TRIVIAL( error )   << "MeSsAgE ERROR    " << m_oss.str(); break;
            case LogHandler::FATAL_SEVERITY:     BOOST_LOG_TRIVIAL( fatal )   << "MeSsAgE FATAL    " << m_oss.str(); break;
            default: throw LogHandlerException() << "Unknwon severity level '" << m_severity << "'."; break;
            }
         }
         else {
            //Do not throw exception yet since not all applications have a log handler initialized and some libraries are using the LogHandler
            //First add the logHandler to all applications, and then reactivate this throw --> Requirement 56618
            //throw LogHandlerException() << "Cannot find log file for current application.";
         }
      }
      catch (LogHandlerException ex) {
         BOOST_LOG_TRIVIAL( error ) << "MeSsAgE ERROR    " << ex.what();
      }
      catch (...){
         BOOST_LOG_TRIVIAL( fatal ) << "MeSsAgE FATAL    " << "Fatal log error.";
      }
   }
}