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

// logging
#include "LogHandler.h"

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

struct jobInfoEnt 
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
   Job( JobSchedulerLSF   * parent
      , const std::string & cwd
      , const std::string & scriptName
      , const std::string & jobName
      , int                 cpus
      , size_t              runTimeLim
      , const std::string & resReq
      , const std::string & scenarioID
      )
   {
      m_parent     = parent;
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
      m_submit.options          = SUB_PROJECT_NAME | SUB_JOB_NAME | SUB_OUT_FILE | SUB_ERR_FILE;
      m_submit.options3         = SUB3_CWD;

      const char * envVar       = getenv( "LSF_CAULDRON_PROJECT_NAME" ); 
      m_submit.projectName      = strdup( envVar ? envVar : LSF_CAULDRON_PROJECT_NAME ); // add project name (must be the same for all cauldron app)
      
#ifdef LSF_XDR_VERSION9_1_3
      envVar                    = getenv( "LSF_CAULDRON_PROJECT_QUEUE" );
      if ( envVar )
      {
         m_submit.queue         = strdup( envVar ); // add project queue
         m_submit.options      |= SUB_QUEUE;
      }
      
      m_submit.jobGroup = strdup( (std::string( "/Cauldron/casa/scenario/")  + (scenarioID.empty() ? "unknown" : scenarioID.c_str() ) ).c_str() );
      m_submit.options2        |= SUB2_JOB_GROUP;

#endif
      m_submit.command          = strdup( scriptName.c_str() );
      m_submit.jobName          = strdup( jobName.c_str() );
      m_submit.outFile          = strdup( (jobName + ".out" ).c_str() ); // redirect stdout to file
      m_submit.errFile          = strdup( (jobName + ".err" ).c_str() ); // redirect stderr to file
         
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
#ifdef LSF_XDR_VERSION9_1_3
      if ( m_submit.userGroup   ) free( m_submit.userGroup   );
      if ( m_submit.queue       ) free( m_submit.queue       );
      if ( m_submit.sla         ) free( m_submit.sla         );
#endif
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
#ifdef LSF_XDR_VERSION9_1_3
      if ( !m_parent->m_prjGrp.empty() && !m_submit.userGroup )
      {
         LogHandler( LogHandler::DEBUG_SEVERITY ) << "Copying project group: " << m_parent->m_prjGrp << " to the job parameters";
         m_submit.userGroup     = strdup( m_parent->m_prjGrp.c_str() ); // add project group
         m_submit.options2     |= SUB2_JOB_GROUP;
      }

      if ( !m_parent->m_sla.empty() && !m_submit.sla )
      {
         LogHandler( LogHandler::DEBUG_SEVERITY ) << "Copying SLA: " << m_parent->m_sla << " to the job parameters";
         m_submit.sla           = strdup( m_parent->m_sla.c_str() );
         m_submit.options2     |= SUB2_SLA; 
      }
#endif
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
      if ( lsberrno == LSBE_LSBLIB && !std::string( "External authentication failed" ).compare( lsb_sysmsg() ) ) { return true; }
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

      memcpy( &m_jobInfo, jobInfo, sizeof( m_jobInfo ) );  // save the status of the job for diagnostic
      
#ifdef LSF_XDR_VERSION9_1_3
      // save sla and projname
      if ( m_parent->m_sla.empty()    && jobInfo->submit.sla       ) { m_parent->m_sla    = jobInfo->submit.sla;       }
      if ( m_parent->m_prjGrp.empty() && jobInfo->submit.userGroup ) { m_parent->m_prjGrp = jobInfo->submit.userGroup; }
#endif
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

   // print the status of the job for diagnostic
   void printJobInfo();

   // Serialization / Deserialization
   // version of serialized object representation
   virtual unsigned int version() const { return 4; }

   // Get type name of the serialaizable object, used in deserialization to create object with correct type
   virtual const char * typeName() const { return "JobSchedulerLSF::Job"; }

   // Serialize object to the given stream
   virtual bool save( CasaSerializer & sz, unsigned int /* version */ ) const
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
      memset( &m_submit,     0, sizeof( m_submit     ) );
      memset( &m_submitRepl, 0, sizeof( m_submitRepl ) );
      memset( &m_jobInfo,    0, sizeof( m_jobInfo    ) );

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
   JobSchedulerLSF  * m_parent;
   bool               m_isFinished; // was this job finished?
   bool               m_isFailed;   // was this job failed?

   // fields related to interaction with LSF
   struct submit      m_submit;     // lsb_submit use values from this structure to submit job
   struct submitReply m_submitRepl; // lsb_submit returns here some info in case of error
   struct jobInfoEnt  m_jobInfo;    // lsb_readjobinfo returns info about the job, here we will keep all infor

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
      setenv( "LSF_ENVDIR", LSF_CONFIG_DIR, 0 ); // LSF_ENVDIR is needed to make LSF API calls
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

   printLSFBParametersInfo();

   const char * envVar  = getenv( "LSF_CAULDRON_PROJECT_GROUP" );
   if ( envVar ) { m_prjGrp = envVar; }

   envVar               = getenv( "LSF_CAULDRON_PROJECT_SERVICE_CLASS_NAME" );    // add project class service ??
   if ( envVar ) { m_sla = envVar; }
}

JobSchedulerLSF::~JobSchedulerLSF() { for ( size_t i = 0; i < m_jobs.size(); ++i ) { delete m_jobs[i]; } } // clean array of scheduled jobs

// Add job to the list
JobScheduler::JobID JobSchedulerLSF::addJob( const std::string & cwd
                                           , const std::string & scriptName
                                           , const std::string & jobName
                                           , int                 cpus
                                           , size_t              runTimeLim
                                           , const std::string & scenarioID
                                           )
{
   ibs::FilePath scriptStatFile( scriptName + ".failed" );
   if ( scriptStatFile.exists() ) scriptStatFile.remove();

   m_jobs.push_back( new Job( this, cwd, scriptName, jobName, cpus, runTimeLim, m_resReqStr, scenarioID ) );
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
         m_jobs[job]->printJobInfo();
      }
   }
   // log job ID
   std::ofstream ofs( RunManager::s_jobsIDListFileName, std::ios_base::out | std::ios_base::app );
   if ( ofs.is_open() ) { ofs << schedulerJobID( job ) << "\n"; ofs.close(); }
 
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

void JobSchedulerLSF::sleep( int secs )
{
   if (      secs < 0 ) { Wait( 10 );   }
   else if ( secs > 0 ) { Wait( secs ); }
}

// Serialize object to the given stream
bool JobSchedulerLSF::save( CasaSerializer & sz, unsigned int /* fileVersion */ ) const
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


void JobSchedulerLSF::Job::printJobInfo()
{

   // Some fields of job info structure are not printed now but may be it will be useful to print them in future
#ifdef WITH_LSF_SCHEDULER
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "LSF job status: " 
   << m_jobInfo.jobId      << " - The job ID that the LSF system assigned to the job.\n"
   << ( m_jobInfo.user ? m_jobInfo.user : "NotSet" ) << " - The name of the user who submitted the job.\n"
   << m_jobInfo.status     << " - The current status of the job.Possible values are shown in job_states.\n"

   //<< reasonTb        << " - Pending or suspending reasons of the job\n"
   //<< numReasons      << " - Length of reasonTb[]\n"

   << m_jobInfo.reasons    << " - The reason a job is pending or suspended.\n"
   << m_jobInfo.subreasons << " - The reason a job is pending or suspended.\n"
   << m_jobInfo.jobPid     << " - The job process ID.\n"
   <<  "The time the job was submitted: " << std::string( ctime( &m_jobInfo.submitTime ) )

   // time_t  reserveTime;   /**< Time when job slots are reserved */
   // time_t  startTime;     /**< The time that the job started running, if it has been dispatched.*/
   // time_t  predictedStartTime;   /**< Job's predicted start time */
   // time_t  endTime;              /**< The termination time of the job, if it has completed.*/
   // time_t  lastEvent;     /**< Last time event */
   // time_t  nextEvent;     /**< Next time event */
   // int     duration;      /**< Duration time (minutes) */

   << m_jobInfo.cpuTime << " - CPU time consumed by the job\n"
   << m_jobInfo.umask   << " - The file creation mask when the job was submitted.\n"
   << ( m_jobInfo.cwd        ? m_jobInfo.cwd        : "NotSet" ) << " - The current working directory when the job was submitted.\n"
   << ( m_jobInfo.subHomeDir ? m_jobInfo.subHomeDir : "NotSet" ) << " - Home directory on submission host.\n"
   << ( m_jobInfo.fromHost   ? m_jobInfo.fromHost   : "NotSet" ) << " - The name of the host from which the job was submitted.\n"

#if 0
   char    **exHosts;     /**< The array of names of hosts on which the job
                        * executes. */
int     numExHosts;    /**< The number of hosts on which the job executes. */
float   cpuFactor;     /**< The CPU factor for normalizing CPU and wall clock
                        * time limits.*/
int     nIdx;          /**< The number of load indices in the loadSched and
                        * loadStop arrays.*/
float   *loadSched;    /**< The values in the loadSched array specify
                        * the thresholds for the corresponding load indices.
                        * Only if the current values of all specified load
                        * indices of a host are within (below or above, 
                        * depending on the meaning of the load index) their
                        * corresponding thresholds may the suspended job be
                        * resumed on this host. 
                        *
                        * For an explanation of the entries in the loadSched,
                        * see \ref lsb_hostinfo.
                        */
float   *loadStop;     /**< The values in the loadStop array specify the
                        * thresholds for job suspension; if any of the current
                        * load index values of the host crosses its threshold,
                        * the job will be suspended. 
                        *
                        * For an explanation of the entries in the loadStop,
                        * see \ref lsb_hostinfo.
                        */
#endif

   << ( m_jobInfo.submit.sla      ? m_jobInfo.submit.sla      : "NotSet" ) << " - SLA under which the job runs.\n"
   << ( m_jobInfo.submit.jobGroup ? m_jobInfo.submit.jobGroup : "NotSet" ) << " - Job group under which the job runs.\n"
   << m_jobInfo.exitStatus << " - Job exit status.\n"
 
#if 0
int     execUid;        /**< Mapped UNIX user ID on the execution host.*/
char    *execHome;      /**< Home directory for the job on the execution host.*/
char    *execCwd;       /**< Current working directory for the job on the
                         * execution host.*/
char    *execUsername;  /**< Mapped user name on the execution host.*/
time_t  jRusageUpdateTime; /**< Time of the last job resource usage update.*/
struct  jRusage runRusage; /**< Contains resource usage information for the job.*/
int     jType;          /**< Job type.N_JOB, N_GROUP, N_HEAD */
char    *parentGroup;   /**< The parent job group of a job or job group. */
char    *jName;         /**< If jType is JGRP_NODE_GROUP, then it is the job
                         * group name. Otherwise, it is the
                         *job name. */
int     counter[NUM_JGRP_COUNTERS];  /**< Index into the counter array, only
                                      * used for job arrays. Possible index values are
                                      * shown in \ref jobgroup_counterIndex*/
u_short port;           /**< Service port of the job. */
int     jobPriority;    /**< Job dynamic priority */
int numExternalMsg;     /**< The number of external messages in the job. */
struct jobExternalMsgReply **externalMsg; /**< This structure contains the
                                           * information required to define
                                           * an external message reply.*/
int     clusterId;      /**< MultiCluster cluster ID. If clusterId is greater
                         * than or equal to 0, the job is a pending remote job,
                         * and \ref lsb_readjobinfo checks for host_name\@cluster_name.
                         * If host name is needed, it should be found in 
                         * jInfoH->remoteHosts. If the remote host name is not
                         * available, the constant string remoteHost is used.*/
char   *detailReason;   /**<  Detail reason field */
float   idleFactor;     /**< Idle factor for job exception handling. If the job
                         * idle factor is less than the specified threshold, LSF
                         * invokes LSF_SERVERDIR/eadmin to trigger the action for
                         * a job idle exception.*/
int     exceptMask;     /**< Job exception handling mask */

char   *additionalInfo; /**< Placement information of LSF jobs. Arbitrary information of
                         * a job stored as a string */
int     exitInfo;       /**< Job termination reason. See lsbatch.h.*/
int    warningTimePeriod; /**< Job warning time period in seconds; -1 if unspecified. */
char   *warningAction;  /**< Warning action, SIGNAL | CHKPNT |
                         *command, NULL if unspecified */
char   *chargedSAAP;    /**< SAAP charged for job */
char   *execRusage;     /**< The rusage satisfied at job runtime */
time_t rsvInActive;     /**< The time when advance reservation expired or was deleted. */
int    numLicense;      /**< The number of licenses reported from License Scheduler. */
char   **licenseNames;  /**< License Scheduler license names.*/
float  aps;             /**< Absolute priority scheduling (APS) priority value.*/
float  adminAps;        /**< Absolute priority scheduling (APS) string set by
                         * administrators to denote static system APS value */
int    runTime;         /**< The real runtime on the execution host. */
int reserveCnt;         /**< How many kinds of resource are reserved by this job*/
struct reserveItem *items; /**< Detail reservation information for each
                            * kind of resource*/
float  adminFactorVal;  /**< Absolute priority scheduling (APS) string set by
                         * administrators to denote ADMIN
                         * factor APS value. */
int    resizeMin;       /**< Pending resize min. 0, if no resize pending. */	
int    resizeMax;       /**< Pending resize max. 0, if no resize pending */	
time_t resizeReqTime;   /**< Time when pending request was issued */
int    jStartNumExHosts; /**< Number of hosts when job starts */
char   **jStartExHosts;  /**< Host list when job starts */
int    jStartNumExHosts4Slots; /**< Number of hosts when job starts */
char   **jStartExHosts4Slots;  /**< Host list when job starts */
time_t lastResizeTime;   /**< Last time when job allocation changed */
int    numhRusages;         /**< The number of host-based rusage entries in the list hostRusage */
struct hRusage * hostRusage; /**< Host based rusage list, one per host */
int maxMem;  /**< job maximum memory usage*/
int avgMem;  /**< job average memory usage */
time_t fwdTime;             /**< Time when job forwarded */
char*  srcCluster;         /**< Cluster name which job accepted */
LS_LONG_INT srcJobId;        /**< The job Id assigned by the cluster which job accepted */
char*  dstCluster;          /**< Cluster name which job forwarded */
LS_LONG_INT dstJobId;        /**< The job Id assigned by the cluster which job forwarded */

time_t brunJobTime;          /**< Time when job is being brun */
char*  brunJobUser;          /**< who issue brun */
char*  appResReq;            /**< The resreq for the application to which the job was submitted. */
char*  qResReq;              /**< The resreq for the queue to which the job was submitted. */
char*  combinedResReq;       /**< The result of mbd merging job/app/queue level resreqs for each job. */
char*  effectiveResReq;      /**< Resource requirements used by Scheduler to dispatch jobs. */
struct acInfo  *acinfo; /**< Dynamic Cluster information */
char*  outdir;          /**< The output directory */
int    isInProvisioning;     /**< Is the job in provisioning except live migration*/
int    acJobWaitTime;        /**< Dynamic Cluster VM job wait time, include provision time */
int    totalProvisionTime;   /**< Dynamic Cluster VM job total provision time */
char*  subcwd;               /**< The submission directory */
int    num_network;     /**< number of PE network allocation */
struct networkAlloc *networkAlloc; /**< PE network allocation */
int    numhostAffinity;      /**< number of hostAffinity */ 
affinityHostInfo_t *hostAffinity; /**< affinity allocation on each host */
double serial_job_energy;        /**< Serial job energy data */
char*  localClusterName;     /**< RFC#4069*/
int    aclMask;    /**< Flag to indicate some special field when security job info level enabled */
int    combinedCpuFrequency;    /**< Combined CPU Frequency */
int    predictedCpuFrequency;   /**< Predicted CPU Frequency */
char*  energyPolicy;            /**< Enery policy name */
char*  energyPolicyTag;         /**< Energy policy tag name */
char*  allocHostfilePath;       /**< user specified allocation hostfile path */
allocHostInfo_t *allocHostsList; /**< user specified allocation hosts */
double cpi;                     /**< Cycles Per Instruction */
double gips;                    /**< Giga Instructions Per Second */ 
double gbs;                     /**< Gigabytes per Second */
double gflops;                  /**< Giga FLoating-point Operations Per Second */
int       numToHosts4Slots;   /**< number of allocated slots */
char      **toHosts4Slots;    /**< host name of allocated slots */
int    nStinFile;           /**< Number of files requested for stage in */
struct stinfile *stinFile;  /**< Array of files requested for stage in */

#endif
   << ( m_jobInfo.dataGrp ?  m_jobInfo.dataGrp : "NotSet" ) << " - Data group name to be used when cache permissions is set to group\n";
#endif
}



void JobSchedulerLSF::printLSFBParametersInfo()
{
#ifdef WITH_LSF_SCHEDULER
   int n;
   struct serviceClass * sc = lsb_serviceClassInfo( &n );
   if ( sc )
   {
      for ( int i = 0; i < n; ++i )
      {
         LogHandler( LogHandler::DEBUG_SEVERITY ) << "Service class: " << i << ",  name: " << sc[i].name << ", user Groups: " << sc[i].userGroups;
      }
   }

   struct parameterInfo * lsfbPrms = lsb_parameterinfo( NULL, NULL, 0 );
   if ( !lsfbPrms ) { return throw ErrorHandler::Exception( ErrorHandler::LSFLibError ) << ls_sysmsg(); }

   LogHandler( LogHandler::DEBUG_SEVERITY ) << lsfbPrms->defaultQueues                << ": DEFAULT_QUEUE: A blank_separated list of queue names for automatic queue selection.\n"
      << lsfbPrms->defaultHostSpec              << ": DEFAULT_HOST_SPEC: The host name or host model name used as the system default for scaling CPULIMIT and RUNLIMIT.\n"
      << lsfbPrms->mbatchdInterval              << ": MBD_SLEEP_TIME: The interval in seconds at which the mbatchd dispatches jobs.\n"
      << lsfbPrms->sbatchdInterval              << ": SBD_SLEEP_TIME: The interval in seconds at which the sbatchd suspends or resumes jobs.\n"
      << lsfbPrms->jobAcceptInterval            << ": JOB_ACCEPT_INTERVAL: The interval at which. a host accepts two successive jobs. (In units of SBD_SLEEP_TIME.)\n"
      << lsfbPrms->maxDispRetries               << ": MAX_RETRY: The maximum number of retries for dispatching a job.\n"
      << lsfbPrms->maxSbdRetries                << ": MAX_SBD_FAIL: The maximum number of retries for reaching an sbatchd.\n"
      << lsfbPrms->preemptPeriod                << ": PREEM_PERIOD: The interval in seconds for preempting jobs running on the same host.\n"
      << lsfbPrms->cleanPeriod                  << ": CLEAN_PERIOD: The interval in seconds during which finished jobs are kept in core.\n"
      << lsfbPrms->maxNumJobs                   << ": MAX_JOB_NUM: The maximum number of finished jobs that are logged in the current event file.\n"
      << lsfbPrms->historyHours                 << ": HIST_HOURS: The number of hours of resource consumption history used for fair share scheduling and scheduling within a host partition.\n"
      << lsfbPrms->pgSuspendIt                  << ": PG_SUSP_IT: The interval a host must be idle before resuming a job suspended for excessive paging.\n"
      << lsfbPrms->defaultProject               << ": The default project assigned to jobs.\n"
      << lsfbPrms->retryIntvl                   << ": Job submission retry interval\n"
      << lsfbPrms->nqsQueuesFlags               << ": For Cray NQS compatiblilty only. Used by LSF to get the NQS queue information\n"
      << lsfbPrms->nqsRequestsFlags             << ": nqsRequestsFlags\n"
      << lsfbPrms->maxPreExecRetry              << ": The maximum number of times to attempt the preexecution command of a job from a remote cluster (MultiCluster only)\n"
      << lsfbPrms->localMaxPreExecRetry         << ": Maximum number of pre-exec retry times for local cluste\n"
      << lsfbPrms->eventWatchTime               << ": Event watching Interval in second\n"
      << lsfbPrms->runTimeFactor                << ": Run time weighting factor for fairshare scheduling\n"
      << lsfbPrms->waitTimeFactor               << ": Used for calcultion of the fairshare scheduling formula\n"
      << lsfbPrms->runJobFactor                 << ": Job slots weighting factor for fairshare scheduling\n"
      << lsfbPrms->eEventCheckIntvl             << ": Default check interval\n"
      << lsfbPrms->rusageUpdateRate             << ": sbatchd report every sbd_sleep_time\n"
      << lsfbPrms->rusageUpdatePercent          << ": sbatchd updates jobs jRusage in mbatchd if more than 10% changes\n"
      << lsfbPrms->condCheckTime                << ": Time period to check for reconfig\n"
      << lsfbPrms->maxSbdConnections            << ": The maximum number of connections between master and slave batch daemons\n"
      << lsfbPrms->rschedInterval               << ": The interval for rescheduling jobs\n"
      << lsfbPrms->maxSchedStay                 << ": Max time mbatchd stays in scheduling routine, after which take a breather\n"
      << lsfbPrms->freshPeriod                  << ": During which load remains fresh\n"
      << lsfbPrms->preemptFor                   << ": The preemption behavior, GROUP_MAX, GROUP_JLP, USER_JLP, HOST_JLU,MINI_JOB, LEAST_RUN_TIME\n"
      << lsfbPrms->adminSuspend                 << ": Flags whether users can resume their jobs when suspended by the LSF administrator\n"
      << lsfbPrms->userReservation              << ": Flags to enable/disable normal user to create advance reservation\n"
      << lsfbPrms->cpuTimeFactor                << ": CPU time weighting factor for fairshare scheduling\n"
      << lsfbPrms->fyStart                      << ": The starting month for a fiscal year\n"
      << lsfbPrms->maxJobArraySize              << ": The maximum number of jobs in a job array\n"
      << lsfbPrms->exceptReplayPeriod           << ": Replay period for exceptions, in seconds\n"
      << lsfbPrms->jobTerminateInterval         << ": The interval to terminate a job\n"
      << lsfbPrms->disableUAcctMap              << ": User level account mapping for remote jobs is disabled\n"
      << lsfbPrms->enforceFSProj                << ": If set to TRUE, Project name for a job will be considerred when doing fairshare scheduling, i.e., as if user has submitted jobs using -G\n"
      << lsfbPrms->enforceProjCheck             << ": Enforces the check to see if the invoker of bsub is in the specifed group when the -P option is used\n"
      << lsfbPrms->jobRunTimes                  << ": Run time for a job\n"
      << lsfbPrms->dbDefaultIntval              << ": Event table Job default interval\n"
      << lsfbPrms->dbHjobCountIntval            << ": Event table Job Host Count\n"
      << lsfbPrms->dbQjobCountIntval            << ": Event table Job Queue Count\n"
      << lsfbPrms->dbUjobCountIntval            << ": Event table Job User Count\n"
      << lsfbPrms->dbJobResUsageIntval          << ": Event table Job Resource Interval\n"
      << lsfbPrms->dbLoadIntval                 << ": Event table Resource Load Interval\n"
      << lsfbPrms->dbJobInfoIntval              << ": Event table Job Info\n"
      << lsfbPrms->jobDepLastSub                << ": Used with job dependency schedulin\n"
//      std::cout << lsfbPrms->dbSelectLoad                 << ": Select resources to be logged\n"
      << lsfbPrms->jobSynJgrp                   << ": Job synchronizes its group status\n"
      << lsfbPrms->pjobSpoolDir                 << ": The batch jobs' temporary output directory\n"
      << lsfbPrms->maxUserPriority              << ": Maximal job priority defined for all users\n"
      << lsfbPrms->jobPriorityValue             << ": Job priority is increased by the system dynamically based on waiting time\n"
      << lsfbPrms->jobPriorityTime              << ": Waiting time to increase Job priority by the system dynamically\n"
      << lsfbPrms->enableAutoAdjust             << ": Enable internal statistical adjustment\n"
      << lsfbPrms->autoAdjustAtNumPend          << ": Start to autoadjust when the user has this number of pending jobs\n"
      << lsfbPrms->autoAdjustAtPercent          << ": If this number of jobs has been visited skip the user\n"
      << lsfbPrms->sharedResourceUpdFactor      << ":  Static shared resource update interval for the cluster actor\n"
      << lsfbPrms->scheRawLoad                  << ": Schedule job based on raw load info\n"
      << lsfbPrms->jobAttaDir                   << ":  The batch jobs' external storage for attached data\n"
      << lsfbPrms->maxJobMsgNum                 << ": Maximum message number for each job\n"
      << lsfbPrms->maxJobAttaSize               << ": Maximum attached data size to be transferred for each message\n"
      << lsfbPrms->mbdRefreshTime               << ": The life time of a child MBD to serve queries in the MT way\n"
      << lsfbPrms->updJobRusageInterval         << ": The interval of the execution cluster updating the job's resource usage\n"
      << lsfbPrms->sysMapAcct                   << ": The account to which all windows workgroup users are to be mappe\n"
      << lsfbPrms->preExecDelay                 << ": Dispatch delay internal\n"
      << lsfbPrms->updEventUpdateInterval       << ": Update duplicate event interval \n"
      << lsfbPrms->resourceReservePerSlot       << ": Resources are reserved for parallel jobs on a per-slot basis\n"
      << lsfbPrms->maxJobId                     << ": Maximum job id --- read from the lsb.params\n"
      << lsfbPrms->preemptResourceList          << ": Define a list of preemptable resource. names\n"
      << lsfbPrms->preemptionWaitTime           << ": The preemption wait time\n"
      << lsfbPrms->maxAcctArchiveNum            << ": Maximum number of rollover lsb.acct files kept by mbatchd.\n"
      << lsfbPrms->acctArchiveInDays            << ": mbatchd Archive Interval\n"
      << lsfbPrms->acctArchiveInSize            << ": mbatchd Archive threshold\n"
      << lsfbPrms->committedRunTimeFactor       << ": Committed run time weighting factor\n"
      << lsfbPrms->enableHistRunTime            << ": Enable the use of historical run time in the calculation of fairshare scheduling priority, Disable the use of historical run time in the calculation of fairshare scheduling priority\n"
      << lsfbPrms->mcbOlmReclaimTimeDelay       << ": Open lease reclaim time\n"
      << lsfbPrms->chunkJobDuration             << ": Enable chunk job dispatch for jobs with CPU limit or run limits\n"
      << lsfbPrms->sessionInterval              << ": The interval for scheduling jobs by scheduler daemon\n"
      << lsfbPrms->publishReasonJobNum          << ": The number of jobs per user per queue whose pending reason is published at the PEND_REASON_UPDATE_INTERVAL interval\n"
      << lsfbPrms->publishReasonInterval        << ": The interval for publishing job pending reason by scheduler daemon\n"
      << lsfbPrms->publishReason4AllJobInterval << ": Interval(in seconds) of pending reason. publish for all jobs\n"
      << lsfbPrms->mcUpdPendingReasonInterval   << ": MC pending reason update interval (0 means no updates)\n"
      << lsfbPrms->mcUpdPendingReasonPkgSize    << ": MC pending reason update package size (0 means no limit)\n"
      << lsfbPrms->noPreemptRunTime             << ": No preemption if the run time is greater. than the value defined in here.\n"
      << lsfbPrms->noPreemptFinishTime          << ": No preemption if the finish time is less than the value defined in here.\n"
      << lsfbPrms->acctArchiveAt                << ": mbatchd Archive Time\n"
      << lsfbPrms->absoluteRunLimit             << ": Absolute run limit for job\n"
      << lsfbPrms->lsbExitRateDuration          << ": The job exit rate duratio\n"
      << lsfbPrms->lsbTriggerDuration           << ":  The duration to trigger eadmin\n"
      << lsfbPrms->maxJobinfoQueryPeriod        << ": Maximum time for job information query commands (for example,with bjobs) to wait\n"
      << lsfbPrms->jobSubRetryInterval          << ": Job submission retrial interval for client\n"
      << lsfbPrms->pendingJobThreshold          << ": System wide max pending jobs\n"
      << lsfbPrms->maxConcurrentJobQuery        << ": Max number of concurrent job query\n"
      << lsfbPrms->minSwitchPeriod              << ": Min event switch time period\n"
      << lsfbPrms->condensePendingReasons       << ": Condense pending reasons enabled\n"
      << lsfbPrms->slotBasedParallelSched       << ": Schedule Parallel jobs based on slots instead of CPUs\n"
      << lsfbPrms->disableUserJobMovement       << ": Disable user job movement operations, like btop/bbot.\n"
      << lsfbPrms->detectIdleJobAfter           << ": Detect and report idle jobs only after specified minutes.\n"
      << lsfbPrms->useSymbolPriority            << ": Use symbolic when specifing priority of symphony jobs\n"
      << lsfbPrms->JobPriorityRound             << ": Priority rounding for symphony jobs\n"
      //std::cout << lsfbPrms->priorityMapping              << ": The mapping of the symbolic priority.for symphony jobs\n"
      << lsfbPrms->maxInfoDirs                  << ": Maximum number of subdirectories under LSB_SHAREDIR/cluster/logdir/info\n"
      << lsfbPrms->minMbdRefreshTime            << ": The minimum period of a child MBD to serve queries in the MT way\n"
      << lsfbPrms->enableStopAskingLicenses2LS  << ": Stop asking license to LS not due to lack license\n"
      << lsfbPrms->expiredTime                  << ": Expire time for finished job which will not taken into account when calculating queue fairshare priority\n"
      << lsfbPrms->mbdQueryCPUs                 << ": MBD child query processes will only run on the following CPUs\n"
      << lsfbPrms->defaultApp                   << ": The default application profile assigned to jobs\n"
      << lsfbPrms->enableStream                 << ": Enable or disable data streaming\n"
      << lsfbPrms->streamFile                   << ": File to which lsbatch data is streamed\n"
      << lsfbPrms->streamSize                   << ": File size in MB to which lsbatch data is streamed\n"
      << lsfbPrms->syncUpHostStatusWithLIM      << ": Sync up host status with master LIM is enabled\n"
      << lsfbPrms->defaultSLA                   << ": Project schedulign default SLA\n"
      << lsfbPrms->slaTimer                     << ": EGO Enabled SLA scheduling timer period\n"
      << lsfbPrms->mbdEgoTtl                    << ": EGO Enabled SLA scheduling time to live\n"
      << lsfbPrms->mbdEgoConnTimeout            << ": EGO Enabled SLA scheduling connection timeout\n"
      << lsfbPrms->mbdEgoReadTimeout            << ": EGO Enabled SLA scheduling read timeout\n"
      << lsfbPrms->mbdUseEgoMXJ                 << ": EGO Enabled SLA scheduling use MXJ flag\n"
      << lsfbPrms->mbdEgoReclaimByQueue         << ": EGO Enabled SLA scheduling reclaim by queue\n"
      << lsfbPrms->defaultSLAvelocity           << ": EGO Enabled SLA scheduling default velocit\n"
      << lsfbPrms->exitRateTypes                << ": Type of host exit rate exception handling types: EXIT_RATE_TYPE\n"
      << lsfbPrms->globalJobExitRate            << ": Type of host exit rate exception handling types: GLOBAL_EXIT_RATE\n"
      << lsfbPrms->enableJobExitRatePerSlot     << ": Type of host exit rate exception handling types ENABLE_EXIT_RATE_PER_SLOT\n"
      << lsfbPrms->enableMetric                 << ": Performance metrics monitor is enabled. flag\n"
      << lsfbPrms->schMetricsSample             << ": Performance metrics monitor sample period flag\n"
      << lsfbPrms->maxApsValue                  << ": Used to bound: (1) factors, (2) weights, and (3) APS values\n"
      << lsfbPrms->newjobRefresh                << ": Child mbatchd gets updated information about new jobs from the parent mbatchd\n"
      << lsfbPrms->preemptJobType               << ": Job type to preempt, PREEMPT_JOBTYPE_BACKFILL, PREEMPT_JOBTYPE_EXCLUSIVE\n"
      << lsfbPrms->defaultJgrp                  << ": The default job group assigned to jobs\n"
      << lsfbPrms->jobRunlimitRatio             << ": Max ratio between run limit and runtime estimation\n"
      << lsfbPrms->jobIncludePostproc           << ": Enable the post-execution processing of the job to be included as part of the job fla\n"
      << lsfbPrms->jobPostprocTimeout           << ": Timeout of post-execution processing\n"
      << lsfbPrms->sschedUpdateSummaryInterval  << ": The interval, in seconds, for updating the session scheduler status summary\n"
      << lsfbPrms->sschedUpdateSummaryByTask    << ": The number of completed tasks for updating the session scheduler status summary\n"
      << lsfbPrms->sschedRequeueLimit           << ": The maximum number of times a task can be requeued via requeue exit values\n"
      << lsfbPrms->sschedRetryLimit             << ": The maximum number of times a task can be retried after a dispatch error\n"
      << lsfbPrms->sschedMaxTasks               << ": The maximum number of tasks that can be submitted in one session\n"
      << lsfbPrms->sschedMaxRuntime             << ": The maximum run time of a single task\n"
      << lsfbPrms->sschedAcctDir                << ": The output directory for task accounting files\n"
      << lsfbPrms->jgrpAutoDel                  << ": If TRUE enable the job group automatic deletion functionality (default is FALSE).\n"
      << lsfbPrms->maxJobPreempt                << ": Maximum number of job preempted times\n"
      << lsfbPrms->maxJobRequeue                << ": Maximum number of job re-queue times\n"
      << lsfbPrms->noPreemptRunTimePercent      << ": No preempt run time percent\n"
      << lsfbPrms->noPreemptFinishTimePercent   << ": No preempt finish time percent\n"
      << lsfbPrms->slotReserveQueueLimit        << ": The reservation request being within JL/U.\n"
      << lsfbPrms->maxJobPercentagePerSession   << ": Job accept limit percentage.\n"
      << lsfbPrms->useSuspSlots                 << ": The low priority job will use the slots freed by preempted jobs.\n"
      << lsfbPrms->maxStreamFileNum             << ": Maximum number of the backup stream.utc files\n"
      << lsfbPrms->privilegedUserForceBkill     << ": If enforced only admin can use bkill -r option\n"
      << lsfbPrms->mcSchedulingEnhance          << ": It controls the remote queue selection flow.\n"
      << lsfbPrms->mcUpdateInterval             << ": It controls update interval of the counters and other original data in MC implementation\n"
      << lsfbPrms->intersectCandidateHosts      << ": Jobs run on only on hosts belonging to the intersection of the queue the job was submitted to, advance reservation hosts, and any hosts specified by bsub -m at the time of submission.\n"
      << lsfbPrms->enforceOneUGLimit            << ": Enforces the limitations of a single specified user group.\n"
      << lsfbPrms->logRuntimeESTExceeded        << ": Enable or disable logging runtime estimation exceeded event\n"
      << lsfbPrms->computeUnitTypes             << ": Compute unit types\n"
      << lsfbPrms->fairAdjustFactor             << ": Fairshare adjustment weighting factor\n"
      << lsfbPrms->simEnableCpuFactor           << ": abs runtime and cputime for LSF simulator\n"
      << lsfbPrms->extendJobException           << ": switch for job exception enhancement\n"
      << lsfbPrms->preExecExitValues            << ": If the pre_exec script of a job in the cluster exits with an exit code specified in preExecExitValues, the job will be re-dispatched to a different host.\n"
      << lsfbPrms->enableRunTimeDecay           << ": Enable the decay of run time in the calculation of fairshare scheduling priority\n"
      << lsfbPrms->advResUserLimit              << ": The maximum number of advanced reservations. For each user or user group.\n"
      << lsfbPrms->noPreemptInterval            << ": Uninterrupted running time (minutes) before job can be preempted.\n"
      << lsfbPrms->maxTotalTimePreempt          << ": Maximum accumulated preemption time (minutes).\n"
      << lsfbPrms->strictUGCtrl                 << ": enable or disable strict user group control\n"
      << lsfbPrms->defaultUserGroup             << ": enable or disable job's default user group\n"
      << lsfbPrms->enforceUGTree                << ": enable or disable enforce user group tree\n"
      << lsfbPrms->preemptDelayTime             << ": The grace period before preemption\n"
      << lsfbPrms->jobPreprocTimeout            << ": Timeout of pre-execution processing\n"
      << lsfbPrms->allowEventType               << ": Log specified events into stream file\n"
      << lsfbPrms->runtimeLogInterval           << ": Interval between runtime status log\n"
      << lsfbPrms->groupPendJobsBy              << ": Group pending jobs by these key fields\n"
      << lsfbPrms->pendingReasonsExclude        << ": Don't log the defined pending reason\n"
      << lsfbPrms->pendingTimeRanking           << ": Group pending jobs by pending time\n"
      << lsfbPrms->includeDetailReasons         << ": Disable to stop log detailed pending reasons\n"
      << lsfbPrms->pendingReasonDir             << ": Full path to the pending reasons cache file directory\n"
      << lsfbPrms->forceKillRunLimit            << ": Force kill the job exceeding RUNLIMIT\n"
      << lsfbPrms->forwSlotFactor               << ": GridBroker slot factor\n"
      << lsfbPrms->gbResUpdateInterval          << ": Resource update interval to GridBroker\n"
      << lsfbPrms->depJobsEval                  << ": Number of jobs threshold of mbatchd to evaluate jobs with dependency\n"
      << lsfbPrms->rmtPendJobFactor             << ": LSF/XL remote pending factor\n"
      << lsfbPrms->numSchedMatchThreads         << ": number scheduler matching threads\n"
      << lsfbPrms->bjobsResReqDisplay           << ": control how many levels resreq bjobs can display\n"
      << lsfbPrms->jobSwitch2Event              << ": Enable/Disable 'JOB_SWITCH2' event log\n"
      << lsfbPrms->enableDiagnose               << ": Enable diagnose class types: query\n"
      << lsfbPrms->diagnoseLogdir               << ": The log directory for query diagnosis\n"
      << lsfbPrms->mcResourceMatchingCriteria   << ": The MC scheduling resource criterion\n"
      << lsfbPrms->lsbEgroupUpdateInterval      << ": Interval to dynamically update egroup managed usergroups\n"
      << lsfbPrms->isPerJobSortEnableFlg        << ": TURE if SCHED_PER_JOB_SORT=Y/y\n"
      << lsfbPrms->defaultJobCwd                << ": default job cwd\n"
      << lsfbPrms->jobCwdTtl                    << ": job cwd TTL\n"
      << lsfbPrms->ac_def_job_memsize           << ": Default memory requirement for a VM job (MB)\n"
      << lsfbPrms->ac_job_memsize_round_up_unit << ": The round-up unit of the memory size for a VM job (MB)\n"
      << lsfbPrms->ac_job_dispatch_retry_num    << ": The number of times that a Dynamic Cluster job can be retried after a dispatch failure\n"
      << lsfbPrms->ac_jobvm_restore_delay_time  << ": The job vm restore delay time\n"
      << lsfbPrms->ac_permit_alternative_resreq << ": Dynamic Cluster alternative resource requirement (logical OR between hardware-specific\n"
      << lsfbPrms->defaultJobOutdir             << ": default job outdir\n"
      << lsfbPrms->bswitchModifyRusage          << ":  bswitch modify job resource usage\n"
      << lsfbPrms->resizableJobs                << ": Enable or disable the resizable job feature\n"
      << lsfbPrms->slotBasedSla                 << ": Enable or disable slots based request for EGO-enabled SLA\n"
      << lsfbPrms->releaseMemForSuspJobs        << ": Do not reserve memory when a job is suspended\n"
      << lsfbPrms->stripWithMinimumNetwork      << ": strip with minimum network\n"
      << lsfbPrms->maxProtocolInstance          << ": maximum allowed window instances for pe job\n"
      << lsfbPrms->jobDistributeOnHost          << ": how to distribute tasks for different jobs on same host\n"
      << lsfbPrms->defaultResReqOrder           << ": batch part default order\n"
      << lsfbPrms->ac_timeout_waiting_sbd_start << ": Dynamic Cluster timeout waiting for sbatchd to start\n"
      << lsfbPrms->maxConcurrentQuery           << ": Max number of concurrent query\n";
#endif
}

}
