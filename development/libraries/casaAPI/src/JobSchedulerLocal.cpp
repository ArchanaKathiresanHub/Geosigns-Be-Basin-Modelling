//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file JobSchedulerLocal.cpp
/// @brief This file keeps methods implementation of the class for local job scheduler.

// CASA
#include "CasaDeserializer.h"
#include "JobSchedulerLocal.h"
#include "RunManager.h"

// FileSystem
#include "FilePath.h"
#include "FolderPath.h"

// STL
#include <iterator>
#include <memory>
#include <sstream>
#include <fstream>

// STD C lib
#ifndef _WIN32
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#else
#include <windows.h>
#include <winbase.h>
#endif

#include <cstdio>
#include <cstdlib>

#ifndef _WIN32
#include <sys/stat.h>
static void Wait( int sec ) { sleep( sec ); }
static size_t NumCPUS() { return sysconf( _SC_NPROCESSORS_ONLN ); }
#else
static void Wait( int milsec ) { Sleep( milsec * 1000 ); }
static size_t NumCPUS() { SYSTEM_INFO sysinfo; GetSystemInfo( &sysinfo ); return sysinfo.dwNumberOfProcessors; }
#endif


#ifndef NDEBUG
#define DEBUG( n, ... ) printf( __VA_ARGS__ );
#else
#define DEBUG( n, ... ) ;
#endif

namespace casa
{

class JSTimer
{
public:
   JSTimer() : m_begin( -1.0 )
   {
#ifdef _WIN32
      LARGE_INTEGER ifrq;
      QueryPerformanceFrequency( &ifrq );
      m_invQPfrq = 1.0 / (double)(ifrq.QuadPart);
#endif
   }

   double time() // Return wall-clock time in seconds
   {
      if ( m_begin < 0 ) m_begin = get_time(); // timer wasn't started start it now
      return get_time() - m_begin;
   }

   void start() { m_begin = get_time();  } // Start timer

protected:
   double m_begin;    // start time
#ifdef _WIN32
   double  m_invQPfrq;
#endif
   double get_time() const // query time
   {
#ifndef _WIN32
      // astronomical time
      struct timeval    tv;
      gettimeofday( &tv, NULL );
      return tv.tv_sec + tv.tv_usec * 1.0e-6;
#else
      // astronomical time
      LARGE_INTEGER tick_count;
      QueryPerformanceCounter( &tick_count );
      return tick_count.QuadPart * m_invQPfrq;
#endif
   }
};

// SystemProcess represents a low-level system process
class SystemProcess
{
public:
   SystemProcess( const std::string & cwd, const std::string & commandString, const std::string & outFile, const std::string & errFile );

   virtual ~SystemProcess( )
   {
#ifndef _WIN32
      if ( m_isOk )
      {
         kill( -m_pid, SIGTERM );
         Wait( 1 );
      }

      // get TMPDIR value if it is set
      const char * tmpDir = getenv( "TMPDIR" );

      // clean $TMPDIR/pid if it is exists
      if ( tmpDir )
      {
         ibs::FolderPath tmpSubPrcDir( tmpDir );
         tmpSubPrcDir << m_pid;
         if ( tmpSubPrcDir.exists() ) tmpSubPrcDir.remove();
      }
#else
      if ( m_isOk   ) { TerminateProcess( hProcess, 0 ); Wait( 1 ); }
      if ( hProcess ) { CloseHandle( hProcess ); }
#endif
   }

   bool isProcessRunning() { return m_isOk; };

   void updateProcessStatus();
   size_t getRunTime() { return static_cast<size_t>( m_procTimer.time() / 60.0 ); }

#ifndef _WIN32
      int m_pid;
      int m_gid;
#else
      HANDLE hProcess;
#endif

private:
   bool    m_isOk;
   JSTimer m_procTimer;
};

// Start a new process using fork/exec, and mangle the file descriptors
SystemProcess::SystemProcess( const std::string & cwd
                            , const std::string & commandString
                            , const std::string & outFile
                            , const std::string & errFile
                            )
{
   DEBUG( 1, "Running command: %s\n", commandString.c_str() );
   m_isOk = false;

   m_procTimer.start();

#ifndef _WIN32 // Unix implementaiton

   // Split command line options in tokens
   std::istringstream       s( commandString );
   std::vector<std::string> argsV = std::vector<std::string>( std::istream_iterator<std::string>( s ), std::istream_iterator<std::string>() );
   std::string              location( argsV[ 0 ] );

   // Fill in argv list
   const char ** args = new const char*[ argsV.size() + 1 ];

   for ( size_t n = 0; n < argsV.size(); ++n ) args[ n ] = argsV[ n ].c_str();
   args[ argsV.size() ] = NULL;

   const char * nwd = cwd.c_str(); // script folder

   // get TMPDIR value if it is set
   const char * tmpDir = getenv( "TMPDIR" );

   int pid = fork();
   if ( pid < 0 ) // fork() failed!
   {
      throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "SystemProcess: fork failed";
   }
   else if ( pid == 0 ) // child
   {
      // change current folder to the script folder
      if ( chdir( nwd ) == 0 )
      {
         // create TMPDIR like $TMPDIR/pid
         if ( tmpDir )
         {
            ibs::FolderPath tmpSubPrcDir( tmpDir );
            tmpSubPrcDir << getpid();
            // if subdir in TMPDIR doesn't exist - create the new one
            if ( !tmpSubPrcDir.exists() ) tmpSubPrcDir.create();
            setenv( "TMPDIR", tmpSubPrcDir.path().c_str(), 1 );
         }

         // redirect outputs to files
         int out = open( outFile.c_str(), O_RDWR | O_CREAT | O_APPEND, 0660 );
         int err = open( errFile.c_str(), O_RDWR | O_CREAT | O_APPEND, 0660 );
         if ( -1 != out ) dup2( out, fileno( stdout ) );
         if ( -1 != err ) dup2( err, fileno( stderr ) );

         // Start the process
         execvp( location.c_str( ), const_cast<char* const*>( args ) );
      }

      // Execution failed - quit the child process
      _exit( 1 );  // This is exit of the child process!
   }
   else // parent
   {
      m_pid = pid;
      setpgid( m_pid, m_pid ); // set group ID for the child process to kill all child subprocesses

      m_isOk = true;
      delete [] args;

      // log job ID
      std::ofstream ofs( RunManager::s_jobsIDListFileName, std::ios_base::out | std::ios_base::app );
      if ( ofs.is_open() ) { ofs << m_pid << std::endl; }
   }
#else // Windows implementation

   const char* batExtension = ".bat";
   const char* exeExtension = ".exe";

   std::string systemCommand = commandString;
   size_t p = systemCommand.find( batExtension );

   if ( p != std::string::npos )
   {
      // when a batch file is specified then prefix the cmd
      // The ShellExecuteEx doesn't work with *.bat files.
      systemCommand = "cmd.exe /C " + systemCommand;
   }
   // find the location of the extension again.
   p = systemCommand.find( exeExtension );

   std::string process( systemCommand.substr( 0, p + 4 ) );
   std::string params( systemCommand.substr( p + 5 ) );

   DEBUG( 1, "Starting process %s with options: %s\n", process.c_str( ), params.c_str( ) );

   SHELLEXECUTEINFO ExecuteInfo = { 0 };
   ExecuteInfo.cbSize           = sizeof( ExecuteInfo );
   ExecuteInfo.fMask            = SEE_MASK_NOCLOSEPROCESS;
   ExecuteInfo.lpVerb           = "open";
   ExecuteInfo.lpFile           = process.c_str();
   ExecuteInfo.lpParameters     = params.c_str();
   ExecuteInfo.lpDirectory      = cwd.c_str();
   ExecuteInfo.nShow            = SW_SHOWMINNOACTIVE;

   if ( ShellExecuteEx( &ExecuteInfo ) )
   {
      hProcess = ExecuteInfo.hProcess;

      DEBUG( 1, "CreatedProcess\n" );
      m_isOk = true;

      // log job ID
      std::ofstream ofs( RunManager::s_jobsIDListFileName, std::ios_base::out | std::ios_base::app );
      if ( ofs.is_open() ) { ofs << GetProcessId( hProcess ) << std::endl; }
   }
   else
   {
      DWORD err = GetLastError();
      DEBUG( 1, "Couldn't start process, err %d\n", err );

      hProcess = NULL;
      m_isOk = false;
   }
#endif
}

// Update the status of the ok flag
void SystemProcess::updateProcessStatus()
{
#ifndef _WIN32 // Unix implementation
   int status;
   pid_t result = waitpid( m_pid, &status, WNOHANG );
   switch ( result )
   {
   case  0:  m_isOk = true;  break; // Child is still running
   case -1:
      DEBUG( 0, "Error getting status of the child process!\n" );
      m_isOk = false;
      break;
   default:  m_isOk = false; break; // Child has exited
   }
#else // Windows implementation
   DWORD exitCode = 0;
   bool result = GetExitCodeProcess( hProcess, &exitCode ) == TRUE ? true : false;
   m_isOk = ( result == true && exitCode == STILL_ACTIVE );
#endif
}

class JobSchedulerLocal::Job : public CasaSerializable
{
public:
   Job( const std::string & cwd, const std::string & scriptName, const std::string & jobName, int cpus, size_t runTimeLim )
   {
      m_cwd        = cwd;
      m_command    = scriptName;
      m_jobName    = jobName;
      m_cpus       = cpus;
      m_jobState   = JobScheduler::NotSubmittedYet;
      m_runTimeLim = runTimeLim;

      m_out = jobName + ".out";  // redirect stdout
      m_err = jobName + ".err";  // redirect stderr
   }

   virtual ~Job() { ; }

   const char * command() const { return m_command.c_str(); }

   int cpus() const { return m_cpus; }

   void submit() { if ( JobScheduler::NotSubmittedYet == m_jobState ) m_jobState = JobScheduler::JobPending; }

   bool run()
   {
      if ( !m_proc.get() ) m_proc.reset( new SystemProcess( m_cwd, m_command, m_out, m_err ) );
      m_jobState = m_proc->isProcessRunning() ? JobScheduler::JobRunning : JobScheduler::JobFailed;
      return m_jobState == JobScheduler::JobRunning;
   }

   bool stop()
   {
      if (      JobScheduler::JobPending == m_jobState ) { m_jobState = JobScheduler::JobFailed; }
      else if ( JobScheduler::JobRunning == m_jobState ) { m_jobState = JobScheduler::JobFailed; m_proc.reset( NULL ); }
      return true;
   }

#ifndef _WIN32
      int    id() { return m_proc.get() ? m_proc->m_pid : -1; }
#else
      HANDLE id() { return m_proc.get() ? m_proc->hProcess : 0; }
#endif


   // check job status
   JobScheduler::JobState status()
   {
      switch ( m_jobState )
      {
         case JobScheduler::NotSubmittedYet:
         case JobScheduler::JobFinished:
         case JobScheduler::JobPending:
         case JobScheduler::JobFailed:
            break;

         case JobScheduler::JobRunning:
            m_proc->updateProcessStatus();

            if ( m_proc->isProcessRunning() )
            {
               if ( m_runTimeLim && m_proc->getRunTime() > m_runTimeLim ) { stop(); }
               else                                                       { m_jobState = JobScheduler::JobRunning; }
            }
            else
            {
               if (      ibs::FilePath ( m_command + ".success" ).exists() ) { m_jobState = JobScheduler::JobFinished; }
               else if ( ibs::FilePath ( m_command + ".failed"  ).exists() ) { m_jobState = JobScheduler::JobFailed;   }
               else                                                          { m_jobState = JobScheduler::JobFailed;   }
            }
            break;

         default: assert(0); break;
      }
      return m_jobState;
   }

   // version of serialized object representation
   virtual unsigned int version() const { return 1; }

   // Get type name of the serialaizable object, used in deserialization to create object with correct type
   virtual std::string typeName() const { return "JobSchedulerLocal::Job"; }

   // Serialize object to the given stream
   virtual bool save( CasaSerializer & sz ) const
   {
      bool ok = sz.save( static_cast<int>(m_jobState), "JobState" );

      ok = ok ? sz.save( m_command, "Command"       ) : ok;
      ok = ok ? sz.save( m_out,     "StdOutLogFile" ) : ok;
      ok = ok ? sz.save( m_err,     "StdErrLogFile" ) : ok;
      ok = ok ? sz.save( m_cwd,     "CWD"           ) : ok;
      ok = ok ? sz.save( m_jobName, "JobName"       ) : ok;
      ok = ok ? sz.save( m_cpus,    "CPUsNum"       ) : ok;
      ok = ok ? sz.save( m_runTimeLim, "runTimeLimMinutes" ) : ok;

      return ok;
   }

   // Create a new instance and deserialize it from the given stream
   Job( CasaDeserializer & dz, const char * objName )
   {
      // read from file object name and version
      unsigned int objVer = version();
      bool ok = dz.checkObjectDescription( typeName(), objName, objVer );

      int js;
      ok = ok ? dz.load( js, "JobState" ) : ok;
      m_jobState = static_cast<JobScheduler::JobState>( js );

      ok = ok ? dz.load( m_command, "Command"       ) : ok;
      ok = ok ? dz.load( m_out,     "StdOutLogFile" ) : ok;
      ok = ok ? dz.load( m_err,     "StdErrLogFile" ) : ok;
      ok = ok ? dz.load( m_cwd,     "CWD"           ) : ok;
      ok = ok ? dz.load( m_jobName, "JobName"       ) : ok;
      ok = ok ? dz.load( m_cpus,    "CPUsNum"       ) : ok;

      if ( objVer > 0 )
      {
         ok = ok ? dz.load( m_runTimeLim, "runTimeLimMinutes" ) : ok;
      }
      else { m_runTimeLim = 0; }

      if ( !ok )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "JobSchedulerLSF::Job deserialization error";
      }
   }

protected:
   std::unique_ptr<SystemProcess> m_proc;

   JobScheduler::JobState m_jobState; // state of the job

   std::string   m_command;           // command to execute
   std::string   m_out;               // name of the output log file
   std::string   m_err;               // name of the output err log file
   std::string   m_cwd;               // path to folder with script name. Will be set as current for the job
   std::string   m_jobName;           // name of the job
   int           m_cpus;              // number of cpus for the job (does not used yet)
   size_t        m_runTimeLim; // runt time job limitation [Minutes]
};


JobSchedulerLocal::JobSchedulerLocal()
{
   m_clusterName = "LOCAL";
   m_avCPUs = NumCPUS();
}

JobSchedulerLocal::~JobSchedulerLocal()
{
   for ( size_t i = 0; i < m_jobs.size( ); ++i )
   {
      delete m_jobs[ i ];     // clean array of scheduled jobs
   }
}

// get cluster name
void JobSchedulerLocal::setClusterName( const char * clusterName )
{
   if ( std::string( clusterName ) != "LOCAL" )
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Local job scheduler can not submit jobs to the cluster " << clusterName;
}

// Add job to the list
JobScheduler::JobID JobSchedulerLocal::addJob( const std::string & cwd
                                             , const std::string & scriptName
                                             , const std::string & jobName
                                             , int                 cpus
                                             , size_t              runTimeLim
                                             , const std::string & /* scenarioID */ // not used yet
                                             )
{
   ibs::FilePath scriptStatFile( scriptName + ".failed" );
   if ( scriptStatFile.exists() ) scriptStatFile.remove();

   m_jobs.push_back( new Job( cwd, scriptName, jobName, cpus, runTimeLim ) );
   return m_jobs.size() - 1; // the position of the new job in the list is it JobID
}


// run job
JobScheduler::JobState JobSchedulerLocal::runJob( JobID job )
{
   if ( job >= m_jobs.size() ) throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "runJob(): no such job in the queue";

   if ( m_jobs[ job ]->status() == JobScheduler::NotSubmittedYet )
   {
      m_jobs[ job ]->submit();
   }

   return jobState( job );
}

// stop submitted job
JobScheduler::JobState JobSchedulerLocal::stopJob( JobID job )
{
   if ( job >= m_jobs.size() )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "runJob(): no job with ID: "  << job << " in the queue";
   }

   switch ( m_jobs[job]->status() )
   {
      case JobScheduler::JobPending:
      case JobScheduler::JobRunning:
         if ( !m_jobs[job]->stop() )
         {
            throw ErrorHandler::Exception( ErrorHandler::LSFLibError ) << "Stopping the job " << m_jobs[ job ]->command() << " failed";
         }
         break;

      default:
         break;
   }

   return JobScheduler::JobFailed;
}


// get job state
JobScheduler::JobState JobSchedulerLocal::jobState( JobID id )
{
   if ( id >= m_jobs.size( ) ) throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "jobState(): no such job in the queue";

   JobScheduler::JobState jobState = m_jobs[id]->status();

   if ( JobScheduler::JobPending == jobState ) // if it's pending try to run job
   {
     // ignore start job request if number of running jobs is equal number of CPU cores
      if ( runningJobsNumber() < m_avCPUs )
      {
         // run job
         if ( !m_jobs[id]->run() )
         {
            throw ErrorHandler::Exception( ErrorHandler::LSFLibError ) << "Submitting job " << m_jobs[ id ]->command() << " failed";
         }
      }
      jobState = m_jobs[ id ]->status();
   }

   return jobState;
}

std::string JobSchedulerLocal::schedulerJobID( JobID id )
{
   if ( id >= m_jobs.size() ) throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "jobState(): no such job in the queue";

   std::ostringstream oss;
   oss << m_jobs[id]->id();

   return oss.str();
}


void JobSchedulerLocal::sleep( int secs )
{
   if (      secs < 0 ) { Wait( 10 );   }
   else if ( secs > 0 ) { Wait( secs ); }
}

// get number of running jobs
size_t JobSchedulerLocal::runningJobsNumber()
{
   size_t jobsRunning = 0;
   // go over jobs and see how many is running
   for ( size_t i = 0; i < m_jobs.size(); ++i )
   {
      if ( JobScheduler::JobRunning == m_jobs[ i ]->status() ) jobsRunning += m_jobs[i]->cpus();
   }

   return jobsRunning;
}

// Serialize object to the given stream
bool JobSchedulerLocal::save( CasaSerializer & sz ) const
{
   bool ok = sz.save( m_clusterName, "ClusterName" );
   ok = ok ? sz.save( m_avCPUs,      "CoresNumber" ) : ok;

   ok = ok ? sz.save( m_jobs.size(), "JobsQueueSize" ) : ok;
   for ( size_t i = 0; i < m_jobs.size() && ok; ++i )
   {
      ok = sz.save( *m_jobs[i], "JobDescr" );
   }
   return ok;
}

// Create a new instance and deserialize it from the given stream
JobSchedulerLocal::JobSchedulerLocal( CasaDeserializer & dz, unsigned int objVer )
{
   if ( version() < objVer )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "Version of object in file is newer. No forward compatibility!";
   }

   bool ok = dz.load( m_clusterName, "ClusterName" );
   ok = ok ? dz.load( m_avCPUs,      "CoresNumber" ) : ok;

   size_t setSize;
   ok = ok ? dz.load( setSize, "JobsQueueSize" ) : ok;
   for ( size_t i = 0; i < setSize && ok; ++i )
   {
      m_jobs.push_back( new Job( dz, "JobDescr" ) );
   }

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "JobSchedulerLocal deserialization error";
   }
}
}

