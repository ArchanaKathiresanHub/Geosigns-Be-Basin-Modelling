/*********************************************************************
*                                                                     
* Package:        GEneric MPI Resource Manager (gempir)
*
* Dependencies:   runs on Linux (64-bit) platforms; 
*                 Qt windowing system; 
*
* Usage Notes:    See Administrators help 
*
*********************************************************************/

/// Command Thread handles a separate thread for detecting
/// 
/// 
/// When it receives a command it sends it to a 
/// 

#include "includestreams.h"  // platform type definitions

// project include files
#include <ResourceManagerController.h>
#include <ResourceManagerUI.hpp>
#include <GempisThread.h>
#include <AppTimer.h>

// Qt include files
#include <qapplication.h> 
#include <qcombobox.h>
#include <qlineedit.h> 
#include <qmessagebox.h> 
#include <qpopupmenu.h> 
#include <qpushbutton.h> 
#include <qstring.h>
#include <qstringlist.h>
#include <qtextedit.h> 
#include <qtimer.h> 
#include <qvaluelist.h> 
#include <qfile.h>
#include <qtextstream.h>
#include <qlistview.h>
#include <qstatusbar.h>


#include <string>
#include <stdio.h>   // putenv
#include <unistd.h>  // sleep



///////////////////////////////////////////////////////////////////////
//
//
//
ResourceManagerController::ResourceManagerController( ResourceManagerUI * select ) :
  m_resource_error( false ),
  m_resourceName( "" ),
  m_processorsTotal( "0" ),
  m_processorsAvailable( "" ),
  m_processorsMemoryper( "" ),
  m_envstring( "" ),
  m_resourceLog( "resources.log" ),
  m_resourceInfoRequested( "" ),
  m_resourceList(),
  m_resourceThread( 0 ),
  m_select( select ),
  m_selectedJobid( "" )
{
#ifdef DEBUG_GEMPIR
  cout << this << " ResourceManagerController::ResourceManagerController()" << endl;
#endif

 // disable values display for resources
  enableResourceInfo( false );

  // update window
  DisplayWindow();

 // define activation signal for resource name combobox select
  connect( m_select->ResourceNamecomboBox, SIGNAL( activated( const QString & ) ),
           this, SLOT( nameSelected( const QString & ) ) ) ;

  // define activation signal for close button
  connect( m_select->ClosepushButton, SIGNAL( clicked() ), 
           this, SLOT( closeClicked() ) ) ;

  // define activation signal for right click
  /*
  connect( m_select->ResourceProcessorsInUsebylistView, 
           SIGNAL( rightButtonClicked(QListViewItem*, const QPoint &, int) ), 
           this, SLOT( listViewItemSelected(QListViewItem*, const QPoint &, int) ) ) ;
  */
  // define popup menu in in use by window
  m_inUseByActionsMenu = new QPopupMenu (m_select->ResourceProcessorsInUsebylistView);
  m_inUseByActionsMenu->insertItem (tr ("Kill Job"), this, SLOT (killSelected()));

  connect( m_select->ResourceProcessorsInUsebylistView, 
           SIGNAL (contextMenuRequested (QListViewItem *, const QPoint &, int)),
           this, SLOT( listViewItemSelected(QListViewItem*, const QPoint &, int) ) ) ;

  // start time to update resource info display every second
  m_appTimer = new AppTimer( this, SLOT(updateWindow()) );
  m_appTimer->Set( 1000 );
  m_appTimer->Start();

  // if resource name given in command line
  if ( qApp->argc() > 1 ) 
    {
      // force name selected
      nameSelected( qApp->argv()[1] );
    }
}


///////////////////////////////////////////////////////////////////////
//
//
//
ResourceManagerController::~ResourceManagerController()
{
#ifdef DEBUG_GEMPIR
  cout << this << " ResourceManagerController::~ResourceManagerController()" << endl;
#endif

  // if a resource request has been defined
  if ( m_resourceThread != 0 )
    {
      stopResourceRequest();
    }

  // stop timer
  delete m_appTimer;
}

// signal handler when list item has been selected
void ResourceManagerController::listViewItemSelected(QListViewItem *item, const QPoint &point, int )
{
#ifdef DEBUG_GEMPIR
  cout << this << " ResourceManagerController::listViewItemSelected()" << endl;
#endif

  if (item)
   {
     // save job id and popup menu
     m_selectedJobid = item->text( 0 );
     m_inUseByActionsMenu->popup (point);
   }
}


// signal handler when list item has been selected
void ResourceManagerController::killSelected( void )
{
#ifdef DEBUG_GEMPIR
  cout << this << " ResourceManagerController::killSelected()" << endl;
#endif

  // give the user a message
  QMessageBox * ptr = new QMessageBox( "Kill Job on Resource", 
                                       "Do you wish to terminate job " 
                                       + m_selectedJobid
                                       + " on resource "
                                       + m_resourceName
                                       + "? \n\n(This will only work if you "
                                       + "are the owner of the job.)",
                                       QMessageBox::Question, 
                                       QMessageBox::Yes, QMessageBox::No, 
                                       QMessageBox::NoButton  );
  // run gempis
  int ans;
  ans = ptr->exec();
  delete ptr;

  // spawn a termination if they said yes
  if ( ans == QMessageBox::Yes )
    {
      m_select->statusBar()->message( "Attempting to terminate job " 
                                      + m_selectedJobid + " on " + m_resourceName 
                                      + ". View the window for the result.", 
                                      5000 );
      spawnKillRequest( m_selectedJobid );
    }

}

///////////////////////////////////////////////////////////////////////
//
// display of window project once the project file and resources has been read 
//
void ResourceManagerController::DisplayWindow( void )
{
#ifdef DEBUG_GEMPIR
  cout << this << " ResourceManagerController::DisplayWindow()" << endl;
#endif

  // load  all resources: system and user
  ResourceFile * resourceData = new ResourceFile();
  resourceData->LoadResources( true );
  m_resourceList = resourceData->GetResourceList();

  // display resource names if available
  if ( m_resourceList.isEmpty() != true )
    {
      m_select->ResourceNamecomboBox->clear();

      // add a blank string here
      m_select->ResourceNamecomboBox->insertItem( "" );

      m_select->ResourceNamecomboBox->insertStringList( m_resourceList );
    }

  // remove loaded resource data
  delete resourceData;

  // if resource name is defined use it
  if ( !m_resourceName.isEmpty() )
    {
      m_select->ResourceNamecomboBox->setCurrentText( m_resourceName );
    }
}

///////////////////////////////////////////////////////////////////////
//
// timer based window update
//
void ResourceManagerController::updateWindow( void )
{
#ifdef DEBUG_GEMPIR
  //  cout << this << " ResourceManagerController::updateWindow()" << endl;
#endif

  parseUpdateResourceInfo();

  // schedule resource resuest if needed
  scheduleResourceUpdate();

  // restart to update myself
  m_appTimer->Start();
}


///////////////////////////////////////////////////////////////////////
//
//
//
void ResourceManagerController::enableResourceInfo( bool isOn )
{
#ifdef DEBUG_GEMPIR
  cout << this << " ResourceManagerController::enableResourceInfo(" << isOn << ")" << endl;
#endif

  // disable values display for resources
  m_select->ResourceProcessorsProcessorsTotallineEdit->setEnabled( isOn );
  m_select->ResourceProcessorsProcessorsAvailablelineEdit->setEnabled( isOn );
  m_select->ResourceProcessorsProcessorsMemorylineEdit->setEnabled( isOn );
  m_select->ResourceProcessorsInUsebylistView->setEnabled( isOn );
}


///////////////////////////////////////////////////////////////////////
//
//
//
void ResourceManagerController::parseUpdateResourceInfo( void )
{
#ifdef DEBUG_GEMPIR
  //  cout << this << " ResourceManagerController::parseUpdateResourceInfo()" << endl;
#endif

  // check the child has terminated
  if ( m_resourceInfoName.isEmpty() )
    {
      // clear fields
      m_select->ResourceProcessorsProcessorsTotallineEdit->clear();
      m_select->ResourceProcessorsProcessorsAvailablelineEdit->clear();
      m_select->ResourceProcessorsProcessorsMemorylineEdit->clear();
      m_select->ResourceProcessorsInUsebylistView->clear();
      m_processorsTotal = "";
      m_processorsAvailable = ""; 
      m_blockOn = false;
    }
  else
    {
      while ( m_resourceThread->GetOutputAvailable() == true )
        {
          QString str;
          str = m_resourceThread->GetNextOutputLine();
          if ( str.isEmpty() )
            {
              continue;
            }
          QStringList field = QStringList::split( " ", str );
          if ( field.count() <= 0 )
            {
              continue;
            }
             
          // Check for special message formats
          if ( field.count() > 1 && field[0] == "MeSsAgE" && field[1] == "DATA" )
            {
              if ( field.count() > 3 )
                {
                  if ( field[2] == "MEMORYPER" )
                    {
                      m_select->ResourceProcessorsProcessorsMemorylineEdit->setText( field[3] );
                      m_processorsMemoryper = field[3];
                      // this field also determines if the resource is valid
                      if ( m_processorsMemoryper == "?" )
                        {
                          m_resource_error = true;
                        }   
                      else
                        {
                          m_resource_error = false;
                        }   
                    }
                  else if ( field[2] == "TOTAL" )
                    {
                      m_select->ResourceProcessorsProcessorsTotallineEdit->setText( field[3] );
                      m_processorsTotal = field[3];
                    }
                  else if ( field[2] == "AVAILABLE" )
                    {
                      m_select->ResourceProcessorsProcessorsAvailablelineEdit->setText( field[3] );
                      m_processorsAvailable = field[3]; 
                     }
                }
            }
          else if ( field[1] == "BLOCK" )
            {
              m_blockOn = true;
              m_errorMessage = "";
              
              // clear the window with new data
              m_select->ResourceProcessorsInUsebylistView->clear();    
            }
          else if ( field[1] == "BLOCKEND" )
            {
              m_blockOn = false;
              
              // update last update window with current time
              QDateTime dt;
              dt = m_resourceThread->GetLastUpdate();
              m_select->ResourceUpdatedlineEdit->setText( dt.toString( "hh:mm:ss" ) );

              if ( m_resource_error == true )
                {
                  // update status bar with any error messages
                  m_select->statusBar()->message( m_errorMessage, 60000 );
                }
            }
          // redirect output to InUseBy window
          else if ( m_blockOn == true )
            {
              if ( m_resource_error == true )
                {
                  if ( field[0] != "MeSsAgE" && field[0] != "JOBID" )
                    {
                      // this error message is directed to the status line
                      m_errorMessage += str + " ";
                    }
                }
              else
                {
                  // the first line contains the job header
                  if ( field[0] != "JOBID" && field[0] != "ID" )
                    {
                      // append text to current end
                      QString jid= "";
                      QString jstate = "";
                      QString jnp = "";
                      QString juid = "";
                      QString jcmd = "";
                      QString jdate = "";
                      if ( field.count() > 0 )
                        {
                          jid = field[0];
                        }
                      if ( field.count() > 1 )
                        {
                          jstate = field[1];
                        }
                      if ( field.count() > 2 )
                        {
                          jnp = field[2];
                        }
                      if ( field.count() > 3 )
                        {
                          juid = field[3];
                        }
                      if ( field.count() > 4 )
                        {
                          jcmd = field[4];
                        }
                      if ( field.count() > 7 )
                        {
                          jdate = field[5] + " " + field[6] + " " + field[7];
                        }
                      // add line
                      new QListViewItem( m_select->ResourceProcessorsInUsebylistView, 
                                         jid, jstate, jnp, juid, jcmd, jdate );
                    }
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////
//
// activated when the resource name have been selected from the drop
// down list
//
void ResourceManagerController::nameSelected( const QString & s )
{
#ifdef DEBUG_GEMPIR
  cout << this << " ResourceManagerController::nameSelected(" << s << ")" << endl;
#endif

  m_resourceNameSelected = s;

  // a failed resource request would have already terminated itself
  if ( m_resource_error == true )
    {
      m_resourceName = "";
    }

  // wipe out any previous displays
  m_select->ResourceProcessorsProcessorsMemorylineEdit->setText( "" );
  m_select->ResourceProcessorsProcessorsTotallineEdit->setText( "" );
  m_select->ResourceProcessorsProcessorsAvailablelineEdit->setText( "" );
  m_select->ResourceProcessorsInUsebylistView->clear();

  // blank status bar of any previous messages
  m_select->statusBar()->message( "" );

  // clear previous values
  enableResourceInfo( true );
}

void ResourceManagerController::scheduleResourceUpdate( void )
{
#ifdef DEBUG_GEMPIR
  //  cout << this << " ResourceManagerController::scheduleResourceUpdate()" << endl;
#endif

  // don't do anything if the name is the same, 
  // this is needed so it doesn't constatly try to restart itself
  if ( m_resourceNameSelected == m_resourceName )
    {
      return;
    }

  // clear last updated field
  m_select->ResourceUpdatedlineEdit->clear();

  // if a name was previously defined kill the resourceInfo child
  if ( ! m_resourceName.isEmpty() )
    {
      stopResourceRequest();
      enableResourceInfo( false );
   }

  // set the new resource name
  m_resourceName = m_resourceNameSelected;

  // set resource name combox box to that resource
  m_select->ResourceNamecomboBox->setCurrentText( m_resourceName );

  // clear fields
  parseUpdateResourceInfo();

  // if name is known, and valid, allow update of resourceInfo
  if ( ! m_resourceName.isEmpty() )
    {
      // reset log files
      LogFile * lf;
      lf = new LogFile( m_resourceLog );
      //      lf->Reset();
      // put a header on the project file
      lf->WriteLine( "//////////////////////////////////////////////////////////////" );
      lf->Write( "-ReSoUrCe Log File:     " );
      lf->WriteLine( m_resourceLog );
      char hostnm[ 128 ];
      gethostname( hostnm, 124 );
      lf->Write( "-ReSoUrCe Hostname:     " );
      lf->WriteLine(  hostnm );
      QDateTime dt;
      dt = QDateTime::currentDateTime();
      lf->Write( "-ReSoUrCe Started on:   " );
      lf->WriteLine( dt.toString( "ddd MMMM d, yyyy hh:mm:ss" ) );
      lf->Write( "-ReSoUrCe Name:         " );
      lf->WriteLine(  m_resourceName );
      delete lf;

      // enable values display for resources
      enableResourceInfo( true );

      // start the resource update
      startResourceRequest();
    }
}


///////////////////////////////////////////////////////////////////////
//
//
//
void ResourceManagerController::startResourceRequest( void )
{
#ifdef DEBUG_GEMPIR
  cout << this << " ResourceManagerController::startResourceRequest()" << endl;
#endif

  // create gempis data and job thread
  m_resourceThread = new GempisThread();

  // this value is used by the ResourceInfo members for the active resource name
  m_resourceInfoName = m_resourceName;

  // add version number if defined
  QString cmd;
  cmd = "gempis ";
  char * cptr;
  cptr = getenv( "IBS_VERSION" );
  if ( cptr != NULL && strlen(cptr) > 1 )
  {
    cmd += "-";
    cmd += cptr;
    cmd += " ";
  } 
  cmd += m_resourceName.latin1();
  cmd += " loop";
  cmd += "</dev/null";

  // define the Gempis command and log file
  m_resourceThread->SetCommand( cmd );
  m_resourceThread->SetLogFileName( m_resourceLog );

  // calling in this way runs the command in the background
  m_resourceThread->start();
}


///////////////////////////////////////////////////////////////////////
//
// terminate job
//
// this command is spawnwed and we don't care the result
void ResourceManagerController::spawnKillRequest( const QString & jid )
{
#ifdef DEBUG_GEMPIR
  cout << this << " ResourceManagerController::spawnKillRequest(" << jid << ")" << endl;
#endif

  QString cmd;
  cmd = "xterm -hold -e 'gempis ";
  char * cptr;
  cptr = getenv( "IBS_VERSION" );
  if ( cptr != NULL && strlen(cptr) > 1 )
  {
    cmd += "-";
    cmd += cptr;
    cmd += " ";
  } 
  cmd += " res ";
  cmd += m_resourceName.latin1();
  cmd += " kill ";
  cmd += jid.latin1();
#ifndef DEBUG_GEMPIR
  // when debugging we WANT to see the output
  cmd += " | grep -v VERBOSE";
#endif
  cmd += "' &";

  system( cmd );
}


///////////////////////////////////////////////////////////////////////
//
//
void ResourceManagerController::stopResourceRequest( void )
{
#ifdef DEBUG_GEMPIR
  cout << this << " ResourceManagerController::stopResourceRequest()" << endl;
#endif
 
  // kill the resource thread
  m_resourceThread->KillThread();

  // Wait for termination
  while ( m_resourceThread->GetRunning() == true )
    {
#ifdef DEBUG_GEMPIR
      cout << this << " ResourceManagerController::stopResourceRequest() thread still running." << endl;
#endif
      sleep( 1 );
    }

#ifdef DEBUG_GEMPIR
  cout << this << " ResourceManagerController::stopResourceRequest() request terminated." << endl;
#endif

  // blank and disable display
  m_resourceInfoName = "";
  parseUpdateResourceInfo();
  enableResourceInfo( false );

  // remove thread
  delete m_resourceThread;

  // indicate that thread is not used
  m_resourceThread = 0;
}


///////////////////////////////////////////////////////////////////////
//
// send an e-mail to helpdesk with project log file
int ResourceManagerController::SendEmail( void )
{
#ifdef DEBUG_GEMPIR
  cout << this << " ResourceManagerController::SendEmail()" << endl;
#endif

  // now tell gempis to send email
  // create gempis job thread
  GempisThread * gempisThread = new GempisThread();

  // define the command to email the log
  QString command;
  command = "gempis ";
  char * cptr;
  cptr = getenv( "IBS_VERSION" );
  if ( cptr != NULL && strlen(cptr) > 1 )
    {
      command += "-";
      command += cptr;
    }
  command += " adm ";
  command += " email ";
  command += m_resourceLog;

  gempisThread->SetCommand( command );
  gempisThread->SetLogFileName( "Email.log" );

  // calling in this way runs the command in the foreground
  gempisThread->run();

  int result = gempisThread->GetResult();

  // remove local data
  delete gempisThread;

  return result;
}


///////////////////////////////////////////////////////////////////////
//
//
//
void ResourceManagerController::closeClicked( void )
{
#ifdef DEBUG_GEMPIR
  cout << this << " ResourceManagerController::closeClicked()" << endl;
#endif

  // turn off timer
  m_appTimer->Stop();

  m_select->close();
  // make sure that nothing else happens after this function
}
