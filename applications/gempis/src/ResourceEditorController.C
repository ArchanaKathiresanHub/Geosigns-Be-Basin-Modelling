///
///
///
/// To do: when resubmitting the application it crashes because 
/// m_commandString is corrupted somewhere. So to avoid this we 
/// reload the job file, reset m_commandString

#include "includestreams.h"

#include <ResourceManagerController.h>
#include <ResourceEditorController.h>
#include <ResourceEditorUI.hpp>

#include <qlineedit.h>
#include <qcombobox.h>
#include <qstring.h>
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

// process info
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>


//
//
ResourceEditorController::ResourceEditorController( ResourceEditorUI * select ) :
  m_contentsChanged( false ),
  m_index( 0 ),
  m_select( select )
{
#ifdef DEBUG_RESOURCEEDITOR
  cout << this << " ResourceEditorController::ResourceEditorController()" << endl;
#endif

  // load only user resources
  m_resourceFile = new ResourceFile();
  m_resourceFile->LoadResources( false );

  // define job types combobox
  QStringList jobtypes;
  jobtypes = m_resourceFile->GetJobTypes();
  m_select->PropertiesJobTypecomboBox->insertItem( "" );
  m_select->PropertiesJobTypecomboBox->insertStringList( jobtypes );

  // define MPI references combobox
  QStringList mpirefs;
  mpirefs = m_resourceFile->GetMPIReferences();
  m_select->PropertiesMPIReferencecomboBox->insertItem( "" );
  m_select->PropertiesMPIReferencecomboBox->insertStringList( mpirefs );

  // reset the other resources
  updateResourceEditorWindow();

  // define activation signal for resource name combobox select
  connect( m_select->ResourcecomboBox, SIGNAL( activated( const QString & ) ),
           this, SLOT( resourceSelected( const QString & ) ) ) ;

  // define activation signal for Add button
  connect( m_select->AddpushButton, SIGNAL( clicked() ), this, SLOT( addClicked() ) ) ;

  // define activation signal for Save button
  connect( m_select->SavepushButton, SIGNAL( clicked() ), this, SLOT( saveClicked() ) ) ;

  // define activation signal for Delete button
  connect( m_select->DeletepushButton, SIGNAL( clicked() ), this, SLOT( deleteClicked() ) ) ;

  // define activation signal for Close button
  connect( m_select->ClosepushButton, SIGNAL( clicked() ), this, SLOT( closeClicked() ) ) ;

  m_select->setModal( true );
  m_select->show();
}


ResourceEditorController::~ResourceEditorController()
{
#ifdef DEBUG_RESOURCEEDITOR
  cout << this << " ResourceEditorController::~ResourceEditorController()" << endl;
#endif

  delete m_resourceFile;
}


void ResourceEditorController::updateResourceEditorWindow()
{
#ifdef DEBUG_RESOURCEEDITOR
  cout << this << " ResourceEditorController::updateResourceEditorWindow()" << endl;
#endif

  // define resource combobox
  QStringList resourceList;
  resourceList = m_resourceFile->GetResourceList();
  m_select->ResourcecomboBox->clear();
  m_select->ResourcecomboBox->insertItem( "" );
  m_select->ResourcecomboBox->insertStringList( resourceList );
  m_select->ResourcecomboBox->setCurrentText( m_resourceEntry.res_name );

  // update display
  m_select->ResourceNamelineEdit->setText( m_resourceEntry.res_name );

  // update head node
  m_select->PropertiesHeadNodelineEdit->setText( m_resourceEntry.res_remote );

  // select job type
  m_select->PropertiesJobTypecomboBox->setCurrentText( m_resourceEntry.res_jobtype );

  // the request line
  m_select->PropertiesJobRequestlineEdit->setText( m_resourceEntry.res_request );

  // the submit line
  m_select->PropertiesJobSubmitlineEdit->setText( m_resourceEntry.res_submit );

  // select MPI Reference
  m_select->PropertiesMPIReferencecomboBox->setCurrentText( m_resourceEntry.res_mpiref );

  // cluster list
  // try to format the data: server_name num_cpus \n
  m_select->PropertiesClusterListtextEdit->clear();
  QString inString = m_resourceEntry.res_cluster.simplifyWhiteSpace();
  QString outString = "";
  for ( int i=0; ; i+=2 )
    {
      // get node name
      QString n;
      n = inString.section( ' ', i, i );
      if ( n.isEmpty() )
        {
          break;
        }

      // get number of cpus
      QString c;
      c = inString.section( ' ', i+1, i+1 );
      // if value is invalid set to 1
      if ( c.toInt() == 0 )
        {
          break;
        }

      // add name and cpus to output
      outString += n + " " + c + '\n';
    }
  m_select->PropertiesClusterListtextEdit->insertParagraph( outString, -1 );
}


void ResourceEditorController::addClicked( void )
{
#ifdef DEBUG_RESOURCEEDITOR
  cout << this << " ResourceEditorController::addClicked()" << endl;
#endif

  // get resource and check for a valid entry
  QString s;
  s = m_select->ResourceNamelineEdit->text();
  if ( s.isEmpty() )
    {
      m_select->showMessage( "No resource defined." );
      return;
    }

  // check that resource name is unique
  long j;
  for ( j=0; j<m_select->ResourcecomboBox->count(); j++ )
    {
      if ( m_select->ResourcecomboBox->text(j) == s )
        {
          break;
        }
    }
  if ( j != m_select->ResourcecomboBox->count() )
    {
      m_select->showMessage( "Resource name already in use, change name or click Save." );
      return;
    }

  // verify that user want this entruy added 
  int result = m_select->askQuestion( "Add this resource?" );
  if ( result )
    {
      return;
    }

  // indicate that the contents have changed
  m_contentsChanged = true;

  // load resource values
  getResourceFromScreen();

  // add resource to file
  m_resourceFile->AddResourceEntry( m_resourceEntry );

  // refresh resource list
  updateResourceEditorWindow();
}


void ResourceEditorController::saveClicked( void )
{
#ifdef DEBUG_RESOURCEEDITOR
  cout << this << " ResourceEditorController::saveClicked()" << endl;
#endif

  // get the current name used
  QString s;
  s = m_select->ResourcecomboBox->currentText();
  if ( s.isEmpty() )
    {
      m_select->showMessage( "Resource not defined. Click Add." );
      return;
    }

  int result = m_select->askQuestion( "Replace current resource?" );
  if ( result )
    {
      return;
    }

  // indicate that the contents have changed
  m_contentsChanged = true;

  // load resource values
  getResourceFromScreen();

  // modify resource entry
  m_resourceFile->ModifyResourceEntry( s, m_resourceEntry );

  // refresh resource list
  updateResourceEditorWindow();
}


void ResourceEditorController::deleteClicked( void )
{
#ifdef DEBUG_RESOURCEEDITOR
  cout << this << " ResourceEditorController::deleteClicked()" << endl;
#endif

  // get the current name used
  QString s;
  s = m_select->ResourceNamelineEdit->text();
  if ( s.isEmpty() )
    {
      cout << "Resource Editor: attempting to save an empty resource" << endl;
      return;
    }

  int result = m_select->askQuestion( "Delete this resource?" );
  if ( result )
    {
      return;
    }

  // indicate that the contents have changed
  m_contentsChanged = true;

  // delete resource file entry
  m_resourceFile->DeleteResourceEntry( s );

  // set to blank resource
  resourceSelected( "" );
}

void ResourceEditorController::closeClicked( void )
{
#ifdef DEBUG_RESOURCEEDITOR
  cout << this << " ResourceEditorController::closeClicked()" << endl;
#endif

  if ( m_contentsChanged )
    {
      int result = m_select->askQuestion( "Save changes?" );
      if ( ! result )
        {
          // save entries to file
          m_resourceFile->SaveResources();
        }
    }

  // close window
  m_select->close();
  // make sure that nothing else happens after this function
}

void ResourceEditorController::resourceSelected( const QString &s )
{
#ifdef DEBUG_RESOURCEEDITOR
  cout << this << " ResourceEditorController::resourceSelected(" << s << ")" << endl;
#endif

  // if resource is empty return
  if ( ! s.isEmpty() )
    {
      // get the associated entry
      QString t = s;
      m_resourceFile->GetResourceEntry( t, & m_resourceEntry );
    }
  else
    {
      // clear data
      m_resourceEntry.res_name = "";
      m_resourceEntry.res_remote = "";
      m_resourceEntry.res_jobtype = "";
      m_resourceEntry.res_request = "";
      m_resourceEntry.res_submit = "";
      m_resourceEntry.res_mpiref = "";
      m_resourceEntry.res_cluster = "";
    }

  // update display
  updateResourceEditorWindow();
}


void ResourceEditorController::getResourceFromScreen()
{
#ifdef DEBUG_RESOURCEEDITOR
  cout << this << " ResourceEditorController::getResourceFromScreen()" << endl;
#endif

  // don't allow a resource name with a space
  m_resourceEntry.res_name = m_select->ResourceNamelineEdit->text();
  m_resourceEntry.res_name.remove( ' ' );

  // jobtype
  m_resourceEntry.res_jobtype = m_select->PropertiesJobTypecomboBox->currentText();
  m_resourceEntry.res_jobtype.simplifyWhiteSpace();

  // MPI reference
  m_resourceEntry.res_mpiref = m_select->PropertiesMPIReferencecomboBox->currentText();
  m_resourceEntry.res_mpiref.simplifyWhiteSpace();

  // head node
  m_resourceEntry.res_remote = m_select->PropertiesHeadNodelineEdit->text();
  m_resourceEntry.res_remote.simplifyWhiteSpace();

  // request option
  m_resourceEntry.res_request = m_select->PropertiesJobRequestlineEdit->text();
  m_resourceEntry.res_request.simplifyWhiteSpace();

  // submit option
  m_resourceEntry.res_submit = m_select->PropertiesJobSubmitlineEdit->text();
  m_resourceEntry.res_submit.simplifyWhiteSpace();

  // not used
  m_resourceEntry.res_prepath = "";

  // replace any carriage returns with spaces
  QString s;
  s = m_select->PropertiesClusterListtextEdit->text();
  s.replace( '\n', " " );
  s.stripWhiteSpace();
  m_resourceEntry.res_cluster = s;
}

