//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// CMB
#include "ErrorHandler.h"

// FileSystem
#include "FilePath.h"

// CASA
#include "CasaDeserializer.h"
#include "JobSchedulerLSF.h"
#include "RunManager.h"

#include "CauldronEnvConfig.h"

// LSF
#ifdef WITH_LSF_SCHEDULER
#include <lsf/lsbatch.h>

#ifdef _WIN32
static int setenv( const char * name, const char * value, int overwrite )
{
   int errcode = 0;
   if( !overwrite )
   {
      size_t envsize = 0;
      errcode = getenv_s( &envsize, NULL, 0, name );
      if( errcode || envsize ) return errcode;
   }
   return _putenv_s( name, value );
}
#endif

#else
// In case we have no lsf.h for the platform, 
// use some mockup to allow deserialization
#define LSF_RLIM_NLIMITS  12
#define DEFAULT_RLIMIT   -1
#define LSF_RLIMIT_RUN    9 

#ifndef _WIN32
#include <sys/types.h>
typedef int64_t LS_LONG_INT;
#else
typedef __int64 LS_LONG_INT;
#endif

struct submit {
   char   * projectName;
   char   * command;
   char   * jobName;
   char   * outFile;
   char   * errFile;
   char   * cwd;
   char   * resReq;
   int      rLimits[LSF_RLIM_NLIMITS];
   int      options; 
   int      options2;
   int      options3;
   int      numProcessors;
   int      maxNumProcessors;
};

struct submitReply
{
   int dummy;
};

#define  SUB_JOB_NAME       0x01
#define  SUB_OUT_FILE       0x10
#define  SUB_ERR_FILE       0x20
#define  SUB_PROJECT_NAME   0x2000000
#define  SUB3_CWD           0x40
#define  SUB_RES_REQ        0x40000
#endif

// STL
#include <iostream>
#include <fstream>

// STD C lib
#include <cstdlib>
#include <cstring>

#ifndef _WIN32
#include <unistd.h>
#include <sys/stat.h>
static void Wait( int sec ) { sleep( sec ); }
#else
#include <windows.h>
static void Wait( int milsec ) { Sleep( milsec * 1000 ); }
#define strdup _strdup
#endif

namespace casa
{

class JobSchedulerLSF::Job : public CasaSerializable
{
public:
   Job( const std::string & cwd
      , const std::string & scriptName
      , const std::string & jobName
      , int                 cpus
      , size_t              runTimeLim
      , const std::string & resReq
      )
   {
      m_lsfJobID   = -1;
      m_isFinished = false;
      m_isFailed   = false;
      m_runTimeLim = runTimeLim;
      m_runAttemptsNum = 0;

      // clean LSF structures
      memset( &m_submit,     0, sizeof( m_submit ) );
      memset( &m_submitRepl, 0, sizeof( m_submitRepl ) );

      // resource limits are initialized to default
      for ( int i = 0; i < LSF_RLIM_NLIMITS; ++i ) { m_submit.rLimits[i] = DEFAULT_RLIMIT; }
      if ( m_runTimeLim > 0 ) { m_submit.rLimits[LSF_RLIMIT_RUN] = static_cast<int>( m_runTimeLim ) * 60; } // convert to sec.
 
      /// Prepare job to submit through LSF
      m_submit.projectName      = strdup( s_LSF_CAULDRON_PROJECT_NAME ); // add project name (must be the same for all cauldron app)
      m_submit.command          = strdup( scriptName.c_str() );
      m_submit.jobName          = strdup( jobName.c_str() );
      m_submit.outFile          = strdup( (jobName + ".out" ).c_str() ); // redirect stdout to file
      m_submit.errFile          = strdup( (jobName + ".err" ).c_str() ); // redirect stderr to file
      m_submit.options          = SUB_PROJECT_NAME | SUB_JOB_NAME | SUB_OUT_FILE | SUB_ERR_FILE;
      m_submit.cwd              = strdup( cwd.c_str() );                 // define current working directory
      m_submit.options3         = SUB3_CWD;
      m_submit.numProcessors    = cpus; // initial number of processors needed by a (parallel) job
      m_submit.maxNumProcessors = cpus; // max num of processors required to run the (parallel) job
      if ( !resReq.empty() )
      {
         m_submit.resReq = strdup( resReq.c_str() );
         m_submit.options = m_submit.options | SUB_RES_REQ;
      }
   }

   ~Job()
   {  // allocated by strdup
      if ( m_submit.projectName ) free( m_submit.projectName );
      if ( m_submit.command     ) free( m_submit.command     );
      if ( m_submit.jobName     ) free( m_submit.jobName     );
      if ( m_submit.cwd         ) free( m_submit.cwd         );
      if ( m_submit.outFile     ) free( m_submit.outFile     );
      if ( m_submit.errFile     ) free( m_submit.errFile     );
      if ( m_submit.resReq      ) free( m_submit.resReq      );
   }

   const char * command() const  { return m_submit.command; }

   // check is job was submitted already
   bool isSubmitted( ) const
   {
#ifdef WITH_LSF_SCHEDULER
      return m_lsfJobID < 0 ? false : true;
#else
      return true;
#endif
   }
   
   bool isFinished() const { return m_isFinished; } // check is job finished?
   bool isFailed()   const { return m_isFailed;   } // check is job failed?

   bool submit()
   {
#ifdef WITH_LSF_SCHEDULER
      m_lsfJobID = lsb_submit( &m_submit, &m_submitRepl );
#endif
      ++m_runAttemptsNum;
      return isSubmitted();
   }

   // sometimes due to infrastructure problems job can't be submitted, in this case
   // we will try to submit ~30 times with 10 sec interval, and fail only after that
   bool shouldRetryFailedToSubmitJob()
   {
      if ( m_runAttemptsNum > 30 ) return false;
#ifdef WITH_LSF_SCHEDULER
      // External authentication failed, something wrong with LSF, retry later
      if ( lsberrno == LSBE_LSBLIB && 
           !std::string( "External authentication failed" ).compare( lsb_sysmsg() ) 
         )
      {
         return true;
      }
#endif
      return false;
   }

   bool stop()
   {
#ifdef WITH_LSF_SCHEDULER
      if ( isSubmitted() ) { return lsb_forcekilljob( m_lsfJobID ) < 0 ? false : true; }
#endif
      return true;
   }

   LS_LONG_INT id() { return m_lsfJobID; }

   // check job status
   JobScheduler::JobState status()
   {
#ifdef WITH_LSF_SCHEDULER
      // something still on the cluster, check status
      if ( lsb_openjobinfo( m_lsfJobID, NULL, NULL, NULL, NULL, ALL_JOB ) < 0 )
      {
         throw ErrorHandler::Exception( ErrorHandler::LSFLibError ) << "Reading job status error code: " << lsberrno <<
            ", message: " << lsb_sysmsg();
      }

      int more;                   /* number of remaining jobs unread */
      struct jobInfoEnt * jobInfo = lsb_readjobinfo( &more ); // detailed job info

      if ( !jobInfo )
      {
         throw ErrorHandler::Exception( ErrorHandler::LSFLibError ) << "Reading job status error code: " << lsberrno <<
            ", message: " << lsb_sysmsg();
      }

      lsb_closejobinfo();

      if ( jobInfo->status & JOB_STAT_DONE )
      {
         m_isFinished = true;
         m_isFailed   = false;
         return JobFinished;   // job is completed successfully
      }

      if ( jobInfo->status & JOB_STAT_RUN ) return JobRunning;     // job is running

      if ( jobInfo->status & JOB_STAT_WAIT  || // chunk job waiting its execution turn
           jobInfo->status & JOB_STAT_PEND  || // job is pending
           jobInfo->status & JOB_STAT_PSUSP || // job is held
           jobInfo->status & JOB_STAT_SSUSP || // job is suspended by LSF batch system
           jobInfo->status & JOB_STAT_USUSP    // job is suspended by user
         ) { return JobPending; }

      if ( jobInfo->status & JOB_STAT_EXIT )
      {
         m_isFinished = true;
         m_isFailed = true;
         return JobFailed; // job exited
      }
#endif
      return Unknown;   // unknown status
   }

   // Serialization / Deserialization
   // version of serialized object representation
   virtual unsigned int version() const { return 3; }

   // Get type name of the serialaizable object, used in deserialization to create object with correct type
   virtual const char * typeName() const { return "JobSchedulerLSF::Job"; }

   // Serialize object to the given stream
   virtual bool save( CasaSerializer & sz, unsigned int version ) const
   {
      bool ok = sz.save(              m_isFinished,                "IsFinished"      );
      ok = ok ? sz.save(              m_isFailed,                  "IsFailed"        ) : ok;
      ok = ok ? sz.save( std::string( m_submit.projectName ),      "CldProjectName"  ) : ok;
      ok = ok ? sz.save( std::string( m_submit.command ),          "ScriptName"      ) : ok;
      ok = ok ? sz.save( std::string( m_submit.jobName ),          "JobName"         ) : ok;
      ok = ok ? sz.save( std::string( m_submit.outFile ),          "StdOutLogFile"   ) : ok;
      ok = ok ? sz.save( std::string( m_submit.errFile ),          "StdErrLogFile"   ) : ok;
      ok = ok ? sz.save(              m_submit.options,            "OptionsFlags"    ) : ok;
      ok = ok ? sz.save( std::string( m_submit.cwd ),              "CWD"             ) : ok;
      ok = ok ? sz.save(              m_submit.options3,           "Options3Flags"   ) : ok;
      ok = ok ? sz.save(              m_submit.numProcessors,      "CPUsNum"         ) : ok;
      ok = ok ? sz.save(              m_submit.maxNumProcessors,   "MaxCPUsNum"      ) : ok;
      ok = ok ? sz.save( static_cast<long long>( m_lsfJobID ),     "LSFJobID"        ) : ok;
      ok = ok ? sz.save(              m_runTimeLim,                "JobRunTimeLimit" ) : ok;
      ok = ok ? sz.save( std::string( m_submit.resReq ? m_submit.resReq : "" ), "ResReq" ) : ok;

      // TODO save necessary fields for submitRepl
      //struct submitReply m_submitRepl; // lsf_submit returns here some info in case of error
      return ok;        
   }

   // Create a new instance and deserialize it from the given stream
   Job( CasaDeserializer & dz, const char * objName )
   {
      // read from file object name and version
      unsigned int objVer = version();
      bool ok = dz.checkObjectDescription( typeName(), objName, objVer );

      if ( objVer > 0 ) { ok = ok ? dz.load( m_isFinished, "IsFinished" ) : ok; }
      else              { m_isFinished = true; }

      if ( objVer > 1 ) { ok = ok ? dz.load( m_isFailed,   "IsFailed" ) : ok; }
      else              { m_isFailed = false; }

      // clean LSF structures
      memset( &m_submit,     0, sizeof( m_submit ) );
      memset( &m_submitRepl, 0, sizeof( m_submitRepl ) );

      // resource limits are initialized to default
      for ( int i = 0; i < LSF_RLIM_NLIMITS; ++i ) { m_submit.rLimits[i] = DEFAULT_RLIMIT; }

      std::string buf;
      long long rjid;
      ok = ok ? dz.load( buf,                       "CldProjectName"  ) : ok; m_submit.projectName = strdup( buf.c_str() );
      ok = ok ? dz.load( buf,                       "ScriptName"      ) : ok; m_submit.command     = strdup( buf.c_str() );
      ok = ok ? dz.load( buf,                       "JobName"         ) : ok; m_submit.jobName     = strdup( buf.c_str() );
      ok = ok ? dz.load( buf,                       "StdOutLogFile"   ) : ok; m_submit.outFile     = strdup( buf.c_str() );
      ok = ok ? dz.load( buf,                       "StdErrLogFile"   ) : ok; m_submit.errFile     = strdup( buf.c_str() );
      ok = ok ? dz.load( m_submit.options,          "OptionsFlags"    ) : ok;
      ok = ok ? dz.load( buf,                       "CWD"             ) : ok; m_submit.cwd         = strdup( buf.c_str() );
      ok = ok ? dz.load( m_submit.options3,         "Options3Flags"   ) : ok;
      ok = ok ? dz.load( m_submit.numProcessors,    "CPUsNum"         ) : ok;
      ok = ok ? dz.load( m_submit.maxNumProcessors, "MaxCPUsNum"      ) : ok;
      ok = ok ? dz.load( rjid,                      "LSFJobID"        ) : ok; m_lsfJobID = static_cast<LS_LONG_INT>( rjid );
      ok = ok ? dz.load( m_runTimeLim,              "JobRunTimeLimit" ) : ok;

      if ( objVer > 2 ) { ok = ok ? dz.load( buf, "ResReq" ) : ok; m_submit.resReq = buf.empty() ? NULL : strdup( buf.c_str() ); }

      if ( !ok )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "JobSchedulerLSF::Job deserialization error";
      }
   }

 private:
   bool               m_isFinished; // was this job finished?
   bool               m_isFailed;   // was this job failed?

   // fields related to interaction with LSF
   struct submit      m_submit;     // lsf_submit use values from this structure to submit job
   struct submitReply m_submitRepl; // lsf_submit returns here some info in case of error

   LS_LONG_INT        m_lsfJobID;   // job ID in LSF
   size_t             m_runTimeLim; // runt time job limitation [Minutes]

   size_t             m_runAttemptsNum; // number of attemts to run job
   // disable copy constructor/operator
   Job( const Job & jb );
   Job & operator = ( const Job & jb );
};


JobSchedulerLSF::JobSchedulerLSF( const std::string & clusterName )
{
#ifdef WITH_LSF_SCHEDULER
   const char * lsfConfDir = getenv( "LSF_ENVDIR" );
   if ( !lsfConfDir )
   {
      setenv( "LSF_ENVDIR", s_LSF_CONFIG_DIR, 0 ); // LSF_ENVDIR is needed to make LSF API calls
   }
#endif

   if ( !clusterName.empty() ) { m_clusterName = clusterName; }
#ifdef WITH_LSF_SCHEDULER
   else
   {
      // get cluster name
      char * clName = ls_getclustername();
      m_clusterName = clName != NULL ? clName : "Undefined";
   }

   // TODO make usage of cluster name
   // initialize LSFBat library
   if ( lsb_init( NULL ) < 0 ) throw ErrorHandler::Exception( ErrorHandler::LSFLibError ) << "LSFBat library initialization failed: " << ls_sysmsg();
#endif
}

JobSchedulerLSF::~JobSchedulerLSF() { for ( size_t i = 0; i < m_jobs.size(); ++i ) { delete m_jobs[i]; } } // clean array of scheduled jobs

// Add job to the list
JobScheduler::JobID JobSchedulerLSF::addJob( const std::string & cwd
                                           , const std::string & scriptName
                                           , const std::string & jobName
                                           , int                 cpus
                                           , size_t              runTimeLim
                                           )
{
   ibs::FilePath scriptStatFile( scriptName + ".failed" );
   if ( scriptStatFile.exists() ) scriptStatFile.remove();

   m_jobs.push_back( new Job( cwd, scriptName, jobName, cpus, runTimeLim, m_resReqStr ) );
   return m_jobs.size() - 1; // the position of the new job in the list is it JobID
}

// run job
JobScheduler::JobState JobSchedulerLSF::runJob( JobID job )
{
   if ( job >= m_jobs.size() )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "runJob(): no job with ID: "  << job << " in the queue"; 
   }

   if ( !m_jobs[job]->isSubmitted() )
   {
      // spawn job to the cluster
      if ( !m_jobs[job]->submit() )
      {
         if ( !m_jobs[job]->shouldRetryFailedToSubmitJob() )
         {
            throw ErrorHandler::Exception( ErrorHandler::LSFLibError ) << "Submitting job " << m_jobs[ job ]->command() << " to the cluster failed"
#ifdef WITH_LSF_SCHEDULER
               << ", LSF error: " << lsberrno << ", message: " << lsb_sysmsg()
#endif
            ;
         }
         else { return NotSubmittedYet; }
      }
   }
   // log job ID
   std::ofstream ofs( RunManager::s_jobsIDListFileName, std::ios_base::out | std::ios_base::app );
   if ( ofs.is_open() ) { ofs << schedulerJobID( job ) << std::endl; }
 
   return jobState( job );
}

// stop submitted job
JobScheduler::JobState JobSchedulerLSF::stopJob( JobID job )
{
   if ( job >= m_jobs.size() ) 
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "runJob(): no job with ID: "  << job << " in the queue";
   }

   if ( m_jobs[job]->isSubmitted() )
   {
      // spawn job to the cluster
      if ( !m_jobs[job]->stop() )
      {
         throw ErrorHandler::Exception( ErrorHandler::LSFLibError ) << "Stopping the job " << m_jobs[ job ]->command() << " on cluster failed"
#ifdef WITH_LSF_SCHEDULER
            << ", LSF error: " << lsberrno << ", message: " << lsb_sysmsg()
#endif
         ;
      }
   }

   return JobFailed;
}

// get job state
JobScheduler::JobState JobSchedulerLSF::jobState( JobID id )
{
   if ( id >= m_jobs.size() ) throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "jobState(): no such job in the queue";

   Job * job = m_jobs[id];
   if (  job->isFinished()  ) return job->isFailed() ? JobFailed : JobFinished;  // if job was finished already for some reason, do not check again
   if ( !job->isSubmitted() ) return NotSubmittedYet;                            // job wasn't submitted yet

   return job->status();                                                         // unknown status
}

std::string JobSchedulerLSF::schedulerJobID( JobID id )
{
   if ( id >= m_jobs.size() ) throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "jobState(): no such job in the queue";

   std::ostringstream oss;
   oss << m_jobs[id]->id();

   return oss.str();
}

void JobSchedulerLSF::sleep()
{
   Wait( 10 );
}

// Serialize object to the given stream
bool JobSchedulerLSF::save( CasaSerializer & sz, unsigned int fileVersion ) const
{
   bool ok = sz.save( m_clusterName, "ClusterName" );
   
   ok = ok ? sz.save( m_resReqStr,   "LSFResRequest" ) : ok;
   ok = ok ? sz.save( m_jobs.size(), "JobsQueueSize" ) : ok;
   for ( size_t i = 0; i < m_jobs.size() && ok; ++i )
   {
      ok = sz.save( *m_jobs[i], "JobDescr" );
   }
   return ok;
}

// Create a new instance and deserialize it from the given stream
JobSchedulerLSF::JobSchedulerLSF( CasaDeserializer & dz, unsigned int objVer )
{
   if ( version() < objVer )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "Version of object in file is newer. No forward compatibility!";
   }

   bool ok = dz.load( m_clusterName, "ClusterName" );
  
   if ( objVer > 0 ) { ok = ok ? dz.load( m_resReqStr,   "LSFResRequest" ) : ok; }

   size_t setSize;
   ok = ok ? dz.load( setSize, "JobsQueueSize" ) : ok;
   for ( size_t i = 0; i < setSize && ok; ++i )
   {
      m_jobs.push_back( new Job( dz, "JobDescr" ) );
   }

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "JobSchedulerLSF deserialization error";
   }
}


void JobSchedulerLSF::printLSFBParametersInfo()
{
#ifdef WITH_LSF_SCHEDULER
   struct parameterInfo * lsfbPrms = lsb_parameterinfo( NULL, NULL, 0 );
   if ( !lsfbPrms ) { return throw ErrorHandler::Exception( ErrorHandler::LSFLibError ) << ls_sysmsg(); }

   std::cout << lsfbPrms->defaultQueues                << ": DEFAULT_QUEUE: A blank_separated list of queue names for automatic queue selection. " << std::endl;
   std::cout << lsfbPrms->defaultHostSpec              << ": DEFAULT_HOST_SPEC: The host name or host model name used as the system default for scaling CPULIMIT and RUNLIMIT. " << std::endl;
   std::cout << lsfbPrms->mbatchdInterval              << ": MBD_SLEEP_TIME: The interval in seconds at which the mbatchd dispatches jobs. " << std::endl;
   std::cout << lsfbPrms->sbatchdInterval              << ": SBD_SLEEP_TIME: The interval in seconds at which the sbatchd suspends or resumes jobs. " << std::endl;
   std::cout << lsfbPrms->jobAcceptInterval            << ": JOB_ACCEPT_INTERVAL: The interval at which. a host accepts two successive jobs. (In units of SBD_SLEEP_TIME.) " << std::endl;
   std::cout << lsfbPrms->maxDispRetries               << ": MAX_RETRY: The maximum number of retries for dispatching a job. " << std::endl;
   std::cout << lsfbPrms->maxSbdRetries                << ": MAX_SBD_FAIL: The maximum number of retries for reaching an sbatchd. " << std::endl;
   std::cout << lsfbPrms->preemptPeriod                << ": PREEM_PERIOD: The interval in seconds for preempting jobs running on the same host. " << std::endl;
   std::cout << lsfbPrms->cleanPeriod                  << ": CLEAN_PERIOD: The interval in seconds during which finished jobs are kept in core. " << std::endl;
   std::cout << lsfbPrms->maxNumJobs                   << ": MAX_JOB_NUM: The maximum number of finished jobs that are logged in the current event file. " << std::endl;
   std::cout << lsfbPrms->historyHours                 << ": HIST_HOURS: The number of hours of resource consumption history used for fair share scheduling and scheduling within a host partition. " << std::endl;
   std::cout << lsfbPrms->pgSuspendIt                  << ": PG_SUSP_IT: The interval a host must be idle before resuming a job suspended for excessive paging. " << std::endl;
   std::cout << lsfbPrms->defaultProject               << ": The default project assigned to jobs. " << std::endl;
   std::cout << lsfbPrms->retryIntvl                   << ": Job submission retry interval " << std::endl;
   std::cout << lsfbPrms->nqsQueuesFlags               << ": For Cray NQS compatiblilty only. Used by LSF to get the NQS queue information " << std::endl;
   std::cout << lsfbPrms->nqsRequestsFlags             << ": nqsRequestsFlags " << std::endl;
   std::cout << lsfbPrms->maxPreExecRetry              << ": The maximum number of times to attempt the preexecution command of a job from a remote cluster (MultiCluster only) " << std::endl;
   std::cout << lsfbPrms->localMaxPreExecRetry         << ": Maximum number of pre-exec retry times for local cluster" << std::endl;
   std::cout << lsfbPrms->eventWatchTime               << ": Event watching Interval in seconds" << std::endl;
   std::cout << lsfbPrms->runTimeFactor                << ": Run time weighting factor for fairshare scheduling " << std::endl;
   std::cout << lsfbPrms->waitTimeFactor               << ": Used for calcultion of the fairshare scheduling formula " << std::endl;
   std::cout << lsfbPrms->runJobFactor                 << ": Job slots weighting factor for fairshare scheduling " << std::endl;
   std::cout << lsfbPrms->eEventCheckIntvl             << ": Default check interval " << std::endl;
   std::cout << lsfbPrms->rusageUpdateRate             << ": sbatchd report every sbd_sleep_time " << std::endl;
   std::cout << lsfbPrms->rusageUpdatePercent          << ": sbatchd updates jobs jRusage in mbatchd if more than 10% changes " << std::endl;
   std::cout << lsfbPrms->condCheckTime                << ": Time period to check for reconfig " << std::endl;
   std::cout << lsfbPrms->maxSbdConnections            << ": The maximum number of connections between master and slave batch daemons " << std::endl;
   std::cout << lsfbPrms->rschedInterval               << ": The interval for rescheduling jobs " << std::endl;
   std::cout << lsfbPrms->maxSchedStay                 << ": Max time mbatchd stays in scheduling routine, after which take a breather " << std::endl;
   std::cout << lsfbPrms->freshPeriod                  << ": During which load remains fresh " << std::endl;
   std::cout << lsfbPrms->preemptFor                   << ": The preemption behavior, GROUP_MAX, GROUP_JLP, USER_JLP, HOST_JLU,MINI_JOB, LEAST_RUN_TIME " << std::endl;
   std::cout << lsfbPrms->adminSuspend                 << ": Flags whether users can resume their jobs when suspended by the LSF administrator " << std::endl;
   std::cout << lsfbPrms->userReservation              << ": Flags to enable/disable normal user to create advance reservation " << std::endl;
   std::cout << lsfbPrms->cpuTimeFactor                << ": CPU time weighting factor for fairshare scheduling " << std::endl;
   std::cout << lsfbPrms->fyStart                      << ": The starting month for a fiscal year " << std::endl;
   std::cout << lsfbPrms->maxJobArraySize              << ": The maximum number of jobs in a job array " << std::endl;
   std::cout << lsfbPrms->exceptReplayPeriod           << ": Replay period for exceptions, in seconds " << std::endl;
   std::cout << lsfbPrms->jobTerminateInterval         << ": The interval to terminate a job " << std::endl;
   std::cout << lsfbPrms->disableUAcctMap              << ": User level account mapping for remote jobs is disabled " << std::endl;
   std::cout << lsfbPrms->enforceFSProj                << ": If set to TRUE, Project name for a job will be considerred when doing fairshare scheduling, i.e., as if user has submitted jobs using -G " << std::endl;
   std::cout << lsfbPrms->enforceProjCheck             << ": Enforces the check to see if the invoker of bsub is in the specifed group when the -P option is used " << std::endl;
   std::cout << lsfbPrms->jobRunTimes                  << ": Run time for a job " << std::endl;
   std::cout << lsfbPrms->dbDefaultIntval              << ": Event table Job default interval " << std::endl;
   std::cout << lsfbPrms->dbHjobCountIntval            << ": Event table Job Host Count " << std::endl;
   std::cout << lsfbPrms->dbQjobCountIntval            << ": Event table Job Queue Count " << std::endl;
   std::cout << lsfbPrms->dbUjobCountIntval            << ": Event table Job User Count " << std::endl;
   std::cout << lsfbPrms->dbJobResUsageIntval          << ": Event table Job Resource Interval " << std::endl;
   std::cout << lsfbPrms->dbLoadIntval                 << ": Event table Resource Load Interval " << std::endl;
   std::cout << lsfbPrms->dbJobInfoIntval              << ": Event table Job Info " << std::endl;
   std::cout << lsfbPrms->jobDepLastSub                << ": Used with job dependency scheduling" << std::endl;
//   std::cout << lsfbPrms->dbSelectLoad                 << ": Select resources to be logged " << std::endl;
   std::cout << lsfbPrms->jobSynJgrp                   << ": Job synchronizes its group status " << std::endl;
   std::cout << lsfbPrms->pjobSpoolDir                 << ": The batch jobs' temporary output directory " << std::endl;
   std::cout << lsfbPrms->maxUserPriority              << ": Maximal job priority defined for all users " << std::endl;
   std::cout << lsfbPrms->jobPriorityValue             << ": Job priority is increased by the system dynamically based on waiting time " << std::endl;
   std::cout << lsfbPrms->jobPriorityTime              << ": Waiting time to increase Job priority by the system dynamically " << std::endl;
   std::cout << lsfbPrms->enableAutoAdjust             << ": Enable internal statistical adjustment " << std::endl;
   std::cout << lsfbPrms->autoAdjustAtNumPend          << ": Start to autoadjust when the user has this number of pending jobs " << std::endl;
   std::cout << lsfbPrms->autoAdjustAtPercent          << ": If this number of jobs has been visited skip the user " << std::endl;
   std::cout << lsfbPrms->sharedResourceUpdFactor      << ":  Static shared resource update interval for the cluster actor " << std::endl;
   std::cout << lsfbPrms->scheRawLoad                  << ": Schedule job based on raw load info " << std::endl;
   std::cout << lsfbPrms->jobAttaDir                   << ":  The batch jobs' external storage for attached data " << std::endl;
   std::cout << lsfbPrms->maxJobMsgNum                 << ": Maximum message number for each job " << std::endl;
   std::cout << lsfbPrms->maxJobAttaSize               << ": Maximum attached data size to be transferred for each message " << std::endl;
   std::cout << lsfbPrms->mbdRefreshTime               << ": The life time of a child MBD to serve queries in the MT way " << std::endl;
   std::cout << lsfbPrms->updJobRusageInterval         << ": The interval of the execution cluster updating the job's resource usage " << std::endl;
   std::cout << lsfbPrms->sysMapAcct                   << ": The account to which all windows workgroup users are to be mapped" << std::endl;
   std::cout << lsfbPrms->preExecDelay                 << ": Dispatch delay internal " << std::endl;
   std::cout << lsfbPrms->updEventUpdateInterval       << ": Update duplicate event interval  " << std::endl;
   std::cout << lsfbPrms->resourceReservePerSlot       << ": Resources are reserved for parallel jobs on a per-slot basis " << std::endl;
   std::cout << lsfbPrms->maxJobId                     << ": Maximum job id --- read from the lsb.params " << std::endl;
   std::cout << lsfbPrms->preemptResourceList          << ": Define a list of preemptable resource. names " << std::endl;
   std::cout << lsfbPrms->preemptionWaitTime           << ": The preemption wait time " << std::endl;
   std::cout << lsfbPrms->maxAcctArchiveNum            << ": Maximum number of rollover lsb.acct files kept by mbatchd. " << std::endl;
   std::cout << lsfbPrms->acctArchiveInDays            << ": mbatchd Archive Interval " << std::endl;
   std::cout << lsfbPrms->acctArchiveInSize            << ": mbatchd Archive threshold " << std::endl;
   std::cout << lsfbPrms->committedRunTimeFactor       << ": Committed run time weighting factor " << std::endl;
   std::cout << lsfbPrms->enableHistRunTime            << ": Enable the use of historical run time in the calculation of fairshare scheduling priority, Disable the use of historical run time in the calculation of fairshare scheduling priority " << std::endl;
   std::cout << lsfbPrms->mcbOlmReclaimTimeDelay       << ": Open lease reclaim time " << std::endl;
   std::cout << lsfbPrms->chunkJobDuration             << ": Enable chunk job dispatch for jobs with CPU limit or run limits " << std::endl;
   std::cout << lsfbPrms->sessionInterval              << ": The interval for scheduling jobs by scheduler daemon " << std::endl;
   std::cout << lsfbPrms->publishReasonJobNum          << ": The number of jobs per user per queue whose pending reason is published at the PEND_REASON_UPDATE_INTERVAL interval " << std::endl;
   std::cout << lsfbPrms->publishReasonInterval        << ": The interval for publishing job pending reason by scheduler daemon " << std::endl;
   std::cout << lsfbPrms->publishReason4AllJobInterval << ": Interval(in seconds) of pending reason. publish for all jobs " << std::endl;
   std::cout << lsfbPrms->mcUpdPendingReasonInterval   << ": MC pending reason update interval (0 means no updates) " << std::endl;
   std::cout << lsfbPrms->mcUpdPendingReasonPkgSize    << ": MC pending reason update package size (0 means no limit) " << std::endl;
   std::cout << lsfbPrms->noPreemptRunTime             << ": No preemption if the run time is greater. than the value defined in here. " << std::endl;
   std::cout << lsfbPrms->noPreemptFinishTime          << ": No preemption if the finish time is less than the value defined in here. " << std::endl;
   std::cout << lsfbPrms->acctArchiveAt                << ": mbatchd Archive Time " << std::endl;
   std::cout << lsfbPrms->absoluteRunLimit             << ": Absolute run limit for job " << std::endl;
   std::cout << lsfbPrms->lsbExitRateDuration          << ": The job exit rate duration" << std::endl;
   std::cout << lsfbPrms->lsbTriggerDuration           << ":  The duration to trigger eadmin " << std::endl;
   std::cout << lsfbPrms->maxJobinfoQueryPeriod        << ": Maximum time for job information query commands (for example,with bjobs) to wait " << std::endl;
   std::cout << lsfbPrms->jobSubRetryInterval          << ": Job submission retrial interval for client " << std::endl;
   std::cout << lsfbPrms->pendingJobThreshold          << ": System wide max pending jobs " << std::endl;
   std::cout << lsfbPrms->maxConcurrentJobQuery        << ": Max number of concurrent job query " << std::endl;
   std::cout << lsfbPrms->minSwitchPeriod              << ": Min event switch time period " << std::endl;
   std::cout << lsfbPrms->condensePendingReasons       << ": Condense pending reasons enabled " << std::endl;
   std::cout << lsfbPrms->slotBasedParallelSched       << ": Schedule Parallel jobs based on slots instead of CPUs " << std::endl;
   std::cout << lsfbPrms->disableUserJobMovement       << ": Disable user job movement operations, like btop/bbot. " << std::endl;
   std::cout << lsfbPrms->detectIdleJobAfter           << ": Detect and report idle jobs only after specified minutes. " << std::endl;
   std::cout << lsfbPrms->useSymbolPriority            << ": Use symbolic when specifing priority of symphony jobs " << std::endl;
   std::cout << lsfbPrms->JobPriorityRound             << ": Priority rounding for symphony jobs " << std::endl;
   //std::cout << lsfbPrms->priorityMapping              << ": The mapping of the symbolic priority.for symphony jobs " << std::endl;
   std::cout << lsfbPrms->maxInfoDirs                  << ": Maximum number of subdirectories under LSB_SHAREDIR/cluster/logdir/info " << std::endl;
   std::cout << lsfbPrms->minMbdRefreshTime            << ": The minimum period of a child MBD to serve queries in the MT way " << std::endl;
   std::cout << lsfbPrms->enableStopAskingLicenses2LS  << ": Stop asking license to LS not due to lack license " << std::endl;
   std::cout << lsfbPrms->expiredTime                  << ": Expire time for finished job which will not taken into account when calculating queue fairshare priority " << std::endl;
   std::cout << lsfbPrms->mbdQueryCPUs                 << ": MBD child query processes will only run on the following CPUs " << std::endl;
   std::cout << lsfbPrms->defaultApp                   << ": The default application profile assigned to jobs " << std::endl;
   std::cout << lsfbPrms->enableStream                 << ": Enable or disable data streaming " << std::endl;
   std::cout << lsfbPrms->streamFile                   << ": File to which lsbatch data is streamed " << std::endl;
   std::cout << lsfbPrms->streamSize                   << ": File size in MB to which lsbatch data is streamed " << std::endl;
   std::cout << lsfbPrms->syncUpHostStatusWithLIM      << ": Sync up host status with master LIM is enabled " << std::endl;
   std::cout << lsfbPrms->defaultSLA                   << ": Project schedulign default SLA " << std::endl;
   std::cout << lsfbPrms->slaTimer                     << ": EGO Enabled SLA scheduling timer period " << std::endl;
   std::cout << lsfbPrms->mbdEgoTtl                    << ": EGO Enabled SLA scheduling time to live " << std::endl;
   std::cout << lsfbPrms->mbdEgoConnTimeout            << ": EGO Enabled SLA scheduling connection timeout " << std::endl;
   std::cout << lsfbPrms->mbdEgoReadTimeout            << ": EGO Enabled SLA scheduling read timeout " << std::endl;
   std::cout << lsfbPrms->mbdUseEgoMXJ                 << ": EGO Enabled SLA scheduling use MXJ flag " << std::endl;
   std::cout << lsfbPrms->mbdEgoReclaimByQueue         << ": EGO Enabled SLA scheduling reclaim by queue " << std::endl;
   std::cout << lsfbPrms->defaultSLAvelocity           << ": EGO Enabled SLA scheduling default velocity" << std::endl;
   std::cout << lsfbPrms->exitRateTypes                << ": Type of host exit rate exception handling types: EXIT_RATE_TYPE " << std::endl;
   std::cout << lsfbPrms->globalJobExitRate            << ": Type of host exit rate exception handling types: GLOBAL_EXIT_RATE " << std::endl;
   std::cout << lsfbPrms->enableJobExitRatePerSlot     << ": Type of host exit rate exception handling types ENABLE_EXIT_RATE_PER_SLOT " << std::endl;
   std::cout << lsfbPrms->enableMetric                 << ": Performance metrics monitor is enabled. flag " << std::endl;
   std::cout << lsfbPrms->schMetricsSample             << ": Performance metrics monitor sample period flag " << std::endl;
   std::cout << lsfbPrms->maxApsValue                  << ": Used to bound: (1) factors, (2) weights, and (3) APS values " << std::endl;
   std::cout << lsfbPrms->newjobRefresh                << ": Child mbatchd gets updated information about new jobs from the parent mbatchd " << std::endl;
   std::cout << lsfbPrms->preemptJobType               << ": Job type to preempt, PREEMPT_JOBTYPE_BACKFILL, PREEMPT_JOBTYPE_EXCLUSIVE " << std::endl;
   std::cout << lsfbPrms->defaultJgrp                  << ": The default job group assigned to jobs " << std::endl;
   std::cout << lsfbPrms->jobRunlimitRatio             << ": Max ratio between run limit and runtime estimation " << std::endl;
   std::cout << lsfbPrms->jobIncludePostproc           << ": Enable the post-execution processing of the job to be included as part of the job flag" << std::endl;
   std::cout << lsfbPrms->jobPostprocTimeout           << ": Timeout of post-execution processing " << std::endl;
   std::cout << lsfbPrms->sschedUpdateSummaryInterval  << ": The interval, in seconds, for updating the session scheduler status summary " << std::endl;
   std::cout << lsfbPrms->sschedUpdateSummaryByTask    << ": The number of completed tasks for updating the session scheduler status summary " << std::endl;
   std::cout << lsfbPrms->sschedRequeueLimit           << ": The maximum number of times a task can be requeued via requeue exit values " << std::endl;
   std::cout << lsfbPrms->sschedRetryLimit             << ": The maximum number of times a task can be retried after a dispatch error " << std::endl;
   std::cout << lsfbPrms->sschedMaxTasks               << ": The maximum number of tasks that can be submitted in one session " << std::endl;
   std::cout << lsfbPrms->sschedMaxRuntime             << ": The maximum run time of a single task " << std::endl;
   std::cout << lsfbPrms->sschedAcctDir                << ": The output directory for task accounting files " << std::endl;
   std::cout << lsfbPrms->jgrpAutoDel                  << ": If TRUE enable the job group automatic deletion functionality (default is FALSE). " << std::endl;
   std::cout << lsfbPrms->maxJobPreempt                << ": Maximum number of job preempted times " << std::endl;
   std::cout << lsfbPrms->maxJobRequeue                << ": Maximum number of job re-queue times " << std::endl;
   std::cout << lsfbPrms->noPreemptRunTimePercent      << ": No preempt run time percent " << std::endl;
   std::cout << lsfbPrms->noPreemptFinishTimePercent   << ": No preempt finish time percent " << std::endl;
   std::cout << lsfbPrms->slotReserveQueueLimit        << ": The reservation request being within JL/U. " << std::endl;
   std::cout << lsfbPrms->maxJobPercentagePerSession   << ": Job accept limit percentage. " << std::endl;
   std::cout << lsfbPrms->useSuspSlots                 << ": The low priority job will use the slots freed by preempted jobs. " << std::endl;
   std::cout << lsfbPrms->maxStreamFileNum             << ": Maximum number of the backup stream.utc files " << std::endl;
   std::cout << lsfbPrms->privilegedUserForceBkill     << ": If enforced only admin can use bkill -r option " << std::endl;
   std::cout << lsfbPrms->mcSchedulingEnhance          << ": It controls the remote queue selection flow. " << std::endl;
   std::cout << lsfbPrms->mcUpdateInterval             << ": It controls update interval of the counters and other original data in MC implementation " << std::endl;
   std::cout << lsfbPrms->intersectCandidateHosts      << ": Jobs run on only on hosts belonging to the intersection of the queue the job was submitted to, advance reservation hosts, and any hosts specified by bsub -m at the time of submission. " << std::endl;
   std::cout << lsfbPrms->enforceOneUGLimit            << ": Enforces the limitations of a single specified user group. " << std::endl;
   std::cout << lsfbPrms->logRuntimeESTExceeded        << ": Enable or disable logging runtime estimation exceeded event " << std::endl;
   std::cout << lsfbPrms->computeUnitTypes             << ": Compute unit types." << std::endl;
   std::cout << lsfbPrms->fairAdjustFactor             << ": Fairshare adjustment weighting factor " << std::endl;
   std::cout << lsfbPrms->simEnableCpuFactor           << ": abs runtime and cputime for LSF simulator " << std::endl;
   std::cout << lsfbPrms->extendJobException           << ": switch for job exception enhancement " << std::endl;
   std::cout << lsfbPrms->preExecExitValues            << ": If the pre_exec script of a job in the cluster exits with an exit code specified in preExecExitValues, the job will be re-dispatched to a different host. " << std::endl;
   std::cout << lsfbPrms->enableRunTimeDecay           << ": Enable the decay of run time in the calculation of fairshare scheduling priority " << std::endl;
   std::cout << lsfbPrms->advResUserLimit              << ": The maximum number of advanced reservations. For each user or user group. " << std::endl;
   std::cout << lsfbPrms->noPreemptInterval            << ": Uninterrupted running time (minutes) before job can be preempted. " << std::endl;
   std::cout << lsfbPrms->maxTotalTimePreempt          << ": Maximum accumulated preemption time (minutes). " << std::endl;
   std::cout << lsfbPrms->strictUGCtrl                 << ": enable or disable strict user group control " << std::endl;
   std::cout << lsfbPrms->defaultUserGroup             << ": enable or disable job's default user group " << std::endl;
   std::cout << lsfbPrms->enforceUGTree                << ": enable or disable enforce user group tree " << std::endl;
   std::cout << lsfbPrms->preemptDelayTime             << ": The grace period before preemption " << std::endl;
   std::cout << lsfbPrms->jobPreprocTimeout            << ": Timeout of pre-execution processing " << std::endl;
   std::cout << lsfbPrms->allowEventType               << ": Log specified events into stream file " << std::endl;
   std::cout << lsfbPrms->runtimeLogInterval           << ": Interval between runtime status log " << std::endl;
   std::cout << lsfbPrms->groupPendJobsBy              << ": Group pending jobs by these key fields " << std::endl;
   std::cout << lsfbPrms->pendingReasonsExclude        << ": Don't log the defined pending reason " << std::endl;
   std::cout << lsfbPrms->pendingTimeRanking           << ": Group pending jobs by pending time " << std::endl;
   std::cout << lsfbPrms->includeDetailReasons         << ": Disable to stop log detailed pending reasons " << std::endl;
   std::cout << lsfbPrms->pendingReasonDir             << ": Full path to the pending reasons cache file directory " << std::endl;
   std::cout << lsfbPrms->forceKillRunLimit            << ": Force kill the job exceeding RUNLIMIT " << std::endl;
   std::cout << lsfbPrms->forwSlotFactor               << ": GridBroker slot factor " << std::endl;
   std::cout << lsfbPrms->gbResUpdateInterval          << ": Resource update interval to GridBroker " << std::endl;
   std::cout << lsfbPrms->depJobsEval                  << ": Number of jobs threshold of mbatchd to evaluate jobs with dependency " << std::endl;
   std::cout << lsfbPrms->rmtPendJobFactor             << ": LSF/XL remote pending factor " << std::endl;
   std::cout << lsfbPrms->numSchedMatchThreads         << ": number scheduler matching threads " << std::endl;
   std::cout << lsfbPrms->bjobsResReqDisplay           << ": control how many levels resreq bjobs can display " << std::endl;
   std::cout << lsfbPrms->jobSwitch2Event              << ": Enable/Disable 'JOB_SWITCH2' event log " << std::endl;
   std::cout << lsfbPrms->enableDiagnose               << ": Enable diagnose class types: query " << std::endl;
   std::cout << lsfbPrms->diagnoseLogdir               << ": The log directory for query diagnosis " << std::endl;
   std::cout << lsfbPrms->mcResourceMatchingCriteria   << ": The MC scheduling resource criterion " << std::endl;
   std::cout << lsfbPrms->lsbEgroupUpdateInterval      << ": Interval to dynamically update egroup managed usergroups " << std::endl;
   std::cout << lsfbPrms->isPerJobSortEnableFlg        << ": TURE if SCHED_PER_JOB_SORT=Y/y " << std::endl;
   std::cout << lsfbPrms->defaultJobCwd                << ": default job cwd " << std::endl;
   std::cout << lsfbPrms->jobCwdTtl                    << ": job cwd TTL " << std::endl;
   std::cout << lsfbPrms->ac_def_job_memsize           << ": Default memory requirement for a VM job (MB) " << std::endl;
   std::cout << lsfbPrms->ac_job_memsize_round_up_unit << ": The round-up unit of the memory size for a VM job (MB) " << std::endl;
   std::cout << lsfbPrms->ac_job_dispatch_retry_num    << ": The number of times that a Dynamic Cluster job can be retried after a dispatch failure " << std::endl;
   std::cout << lsfbPrms->ac_jobvm_restore_delay_time  << ": The job vm restore delay time " << std::endl;
   std::cout << lsfbPrms->ac_permit_alternative_resreq << ": Dynamic Cluster alternative resource requirement (logical OR between hardware-specific " << std::endl;
   std::cout << lsfbPrms->defaultJobOutdir             << ": default job outdir " << std::endl;
   std::cout << lsfbPrms->bswitchModifyRusage          << ":  bswitch modify job resource usage " << std::endl;
   std::cout << lsfbPrms->resizableJobs                << ": Enable or disable the resizable job feature " << std::endl;
   std::cout << lsfbPrms->slotBasedSla                 << ": Enable or disable slots based request for EGO-enabled SLA " << std::endl;
   std::cout << lsfbPrms->releaseMemForSuspJobs        << ": Do not reserve memory when a job is suspended " << std::endl;
   std::cout << lsfbPrms->stripWithMinimumNetwork      << ": strip with minimum network " << std::endl;
   std::cout << lsfbPrms->maxProtocolInstance          << ": maximum allowed window instances for pe job " << std::endl;
   std::cout << lsfbPrms->jobDistributeOnHost          << ": how to distribute tasks for different jobs on same host " << std::endl;
   std::cout << lsfbPrms->defaultResReqOrder           << ": batch part default order " << std::endl;
   std::cout << lsfbPrms->ac_timeout_waiting_sbd_start << ": Dynamic Cluster timeout waiting for sbatchd to start " << std::endl;
   std::cout << lsfbPrms->maxConcurrentQuery           << ": Max number of concurrent query " << std::endl;
#endif
}

}
