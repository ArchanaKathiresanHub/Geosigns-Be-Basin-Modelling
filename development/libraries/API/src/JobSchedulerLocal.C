//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file JobSchedulerLocal.C
/// @brief This file keeps methods implementation of the class for local job scheduler.

#include "JobSchedulerLocal.h"

#ifndef _WIN32
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#else
#include <windows.h>
#endif

#include <cstdio>
#include <cstdlib>

#include <iterator>
#include <memory>
#include <sstream>

#ifndef _WIN32
#include <sys/stat.h>
static void Wait( int sec ) { sleep( sec ); }
#else
static void Wait( int milsec ) { Sleep( milsec * 1000 ); }
#endif



#ifndef NDEBUG 
#define DEBUG( n, ... ) printf( __VA_ARGS__ );
#else
#define DEBUG( n, ... ) ;
#endif

namespace casa
{

// SystemProcess represents a low-level system process
class SystemProcess
{
public:
   SystemProcess( const std::string & cwd, const std::string& commandString, const std::string & outFile, const std::string & errFile );
   
   virtual ~SystemProcess( )
   {
#ifndef _WIN32
      if ( m_isOk ) kill( m_pid, SIGTERM );
#else
      if ( m_isOk   ) { TerminateProcess( hProcess, 0 ); }
      if ( hProcess ) { CloseHandle( hProcess ); }
#endif
   }

   bool isProcessRunning() { return m_isOk; };
   
   void updateProcessStatus();
private:
   bool m_isOk;
#ifndef _WIN32
   int m_pid;
#else
   HANDLE hProcess;
#endif

};

// Start a new process using fork/exec, and mangle the file descriptors
SystemProcess::SystemProcess( const std::string & cwd, const std::string & commandString, const std::string & outFile, const std::string & errFile )
{
   DEBUG( 1, "Running command: %s\n", commandString.c_str( ) );
   m_isOk = false;

#ifndef _WIN32 // Unix implementaiton
   
   int pid = fork( );
   if ( pid < 0 )
   {
      // fork() failed!
      DEBUG( 1, "fork() failed\n" );
      throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "SystemProcess: fork failed";
   }
   else if ( pid == 0 ) // child
   {
      // Split command line options in tokens
      std::istringstream       s( commandString );
      std::vector<std::string> argsV = std::vector<std::string>( std::istream_iterator<std::string>( s ), std::istream_iterator<std::string>() );
      std::string              location( argsV[ 0 ] );

      // Fill in argv list
      const char** args = new const char*[ argsV.size() + 1 ];
      for ( size_t n = 0; n < argsV.size(); ++n )
      {
         args[ n ] = argsV[ n ].c_str();
      }
      args[ argsV.size() ] = NULL;

      DEBUG( 2, "Starting process %s\n", location.c_str( ) );
      for ( size_t n = 0; n < argsV.size( ); n++ )
      {
         DEBUG( 2, "  Arg %d: %s\n", (int)n, args[ n ] ? args[ n ] : "(null)" );
      }

      // change current dir to the script dir
      chdir( cwd.c_str() );

      // redirect outputs to files
      int out = open( outFile.c_str(), O_RDWR | O_CREAT | O_APPEND, 0660 );
      int err = open( errFile.c_str(), O_RDWR | O_CREAT | O_APPEND, 0660 );
      if ( -1 != out ) dup2( out, fileno( stdout ) );
      if ( -1 != err ) dup2( err, fileno( stderr ) );

      // Start the process
      execvp( location.c_str( ), const_cast<char* const*>( args ) );

      // Execution failed - quit the child process
      DEBUG( 0, "Couldn't exec process!\n" );
      _exit( 1 );  // This is exit of the child process!
   }
   else // parent
   {
      m_pid = pid;
      m_isOk = true;
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
   }
   else
   {
      DWORD err = GetLastError();
      DEBUG( 1, "Couldn't start process, err %s\n", err );

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
   case 0: // Child is still running
      m_isOk = true;
      break;
   case -1:
      DEBUG( 0, "Error getting status of the child process!\n" );
      m_isOk = false;
      break;
   default: // Child has exited
      m_isOk = false;
      break;
   }
#else // Windows implementation
   DWORD exitCode = 0;
   bool result = GetExitCodeProcess( hProcess, &exitCode ) == TRUE ? true : false;

   m_isOk = ( result == true && exitCode == STILL_ACTIVE );
#endif
}


class JobSchedulerLocal::Job
{
public:
   Job( const std::string & cwd, const std::string & scriptName, const std::string & jobName, int cpus )
   {
      m_cwd = cwd;
      m_command = scriptName;
      m_jobName = jobName;
      m_cpus = cpus;
      m_isFinished = false;  
      m_out = jobName + ".out";  // redirect stdout
      m_err = jobName + ".err";  // redirect stderr
   }

   virtual ~Job() { ; }

   const char * command() const { return m_command.c_str(); }
   
   // check is job was submitted already
   bool isSubmitted() const { return m_proc.get() != NULL ? true : false; }

   // check is job finished?
   bool isFinished() const { return m_isFinished; }

   bool submit() { 
      if ( !m_proc.get( ) ) m_proc.reset( new SystemProcess( m_cwd, m_command, m_out, m_err ) );
      return m_proc->isProcessRunning();
   }

   // check job status
   JobScheduler::JobState status()
   {
      if ( !isSubmitted() ) return JobScheduler::NotSubmittedYet;
      if ( m_isFinished ) return JobScheduler::JobFinished;

      m_proc->updateProcessStatus();

      if ( m_proc->isProcessRunning() )
      {
         return JobScheduler::JobRunning;
      }
      else
      {
         m_isFinished = true;
         return JobScheduler::JobFinished;
      }

      return JobScheduler::Unknown;
   }

protected:
   std::auto_ptr<SystemProcess> m_proc;

   bool          m_isFinished; // is job finished?
   std::string   m_command;    // command to execute
   std::string   m_out;        // name of the output log file
   std::string   m_err;        // name of the output err log file
   std::string   m_cwd;        // path to folder with script name. Will be set as current for the job
   std::string   m_jobName;    // name of the job
   std::string   m_cpus;       // number of cpus for the job (does not used yet)
};


JobSchedulerLocal::JobSchedulerLocal()
{
   m_clusterName = "LOCAL";
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
   if ( clusterName != "LOCAL" )
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Local job scheduler can not submit jobs to the cluster " << clusterName;
}

// Add job to the list
JobScheduler::JobID JobSchedulerLocal::addJob( const std::string & cwd, const std::string & scriptName, const std::string & jobName, int cpus )
{
   m_jobs.push_back( new Job( cwd, scriptName, jobName, cpus ) );
   return m_jobs.size() - 1; // the position of the new job in the list is it JobID
}

// run job
void JobSchedulerLocal::runJob( JobID job )
{
   if ( job >= m_jobs.size() ) throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "runJob(): no such job in the queue";

   if ( !m_jobs[ job ]->isSubmitted() )
   {
      // spawn job to the cluster
      if ( !m_jobs[ job ]->submit() )
      {
         throw ErrorHandler::Exception( ErrorHandler::LSFLibError ) << "Submitting job " << m_jobs[ job ]->command() << " failed";
      }
   }
}

// get job state
JobScheduler::JobState JobSchedulerLocal::jobState( JobID id )
{
   if ( id >= m_jobs.size( ) ) throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "jobState(): no such job in the queue";
   return m_jobs[ id ]->status( ); 
}

void JobSchedulerLocal::sleep( )
{
   Wait( 10 );
}

}

