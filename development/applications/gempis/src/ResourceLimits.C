/*********************************************************************
*                                                                     
* Package:        GEneric MPI Launcher (gempil) Resource Limits
*
*
*********************************************************************/

/// 
///
/// 

#include "includestreams.h"  // platform type definitions

// Qt include files
#include <qstring.h>
#include <qstringlist.h>
#include <qfile.h>
#include <qtextstream.h>

// project include files
#include <GempisThread.h>
#include <ResourceFile.h>
#include <ResourceLimits.h>

#include <string>
#include <stdio.h>        // putenv
#include <unistd.h>       // sleep
#include <sys/sysinfo.h>  // sysinfo and get_nprocs
#include <errno.h>        // errno



///////////////////////////////////////////////////////////////////////
//
// c'tor
//
ResourceLimits::ResourceLimits( QString & resource ) :
  m_defined( false ),
  m_createFailed( false ),
  m_resourceName( resource ),
  m_error( 0 ),
  m_totalCores( 1 ),
  m_memoryPerCore( 1 )
{
#ifdef DEBUG_RESOURCELIMITS
   cout << this << " ResourceLimits::ResourceLimits()" << endl;
#endif

   // undefined value then set
   if ( resource == "" )
     {
       m_resourceName = "LOCAL";
     }

   // load resource configuration, get the current entry
   t_resourceEntry resourceEntry;
   ResourceFile * rd = new ResourceFile();
   rd->LoadResources( true );
   rd->GetResourceEntry( m_resourceName, &resourceEntry );
   m_jobtype = resourceEntry.res_jobtype;
   delete rd;

   // if jobtype is standalone read computer, otherwise read file
   if ( m_jobtype == "STANDALONE" )
    {
      readComputer();
    }
  else
    {
      readFile();
    }
}

///////////////////////////////////////////////////////////////////////
//
// d'tor
//
ResourceLimits::~ResourceLimits()
{
#ifdef DEBUG_RESOURCELIMITS
  cout << this << " ResourceLimits::~ResourceLimits()" << endl;
#endif

}

///////////////////////////////////////////////////////////////////////
//
// get values from class
//
// get the status of the resource limits
long int ResourceLimits::GetStatus( void ) const
{
#ifdef DEBUG_RESOURCELIMITS
  cout << this << " ResourceLimits::GetStatus()" << endl;
#endif

  // check status
  if ( m_defined == true )
    {
      return HAVE_VALUES;
    }
  if ( m_createFailed == true )
    {
      return CREATE_FAILED;
    }

  return CREATE_LIMITS;
}

// return the total number of cores
long int ResourceLimits::GetTotalCores( void ) const
{
#ifdef DEBUG_RESOURCELIMITS
  cout << this << " ResourceLimits::GetTotalCores()" << endl;
#endif

  return m_totalCores;
}

// returns the memory available per core
long int ResourceLimits::GetMemoryPerCore( void ) const
{
#ifdef DEBUG_RESOURCELIMITS
  cout << this << " ResourceLimits::GetMemoryPerCore()" << endl;
#endif

  return m_memoryPerCore;
}


///////////////////////////////////////////////////////////////////////
//
// submit Gempis once to create resource limit file
//
void ResourceLimits::CreateLimits( void )
{
#ifdef DEBUG_RESOURCELIMITS
  cout << this << " ResourceLimits::createFile()" << endl;
#endif

  // the member is not applicable to STANDALONE jobtypes
  if ( m_jobtype == "STANDALONE" )
    {
      return;
    }

  // define the Gempis command 
  GempisThread * reqThread = new GempisThread();
  QString command;
  command = "gempis ";
  char * cptr;
  cptr = getenv( "IBS_VERSION" );
  if ( cptr != NULL && strlen(cptr) > 1 )
    {
      command += "-";
      command += cptr;
    }
  command += " ";
  command += m_resourceName;
  reqThread->SetCommand( command );

  // define log file name
  QString logfile;
  logfile = m_resourceName + ".log";
  reqThread->SetLogFileName( logfile );

  // calling in this way runs the command in the foreground
  reqThread->run();

  // if we were successful 
  m_createFailed = true;
  if ( !reqThread->GetResult() )
    {
      // wait a max of 60 secs. for the NFS file to be seen by the calling computer
      for ( int i=0; i<60; i++ )
        {
          QFile f( m_resourceFileName );
          if ( f.exists() )
            {
              m_createFailed = false;
              m_defined = true;
              break;
            }
          sleep( 1 );
        }
      // if successful, remove the log file
      QFile f( logfile );
      f.remove();
    }

  // remove local data
  delete reqThread;

  // load limit values in
  readFile();
}



///////////////////////////////////////////////////////////////////////
//
// get resources define in the resource files
void ResourceLimits::readFile( void )
{
#ifdef DEBUG_RESOURCELIMITS
  cout << this << " ResourceLimits::readFile()" << endl;
#endif

  // define the name of the resource file
  QString resourceFileName;
  char * cptr;
  cptr = getenv( "HOME" );
  if ( cptr == NULL || strlen(cptr) < 1 )
    {
      // update status bar with any error messages
      cerr << "Error! Could not determine HOME variable." << endl;
      return;
    }
  resourceFileName = cptr;
  resourceFileName += "/.gempis/resources/";
  resourceFileName += m_resourceName;

  // save this value so it can be used by createFile if necessary
  m_resourceFileName = resourceFileName;

  // open resource limit, if it does not exist then call gempis to create it
  int tries = 0;
  QFile f( resourceFileName );
  if ( !f.exists() )
    {
      return;
    }

  if ( !f.open( IO_ReadOnly ) )
    {
      // indicate that the resource limits are available
      cerr << "Error! Could not read limits file for resource " << m_resourceName << "." << endl;
      return;
    }

  // fields are "total memoryper"
  QTextStream ts( &f );
  QString str;
  str = ts.readLine();
  f.close();

  // assign the total number of processors and check that it is valid
  QString size = str.section( ' ', 0, 0, QString::SectionSkipEmpty );
  if ( size.toLong() < 1 )
    {
      m_totalCores = 1;
    }
  else
    {
      m_totalCores = size.toLong();
    }

  // assign memory available per processor
  QString mem;
  mem = str.section( ' ', 1, 1, QString::SectionSkipEmpty );
  m_memoryPerCore = mem.toLong();

  // indicate that the resource limits are available
  m_defined = true;
}

///////////////////////////////////////////////////////////////////////
//
// get limits for current computer
// called when needing information about the STANDALONE jobtype
//
void ResourceLimits::readComputer( void )
{
#ifdef DEBUG_RESOURCELIMITS
  cout << this << " ResourceLimits::readComputer()" << endl;
#endif

  // assign the total number of processors, and check that its's valid
  int nprocs = get_nprocs();
#ifdef DEBUG_RESOURCELIMITS
  cout << this << " ResourceLimits:: nprocs = " << nprocs << endl;
#endif

  // assign the total number of processors and check that it is valid
  if ( nprocs < 1 )
    {
      m_totalCores = 1;
    }
  else
    {
      m_totalCores = nprocs;
    }

  // get the memory per core
  struct sysinfo info;
  long int ram = 0;
  if ( sysinfo( &info ) != 0 )
    {
      // update status bar with any error messages
      QString myerr;
      myerr.setNum( errno );
      cerr << "Error! sysinfo failed, errno = " << myerr << "." << endl;
      return;
    }
  else
    {
#ifdef DEBUG_RESOURCELIMITS
      cout << this << " JobLauncherController:: total ram   = " << info.totalram << endl;
      cout << this << " JobLauncherController:: free ram    = " << info.freeram << endl;
      cout << this << " JobLauncherController:: shared ram  = " << info.sharedram << endl;
      cout << this << " JobLauncherController:: buffer ram  = " << info.bufferram << endl;
      cout << this << " JobLauncherController:: total swap  = " << info.totalswap << endl;
      cout << this << " JobLauncherController:: free swap   = " << info.freeswap << endl;
      cout << this << " JobLauncherController:: procs       = " << info.procs << endl;
      cout << this << " JobLauncherController:: total high  = " << info.totalhigh << endl;
      cout << this << " JobLauncherController:: avail high  = " << info.freehigh << endl;
      cout << this << " JobLauncherController:: memory unit = " << info.mem_unit << endl;
#endif
      ram = info.totalram;

      // take off 500 MB that the computer probably needs 
      // and give the result in MB
      // 
      ram = ( ( ram * info.mem_unit - 536870912 ) / 1048576 ) / nprocs;
    }
  m_memoryPerCore = ram;

  // indicate that resources limits are available
  m_defined = true;
}

