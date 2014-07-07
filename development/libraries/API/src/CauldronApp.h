//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file CauldronApp.h
/// @brief This file keeps API declaration for dealing with Cauldron app executables

#ifndef CASA_API_CAULDRON_APP_H
#define CASA_API_CAULDRON_APP_H

#include "ErrorHandler.h"
#include "FilePath.h"
#include "FolderPath.h"

#include <map>
#include <string>
#include <sstream>
#include <vector>

namespace casa
{
   /// @brief Class for creating command line for cauldron application
   class CauldronApp 
   {
   public:   
      /// @brief Shell which will be used for script generation
      typedef enum
      {
         bash = 0,
         csh,
         cmd
      } ShellType;

      /// @brief Constructor
      /// @param sh shell which will be used for scripts
      /// @param appName name of an application like fastcauldron/fasttouch7/fastgenex6/...
      /// @param isParallel is this application will be run as a parallel app (requires mpirun in command line)
      CauldronApp( ShellType sh, const std::string & appName, bool isParallel = true );

      /// @brief Destructor
      virtual ~CauldronApp() {;}

      /// @brief Set shell for scripts. The default is bash
      /// @param sh shell name
      void setShellType( ShellType sh ) { m_sh = sh; }
      
      /// @brief Set which version of the Cauldron app will be used. This string is used as part of path to an application
      /// @param ver cauldron version like "v2014.7nightly"
      virtual void setCauldronVersion( const std::string & ver ) { if ( !ver.empty() ) m_version = ver; }

      /// @brief Generates script file which contains environment set up and application run for given input/output project file
      /// @param inProjectFile input project file name
      /// @param outProjectFile output project file name
      /// @return generated script as a string
      virtual std::string generateScript( const std::string & inProjectFile, const std::string & outProjectFile );

      /// @brief Set path where applications with different versions are. Could be also set through IBS_ROOT environment variable
      /// @param rootPath path to IBS folder with different versions of cauldron
      virtual void setPathToApp( const std::string & rootPath ) { m_rootPath = rootPath; }

      /// @brief Set the number of MPI processes which will be used for the application, if it is a parallel application
      /// @param numOfCPUs number of cpus for parallel simulation
      void setCPUs( int numOfCPUs ) { m_cpus = numOfCPUs; }

      /// @brief Get cpus number for this application
      /// @return cpus number
      int cpus() { return m_cpus; }

      /// @brief Add application option
      /// @param opt new option
      void addOption( const std::string & opt ) { m_optionsList.push_back( opt ); }

   protected:

      std::map< std::string, std::string >   m_env; ///< keeps environment variables in map: [variable] -> value
      std::string                            m_appName;      ///< name of application like fastcauldron/genex6/...
      bool                                   m_parallel;     ///< is this application parallel?
      int                                    m_cpus;         ///< how many cpus should use application

      ShellType                              m_sh;  ///< type of shell (setting environment variables differ in different shell scripts

      std::string                            m_version;      ///< Cauldron version which will be used
      std::string                            m_rootPath;     ///< path prefix to applications root like /apps/sssdev/ibs
      std::string                            m_mpiEnv;       ///< path to script with environment set up for mpirun
      std::string                            m_mpirunCmd;    ///< mpirun command with parameters

      std::vector< std::string >             m_optionsList;  ///< List of options for the application
   
      /// @brief get environment variable
      const char * env( const char * varName );

      /// @brief Print list of environment variable to the output stream
      /// @param oss output stream
      void dumpEnv( std::ostream & oss );

      /// @brief Add environment variable to the list. If such variable exists in environment,\n
      ///        picks up it value from environment and does not use given variable value
      /// @param varName name of environment variable
      /// @param varValue the value of the environment variable if it does not exist in environment
      /// @return true if given varValue was used, false otherwise
      bool pushDefaultEnv( const std::string & varName, const std::string & varValue );

      CauldronApp( const CauldronApp & anotherApp );
      CauldronApp & operator = ( const CauldronApp & anotherApp );
   };
}

#endif // CASA_API_CAULDRON_APP_H

