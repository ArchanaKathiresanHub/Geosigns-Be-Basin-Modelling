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

// CASA API
#include "FilePath.h"
#include "FolderPath.h"
#include "CauldronApp.h"
#include "CauldronEnvConfig.h"

// CMB API
#include "cmbAPI.h"

#ifndef _WIN32
#include <sys/stat.h>
#endif

// STL/C lib
#include <fstream>
#include <iostream>
#include <cstring>
      
namespace casa
{
   const char * CauldronApp::s_resFilesList[][6]  = { { "PressureAndTemperature_Results.HDF"             // PTSolve 
                                                      , "HydrostaticTemperature_Results.HDF"
                                                      , "HydrostaticDecompaction_Results.HDF"
                                                      , "AllochthonousModelling_Results.HDF"
                                                      , "InterpolationResults.HDF"
                                                      , ""
                                                      }
                                                    , { "CrustalThicknessCalculator_Results.HDF", "", "", "", "", "" }     
                                                    , { "Genex5_Results.HDF"                    , "", "", "", "", "" }  // Genex
                                                    , { "HighResDecompaction_Results.HDF"       , "", "", "", "", "" }  // HiResDecompaction
                                                    , { "HighResMigration_Results.HDF"          , "", "", "", "", "" }  // Migration
                                                    , { "FastTouch_Results.HDF"                 , "", "", "", "", "" }
                                                    , { "datadriller_Results.HDF"               , "", "", "", "", "" }  // Postprocessing
                                                    };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // CauldronApp methods definition
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   // Constructor
   CauldronApp::CauldronApp( ShellType sh, const std::string & appName, bool isParallel )
      : m_appName( appName )
      , m_parallel( isParallel )
      , m_cpus( 1 )
      , m_sh( sh )
      , m_inputOpt( "-project" )
      , m_outputOpt( "-save" )
      , m_clearSnapshots( false )
      , m_appDepLevel( PTSolver )
   {
      std::string miscPath;

      switch ( m_sh )
      {
      case cmd:
         miscPath= "%CAULDRON_MISC_PATH%";
         break;

      case bash:
         miscPath = "${CAULDRON_MISC_PATH}";
         break;
      }

      if ( !env( "SIEPRTS_LICENSE_FILE" ) ) m_env["SIEPRTS_LICENSE_FILE"] = LICENSE_SERVER;

      m_version   = env( "CAULDRON_VERSION" )    ? env( "CAULDRON_VERSION" )    : DEFAULT_VERSION;  // default is the ver. of the build itself
      m_rootPath  = env( "IBS_ROOT" )            ? env( "IBS_ROOT" )            : IBS_INSTALL_PATH; // path to IBS folder
      m_mpirunCmd = env( "CAULDRON_MPIRUN_CMD" ) ? env( "CAULDRON_MPIRUN_CMD" ) : "";                 //
         
      // do some tunning depends on application name
      if ( m_appName == "fastcauldron" )
      {
         pushDefaultEnv( "EOSPACKDIR", (ibs::FolderPath( miscPath ) << "eospack").path() );
         pushDefaultEnv( "GENEXDIR",   (ibs::FolderPath( miscPath ) << "genex40").path() );
         pushDefaultEnv( "GENEX5DIR",  (ibs::FolderPath( miscPath ) << "genex50").path() );
         pushDefaultEnv( "GENEX6DIR",  (ibs::FolderPath( miscPath ) << "genex60").path() );
         pushDefaultEnv( "OTGCDIR",    (ibs::FolderPath( miscPath ) << "OTGC"   ).path() );
         pushDefaultEnv( "CTCDIR",     (ibs::FolderPath( miscPath )             ).path() );
      }
      else if ( m_appName == "fastgenex6" )
      {
         pushDefaultEnv( "EOSPACKDIR", (ibs::FolderPath( miscPath ) << "eospack").path() );
         pushDefaultEnv( "OTGCDIR",    (ibs::FolderPath( miscPath ) << "OTGC"   ).path() );
         pushDefaultEnv( "GENEXDIR",   (ibs::FolderPath( miscPath ) << "genex40").path() );
         pushDefaultEnv( "GENEX5DIR",  (ibs::FolderPath( miscPath ) << "genex50").path() );
         pushDefaultEnv( "GENEX6DIR",  (ibs::FolderPath( miscPath ) << "genex60").path() );
         pushDefaultEnv( "CTCDIR",     (ibs::FolderPath( miscPath )             ).path() );
         m_appDepLevel = Genex;
      }
      else if ( m_appName == "fastmig" )
      {
         // set up environment vars
         pushDefaultEnv( "EOSPACKDIR", (ibs::FolderPath( miscPath ) << "eospack").path() );
         pushDefaultEnv( "OTGCDIR",    (ibs::FolderPath( miscPath ) << "OTGC"   ).path() );
         pushDefaultEnv( "GENEXDIR",   (ibs::FolderPath( miscPath ) << "genex40").path() );
         pushDefaultEnv( "GENEX5DIR",  (ibs::FolderPath( miscPath ) << "genex50").path() );
         pushDefaultEnv( "CTCDIR",     (ibs::FolderPath( miscPath )             ).path() );
         m_appDepLevel = Migration;
      }
      else if ( m_appName == "fastctc" )
      {
         pushDefaultEnv( "CTCDIR",     (ibs::FolderPath( miscPath )             ).path() );
      }
      else if ( m_appName == "datadriller" )
      {
         pushDefaultEnv( "EOSPACKDIR", (ibs::FolderPath( miscPath ) << "eospack").path() );
         pushDefaultEnv( "OTGCDIR",    (ibs::FolderPath( miscPath ) << "OTGC"   ).path() );
         pushDefaultEnv( "GENEX6DIR",  (ibs::FolderPath( miscPath ) << "genex60").path() );
         pushDefaultEnv( "CTCDIR",     (ibs::FolderPath( miscPath )             ).path() );
         m_inputOpt = "-input";
         m_outputOpt = "-output";
         m_appDepLevel = Postprocessing;
      }
      else if ( m_appName == "tracktraps" )
      {
         pushDefaultEnv( "EOSPACKDIR", (ibs::FolderPath( miscPath ) << "eospack").path() );
         m_outputOpt = "-output";
         m_appDepLevel = Postprocessing;
      }
      else if ( m_appName == "track1d" )
      {
         pushDefaultEnv( "CTCDIR",     (ibs::FolderPath( miscPath )             ).path() );
         pushDefaultEnv( "EOSPACKDIR", (ibs::FolderPath( miscPath ) << "eospack").path() );
         m_outputOpt = "| sed '1,4d' > track1d_results.csv";
         m_appDepLevel = Postprocessing;
      }
      else if ( m_appName == "casa" )
      {
        m_inputOpt = "";
        m_outputOpt = "";
      }
   }

   // generate script the application
   std::string CauldronApp::generateScript( const std::string & inProjectFile, const std::string & outProjectFile, const std::string & scenarioID )
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
      case bash: oss << "#!/bin/bash\n\n"; oss << "export CASA_SCENARIO_ID='"  << scenarioID << "'\n\n";  break;
      case cmd:  oss << "@echo off\n\n";   oss << "set    CASA_SCENARIO_ID=\"" << scenarioID << "\"\n\n"; break;
      }

      // dump all necessary predefined environment variables to the script
      dumpEnv( oss );

      oss << "\n";

      if ( m_clearSnapshots )
      {
         switch ( m_sh )
         {
         case bash: oss << "\nrm -rf " << ibs::FilePath( inProjectFile ).fileNameNoExtension( ) << mbapi::Model::s_ResultsFolderSuffix << "/Time*.h5\n\n"; break;
         case cmd:  oss << "\ndel " << ibs::FilePath( inProjectFile ).fileNameNoExtension( ) << mbapi::Model::s_ResultsFolderSuffix << "\\*Time*.h5\n\n"; break;
         }         
      }

      // if application is parallel, add mpirun dirrective with options
      if ( m_parallel )
      {
         if ( m_mpirunCmd.empty() ) { oss  << "${APP_BIN_PATH}/" << MPIRUN_CMD; }
         else                       { oss << m_mpirunCmd;                       }
#ifndef _WIN32
         oss << " -env I_MPI_DEBUG 5 -outfile-pattern '" + m_appName + "-output-rank-%r.log' ";
#endif

#ifndef _WIN32
         if ( m_cpus > 0 ) { oss << " -n " << m_cpus << " "; }
#else
         if ( m_cpus > 0 ) { oss << " -np " << m_cpus << " "; }
#endif
      }

      // dump application name with full path
      switch ( m_sh )
      {
      case bash: oss << "${APP}"; break;
      case cmd:  oss << "\"%APP%\""; break;
      }

      // dump app options list
      for ( size_t i = 0; i < m_optionsList.size(); ++i ) { oss << " " << m_optionsList[i]; }

      // dump input/output project name
      if ( m_appName.substr( 0, 4 ) != "casa" )
      {
         oss << " " << m_inputOpt << " " << inProjectFile;
      }

      if (      m_appName.substr( 0, 7 ) == "track1d" ) { oss << " " << m_outputOpt; }
      else if ( m_appName.substr( 0, 4 ) == "casa"    ) { ; }
      else if ( !outProjectFile.empty()               ) { oss << " " << m_outputOpt << " " << outProjectFile; }

      // redirect stdout & stderr
      switch ( m_sh )
      {
      case bash: break;
      case cmd:  oss << " > " << m_appName << ".log" << " 2> " << m_appName << ".err"; break;
      }

      oss << "\n\n";

      // add to scrip checking of the return code of the mpirun. If it is 0 - create file Stage_X.sh.success, or Stage_X.ch.failed otherwise
      switch ( m_sh )
      {
      case bash: oss << "if [ $? -eq 0 ];    then\n   touch $(basename $BASH_SOURCE).success\n   exit 0\nelse\n   touch $(basename $BASH_SOURCE).failed\n   exit 1\nfi\n";    break;
      case cmd:  oss << "if errorlevel 1 (\n   type NUL > %~n0.bat.failed\n) else (\n   type NUL > %~n0.bat.success\n)\n"; break;
      }
      return oss.str();
   }

   const char * CauldronApp::scriptSuffix() const
   {
      switch ( m_sh )
      {
      case bash: return ".sh";
      case cmd:  return ".bat";
      }
      return "";
   }

   // Generate script which will create links for files with simulation results
   std::string CauldronApp::generateCopyResultsScript( const std::string & fromProj, const std::string & toProj, const std::string & scenarioID )
   {
      std::ostringstream oss;

      // create path to projdiff
      ibs::Path appPath( m_version );  // version could be set as a full path to the application executable
      ibs::Path miscPath( appPath );

      appPath << "projdiff";

      switch ( m_sh )
      {
      case bash: 
         oss << "#!/bin/bash\n\n"; oss << "export CASA_SCENARIO_ID='"  << scenarioID << "'\n\n";
         oss << "CAULDRON_VERSION=" << m_version << "\nIBS_ROOT=" << m_rootPath << "\n\n";

            // compute path to the projdiff utility
         oss << "os1=`/apps/sss/share/getos2` || { echo 'Warning: Could not determine OS version. Are we in Shell Linux?'; os1='.'; }\n"
             << "os2=`/apps/sss/share/getos2 --os --ver` || { echo 'Warning: Could not determine OS version. Are we in Shell Linux?'; os2='.'; }\n"
             << "APP=" << m_rootPath << '/' << m_version << "/${os1}/bin/projdiff\n"
             << "if [ ! -e $APP ]; then\n"
             << "   APP=" << ( ibs::Path::applicationFullPath() << "projdiff" ).path() << "\n"
             << "fi\n"
             << "if [ ! -e $APP ]; then\n   echo Could not find application executable: ${APP}\n   exit 1\nfi\n\nallOk=0;\n\n";
         break;

      case cmd:
         oss << "@echo off\n\nset    CASA_SCENARIO_ID=\"" << scenarioID << "\"\n\n";     
         oss << "set CAULDRON_VERSION=" << m_version << "\nset IBS_ROOT=" << m_rootPath << "\n\n";

         if ( !appPath.exists() ) { oss << "set APP=" << ( ibs::Path::applicationFullPath() << "projdiff").path(); }
         else                     { oss << "set APP=" << appPath.path() << '\n'; }
         oss << "\nset allOK=0\n\n";
         break;
      }

      ibs::FilePath fromPath( fromProj );
      ibs::FilePath toPath( toProj );

      // construct full paths to the results files
      const std::string & fromProjectName = fromPath.fileNameNoExtension();
      fromPath.cutLast(); // cut project file name
      fromPath << fromProjectName + mbapi::Model::s_ResultsFolderSuffix;
      
      const std::string & newProjectName  = toPath.fileNameNoExtension();
      toPath.cutLast(); // cut project file name
      toPath << newProjectName + mbapi::Model::s_ResultsFolderSuffix;

      // create new folder for results files if it doesn't exist
      if ( !toPath.exists() ) ibs::FolderPath( toPath.path() ).create();

      switch( m_appDepLevel ) 
      {
         case PTSolver:
            switch( m_sh )
            {
               case bash: 
                  oss << "# Linking time stamp 3d prop files:\n"; 
                  oss << "ln -s " << fromPath.path() << "/Time_*.h5 " << toPath.path() << "\nif [ $? -ne 0 ]; then allOk=1; fi\n\n";
                  oss << "${APP} -merge -table SnapshotIoTbl,3DTimeIoTbl " << fromProj << " " << toProj << "\n"; 
                  break;
               case cmd:
                  oss << "REM Linking time stamp 3d prop files:\n"; 
                  oss << "FOR %%c in (" << fromPath.path() << "\\Time_*.h5) DO (mklink /h .\\%%~nxc \\%%c)\nif errorlevel 1 (\n set allOk=1  \n)\n";
                  oss << "%APP% -merge -table SnapshotIoTbl,3DTimeIoTbl " << fromProj << " " << toProj;
                  break;
            }
            break;

         case Genex:             break;
         case HiResDecompaction: break;
         case Migration:         break;
         case Postprocessing:    break;
         default:                break;
      }

      for ( int i = 0; strlen( s_resFilesList[m_appDepLevel][i] ) > 0; ++i )
      {
         ibs::FilePath rfp( fromPath );
         ibs::FilePath tfp( toPath );
         rfp << s_resFilesList[m_appDepLevel][i];
         tfp << s_resFilesList[m_appDepLevel][i];
         switch( m_sh )
         {
            case bash: 
               oss << "if [ -e " << rfp.path() << " ]\nthen\n   ln -s " << rfp.path() << " " << tfp.path() << "\n";
               oss << "   ${APP} -merge -table TimeIoTbl -filter TimeIoTbl:MapFileName:" << s_resFilesList[m_appDepLevel][i] << " ";
               oss << fromProj << " " << toProj << "\nfi\nif [ $? -ne 0 ]; then allOk=1; fi\n\n";
               break;

            case cmd:
               oss << "if exist " << rfp.path() << " mklink /h " << tfp.path() << " " << rfp.path() << "\n";
               oss << "if exist " << tfp.path() << " %APP% -merge -table TimeIoTbl -filter TimeIoTbl:MapFileName:" << 
                       s_resFilesList[m_appDepLevel][i] << " " << fromProj << " " << toProj << "\n";
               oss << "if errorlevel 1 (\n set allOk=1  \n)\n";
               break;
         }
      }
      // add to scrip checking of the return code of the mpirun. If it is 0 - create file Stage_X.sh.success, or Stage_X.ch.failed otherwise
      switch ( m_sh )
      {
      case bash:
         oss << "if [ $allOk -eq 0 ];    then\n   touch $(basename $BASH_SOURCE).success\n   exit 0\nelse\n";
         oss << "touch $(basename $BASH_SOURCE).failed\n   exit 1\nfi\n";
         break;

      case cmd:  oss << "if allOk 1 (\n   type NUL > %~n0.bat.failed\n) else (\n   type NUL > %~n0.bat.success\n)\n"; break;
      }

      return oss.str();
   }


   void CauldronApp::addOption( const std::string & opt )
   {
      m_optionsList.push_back( opt );
      if ( m_appName == "fastcauldron" && opt == "-hrdecompaction" ) { m_appDepLevel = HiResDecompaction; }
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
      ibs::FilePath appPath( m_version );  // version could be set as a full path to the application executable
      ibs::FolderPath miscPath( appPath.path() );
#ifdef _WIN32
      std::string osAppName = m_appName + ".exe";
#else
      std::string osAppName = m_appName;
#endif 
      appPath << osAppName;
      switch ( m_sh )
      {
      case bash:
         oss << "CAULDRON_VERSION=" << m_version << "\n";
         oss << "IBS_ROOT=" << m_rootPath << "\n\n";

         if ( !appPath.exists() )
         {
            oss << "CAULDRON_MISC_PATH=" << m_rootPath << '/' << m_version << "/misc\n";

            // compute path to the application
            oss << "os1=`/apps/sss/share/getos2` || { echo 'Warning: Could not determine OS version. Are we in Shell Linux?'; os1='.'; }\n"
                << "os2=`/apps/sss/share/getos2 --os --ver` || { echo 'Warning: Could not determine OS version. Are we in Shell Linux?'; os2='.'; }\n"
                << "APP=\"" << m_rootPath << '/' << m_version << "/${os1}/bin/" << osAppName << "\"" << '\n'
                << "APP_BIN_PATH=\"" << m_rootPath << '/' << m_version << "/${os1}/bin/\"" << '\n'
                << "if [ ! -e $APP ]; then" << '\n'
                << "   APP=\"" << m_rootPath << '/' << m_version << "/${os2}/bin/" << osAppName << "\"" << '\n'
                << "   APP_BIN_PATH=\"" << m_rootPath << '/' << m_version << "/${os2}/bin/\"" << '\n'
                << "fi\n";
         }
         else
         {
            miscPath << ".." << "misc";
            if ( !miscPath.exists() )
            {
               miscPath = appPath.filePath();
               miscPath << ".." << ".." << "misc";

            }
            oss << "CAULDRON_MISC_PATH=" << miscPath.fullPath().path() << "\n";
            oss << "APP=" << appPath.path() << '\n';
            oss << "APP_BIN_PATH=\"" << appPath.filePath() << "\"\n";
         }

         oss << "if [ ! -e $APP ]; then\n"
            << "   echo Could not find application executable\n"
            << "   exit 1\n"
            << "fi\n\n";
           
         break;

      case cmd:
         oss << "set CAULDRON_VERSION=" << m_version << "\n";
         oss << "set IBS_ROOT=" << m_rootPath << "\n\n";

         if ( !appPath.exists() )
         {
            // compute path to the application
            oss << "set APP=" << ( ibs::FilePath( m_rootPath ) << m_version << "bin" << osAppName ).path( );
         }
         else
         {
            miscPath << ".." << "misc";
            if ( !miscPath.exists() )
            {
               miscPath = appPath.filePath();
               miscPath << ".." << ".." << "misc";
            }
            oss << "set CAULDRON_MISC_PATH=" << miscPath.path() << "\n";
            oss << "set APP=" << appPath.path() << '\n';
         }
         break;
      }
      oss << "\n";

      // dump set of environment variables
      for ( std::map< std::string, std::string >::const_iterator it = m_env.begin(); it != m_env.end(); ++it )
      {
         switch ( m_sh )
         {
         case bash:  oss << "export " << it->first << "=" << it->second << "\n"; break;
         case cmd:   oss << "set "    << it->first << "=" << it->second << "\n"; break;
         }
      }
   }

   // Serialize object to the given stream
   bool CauldronApp::save( CasaSerializer & sz, unsigned int /* fileVersion */ ) const
   {
      bool ok = true;

      // initial implementation
      ok =  ok ? sz.save( m_env.size(), "EnvVarList" ) : ok;
      for ( std::map< std::string, std::string >::const_iterator it = m_env.begin(); it != m_env.end() && ok; ++it )
      {
         ok = sz.save( it->first, "EnvVarName" );
         ok = ok ? sz.save( it->second, "EnvVarVal" ) : ok;
      }

      ok = ok ? sz.save( m_appName,                       "AppName"         ) : ok;
      ok = ok ? sz.save( m_scriptBody,                    "ScriptBody"      ) : ok;
      ok = ok ? sz.save( m_parallel,                      "IsAppParallel"   ) : ok;
      ok = ok ? sz.save( m_cpus,                          "CPUsNum"         ) : ok;
      ok = ok ? sz.save( static_cast<int>( m_sh ),        "ShellType"       ) : ok;
      ok = ok ? sz.save( m_version,                       "CldVersion"      ) : ok;
      ok = ok ? sz.save( m_rootPath,                      "IBSROOT"         ) : ok;
      ok = ok ? sz.save( m_mpirunCmd,                     "MPIRunCmd"       ) : ok;
      ok = ok ? sz.save( m_inputOpt,                      "InputOpt"        ) : ok;
      ok = ok ? sz.save( m_outputOpt,                     "OutputOpt"       ) : ok;
      ok = ok ? sz.save( m_optionsList,                   "AppOptionsList"  ) : ok;
      ok = ok ? sz.save( m_runTimeLim,                    "AppRunTimeLimit" ) : ok;
      ok = ok ? sz.save( static_cast<int>(m_appDepLevel), "AppDepLevel"     ) : ok;
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

      ok = ok ? dz.load( m_appName,       "AppName"         ) : ok;
      ok = ok ? dz.load( m_scriptBody,    "ScriptBody"      ) : ok;
      ok = ok ? dz.load( m_parallel,      "IsAppParallel"   ) : ok;
      ok = ok ? dz.load( m_cpus,          "CPUsNum"         ) : ok;
      int sht;
      ok = ok ? dz.load( sht,             "ShellType"       ) : ok;
      m_sh = static_cast<ShellType>( sht );
      ok = ok ? dz.load( m_version,       "CldVersion"      ) : ok;
      ok = ok ? dz.load( m_rootPath,      "IBSROOT"         ) : ok;
      ok = ok ? dz.load( m_mpirunCmd,     "MPIRunCmd"       ) : ok;
      ok = ok ? dz.load( m_inputOpt,      "InputOpt"        ) : ok;
      ok = ok ? dz.load( m_outputOpt,     "OutputOpt"       ) : ok;
      ok = ok ? dz.load( m_optionsList,   "AppOptionsList"  ) : ok;
      ok = ok ? dz.load( m_runTimeLim,    "AppRunTimeLimit" ) : ok;

      if ( objVer > 0 )
      {
         ok = ok ? dz.load( sht,          "AppDepLevel"     ) : ok;
         m_appDepLevel = static_cast<AppPipelineLevel>( sht );
      }
      else { m_appDepLevel = PTSolver; }
 
      if ( !ok )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "DataDiggerImpl deserialization error";
      }
   }
}


