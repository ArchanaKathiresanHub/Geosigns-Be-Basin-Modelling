//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file RunManagerImpl.C
/// @brief This file keeps API implementation of Run Manager

#include "FilePath.h"
#include "CauldronApp.h"
#include "RunCase.h"
#include "RunManagerImpl.h"

#include "CauldronEnvConfig.h"

#include "JobSchedulerLocal.h"
#include "JobSchedulerLSF.h"

#ifndef _WIN32
#include <sys/stat.h>
#endif

#include <fstream>
#include <iostream>
#include <cstring>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Definition the set of cauldron applications
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace casa
{

   // fastcauldron application wrapper
   class FastCauldronApp : public CauldronApp
   {
   public:
      /// Constructor of fastcauldron app
      FastCauldronApp( ShellType sh = bash ) : CauldronApp( sh, "fastcauldron", true )
      {
         m_cpus = 1;
         
         // set up needed for simulators environment vars
         pushDefaultEnv( "GENEXDIR",   (ibs::FolderPath( m_rootPath ) << m_version << "misc" << "genex40").path() );
         pushDefaultEnv( "GENEX5DIR",  (ibs::FolderPath( m_rootPath ) << m_version << "misc" << "genex50").path() );
         pushDefaultEnv( "GENEX6DIR",  (ibs::FolderPath( m_rootPath ) << m_version << "misc" << "genex60"   ).path() );
         pushDefaultEnv( "OTGCDIR",    (ibs::FolderPath( m_rootPath ) << m_version << "misc" << "OTGC"   ).path() );
         pushDefaultEnv( "EOSPACKDIR", (ibs::FolderPath( m_rootPath ) << m_version << "misc" << "eospack").path() );
         pushDefaultEnv( "CTCDIR",     (ibs::FolderPath( m_rootPath ) << m_version << "misc"             ).path() );
         
      }
   };
   
   // fastgenex6 application wrapper
   class FastGenex6App : public CauldronApp
   {
   public:
      // Constructor of fastgenex6 app
      FastGenex6App( ShellType sh = bash ) : CauldronApp( sh, "fastgenex6", true )
      {
         m_cpus = 1;
         
         // set up environment vars
         pushDefaultEnv( "EOSPACKDIR", (ibs::FolderPath( m_rootPath ) << m_version << "misc" << "eospack").path() );
         pushDefaultEnv( "OTGCDIR",    (ibs::FolderPath( m_rootPath ) << m_version << "misc" << "OTGC"   ).path() );
         pushDefaultEnv( "GENEX5DIR",  (ibs::FolderPath( m_rootPath ) << m_version << "misc" << "genex50").path() );
         pushDefaultEnv( "GENEX6DIR",  (ibs::FolderPath( m_rootPath ) << m_version << "misc" << "genex60"   ).path() );
      }
   };
 
   // fastmig application wrapper
   class FastMigApp : public CauldronApp
   {
   public:
      // Constructor of fastgenex6 app
      FastMigApp( ShellType sh = bash ) : CauldronApp( sh, "fastmig", true )
      {
         m_cpus = 1;
         
         // set up environment vars
         pushDefaultEnv( "EOSPACKDIR", (ibs::FolderPath( m_rootPath ) << m_version << "misc" << "eospack").path() );
         pushDefaultEnv( "OTGCDIR",    (ibs::FolderPath( m_rootPath ) << m_version << "misc" << "OTGC"   ).path() );
         pushDefaultEnv( "GENEXDIR",   (ibs::FolderPath( m_rootPath ) << m_version << "misc" << "genex40").path() );
         pushDefaultEnv( "GENEX5DIR",  (ibs::FolderPath( m_rootPath ) << m_version << "misc" << "genex50").path() );
      }
   };
   
   // fastctc application wrapper
   class FastCtcApp : public CauldronApp
   {
   public:
      // Constructor of fastgenex6 app
      FastCtcApp( ShellType sh = bash ) : CauldronApp( sh, "fastctc", true )
      {
         m_cpus = 1;
         
         // set up environment vars
         pushDefaultEnv( "CTCDIR",     (ibs::FolderPath( m_rootPath ) << m_version << "misc"             ).path() );
      }
   };

   // datadriller application wrapper
   class DataDrillerApp : public CauldronApp
   {
   public:
      // Constructor of fastgenex6 app
      DataDrillerApp( ShellType sh = bash ) : CauldronApp( sh, "datadriller", false )
      {
         m_cpus = 1;

         // set up environment vars
         pushDefaultEnv( "EOSPACKDIR", ( ibs::FolderPath( m_rootPath ) << m_version << "misc" << "eospack" ).path() );
         pushDefaultEnv( "OTGCDIR",    ( ibs::FolderPath( m_rootPath ) << m_version << "misc" << "OTGC"    ).path() );
         pushDefaultEnv( "GENEX6DIR",  ( ibs::FolderPath( m_rootPath ) << m_version << "misc" << "genex60" ).path() );
         pushDefaultEnv( "CTCDIR",     ( ibs::FolderPath( m_rootPath ) << m_version << "misc"              ).path() );
      }
   protected:
      virtual std::string inputProjectOption() { return "-input"; }
      virtual std::string outputProjectOption() { return "-output"; }
   };

   // fasttouch7 application wrapper

   // track1d application wrapper
   class Track1DApp : public CauldronApp
   {
   public:
      /// Constructor of track1d app
      Track1DApp( ShellType sh = bash ) : CauldronApp( sh, "track1d", false )
      {
         pushDefaultEnv( "EOSPACKDIR", (ibs::FolderPath( m_rootPath ) << m_version << "misc" << "eospack").path() );
      }
      
      // Generates script file which contains environment set up and application run for given input/output project file
      virtual std::string generateScript( const std::string & inProjectFile, const std::string & outProjectFile )
      {
         // dump script top line with shell preference
         std::ostringstream oss;
         switch ( m_sh )
         {
            case bash: oss << "#!/bin/bash\n"; break;
            case csh:  oss << "#!/bin/csh\n";  break;
            case cmd:                          break;
         }
         oss << "\n";
         
         // dump all necessary environment variables to the script
         dumpEnv( oss );
         oss << "\n";
         
         // dump application name with full path
         oss << ( ibs::FilePath( m_rootPath ) << m_version << "Linux" << "bin" << m_appName ).path();
         
         // dump app options list
         for ( size_t i = 0; i < m_optionsList.size(); ++i ) { oss << " " << m_optionsList[i]; }
         
         // dump input/output project name
         oss << " -project " << inProjectFile << " | sed '1,4d' > " << ( outProjectFile.empty() ? (inProjectFile + "_track1d.csv") : outProjectFile ) << "\n";
         
         return oss.str();
      }
   };
   
   // Some generic app. Script command line, passed as a parameter of constructor
   class GenericApp : public CauldronApp
   {
   public:
      // Constructor
      GenericApp( const std::string & cmdLine ) : CauldronApp( bash, "unknown", false ) { m_cmdLine = cmdLine; }
      
      // Destructor
      virtual ~GenericApp() {;}
      
      // Generates script file which contains the given to constructor script body
      virtual std::string generateScript( const std::string & /*inFile*/, const std::string & /*outFile*/ ) { return m_cmdLine; }
      
   private:
      std::string m_cmdLine;  ///< script body
   };
   
  
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // CauldronApp methods definition
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
   // get environment variable
   const char * CauldronApp::env( const char * varName ) { return getenv( varName ); }
   
   // add the default value of environment variable to the list if it does not set up in environment
   bool CauldronApp::pushDefaultEnv( const std::string & varName, const std::string & varValue )
   {
      const char * envValue = env( varName.c_str() );
      m_env[ varName ] = envValue ? envValue : varValue;
      return envValue ? false : true;
   }

   void CauldronApp::dumpEnv( std::ostream & oss )
   {
      for ( std::map< std::string, std::string >::const_iterator it = m_env.begin(); it != m_env.end(); ++it )
      {
         switch( m_sh )
         {
            case bash:  oss << "export " << it->first << "="   << it->second << "\n";   break;
            case csh:   oss << "setenv " << it->first << " "   << it->second << "\n";   break;
            case cmd:   oss << "set "    << it->first << "=\"" << it->second << "\"\n"; break;
         }
      }
   }
   
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // CauldronApp methods definition
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   CauldronApp::CauldronApp( ShellType sh, const std::string & appName, bool isParallel ) :
                             m_appName( appName ), m_parallel( isParallel )
   {
      setShellType( sh );

      if ( !env( "SIEPRTS_LICENSE_FILE" ) ) m_env["SIEPRTS_LICENSE_FILE"] = s_LICENSE_SERVER;
    
      m_version   = env( "CAULDRON_VERSION" )    ? env( "CAULDRON_VERSION" )    : "v2014.0703";        // the default version is the latest available release for now
      m_rootPath  = env( "IBS_ROOT" )            ? env( "IBS_ROOT" )            : "/apps/sssdev/ibs";  // path to IBS folder where the different versions are
      m_mpirunCmd = env( "CAULDRON_MPIRUN_CMD" ) ? env( "CAULDRON_MPIRUN_CMD" ) : "";                  // 
      
      if ( m_mpirunCmd.empty() )
      {
         m_mpirunCmd =  std::string( s_MPIRUN_CMD ) + " -env I_MPI_DEBUG 5";

         m_mpiEnv = std::string( "source " ) + s_MPI_ROOT;

         switch( m_sh )
         {
            case bash: m_mpiEnv += "/bin/mpivars.sh\n\n";  break;
            case csh:  m_mpiEnv += "/bin/mpivars.csh\n\n"; break;
            case cmd:  m_mpiEnv = ""; break;
         }
      }
   }

   std::string CauldronApp::generateScript( const std::string & inProjectFile, const std::string & outProjectFile )
   {
      // dump script top line with shell preference
      std::ostringstream oss;
      switch ( m_sh )
      {
         case bash: oss << "#!/bin/bash\n\n"; break;
         case csh:  oss << "#!/bin/csh\n\n";  break;
         case cmd:                            break;
      }
      
      // dump all necessary environment variables to the script
      dumpEnv( oss );
      
      oss << "\n";
      
      // if application is parallel, add mpirun dirrective with options
      if ( m_parallel )
      {
         oss << m_mpiEnv << m_mpirunCmd;
         oss << " -outfile-pattern '" + m_appName + "-output-rank-%r.log' ";
         if ( m_cpus > 0 ) { oss << "-np " << m_cpus << " "; }
      }
      
      // dump application name with full path
      oss << ( ibs::FilePath( m_rootPath ) << m_version << "Linux" << "bin" << m_appName ).path();
      
      // dump app options list
      for ( size_t i = 0; i < m_optionsList.size(); ++i ) { oss << " " << m_optionsList[i]; }
      
      // dump input/output project name
      oss                                << " " << inputProjectOption()  << " " << inProjectFile;
      if ( !outProjectFile.empty() ) oss << " " << outputProjectOption() << " " << outProjectFile;
      oss << "\n\n";
     
      // add to scrip checking of the return code of the mpirun. If it is 0 - create file Stage_X.sh.success, or Stage_X.ch.failed otherwise
      switch ( m_sh )
      {
         case bash: oss << "if [ $? -eq 0 ]; then\n   touch $(basename $BASH_SOURCE).success\nelse\n   touch $(basename $BASH_SOURCE).failed\nfi\n"; break;
         case csh:  oss << "if ( $status != 0 ) then\n   touch `basename $0`.failed\nelse\n   touch `basename $0`.success\nendif\n"; break;
         case cmd:  break;
      }
      return oss.str();
   }
}

namespace casa
{

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RunManager / RunManagerImpl methods definition
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CauldronApp * RunManager::createApplication( ApplicationType appType, int cpus, std::string cmdLine )
{
   CauldronApp * app = NULL;

   switch ( appType ) {
         
      case fastcauldron: app = new FastCauldronApp( CauldronApp::bash ); break;
      case fastgenex6:   app = new FastGenex6App(   CauldronApp::bash ); break;
      case fastmig:      app = new FastMigApp(      CauldronApp::bash ); break;
      case fastctc:      app = new FastCtcApp(      CauldronApp::bash ); break;
      case track1d:      app = new Track1DApp(      CauldronApp::bash ); break;
      case generic:      app = new GenericApp(      cmdLine ); break;
         
      default: break;
   }
   if ( app ) app->setCPUs( cpus );

   return app;
}


///////////////////////////////////////////////////////////////////////////////
RunManagerImpl::RunManagerImpl( const std::string & clusterName )
{
   // create instance of job scheduler
#if defined (_WIN32) || !defined (WITH_LSF_SCHEDULER)
   m_jobSched.reset( new JobSchedulerLocal() );
#else
   if ( clusterName == "LOCAL" )
   {
      m_jobSched.reset( new JobSchedulerLocal() );
   }
   else
   {
      m_jobSched.reset( new JobSchedulerLSF( clusterName ) );
   }
#endif

   CauldronApp * dda = new DataDrillerApp();
   addApplication( dda ); // insert datadriller application to extract data results
}

///////////////////////////////////////////////////////////////////////////////
RunManagerImpl::~RunManagerImpl()
{
}

// Add application to the list of simulators for pipeline calculation definitions
ErrorHandler::ReturnCode RunManagerImpl::addApplication( CauldronApp * app )
{
   if ( app )
   {
      if ( m_appList.empty() ) m_appList.push_back( app );
      else m_appList.insert( (m_appList.end() - 1), app ); // always have at least 1 element (datadriller app)
      return NoError;
   }
   return reportError( ValidationError, "RunManager::addApplication(): No app object was given" );
}


///////////////////////////////////////////////////////////////////////////////
// Add Case to set
ErrorHandler::ReturnCode RunManagerImpl::scheduleCase( const RunCase & newRun )
{
   // do not add cases which has no project file
   if ( !newRun.projectPath() ) return reportError( WrongPath, "Case with empty path to project file was given" );

   // get project file path
   ibs::FilePath pfp( newRun.projectPath() );

   if ( !pfp.exists() ) return reportError( WrongPath, "Project file does not exist, can't schedule given case" );

   // if no project defined - report error
   if ( !pfp.exists() ) return reportError( WrongPath, "Wrong path to case project file was given" );

   // add new empty row to jobs list
   m_jobs.push_back( std::vector< JobScheduler::JobID >() );

   // construct case name, use name of the directory where project is located or just Case_N
   size_t sz = pfp.size();
   std::string caseName = sz > 2 ? pfp[sz - 2 ] : (std::string( "Case_" ) + ibs::to_string( m_jobs.size() ) );

   // go through pipelines and populate jobs list/generate scripts for all cases
   for ( size_t i = 0; i < m_appList.size(); ++i )
   {
      // generate script
      int cpus = m_appList[i]->cpus(); // save cpus for the application

      // if cpus number will be defined by scheduler itself do not put -np CPUS in mpirun command
      if ( m_jobSched->cpusNumberByScheduler() )
      {
         m_appList[ i ]->setCPUs( 0 );      // number of cpus is defined by the scheduler, exclude it from the script
      }

      if ( !m_cldVersion.empty() ) m_appList[i]->setCauldronVersion( m_cldVersion ); // if another version is defined by user, set up it

      const std::string appScript = m_appList[i]->generateScript( pfp.fileName(), "" );
      
      if ( m_jobSched->cpusNumberByScheduler() )
      {
         m_appList[ i ]->setCPUs( cpus ); // restore number of cpus back
      }

      // generate script file name
      ibs::FilePath scriptFile( pfp.filePath() );
      scriptFile << (std::string( "Stage_" ) + ibs::to_string( i ) + ".sh");
      
      // save script to file
      std::ofstream ofs( scriptFile.path().c_str(), std::ios_base::out | std::ios_base::trunc );
      ofs << appScript;
      ofs.close();

      // make script executable
#ifndef _WIN32
      chmod( scriptFile.path().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
#endif
      // construct job name 
      std::ostringstream oss;
      oss << caseName << "_stage_" << ibs::to_string( i );

      ////////////////////////////////////////
      /// put job to the queue through job scheduler
      JobScheduler::JobID id = m_jobSched->addJob( pfp.filePath().c_str(),  // cwd
                                                   scriptFile.path(),       // script name
                                                   oss.str(),               // job name
                                                   m_appList[i]->cpus()     // number of CPUs for this job
                                                 );

      // put job to the queue for the current case
      m_jobs.back().push_back( id );
   }

   return NoError;
}

///////////////////////////////////////////////////////////////////////////////
// Execute all scheduled cases. Very loooong cycle
ErrorHandler::ReturnCode RunManagerImpl::runScheduledCases( bool asyncRun )
{
   bool allFinished = false;
   
   // just for info
   int finished  = 0;
   int submitted = 0;
   int crashed   = 0;

   while ( !allFinished )
   {
      int running   = 0;
      int pending   = 0;

      // loop over all cases
      for ( size_t i = 0; i < m_jobs.size(); ++i )
      {
         for ( size_t j = 0; j < m_jobs[i].size(); ++j )
         {
            JobScheduler::JobID job = m_jobs[i][j];

            JobScheduler::JobState jobState = m_jobSched->jobState( job );

            if ( JobScheduler::JobFinished == jobState ) continue; // skip finished jobs

            if ( JobScheduler::NotSubmittedYet == jobState ) // not submitted yet
            {
               try                     { m_jobSched->runJob( job ); }
               catch( Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }

               submitted++;
               break;
            }
            else // something could be still on the cluster, check status
            {
               switch( jobState )
               {
                  case JobScheduler::JobFailed: // job failed!!! should not run others in a queue! 
                     ++crashed;
                     m_jobs[i].resize( j+1 ); // drop all other jobs for this case
                     break;
                  case JobScheduler::JobSucceeded: ++finished; break; // job succeeded
                  case JobScheduler::JobPending:   ++pending;  break; // job pending
                  case JobScheduler::JobRunning:   ++running;  break; // job is running on cluster
                  default: break;
               }
               if ( jobState == JobScheduler::JobSucceeded ) continue; // start another job
            }
            break;
         }
      }
      // run over all cases, make a pause, get a Twix
      std::cout << "submitted: " << submitted << ", finished: " << finished << ", failed: " << crashed << ", pending: " << pending << ", running: " << running << std::endl;
      
      if ( submitted == finished )
      {
         allFinished = true;
      }
      else
      {
         m_jobSched->sleep(); // wait a bit till go to the next loop
      }

   }
   return NoError;
}


///////////////////////////////////////////////////////////////////////////////
// Set cluster name from job scheduler
ErrorHandler::ReturnCode RunManagerImpl::setClusterName( const char * clusterName ) 
{ 
   if ( !clusterName || !strlen( clusterName ) ) return reportError( OutOfRangeValue, "Wrong cluster name" );

   if ( m_jobSched->clusterName() == clusterName ) return NoError;

#if defined (_WIN32) || !defined (WITH_LSF_SCHEDULER)
   m_jobSched.reset( new JobSchedulerLocal() );
#else
   if ( !strcmp( clusterName, "LOCAL" ) )
   {
      m_jobSched.reset( new JobSchedulerLocal( ) );
   }
   else
   {
      m_jobSched.reset( new JobSchedulerLSF( clusterName ) );
   }
#endif
   return NoError;
}

///////////////////////////////////////////////////////////////////////////////
// Get cluster name from job scheduler
std::string RunManagerImpl::clusterName() { return m_jobSched->clusterName(); }

}

