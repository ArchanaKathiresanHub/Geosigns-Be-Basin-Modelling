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

// FileSystem
#include "FilePath.h"

// CASA
#include "CauldronApp.h"
#include "RunCase.h"
#include "RunManagerImpl.h"

#include "CauldronEnvConfig.h"

#include "JobSchedulerLocal.h"
#include "JobSchedulerLSF.h"

#ifndef _WIN32
#include <sys/stat.h>
#endif

// STL
#include <fstream>
#include <iostream>

// STD C lib
#include <cstring>

namespace casa
{
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RunManager / RunManagerImpl methods definition
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CauldronApp * RunManager::createApplication( ApplicationType appType, int cpus, CauldronApp::ShellType sh, std::string cmdLine )
{
   std::auto_ptr<CauldronApp> app;
   switch ( appType )
   {
      case fastcauldron: app.reset( new CauldronApp( sh, "fastcauldron", true  ) ); break;
      case fastgenex6:   app.reset( new CauldronApp( sh, "fastgenex6",   true  ) ); break;
      case fastmig:      app.reset( new CauldronApp( sh, "fastmig",      true  ) ); break;
      case fastctc:      app.reset( new CauldronApp( sh, "fastctc",      true  ) ); break;
      case datadriller:  app.reset( new CauldronApp( sh, "datadriller",  false ) ); break;
      case tracktraps:   app.reset( new CauldronApp( sh, "tracktraps",   false ) ); break;
      case track1d:      app.reset( new CauldronApp( sh, "track1d",      false ) ); break;
      case generic:      app.reset( new CauldronApp( sh, "unknown",      false ) );
         app->setScriptBody( cmdLine );
         break;
      default: break;
   }
   if ( app.get() ) app->setCPUs( cpus );

   return app.release();
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

   CauldronApp * dda = new CauldronApp(
#ifdef _WIN32
      CauldronApp::cmd
#else
      CauldronApp::bash
#endif
      , "datadriller", false );
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
      scriptFile << (std::string( "Stage_" ) + ibs::to_string( i ) + m_appList[i]->scriptSuffix() );
      
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
  
   try 
   {
      while ( !allFinished ) // loop till all will be finished
      {
         // counters for jobs states
         int finished  = 0;
         int submitted = 0;
         int crashed   = 0;
         int running   = 0;
         int pending   = 0;
         int totJobs   = 0;

         // loop over all cases
         for ( size_t i = 0; i < m_jobs.size(); ++i )
         {
            bool contAppPipeline = true;

            for ( size_t j = 0; j < m_jobs[i].size() && contAppPipeline; ++j )
            {
               JobScheduler::JobID job = m_jobs[i][j];

               JobScheduler::JobState jobState = m_jobSched->jobState( job );
               ++totJobs;

               if ( JobScheduler::NotSubmittedYet == jobState )
               {
                  jobState = m_jobSched->runJob( job ); // submit job
                  ++submitted;
               }

               switch ( jobState )
               {
                  case JobScheduler::JobFailed: // job failed!!! shouldn't run others in a pipeline! 
                     ++crashed;
                     m_jobs[i].resize( j+1 ); // drop all other jobs for this case
                     break;

                  case JobScheduler::JobFinished:  ++finished; break; // skip finished jobs
                  case JobScheduler::JobSucceeded: ++finished; break; // job succeeded
                  case JobScheduler::JobPending:   ++pending;  break; // job pending
                  case JobScheduler::JobRunning:   ++running;  break; // job is running

                  default: break;
               }

               // analyse job state from point of view further pipeline processing
               switch ( jobState )
               {
                  case JobScheduler::NotSubmittedYet:
                  case JobScheduler::JobFailed: 
                  case JobScheduler::JobPending:
                  case JobScheduler::JobRunning:
                     contAppPipeline = false; // stop going further in applications pipeline for this case
                     break;

                  case JobScheduler::JobFinished: 
                  case JobScheduler::JobSucceeded:
                     continue; // continue pipeline processing
                     break;
               }
            }
         }
         // run over all cases, make a pause, get a Twix
         std::cout << "total: "       << totJobs   << 
                      ", submitted: " << submitted <<
                      ", finished: "  << finished  << 
                      ", failed: "    << crashed   << 
                      ", pending: "   << pending   << 
                      ", running: "   << running   << std::endl;
         
         if ( totJobs == (finished+crashed) )
         {
            allFinished = true;
         }
         else
         {
            m_jobSched->sleep(); // wait a bit till go to the next loop
         }
      }
   }
   catch ( Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }
   
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


// Serialize object to the given stream
bool RunManagerImpl::save( CasaSerializer & sz, unsigned int fileVersion ) const
{
   bool ok = true;
   if ( fileVersion >= 0 ) // initial version implementation
   {
      ok = ok ? sz.save( m_cldVersion, "CauldronVersion" ) : ok;
      ok = ok ? sz.save( m_ibsRoot,    "IBSRoot"         ) : ok;

      ok = ok ? sz.save( m_appList.size(), "AppListSize" ) : ok;
      for ( size_t i = 0; i < m_appList.size() && ok; ++i )
      {
         ok = sz.save( *m_appList[i], "CldApp" );
      }

      ok = ok ? sz.save( *m_jobSched.get(), "JobScheduler" ) : ok;

      ok = ok ? sz.save( m_jobs.size(), "NumberOfCases" ) : ok;
      for ( size_t i = 0; i < m_jobs.size() && ok; ++i )
      {
         ok = ok ? sz.save( m_jobs[i], "CaseJobsQueueIDs" ) : ok;
      }
   }
   return ok;
}

// Create a new instance and deserialize it from the given stream
RunManagerImpl::RunManagerImpl( CasaDeserializer & dz, const char * objName )
{
   // read from file object name and version
   bool ok = dz.checkObjectDescription( typeName(), objName, version() );

   ok = ok ? dz.load( m_cldVersion, "CauldronVersion" ) : ok;
   ok = ok ? dz.load( m_ibsRoot,    "IBSRoot"         ) : ok;

   size_t setSize;
   ok = ok ? dz.load( setSize, "AppListSize" ) : ok;
   for ( size_t i = 0; i < setSize && ok; ++i )
   {
      CauldronApp * app = new CauldronApp( dz, "CldApp" );
      ok = app ? true : false;
      m_appList.push_back( app );
   }

   m_jobSched.reset( ok ? JobScheduler::load( dz, "JobScheduler" ) : 0 );
   ok = ok ? (m_jobSched.get() ? true : false) : ok;

   size_t sz = 0;
   ok = ok ? dz.load( sz, "NumberOfCases" ) : ok;
   for ( size_t i = 0; i < sz && ok; ++i )
   {
      m_jobs.push_back( std::vector<size_t>() );
      ok = ok ? dz.load( m_jobs.back(), "CaseJobsQueueIDs" ) : ok;
   }

   if ( !ok ) throw Exception( DeserializationError ) << "RunManagerImpl deserialization error";
}

}

