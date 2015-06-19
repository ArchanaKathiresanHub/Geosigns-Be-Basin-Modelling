//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file CauldronApp.C
/// @brief This file keeps implementation the generic part of Cauldron Applications set

#include "FilePath.h"
#include "CauldronApp.h"

#include "CauldronEnvConfig.h"

#ifndef _WIN32
#include <sys/stat.h>
#endif

#include <fstream>
#include <iostream>
#include <cstring>

namespace casa
{
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // CauldronApp methods definition
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   // Constructor
   CauldronApp::CauldronApp( ShellType sh, const std::string & appName, bool isParallel )
      : m_appName( appName )
      , m_parallel( isParallel )
      , m_sh( sh )
      , m_cpus( 1 )
      , m_inputOpt( "-project" )
      , m_outputOpt( "-save" )
      , m_clearSnapshots( false )
   {
      std::string version;
      std::string rootPath;

      switch ( m_sh )
      {
      case cmd:
         m_appName += ".exe"; // on windows all applications have .exe suffix
         version    = "%CAULDRON_VERSION%";
         rootPath   = "%IBS_ROOT%";
         break;

      case csh:
      case bash:
         version  = "${CAULDRON_VERSION}";
         rootPath = "${IBS_ROOT}";
         break;
      }

      if ( !env( "SIEPRTS_LICENSE_FILE" ) ) m_env["SIEPRTS_LICENSE_FILE"] = s_LICENSE_SERVER;

      m_version = env( "CAULDRON_VERSION" ) ? env( "CAULDRON_VERSION" ) : "v2014.0710";        // the default version is the latest available release for now
      m_rootPath = env( "IBS_ROOT" ) ? env( "IBS_ROOT" ) : "/apps/sssdev/ibs";  // path to IBS folder where the different versions are
      m_mpirunCmd = env( "CAULDRON_MPIRUN_CMD" ) ? env( "CAULDRON_MPIRUN_CMD" ) : "";                  //

      if ( m_mpirunCmd.empty() )
      {
         ibs::FilePath mpirunPath( s_MPIRUN_CMD );
         if ( !mpirunPath.exists() )
         {
            ibs::FilePath appPath( ibs::FilePath::pathToExecutable() );
            appPath << mpirunPath.fileName();
            m_mpirunCmd = appPath.path();
         }
         else
         {
            m_mpirunCmd = std::string( s_MPIRUN_CMD );
         }
#ifndef _WIN32
         m_mpirunCmd += " -env I_MPI_DEBUG 5";
#endif
      }

      // do some tunning depends on application name
      if ( appName == "fastcauldron" )
      {
         pushDefaultEnv( "EOSPACKDIR", (ibs::FolderPath( rootPath ) << version << "misc" << "eospack").path() );
         pushDefaultEnv( "GENEXDIR",   (ibs::FolderPath( rootPath ) << version << "misc" << "genex40").path() );
         pushDefaultEnv( "GENEX5DIR",  (ibs::FolderPath( rootPath ) << version << "misc" << "genex50").path() );
         pushDefaultEnv( "GENEX6DIR",  (ibs::FolderPath( rootPath ) << version << "misc" << "genex60").path() );
         pushDefaultEnv( "OTGCDIR",    (ibs::FolderPath( rootPath ) << version << "misc" << "OTGC"   ).path() );
         pushDefaultEnv( "CTCDIR",     (ibs::FolderPath( rootPath ) << version << "misc"             ).path() );
      }
      else if ( appName == "fastgenex6" )
      {
         pushDefaultEnv( "EOSPACKDIR", (ibs::FolderPath( rootPath ) << version << "misc" << "eospack").path() );
         pushDefaultEnv( "OTGCDIR",    (ibs::FolderPath( rootPath ) << version << "misc" << "OTGC"   ).path() );
         pushDefaultEnv( "GENEXDIR",   (ibs::FolderPath( rootPath ) << version << "misc" << "genex40").path() );
         pushDefaultEnv( "GENEX5DIR",  (ibs::FolderPath( rootPath ) << version << "misc" << "genex50").path() );
         pushDefaultEnv( "GENEX6DIR",  (ibs::FolderPath( rootPath ) << version << "misc" << "genex60").path() );
      }
      else if ( appName == "fastmig" )
      {
         // set up environment vars
         pushDefaultEnv( "EOSPACKDIR", (ibs::FolderPath( rootPath ) << version << "misc" << "eospack").path() );
         pushDefaultEnv( "OTGCDIR",    (ibs::FolderPath( rootPath ) << version << "misc" << "OTGC"   ).path() );
         pushDefaultEnv( "GENEXDIR",   (ibs::FolderPath( rootPath ) << version << "misc" << "genex40").path() );
         pushDefaultEnv( "GENEX5DIR",  (ibs::FolderPath( rootPath ) << version << "misc" << "genex50").path() );
         pushDefaultEnv( "CTCDIR",     (ibs::FolderPath( rootPath ) << version << "misc"             ).path() );
      }
      else if ( appName == "fastctc" )
      {
         pushDefaultEnv( "CTCDIR",     (ibs::FolderPath( rootPath ) << version << "misc"             ).path() );
      }
      else if ( appName == "datadriller" )
      {
         pushDefaultEnv( "EOSPACKDIR", (ibs::FolderPath( rootPath ) << version << "misc" << "eospack").path() );
         pushDefaultEnv( "OTGCDIR",    (ibs::FolderPath( rootPath ) << version << "misc" << "OTGC"   ).path() );
         pushDefaultEnv( "GENEX6DIR",  (ibs::FolderPath( rootPath ) << version << "misc" << "genex60").path() );
         pushDefaultEnv( "CTCDIR",     (ibs::FolderPath( rootPath ) << version << "misc"             ).path() );
         m_inputOpt = "-input";
         m_outputOpt = "-output";
      }
      else if ( appName == "tracktraps" )
      {
         pushDefaultEnv( "EOSPACKDIR", (ibs::FolderPath( rootPath ) << version << "misc" << "eospack").path() );
         m_outputOpt = "-output";
      }
      else if ( appName == "track1d" )
      {
         pushDefaultEnv( "EOSPACKDIR", (ibs::FolderPath( rootPath ) << version << "misc" << "eospack").path() );
         m_outputOpt = "| sed '1,4d' > track1d_results.csv";
      }
   }

   // generate script the application
   std::string CauldronApp::generateScript( const std::string & inProjectFile, const std::string & outProjectFile )
   {
      // in case of generic app we should put in the script the given script body
      if ( !m_scriptBody.empty() ) return m_scriptBody;

      // check if we have decompaction/overpressure/itcoupled/temperature run to clean previous 3D results
      if ( m_appName == "fastcauldron" )
      {
         for ( size_t i = 0; i < m_optionsList.size() && !m_clearSnapshots; ++i )
         {
            if (      m_optionsList[i] == "-overpressure" ) m_clearSnapshots = true;
            else if ( m_optionsList[i] == "-itcoupled"    ) m_clearSnapshots = true;
            else if ( m_optionsList[i] == "-decompaction" ) m_clearSnapshots = true;
            else if ( m_optionsList[i] == "-temperature"  )
            {
               m_clearSnapshots = true;
               for ( size_t j = 0; j < m_optionsList.size(); ++j )
               {
                  if ( m_optionsList[j] == "-coupled" ) m_clearSnapshots = false;
               }
            }
         }
      }

      // dump script top line with shell preference
      std::ostringstream oss;

      switch ( m_sh )
      {
      case bash: oss << "#!/bin/bash\n\n"; break;
      case csh:  oss << "#!/bin/csh\n\n";  break;
      case cmd:  oss << "@echo off\n\n";   break;
      }

      // dump all necessary predefined environment variables to the script
      dumpEnv( oss );

      oss << "\n";

      if ( m_clearSnapshots )
      {
         oss << "\n rm -rf " << ibs::FilePath( inProjectFile ).fileNameNoExtension() << "_CauldronOutputDir/Time*.h5\n";
      }

      // if application is parallel, add mpirun dirrective with options
      if ( m_parallel )
      {
         oss << m_mpirunCmd;
#ifndef _WIN32
         oss << " -outfile-pattern '" + m_appName + "-output-rank-%r.log' ";
#endif
         if ( m_cpus > 0 ) { oss << " -np " << m_cpus << " "; }
      }

      // dump application name with full path
      switch ( m_sh )
      {
      case csh:
      case bash: oss << "${APP}"; break;
      case cmd:  oss << "\"%APP%\""; break;
      }

      // dump app options list
      for ( size_t i = 0; i < m_optionsList.size(); ++i ) { oss << " " << m_optionsList[i]; }

      // dump input/output project name
      oss << " " << m_inputOpt << " " << inProjectFile;
      if ( m_appName.substr( 0, 7 ) == "track1d" )
      {
         oss << " " << m_outputOpt;
      }
      else if ( !outProjectFile.empty() ) oss << " " << m_outputOpt << " " << outProjectFile;

      // redirect stdout & stderr
      switch ( m_sh )
      {
      case csh:
      case bash: break;
      case cmd:  oss << " > " << m_appName << ".log" << " 2> " << m_appName << ".err"; break;
      }

      oss << "\n\n";

      // add to scrip checking of the return code of the mpirun. If it is 0 - create file Stage_X.sh.success, or Stage_X.ch.failed otherwise
      switch ( m_sh )
      {
      case bash: oss << "if [ $? -eq 0 ];    then\n   touch $(basename $BASH_SOURCE).success\n   exit 0\nelse\n   touch $(basename $BASH_SOURCE).failed\n   exit 1\nfi\n";    break;
      case csh:  oss << "if ( $status != 0 ) then\n   touch             `basename $0`.failed\n   exit 1\nelse\n   touch           `basename $0`.success\n   exit 0\nendif\n"; break;
      case cmd:  oss << "if errorlevel 1 (\n   type NUL > %~n0.bat.failed\n) else (\n   type NUL > %~n0.bat.success\n)\n"; break;
      }
      return oss.str();
   }

   const char * CauldronApp::scriptSuffix() const
   {
      switch ( m_sh )
      {
      case bash: return ".sh";
      case csh:  return ".csh";
      case cmd:  return ".bat";
      }
      return "";
   }

   // get environment variable
   const char * CauldronApp::env( const char * varName ) { return getenv( varName ); }

   // add the default value of environment variable to the list if it does not set up in environment
   bool CauldronApp::pushDefaultEnv( const std::string & varName, const std::string & varValue )
   {
      const char * envValue = env( varName.c_str() );
      m_env[varName] = envValue ? envValue : varValue;
      return envValue ? false : true;
   }

   // print the defined set of environment variables to stream
   void CauldronApp::dumpEnv( std::ostream & oss )
   {
      switch ( m_sh )
      {
      case bash:
         oss << "CAULDRON_VERSION=" << m_version << "\n";
         oss << "IBS_ROOT=" << m_rootPath << "\n\n";
         // compute path to the application
         oss << "os1=`/apps/sss/share/getos2` || { echo 'Warning: Could not determine OS version. Are we in Shell Linux?'; os1='.'; }\n"
            << "os2=`/apps/sss/share/getos2 --os --ver` || { echo 'Warning: Could not determine OS version. Are we in Shell Linux?'; os2='.'; }\n"
            << "APP=" << m_rootPath << '/' << m_version << "/${os1}/bin/" << m_appName << '\n'
            << "if [ ! -e $APP ]; then\n"
            << "   APP=" << m_rootPath << '/' << m_version << "/${os2}/bin/" << m_appName << '\n'
            << "fi\n"
            << "if [ ! -e $APP ]; then\n"
            << "   echo Could not find application executable\n"
            << "   exit 1\n"
            << "fi\n";         break;

      case csh:
         oss << "set CAULDRON_VERSION=" << m_version << "\n";
         oss << "set IBS_ROOT=" << m_rootPath << "\n\n";
         // compute path to the application
         oss << "set os1=`/apps/sss/share/getos2` || { echo 'Warning: Could not determine OS version. Are we in Shell Linux?'; os1='.'; }\n"
            << "set os2=`/apps/sss/share/getos2 --os --ver` || { echo 'Warning: Could not determine OS version. Are we in Shell Linux?'; os2='.'; }\n"
            << "set APP=" << m_rootPath << '/' << m_version << "/${os1}/bin/" << m_appName << '\n'
            << "if ( ! -l $APP ) then\n"
            << "   APP=" << m_rootPath << '/' << m_version << "/${os2}/bin/" << m_appName << '\n'
            << "endif\n"
            << "if ( ! -l $APP ) then\n"
            << "   echo Could not find application executable\n"
            << "   exit 1\n"
            << "endif\n";         break;

      case cmd:
         oss << "set CAULDRON_VERSION=" << m_version << "\n";
         oss << "set IBS_ROOT=" << m_rootPath << "\n\n";
         // compute path to the application
         oss << "set APP=" << (ibs::FilePath( m_rootPath ) << m_version << "bin" << m_appName).path();
         break;
      }
      oss << "\n";

      // dump set of environment variables
      for ( std::map< std::string, std::string >::const_iterator it = m_env.begin(); it != m_env.end(); ++it )
      {
         switch ( m_sh )
         {
         case bash:  oss << "export " << it->first << "=" << it->second << "\n"; break;
         case csh:   oss << "setenv " << it->first << " " << it->second << "\n"; break;
         case cmd:   oss << "set "    << it->first << "=" << it->second << "\n"; break;
         }
      }
   }

   // Serialize object to the given stream
   bool CauldronApp::save( CasaSerializer & sz, unsigned int fileVersion ) const
   {
      bool ok = true;

      // initial implementation
      if ( fileVersion >= 0 )
      {
         ok =  ok ? sz.save( m_env.size(), "EnvVarList" ) : ok;
         for ( std::map< std::string, std::string >::const_iterator it = m_env.begin(); it != m_env.end() && ok; ++it )
         {
            ok = sz.save( it->first, "EnvVarName" );
            ok = ok ? sz.save( it->second, "EnvVarVal" ) : ok;
         }

         ok = ok ? sz.save( m_appName,                "AppName"        ) : ok;
         ok = ok ? sz.save( m_scriptBody,             "ScriptBody"     ) : ok;
         ok = ok ? sz.save( m_parallel,               "IsAppParallel"  ) : ok;
         ok = ok ? sz.save( m_cpus,                   "CPUsNum"        ) : ok;
         ok = ok ? sz.save( static_cast<int>( m_sh ), "ShellType"      ) : ok;
         ok = ok ? sz.save( m_version,                "CldVersion"     ) : ok;
         ok = ok ? sz.save( m_rootPath,               "IBSROOT"        ) : ok;
         ok = ok ? sz.save( m_mpirunCmd,              "MPIRunCmd"      ) : ok;
         ok = ok ? sz.save( m_inputOpt,               "InputOpt"       ) : ok;
         ok = ok ? sz.save( m_outputOpt,              "OutputOpt"      ) : ok;
         ok = ok ? sz.save( m_optionsList,            "AppOptionsList" ) : ok;
      }
      return ok;
   }

   // Create a new instance and deserialize it from the given stream
   CauldronApp::CauldronApp( CasaDeserializer & dz, const char * objName )
   {
      // read from file object name and version
      unsigned int objVer = version();
      bool ok = dz.checkObjectDescription( typeName(), objName, objVer );

      size_t setSize;
      ok = ok ? dz.load( setSize, "EnvVarList" ) : ok;
      for ( size_t i = 0; i < setSize && ok; ++i )
      {
         std::string vn, vv;

         ok = ok ? dz.load( vn, "EnvVarName" ) : ok;
         ok = ok ? dz.load( vv, "EnvVarVal"  ) : ok;
         m_env[vn] = vv;
      }

      ok = ok ? dz.load( m_appName,    "AppName"         ) : ok;
      ok = ok ? dz.load( m_scriptBody, "ScriptBody"      ) : ok;
      ok = ok ? dz.load( m_parallel,   "IsAppParallel"   ) : ok;
      ok = ok ? dz.load( m_cpus,       "CPUsNum"         ) : ok;
      int sht;
      ok = ok ? dz.load( sht,           "ShellType"      ) : ok;
      m_sh = static_cast<ShellType>( sht );
      ok = ok ? dz.load( m_version,     "CldVersion"     ) : ok;
      ok = ok ? dz.load( m_rootPath,    "IBSROOT"        ) : ok;
      ok = ok ? dz.load( m_mpirunCmd,   "MPIRunCmd"      ) : ok;
      ok = ok ? dz.load( m_inputOpt,    "InputOpt"       ) : ok;
      ok = ok ? dz.load( m_outputOpt,   "OutputOpt"      ) : ok;
      ok = ok ? dz.load( m_optionsList, "AppOptionsList" ) : ok;

      if ( !ok )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "DataDiggerImpl deserialization error";
      }
   }
}

