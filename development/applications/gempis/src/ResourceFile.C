///
///
///

#include "includestreams.h"

#include "ResourceFile.h"

#include <qlineedit.h>
#include <qcombobox.h>
#include <qtextedit.h> 
#include <qpushbutton.h> 
#include <qapplication.h> 
#include <qdatetime.h> 
#include <qtimer.h> 
#include <qmessagebox.h> 
#include <qdeepcopy.h> 
#include <qfile.h>
#include <qtextstream.h>

#include <string>
#include <stdio.h>
#include <stdlib.h>

// process info
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>



ResourceFile::ResourceFile() :
  m_resourceEntry( 0 ),
  m_ResourceNames(),
  m_JobTypes(),
  m_MpiRefs()
{
#ifdef DEBUG_RESOURCEFILE
  cout << this << " ResourceFile::ResourceFile()" << endl;
#endif

  // define Job types
  readJobTypes();

  // get MPI references
  readMpiRefs();
}


ResourceFile::~ResourceFile()
{
#ifdef DEBUG_RESOURCEFILE
  cout << this << " ResourceFile::~ResourceFile()" << endl;
#endif
}


void ResourceFile::LoadResources( bool loadAll )
{
#ifdef DEBUG_RESOURCEFILE
  cout << this << " ResourceFile::LoadResources()" << endl;
#endif

  // clear out any previous stuff
  m_resourceEntry.clear();

  // Add an empty resource first
  resetRETemp();

  if ( loadAll )
    {
      // load system configuration
      readResources( true );
    }

  // load resource list from user config file
  readResources( false );
}



void ResourceFile::SaveResources()
{
#ifdef DEBUG_RESOURCEFILE
  cout << this << " ResourceFile::SaveResources()" << endl;
#endif

  // write data to resource file
  writeResources();
}



QStringList ResourceFile::GetResourceList()
{
#ifdef DEBUG_RESOURCEFILE
  cout << this << " ResourceFile::GetResourceList()" << endl;
#endif

  // define output list, add a blank entry
  m_ResourceNames.clear();

  // Add empty entry
  for ( unsigned int i=0; i<m_resourceEntry.size(); i++ )
    {
      m_ResourceNames << m_resourceEntry[i].res_name;
    }

  return m_ResourceNames;
}


// define job types 
QStringList ResourceFile::GetJobTypes()
{
#ifdef DEBUG_RESOURCEFILE
  cout << this << " ResourceFile::GetJobTypes()" << endl;
#endif

  return m_JobTypes;
}


// the only point here is to load the mpi list with values
QStringList ResourceFile::GetMPIReferences()
{
#ifdef DEBUG_RESOURCEFILE
  cout << this << " ResourceFile::GetMPIReferences()" << endl;
#endif

  return m_MpiRefs;
}


void ResourceFile::GetResourceEntry( QString & s, t_resourceEntry * pEntry )
{
#ifdef DEBUG_RESOURCEFILE
  cout << this << " ResourceFile::GetResourceEntry( " << s << " )" << endl;
#endif

  int i = getIndex( s );
  if ( i < 0 ) 
    { 
      return;
    }

  // copy reference over
  *pEntry = m_resourceEntry[i];
}


void ResourceFile::ModifyResourceEntry( QString & s, t_resourceEntry & rEntry )
{
#ifdef DEBUG_RESOURCEFILE
  cout << this << " ResourceFile::ModifyResourceEntry()" << endl;
#endif

  int i = getIndex( s );
  if ( i < 0 ) 
    { 
      return;
    }

  // save data
  m_resourceEntry[i] = rEntry;
}


void ResourceFile::AddResourceEntry( t_resourceEntry & rEntry )
{
#ifdef DEBUG_RESOURCEFILE
  cout << this << " ResourceFile::AddResourceEntry(" << rEntry.res_name << ")" << endl;
#endif

  // make sure that this entry doesn't already exist
  int i = getIndex( rEntry.res_name );
  if ( i >= 0 ) 
    { 
#ifdef DEBUG_RESOURCEFILE
      cout << "    Resource [" << rEntry.res_name << "] already exists at index " << i << endl;
#endif
      return;
    }

  // add resource to selection list
  m_resourceEntry.push_back( rEntry );
}


void ResourceFile::DeleteResourceEntry( QString & s )
{
#ifdef DEBUG_RESOURCEFILE
  cout << this << " ResourceFile:: DeleteResourceEntry(" << s << ")" << endl;
#endif

  // find a place in the array 
  int i = getIndex( s );
  if ( i < 0 ) 
    { 
      return;
    }

  // wipe out data values
  m_resourceEntry.erase( m_resourceEntry.begin()+i, m_resourceEntry.begin()+i+1 );
}


//////////////////////////////////////////////////////////////////////////////////////////
// private member functions



// find a place in the vector where the name is used
// Output: >= 0: index to string value
//         < 0:  string not found
int ResourceFile::getIndex( QString s )
{
  unsigned int i;
  for ( i=0; i<m_resourceEntry.size(); i++ )
    {
      if ( s == m_resourceEntry[i].res_name )
        {
          break;
        }
    }

  // check that a valid index was found
  if ( i == m_resourceEntry.size() )
    {
      return -1;
    }

  // return index
  return i;
}


// reads file and appends jobtype entries
void ResourceFile::readJobTypesFile( QString fn )
{
#ifdef DEBUG_RESOURCEFILE
  cout << this << " ResourceFile::readJobTypesFile(" << fn << ")" << endl;
#endif

  QFile f( fn );
  if ( !f.open( IO_ReadOnly ) )
    {
      return;
    }

  // parse input file
  QTextStream ts( &f );
  while ( true )
  {
    QString str;
    str = ts.readLine();
    if ( str.isNull() )
      break;

    // string found if string and no comment char
    if ( str.contains( "GEMPIS_JOBTYPES", true ) &&  str.contains('#') == 0 )
      {
        QString opt;
        opt = str.section( '"', 1, 1 );
        if ( opt == NULL )
          {
            cout << "Format error in line [" << str << "]." << endl;
            break;
          }

        // split string into parts
        for ( int i=0; i<51; i++ )
          {
            QString val;
            val = opt.section( ' ', i, i );
            if ( val == NULL )
              {
                break;
              }
 
            // add entry to list
            m_JobTypes.append( val );
          }
      } 
  }
  // close file
  f.close();
}

// Load Jobtype references
void ResourceFile::readJobTypes()
{
#ifdef DEBUG_RESOURCEFILE
  cout << this << " ResourceFile::readJobTypes()" << endl;
#endif

  // make first entry blank
  m_JobTypes.clear();

  // Determine location of system references
  QString fn;
  fn = getenv( "IBS_NAMES" );
  if ( fn == NULL )
    {
      cout << "Error! Environment variable IBS_NAMES not defined." << endl;
      return;
    }
  fn += "/gempis/definitions_system";
  readJobTypesFile( fn );

  // Determine location of user references
  fn = getenv( "HOME" );
  if ( fn == NULL )
    {
      cout << "Error: HOME not defined." << endl;
      return;
    }
  fn += "/.gempis/definition_user";
  readJobTypesFile( fn );
}


// the only point here is to load the mpi list with values
void ResourceFile::readMpiRefsFile( QString fn )
{
#ifdef DEBUG_RESOURCEFILE
  cout << this << " ResourceFile::readMpiRefsFile(" << fn << ")" << endl;
#endif

  QFile f( fn );
  if ( !f.open( IO_ReadOnly ) )
    {
      return;
    }

  // parse input file
  QTextStream ts( &f );
  while ( true )
  {
    QString str;
    str = ts.readLine();
    if ( str.isNull() )
      break;

    /*
#ifdef DEBUG_RESOURCEFILE
    cout << "    " << str << endl;
#endif
    */

    // string found if string and no comment char
    if ( str.contains( "mpi_name", true ) &&  str.contains('#') == 0 )
      {
        QString opt;
        opt = str.section( '"', 1, 1 );
        if ( opt == NULL )
          {
            cout << "Format error in line [" << str << "]." << endl;
            break;
          }
        // add entry to list
        m_MpiRefs.append( opt );
      } 
  }
  // close file
  f.close();
}

// the only point here is to load the mpi list with values
void ResourceFile::readMpiRefs()
{
#ifdef DEBUG_RESOURCEFILE
  cout << this << " ResourceFile::readMpiRefs()" << endl;
#endif

  // make first entry blank
  m_MpiRefs.clear();

  // Determine location of system references
  QString fn;
  fn = getenv( "IBS_NAMES" );
  if ( fn == NULL )
    {
      cout << "Error! Environment variable IBS_NAMES not defined." << endl;
      return;
    }
  fn += "/gempis/mpirefs_system";
  readMpiRefsFile( fn );

  // Determine location of user references
  fn = getenv( "HOME" );
  if ( fn == NULL )
    {
      cout << "Error: HOME not defined." << endl;
      return;
    }
  fn += "/.gempis/mpirefs_user";
  readMpiRefsFile( fn );
}


void ResourceFile::resetRETemp()
{
#ifdef DEBUG_RESOURCEFILE
  cout << this << " ResourceFile::resetRETemp()" << endl;
#endif

  m_RETemp.res_name = "";
  m_RETemp.res_jobtype = "";
  m_RETemp.res_mpiref = "";
  m_RETemp.res_remote = "";
  m_RETemp.res_request = "";
  m_RETemp.res_submit = "";
  m_RETemp.res_prepath = "";
  m_RETemp.res_cluster = "";
}


// get user definitions
void ResourceFile::readDefinitionsFile( QString fn )
{
#ifdef DEBUG_RESOURCEFILE
  cout << this << " ResourceFile::readDefinitionsFile(" << fn << ")" << endl;
#endif

  // define default name
  m_RegionName = "TEMPLATE";

  // open file for reading only
  QFile f( fn );
  if ( !f.open( IO_ReadOnly ) )
    {
      cout << "Error! Could not open file " << fn << " for reading." << endl;
      return;
    }

  // parse input file
  QTextStream ts( &f );

  while ( true )
  {
    QString str;
    str = ts.readLine();
    if ( str.isNull() )
      break;

    // skip the line if it begins with a comment '#'
    if ( str.find( '#' ) == 0 )
      {
        continue;
      }

    // remove any double quotes in the line
    str.remove( '"' );

    // string found if string
    if ( str.contains( "GEMPIS_REGION_NAME", true ) )
      {
        // get the assigned value
        QString val;
        m_RegionName = str.section( '=', 1, 1 );
        if ( m_RegionName == NULL )
          {
            m_RegionName = "TEMPLATE";
            cout << "Error! Value format error in line [" << str << "]." << endl;
          }
      }
  }

  // close file
  f.close();
}


// continue parsing file, ignore errors
bool ResourceFile::readResourceFile( QString fn )
{
#ifdef DEBUG_RESOURCEFILE
  cout << this << " ResourceFile::readResourceFile(" << fn << ")" << endl;
#endif

  // open file for reading only
  QFile f( fn );
  if ( !f.open( IO_ReadOnly ) )
    {
      //      cout << "Error! Could not open file " << fn << " for reading." << endl;
      return false;
    }

  // parse input file
  QTextStream ts( &f );

  // clean temporary Resource File entry
  resetRETemp();

  while ( true )
  {
    QString str;
    str = ts.readLine();
    if ( str.isNull() )
      break;

    // skip the line if it begins with a comment '#'
    if ( str.find( '#' ) == 0 )
      {
        continue;
      }

    // remove any double quotes in the line
    str.remove( '"' );

    // string found if string
    if ( str.contains( "res_name", true ) ||  str.contains( "res_jobtype", true )
         || str.contains( "res_mpiref", true ) ||  str.contains( "res_remote", true )
         || str.contains( "res_request", true ) ||  str.contains( "res_submit", true )
         || str.contains( "res_prepath", true ) ||  str.contains( "res_cluster", true ) )
      {
        // give the line to the parser to determine what to do
        parseResourceLine( str );
      }
  }

  // Resources are only added when a defined value is found
  if ( !m_RETemp.res_name.isEmpty() )
    {
      AddResourceEntry( m_RETemp );
    }

  // close file
  f.close();

  return true;
}


// continue parsing file, ignore errors
void ResourceFile::readResources( bool systemFile )
{
#ifdef DEBUG_RESOURCEFILE
  cout << this << " ResourceFile::readResources()" << endl;
#endif

  QString fn;

  // if HOME is not defined nothing will work here
  fn = getenv( "HOME" );
  if ( fn == NULL )
    {
      cout << "Error! Environment variable HOME not defined." << endl;
      return;
    }

  // do we want the system or the user config file
  if ( systemFile )
    {
      // define path to the gempis system files
      fn = getenv( "IBS_NAMES" );
      if ( fn == NULL )
        {
          cout << "Error! Environment variable IBS_NAMES not defined." << endl;
          return;
        }
      fn += "/gempis/resources_system";
      readResourceFile( fn );

      // find out if there is a regional resource file name
      fn = getenv( "HOME" );
      fn += "/.gempis/definitions_user";
      readDefinitionsFile( fn );
      
      // check if there is local regional resource file
      fn = getenv( "HOME" );
      fn += "/.gempis/resources_";
      fn += m_RegionName;
      if ( readResourceFile( fn ) == false )
        {
          // if the file is not in .gempis, get the system definition
          fn = getenv( "IBS_NAMES" );
          fn += "/gempis/regional_resources/resources_";
          fn += m_RegionName;
          readResourceFile( fn );
        }
    }
  else
    {
      // define path to user's gempis config file
      fn = getenv( "HOME" );
      fn += "/.gempis/resources_user";
      readResourceFile( fn );
    }
}


void ResourceFile::writeResourceFile( QString fn )
{
#ifdef DEBUG_RESOURCEFILE
  cout << this << " ResourcFile::writeResourceFile(" << fn << ")" << endl;
#endif

  // open file for recreation
  QFile f( fn );
  if ( !f.open( IO_WriteOnly | IO_Truncate ) )
    {
      cout << "Error! Could not open file " << fn << " for writing." << endl;
      return;
    }

  // define text stram to file
  QTextStream ts( &f );
  ts.setEncoding( QTextStream::UnicodeUTF8 );

  // write file header
  ts << "# gempis.conf - GEMPIS User resource configuration file" << endl;
  ts << "#" << endl;
  ts << "# Purpose: defines GEMPIS user resources." << endl;
  ts << "#" << endl;
  ts << "# References: See GEMPIS User manual for details." << endl;
  ts << "#" << endl;
  ts << "# This file is executed in a korn shell to define references" << endl;
  ts << "# Lines preceeded by a '#' are treated as comments." << endl;
  ts << "# Not all properties need to be defined." << endl;
  ts << "# Properties with the same index value describe the same object." << endl;
  ts << "#" << endl;
  ts << "# Format: <property>[<index>]=\"<value>\"" << endl;
  ts << "#" << endl;
  ts << "# Only index values from 11 to 50 are permitted." << endl;
  ts << "# Do not use spaces between '=' character." << endl;
  ts << "# Make sure group definitions use different index values." << endl;
  ts << "# System definitions end at index value 10." << endl;
  ts << "#" << endl;
  ts << "# Example: res_jobtype[12]=\"MPICH\"" << endl;
  ts << "#" << endl;
  ts << "# Properties:" << endl;
  ts << "# res_name      - (required) Resource name as used in the gempis command." << endl;
  ts << "# res_jobtype   - (required) Job Type: STANDALONE, LSF, or LSFHPC, or CUSTOM." << endl;
  ts << "# res_mpiref    - (required) MPI Reference name: refers to an entry in the " << endl;
  ts << "#       gempis_MPIRef file." << endl;
  ts << "# res_remote    - Name of remote computer to submit the gempis command." << endl;
  ts << "# res_request   - Identifier to use when requesting resource information." << endl;
  ts << "# res_submit    - Identifier to use when submitting a job." << endl;
  ts << "# res_prepath   - Prepend directory list to PATH (separate entries using the" << endl;
  ts << "#       colon character ':' as used in the PATH)" << endl;
  ts << "# res_cluster   - Cluster list: a list of computer names and number of" << endl;
  ts << "#       processors when Job Type is CUSTOM." << endl;
  ts << "#" << endl;

  // put in data
  for ( unsigned int i=0, j=NUM_RESOURCE_OFFSET; i<m_resourceEntry.size(); i++, j++ )
    {
      if ( m_resourceEntry[i].res_name.isEmpty() )
        {
          // this should not happen
          continue;
        }

      // the resource name must be defined
      ts << "res_name[" << j << "]=\"" << m_resourceEntry[i].res_name << "\"" << endl;

      // fill in further options when defined
      if ( !m_resourceEntry[i].res_jobtype.isEmpty() )
        {
          ts << "res_jobtype[" << j << "]=\"" << m_resourceEntry[i].res_jobtype << "\"" << endl;
        }
      if ( !m_resourceEntry[i].res_mpiref.isEmpty() )
        {
          ts << "res_mpiref[" << j << "]=\"" << m_resourceEntry[i].res_mpiref << "\"" << endl;
        }
      if ( !m_resourceEntry[i].res_remote.isEmpty() )
        {
          ts << "res_remote[" << j << "]=\"" << m_resourceEntry[i].res_remote << "\"" << endl;
        }
      if ( !m_resourceEntry[i].res_request.isEmpty() )
        {
          ts << "res_request[" << j << "]=\"" << m_resourceEntry[i].res_request << "\"" << endl;
        }
      if ( !m_resourceEntry[i].res_submit.isEmpty() )
        {
          ts << "res_submit[" << j << "]=\"" << m_resourceEntry[i].res_submit << "\"" << endl;
        }
      if ( !m_resourceEntry[i].res_prepath.isEmpty() )
        {
          ts << "res_prepath[" << j << "]=\"" << m_resourceEntry[i].res_prepath << "\"" << endl;
        }
      if ( !m_resourceEntry[i].res_cluster.isEmpty() && m_resourceEntry[i].res_cluster.length() > 4 )
        {
          ts << "res_cluster[" << j << "]=\"" << m_resourceEntry[i].res_cluster << "\"" << endl;
        }
   }

  // close file
  f.close();
}


void ResourceFile::writeResources()
{
#ifdef DEBUG_RESOURCEFILE
  cout << this << " ResourceFile::writeResources()" << endl;
#endif

  // determine the path to the gempis resource directory
  QString fn;
  // define path to user's gempis config file
  fn = getenv( "HOME" );
  if ( fn == NULL )
    {
      cout << "Error! Environment variable HOME not defined." << endl;
      return;
    }
  fn += "/.gempis/resources_user";

  writeResourceFile( fn );
}


// exit routine on failure
void ResourceFile::parseResourceLine( QString & str )
{
#ifdef DEBUG_RESOURCEFILE
  cout << this << " ResourceFile::parseResourceLine(" << str << ")" << endl;
#endif

  // determine the index
  QString idx;
  idx = str.section( '[', 1, 1 );
  if ( idx == NULL )
    {
      cout << "Error! Index format error in line [" << str << "]." << endl;
      return;
    }
  idx = idx.section( ']', 0, 0 );
  if ( idx == NULL )
    {
      cout << "Error! Index format error in line [" << str << "]." << endl;
      return;
    }

  // get the assigned value
  QString val;
  val = str.section( '=', 1, 1 );
  if ( val == NULL )
    {
      cout << "Error! Value format error in line [" << str << "]." << endl;
      return;
    }

  // save the result
  if ( str.contains( "res_name", true ) )
    {
      // when this occurs you should save the previous data on the vector
      if ( ! m_RETemp.res_name.isEmpty() )
        {
          // this routine will also check that no duplicates are added
          AddResourceEntry( m_RETemp );
          resetRETemp();
        }
      // the resource name can have the format: <resourceName>.<platform>
      // In this case use only save the resource name.
      m_RETemp.res_name = val.section( '.', 0, 0 );
    }
  else if ( str.contains( "res_jobtype", true ) )
    {
      m_RETemp.res_jobtype = val;
    }
  else if ( str.contains( "res_mpiref", true ) )
    {
      m_RETemp.res_mpiref = val;
    }
  else if ( str.contains( "res_remote", true ) )
    {
      m_RETemp.res_remote = val;
    }
  else if ( str.contains( "res_request", true ) )
    {
      m_RETemp.res_request = val;
    }
  else if ( str.contains( "res_submit", true ) )
    {
      m_RETemp.res_submit = val;
    }
  else if ( str.contains( "res_prepath", true ) )
    {
      m_RETemp.res_prepath = val;
    }
  else if ( str.contains( "res_cluster", true ) )
    {
      m_RETemp.res_cluster = val;
    }
}



