//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
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
#include "RunCaseImpl.h"
#include "RunManagerImpl.h"

#include "CauldronEnvConfig.h"

#include "JobSchedulerLocal.h"
#include "JobSchedulerLSF.h"

#include "LogHandler.h"

#ifndef _WIN32
#include <sys/stat.h>
#endif

// STL
#include <fstream>
#include <iostream>

// STD C lib
#include <cstring>
#include <ctime>

namespace casa
{

const char * RunManager::s_scenarioExecStopFileName = "stop_exec_scenario";
const char * RunManager::s_jobsIDListFileName       = "casa_jobs_list.txt";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RunManager / RunManagerImpl methods definition
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CauldronApp * RunManager::createApplication( ApplicationType        appType
                                           , int                    cpus
                                           , size_t                 runTimeLimit
                                           , CauldronApp::ShellType sh
                                           , std::string            cmdLine 
                                           )
{
   std::unique_ptr<CauldronApp> app;
   switch ( appType )
   {
      case fastcauldron: app.reset( new CauldronApp( sh, "fastcauldron", cpus > 1 ? true : false ) ); break;
      case fastgenex6:   app.reset( new CauldronApp( sh, "fastgenex6",   cpus > 1 ? true : false ) ); break;
      case fastmig:      app.reset( new CauldronApp( sh, "fastmig",      cpus > 1 ? true : false ) ); break;
      case fastctc:      app.reset( new CauldronApp( sh, "fastctc",      cpus > 1 ? true : false ) ); break;
      case datadriller:  app.reset( new CauldronApp( sh, "datadriller",  false ) ); break;
      case tracktraps:   app.reset( new CauldronApp( sh, "tracktraps",   false ) ); break;
      case track1d:      app.reset( new CauldronApp( sh, "track1d",      false ) ); break;
      case casa:         app.reset( new CauldronApp( sh, "casa",         false ) ); break;
      case generic:      app.reset( new CauldronApp( sh, "unknown",      false ) ); app->setScriptBody( cmdLine ); break;
      default:                                                                      break;
   }
   if ( app.get() )
   {
      app->setCPUs( cpus );
      app->setRunTimeLimit( runTimeLimit );
   }

   return app.release();
}


///////////////////////////////////////////////////////////////////////////////
RunManagerImpl::RunManagerImpl( const std::string & clusterName ) : m_maxPendingJobs( 0 )
{
   // create instance of job scheduler
   createJobScheduler( clusterName );

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
   for ( auto app : m_appList ) { delete app; }
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
ErrorHandler::ReturnCode RunManagerImpl::scheduleCase(RunCase & newRun, const std::string & scenarioID )
{
   if ( newRun.runStatus() != RunCase::NotSubmitted ) return NoError;

   // do not add cases which has no project file
   if ( !newRun.projectPath() ) return reportError( WrongPath, "Case with empty path to project file was given" );

   // get project file path
   ibs::FilePath pfp( newRun.projectPath() );

   // if no project defined - report error
   if ( !pfp.exists() ) return reportError( WrongPath, "Wrong path to case project file was given" );

   // add new empty row to jobs list
   m_jobs.push_back( std::vector< JobScheduler::JobID >() );
   m_cases.push_back( dynamic_cast<RunCaseImpl*>( &newRun ) );

   // construct case name, use name of the directory where project is located or just Case_N
   size_t sz = pfp.size();
   std::string caseName = sz > 2 ? pfp[sz - 2 ] : ( std::string( "Case_" ) + ibs::to_string( m_jobs.size() ) );

   // go through pipelines and populate jobs list/generate scripts for all cases
   for ( size_t i = 0; i < m_appList.size(); ++i )
   {
      // generate script
      int cpus = m_appList[i]->cpus(); // save cpus for the application

      // if cpus number will be defined by scheduler itself do not put -np CPUS in mpirun command
      if ( m_jobSched->cpusNumberByScheduler() )
      {
         m_appList[i]->setCPUs(0);      // number of cpus is defined by the scheduler, exclude it from the script
      }

      if ( !m_cldVersion.empty() ) m_appList[i]->setCauldronVersion( m_cldVersion ); // if another version is defined by user, set up it

      size_t depOnID = m_cases.size() + 1; // define dependency ID outside of scheduled cases to use it as a flag also (size_t can't be negative)

      // run over previously added jobs to detect runs with the same parameters
      if ( m_appList[i]->appSolverDependencyLevel() < Postprocessing )
      {
         for ( size_t cs = 0; ( cs < m_cases.size() - 1 ) && depOnID > m_cases.size(); ++cs )
         {
            if ( newRun.isEqual( *(m_cases[cs]), m_appList[i]->appSolverDependencyLevel() ) ) // compare parameters value taking in account dep. level
            {
               depOnID = cs; // this case, up to this application, is the same as cs, results could be just copied
            }  
         }
      }

      // create script body for application run or results copy
      const std::string appScript = depOnID > m_cases.size()
                                  ? m_appList[i]->generateScript( pfp.fileName(), "", scenarioID )
                                  : m_appList[i]->generateCopyResultsScript( std::string( m_cases[depOnID]->projectPath() )
                                                                           , std::string( m_cases.back()->projectPath() )
                                                                           , scenarioID
                                                                           );
      if ( m_jobSched->cpusNumberByScheduler() )
      {
         m_appList[i]->setCPUs( cpus ); // restore number of cpus back
      }

      // generate script file name
      ibs::FilePath scriptFile( pfp.filePath() );
      scriptFile << ( std::string( "Stage_" ) + ibs::to_string(i) + m_appList[i]->scriptSuffix() );

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
      /// put the job to the queue through a job scheduler
      JobScheduler::JobID id = m_jobSched->addJob( pfp.filePath().c_str()       // cwd
                                                 , scriptFile.path()            // script name
                                                 , oss.str()                    // job name
                                                 , m_appList[i]->cpus()         // number of CPUs for this job
                                                 , m_appList[i]->runTimeLimit() // run time limit
                                                 , scenarioID
                                                 );

      // put job to the queue for the current case
      m_jobs.back().push_back( id );

      // if such exists - add jobs dependency
      if ( depOnID < m_cases.size() )
      {
         if ( m_depOnJob.count( id ) > 0 ) { m_depOnJob[ id ].push_back( m_jobs[depOnID][i] );  }
         else                              { m_depOnJob[ id ] = std::vector<JobScheduler::JobID>( 1, m_jobs[depOnID][i] ); }

         if ( i + 1 < m_appList.size() ) // also add dependency for the next job, to allow copy results before go further
         {
            if ( m_depOnJob.count( m_jobs[depOnID][i+1] ) > 0 ) { m_depOnJob[ m_jobs[depOnID][i+1] ].push_back( id ); }
            else                                                { m_depOnJob[ m_jobs[depOnID][i+1] ] = std::vector<JobScheduler::JobID>( 1, id ); }
         }
      }
   }
   
   m_cases.back()->setRunStatus( RunCase::Scheduled );

   return NoError;
}


///////////////////////////////////////////////////////////////////////////////
// Set max number of pending jobs
ErrorHandler::ReturnCode RunManagerImpl::setMaxNumberOfPendingJobs( size_t pendJobsNum )
{
   m_maxPendingJobs = pendJobsNum;
   return NoError;
}


ErrorHandler::ReturnCode RunManagerImpl::setResourceRequirements( const std::string & resReqStr )
{
   try
   {
      if ( !m_jobSched.get() )
      {
         throw Exception( ErrorHandler::UndefinedValue ) << "Cluster name is not defined, can not request cluster specific resources: " << resReqStr;
      }

      JobSchedulerLSF * jsc = dynamic_cast<JobSchedulerLSF*>( m_jobSched.get() );
      if ( ! jsc )
      {
         throw Exception( ErrorHandler::OutOfRangeValue ) << "Cluster " << m_jobSched->clusterName() <<
            " does not support such resource request: " << resReqStr;
      }
      jsc->setResourceRequirements( resReqStr );

   }
   catch ( const Exception & ex )
   {
      return reportError( ex.errorCode(), ex.what() );
   }

   return NoError;
}

///////////////////////////////////////////////////////////////////////////////
// run over all jobs and collect runs statistics. Also report progress if any change in numbers
void RunManagerImpl::collectStatistics( size_t & pFinished, size_t & pPending, size_t & pRunning, size_t & pToBeSubmitted )
{
   size_t total         = 0;
   size_t finished      = 0;
   size_t failed        = 0; 
   size_t pending       = 0;
   size_t running       = 0; 
   size_t toBeSubmitted = 0;

   for ( size_t i = 0; i < m_jobs.size(); ++i )
   {
      for ( size_t j = 0; j < m_jobs[i].size(); ++j )
      {
         total++;
         switch( m_jobSched->jobState( m_jobs[i][j] ) )
         {
            case JobScheduler::NotSubmittedYet: toBeSubmitted++;             break;
            case JobScheduler::JobFailed:       failed++;        finished++; break;
            case JobScheduler::JobPending:      pending++;                   break;
            case JobScheduler::JobRunning:      running++;                   break;
            case JobScheduler::JobFinished:     finished++;                  break;

            case JobScheduler::Unknown:         
            default:                            assert(0);                   break;
         }
      }
   }

   // check if it was a change in any state of jobs, report it
   if ( pToBeSubmitted != toBeSubmitted || pFinished != finished || pPending != pending || pRunning != running )
   {
      time_t curTm = time( 0 );
      std::string curStrTime( ctime( &curTm ) );
      curStrTime.resize( curStrTime.size() - 1 ); // "remove ending \n"

      LogHandler( LogHandler::INFO_SEVERITY ) << curStrTime <<
                                                 ": total: "     << total     << 
                                                 ", finished: "  << finished  << 
                                                 ", failed: "    << failed    << 
                                                 ", pending: "   << pending   << 
                                                 ", running: "   << running   << 
                                                 ", not submitted yet: " << toBeSubmitted;
   }

   pToBeSubmitted = toBeSubmitted;
   pFinished      = finished;
   pPending       = pending;
   pRunning       = running;
}

///////////////////////////////////////////////////////////////////////////////
// do check are all cases are completed?
bool RunManagerImpl::isAllDone() const
{
   // Check are all cases were processed
   bool allDone = true;
   for ( size_t i = 0; i < m_cases.size() && allDone; ++i )
   {
      switch( m_cases[i]->runStatus() )
      {
         case RunCase::NotSubmitted: allDone = false; break;
         case RunCase::Scheduled:    allDone = false; break;
         case RunCase::Completed:                     break;
         case RunCase::Failed:                        break;
         default:                    assert( 0 );     break;
      }
   }
   return allDone;
}

///////////////////////////////////////////////////////////////////////////////
// Execute all scheduled cases. Very loooong cycle
ErrorHandler::ReturnCode RunManagerImpl::runScheduledCases( int updateStateTimeInterval )
{
   try 
   {
      size_t prevFinished      = 0;
      size_t prevToBeSubmitted = 0;
      size_t prevRunning       = 0;
      size_t prevPending       = 0;

      do
      {
         if ( ibs::FilePath( std::string( "./" ) + RunManager::s_scenarioExecStopFileName ).exists() ) { return stopAllSubmittedJobs(); }
        
         // loop over all cases
         for ( size_t i = 0; i < m_jobs.size(); ++i )
         {
            bool contAppPipeline = true;
            
            for ( size_t j = 0; j < m_jobs[i].size() && contAppPipeline; ++j )
            {
               JobScheduler::JobID job = m_jobs[i][j];
               JobScheduler::JobState jobState = m_jobSched->jobState( job );

               // if job is not submited yet - run it
               if ( (m_maxPendingJobs < 1 || prevPending < m_maxPendingJobs) && JobScheduler::NotSubmittedYet == jobState )
               {
                  if ( m_depOnJob.count( job ) > 0 )
                  {  // job is dependent on the job from anothe case, check it status
                     const std::vector<JobScheduler::JobID> & jbs = m_depOnJob[job];
                     bool allJobFinished = true;
                     for ( size_t k = 0; k < jbs.size() && allJobFinished; ++k )
                     {
                        switch( m_jobSched->jobState( jbs[k] ) )
                        {
                           case JobScheduler::NotSubmittedYet: jobState = JobScheduler::NotSubmittedYet; allJobFinished = false; break; 
                           case JobScheduler::JobFailed:       jobState = JobScheduler::JobFailed;       allJobFinished = false; break;
                           case JobScheduler::JobPending:
                           case JobScheduler::JobRunning:      jobState = JobScheduler::JobPending;      allJobFinished = false; break;
                           // submit the job if other job is succeeded
                           case JobScheduler::JobFinished:                                                                       break;
                           default: assert( 0 );
                        }
                     }
                     if ( allJobFinished )
                     { 
                        jobState = m_jobSched->runJob( job );
                        if ( JobScheduler::JobPending == jobState ) ++prevPending; // take into account just submitted job
                     }
                  }
                  else
                  {
                     jobState = m_jobSched->runJob( job ); // submit the job if it not dependent on any other job
                     if ( JobScheduler::JobPending == jobState ) ++prevPending; // take into account just submitted job
                  }
               }

               // analyse job state from point of view further pipeline processing
               switch ( jobState )
               {
                  case JobScheduler::JobFailed: // job failed!!! shouldn't run others in a pipeline!
                     m_jobs[i].resize( j+1 ); // drop all other jobs for this case
                     m_cases[i]->setRunStatus( RunCase::Failed );
                     contAppPipeline = false; 
                     break;

                  case JobScheduler::JobPending:
                  case JobScheduler::NotSubmittedYet:
                  case JobScheduler::JobRunning:
                     contAppPipeline = false; // stop going further in applications pipeline for this case
                     break;

                  case JobScheduler::JobFinished: // continue pipeline processing
                     break;

                  default: assert(0); break; // unknown state, must not happen
               }
            }

            // If pipeline was successfully completed and case marked as scheduled - move it to completed state
            if ( contAppPipeline && m_cases[i]->runStatus() == RunCase::Scheduled )
            {
               m_cases[i]->setRunStatus( RunCase::Completed );
            }
         }

         collectStatistics( prevFinished, prevPending, prevRunning, prevToBeSubmitted );
         m_jobSched->sleep( updateStateTimeInterval ); // wait a bit till go to the next loop

      } while ( !isAllDone() ); // loop till all will be finished
   }
   catch ( Exception & ex )
   {
      // We need to kill all job before exit with exception
      stopAllSubmittedJobs();

      return reportError( ex.errorCode(), ex.what() ); 
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



// in case of scenario execution aborted - kill all submitted not finished jobs
ErrorHandler::ReturnCode RunManagerImpl::stopAllSubmittedJobs()
{
   ReturnCode ret = NoError;

   // loop over all cases
   for (    size_t i = 0; i < m_jobs.size(); ++i )
   {
      for ( size_t j = 0; j < m_jobs[i].size(); ++j )
      {
         JobScheduler::JobState jobState = m_jobSched->jobState( m_jobs[i][j] );

         try
         {
            switch ( jobState )
            {
               case JobScheduler::NotSubmittedYet:
               case JobScheduler::JobFailed: 
               case JobScheduler::JobFinished:
                  break; // job is not in queue - do nothing

               case JobScheduler::JobPending:
               case JobScheduler::JobRunning:
                  m_jobSched->stopJob( m_jobs[i][j] );         // kill the job
                  m_cases[i]->setRunStatus( RunCase::Failed ); // invalidate case
                  ret = RunManagerAborted;
                  break;

               default: break;
            }
         }
         catch( Exception & ex )
         {
            LogHandler( LogHandler::DEBUG_SEVERITY ) << "Job id: " << m_jobs[i][j] << " failed to stop due to error: " << ex.what();
         }
      }
   }
   return ret;
}


///////////////////////////////////////////////////////////////////////////////
// Get cluster name from job scheduler
std::string RunManagerImpl::clusterName() { return m_jobSched.get() ? m_jobSched->clusterName() : "LOCAL"; }


void RunManagerImpl::restoreCaseStatus( RunCase * cs )
{
   RunCaseImpl * rcs = dynamic_cast<RunCaseImpl *>( cs );

   // check if the case already in list
   bool found = false;
   for ( size_t i = 0; i < m_cases.size() && !found; ++i )
   {
      if ( m_cases[i] == rcs ) found = true;
   }
   if ( found ) return; // already in the list - do nothing

   // do not add cases which has no project file
   if ( !rcs->projectPath() ) throw Exception( WrongPath ) << "Case with empty path to project file was given";

   // get project file path
   ibs::FilePath pfp( rcs->projectPath() );

   // if no project defined - report error
   if ( !pfp.exists() ) throw Exception( WrongPath ) << "Wrong path to case project file was given";

   // add empty row to jobs list
   m_jobs.push_back( std::vector< JobScheduler::JobID >() );
   m_cases.push_back( rcs );

   // construct case name, use name of the directory where project is located or just Case_N
   size_t sz = pfp.size();
   std::string caseName = sz > 2 ? pfp[sz - 2 ] : (std::string( "Case_" ) + ibs::to_string( m_jobs.size() ) );
   
   // go through pipelines and populate jobs list check generated scripts for all stages
   if ( !m_appList.empty() )
   {
      std::vector< JobScheduler::JobState > stageState;
      std::vector< std::string >            stageScript;

      for ( size_t i = 0; i < m_appList.size(); ++i )
      {

         // generate script file name
         ibs::FilePath scriptFile( pfp.filePath() );
         scriptFile << (std::string( "Stage_" ) + ibs::to_string( i ) + m_appList[i]->scriptSuffix() );
      
         if ( !scriptFile.exists() ) throw Exception( WrongPath ) << scriptFile.path() << " does not exist, can't reload case state";

         // construct job name 
         std::ostringstream oss;
         oss << caseName << "_stage_" << ibs::to_string( i );

         stageScript.push_back( scriptFile.path() );

         stageState.push_back( m_jobSched->restoreJobState( pfp.filePath().c_str(),  // cwd
                                                            stageScript.back(),      // script name
                                                            oss.str()
                                                          ) );
      }

      if (      stageState.front() == JobScheduler::NotSubmittedYet ) { rcs->setRunStatus( RunCase::NotSubmitted ); }
      else if ( stageState.back()  == JobScheduler::JobFinished     ) { rcs->setRunStatus( RunCase::Completed       ); }
      else                                                            { rcs->setRunStatus( RunCase::Failed          ); }

      if ( rcs->runStatus() != RunCase::Completed )
      {
         for ( size_t i = 0; i < stageState.size(); ++i )
         {
            if ( stageState[i] == JobScheduler::JobFinished ) continue;

            // construct job name 
            std::ostringstream oss;
            oss << caseName << "_stage_" << ibs::to_string( i );

            ////////////////////////////////////////
            /// put job to the queue through job scheduler
            JobScheduler::JobID id = m_jobSched->addJob( pfp.filePath().c_str()       // cwd
                                                       , stageScript[i]               // script name
                                                       , oss.str()                    // job name
                                                       , m_appList[i]->cpus()         // number of CPUs for this job
                                                       , m_appList[i]->runTimeLimit() // run time limit
                                                       , ""
                                                       );

            // put job to the queue for the current case
            m_jobs.back().push_back( id );
         }
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
// Clean application pipeline, jobs and recreate job scheduler. Keep all RunManager settings
void RunManagerImpl::resetState( bool cleanApps )
{
   // save some settings of current state
   std::string clusterName = m_jobSched->clusterName();

   std::string       resourcReq;
   JobSchedulerLSF * jsc = dynamic_cast<JobSchedulerLSF*>( m_jobSched.get() );
   if ( jsc ) { resourcReq = jsc->resourceRequirements(); }

   // re create instance of job scheduler
   createJobScheduler( clusterName );
   if ( !resourcReq.empty() ) { setResourceRequirements( resourcReq ); }

   // delete apps list
   if ( cleanApps )
   {
      for ( auto app : m_appList ) { delete app; }
      m_appList.clear();
   }

   // clean queues
   m_jobs.clear();
   m_cases.clear();
   m_depOnJob.clear();
}
 
// Serialize object to the given stream
bool RunManagerImpl::save( CasaSerializer & sz, unsigned int /* fileVersion */ ) const
{
   bool ok = true;

   ok = ok ? sz.save( m_cldVersion, "CauldronVersion" ) : ok;
   ok = ok ? sz.save( m_ibsRoot,    "IBSRoot"         ) : ok;

   ok = ok ? sz.save( m_appList.size(), "AppListSize" ) : ok;
   for ( size_t i = 0; i < m_appList.size() && ok; ++i )
   {
      ok = sz.save( *m_appList[i], "CldApp" );
   }

   ok = ok ? sz.save( *m_jobSched.get(), "JobScheduler"  ) : ok;
   ok = ok ? sz.save( m_jobs.size(),     "NumberOfCases" ) : ok;

   for ( size_t i = 0; i < m_jobs.size() && ok; ++i )
   {
      ok = ok ? sz.save( m_jobs[i], "CaseJobsQueueIDs" ) : ok;
   }

   // save array of run case object ids, the size of array exactly the same as m_jobs size
   assert( m_jobs.size() == m_cases.size() );
   for ( size_t i = 0; i < m_cases.size(); ++i )
   {
      CasaSerializer::ObjRefID rcID = sz.ptr2id( m_cases[i] );
      ok = ok ? sz.save( rcID, "RunCaseID" ) : ok;
   }

   ok = ok & sz.save( m_maxPendingJobs, "maxPendingJobs" );

   return ok;
}

// Create a new instance and deserialize it from the given stream
RunManagerImpl::RunManagerImpl( CasaDeserializer & dz, const char * objName )
{
   // read from file object name and version
   unsigned int objVer = version();
   bool ok = dz.checkObjectDescription( typeName(), objName, objVer );

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

   // load array of run case object ids, the size of array exactly the same as m_jobs size
   for ( size_t i = 0; i < m_jobs.size() && ok; ++i )
   {
      CasaDeserializer::ObjRefID rcID;
      ok = ok ? dz.load( rcID, "RunCaseID" ) : ok;

      RunCaseImpl * rc = ok ? const_cast<RunCaseImpl*>( dz.id2ptr<RunCaseImpl>( rcID ) ) : 0;

      if ( rc ) m_cases.push_back( rc );
      else ok = false;
   }

   ok = ok ? dz.load( m_maxPendingJobs, "maxPendingJobs" ) : ok;

   if ( !ok ) throw Exception( DeserializationError ) << "RunManagerImpl deserialization error";
}

// create job scheduler depending on cluster name and OS
void RunManagerImpl::createJobScheduler( const std::string & clusterName )
{
#if defined (_WIN32) || !defined (WITH_LSF_SCHEDULER)
   m_jobSched.reset( new JobSchedulerLocal() );
#else
   if ( clusterName == "LOCAL" ) { m_jobSched.reset( new JobSchedulerLocal()            ); }
   else                          { m_jobSched.reset( new JobSchedulerLSF( clusterName ) ); }
#endif
   // delete file with jobs list if exist
   ibs::FilePath jobsIDFile( "." ); 
   jobsIDFile << s_jobsIDListFileName;
   if ( jobsIDFile.exists() ) { jobsIDFile.remove(); }
}
 
}

