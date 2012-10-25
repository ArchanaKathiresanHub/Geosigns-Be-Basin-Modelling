//
// Main Gui for Trap Tracking tool
// Takes in user entry for choice of reservoir, trap id and age of trap
// Also, generates line graphs, re-reads migration history and generates trap history data 
// based on user request through gui buttons
//

#ifndef __traptrackingform__
#define __traptrackingform__

#include "filestatus.h"
#include "traptrackingkernel.h"
#include "readonlydatabase.h"
#include "graphobjectslist.h"
#include "global_typedefs.h"
#include "pipemessage.h"
#include "environmentsettings.h"
#include "trapchangeevent.h"
#include "commandthread.h"

#include <qfiledialog.h>
#include <qmenubar.h>
#include <qsplashscreen.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qmessagebox.h>

#include <fstream.h>
#include <time.h>

//
// CTOR / DTOR (init/destroy)
//
void TrapTrackingForm::init()
{
   setUserRequestNotUpToDate ();
   m_kernel.setGui (this);
   createMainMenu ();
   createHelpMenu();
   m_openProjectFileDialog = 0;
   m_commandThread = 0;
   m_userSettings = new UserSettings ();
   //createMessageBox ();
   setShowErrors (true);
}

// not naming this function destroy because destroy is only called from hide
// if child processes are not running, so this is a more general function
// to tidy up everything, used when being destroyed
void TrapTrackingForm::tidyUp()
{  
   if ( m_openProjectFileDialog ) delete m_openProjectFileDialog;
  
   deleteMainMenu ();
   setUserRequestNotUpToDate ();
   
   // make sure all graphs have been /deleted
   closeGraphs ();
   
   closeCommandThread ();
   
   if ( m_userSettings) delete m_userSettings;
}

//
// Execute Command line arguments
//
void TrapTrackingForm::executeCommandLineArgs (int argc, char **argv)
{ 
   setShowErrors (false);
   
   // parse command line arguments
   for ( int i=0; i < argc; ++i )
   {
      if ( strcmp (argv[i], "-r") == 0 )
      {
         string resName = getReservoirArgument (argv, argc, i);
         setReservoirName (resName.c_str());
      }
      else if ( strcmp (argv[i], "-p") == 0 )
      {
         setProjectName (argv[i+1]);
      }
      else if ( strcmp (argv[i], "-a") == 0 )
      {
         setUserAge (argv[i+1]);
      }
      else if ( strcmp (argv[i], "-t") == 0 )
      {
         setUserTrapId (argv[i+1]);
      }
      else if ( strcmp (argv[i], "-m") == 0 )
      {
         createCommandThread ();
      }
   }
   
   // switch errors back on  
   setShowErrors (true);
   
   // a new trap id has been selected or set 
   // so execute appropriate functionality   
   trapIdEditedSlot(); 
 }

string TrapTrackingForm::getReservoirArgument (char **argv, int argc, int index)
{
   // assume first arg after flag is reservoir name
   string resName = argv[++index]; 

   while ( ++index < argc )
   {  
      if ( isFlag (argv[index]) ) break;

      // the following arg is not a flag so must be further part of the name
      resName += " ";
      resName += argv[index]; 
   }

   return resName;
}

bool TrapTrackingForm::isFlag (const char *str)
{
   return 
         (strcmp (str, "-p") == 0) ||
         (strcmp (str, "-r")  == 0) ||
         (strcmp (str, "-a")  == 0) ||
         (strcmp (str, "-t")  == 0) ||
         (strcmp (str, "-m") == 0); 
}

//
// SET AND GET FUNCTIONS FOR USER DATA
//
void TrapTrackingForm::setProjectName( const char* name )
{ 
   // check for previously opened graphs
   // and ask user if want to go ahead even tho
   // graphs will be closed
   if ( ! closeGraphsIfUserRequest () ) return;
   
   projNameLabel->setText (name);  
   m_projectNameStr = name;

   // set output directory name based on project name
   setOutputDirectoryName (createStdOutputDirName (name));
 
   refreshAll ();
}

void TrapTrackingForm::refreshAll()
{  
   // create splash screen
   QPixmap pic (QPixmap::fromMimeSource ("trapper_splash_screen.png"));
   QSplashScreen *splash = new QSplashScreen (pic);
   splash->show ();
   splash->message (QObject::tr("Opening Project File..."), Qt::AlignRight | Qt::AlignBottom, Qt::red);

   // reset persistent traps if already created
   m_kernel.clearPersisTraps ();
   
   // open database corresponding to new project file
   if ( openGraphDatabase () )
   {
      // check project file version
      if ( m_kernel.checkProjectVersion () )
      {      
         // update splash screen
         splash->message (QObject::tr("Creating Trap Id History..."), Qt::AlignRight | Qt::AlignBottom, Qt::red);

         // update input data
         refreshMigrationHistory();
         generateTrapIdHistory();

         // update splash screen
         splash->message (QObject::tr("Reading User Settings..."), Qt::AlignRight | Qt::AlignBottom, Qt::red);
 
         // set up user settings object
         m_userSettings->setProjectName (m_projectNameStr.c_str());
      }
      else
      {
         showMessage ("Trapper is incompatable with this project file.\nMigration must be run with Cauldron 2005.02 or later");
      }
   }
   else
   {
      char *msg = new char [m_projectNameStr.size() + 50];
      sprintf(msg, "Cannot open project file %s\n", m_projectNameStr.c_str());
      showMessage (msg);
      delete []msg;
   }
   
   // rebuild reservoir drop down lists if any exist
   bool showErrorsState = showErrors ();
   setShowErrors (false);   
   buildReservoirComboBox ();
   setShowErrors (showErrorsState);

   // delete splash screen
   splash->finish (this);
   delete splash;
}

string& TrapTrackingForm::getProjectNameStr()
{
   return m_projectNameStr;
}

void TrapTrackingForm::setReservoirName( const char* name )
{
   string reservoirName (name);
   m_kernel.convertUstoSpace (reservoirName);
   setComboItem (reservoirComboBox, reservoirName.c_str ());
   reservoirEditSlot ();
}

string& TrapTrackingForm::getReservoirNameStr ()
{
   return m_reservoirStr;
}

void TrapTrackingForm::setOutputDirectoryName( const char* name )
{
   m_outputDirStr = name;
}

string& TrapTrackingForm::getOutputDirectoryNameStr()
{
   return m_outputDirStr;
}

void TrapTrackingForm::setUserAge( const char* name )
{
   setComboItem (ageComboBox, name);
   ageEditSlot ();
}

double TrapTrackingForm::getUserAge()
{
   return (ageComboBox->currentText().isEmpty()) 
         ? 0 : atof (ageComboBox->currentText());
}

string& TrapTrackingForm::getUserAgeStr()
{
   return m_ageStr;
}

void TrapTrackingForm::setUserTrapId( const char* name )
{
   setTrapIdComboItem (trapIdComboBox, name);
}

int TrapTrackingForm::getUserTrapId()
{
   if ( trapIdComboBox->currentText ().isEmpty() )
   {
      return 0;
   }
   
   string tempId = trapIdComboBox->currentText ().ascii();
   return atoi (tempId.substr (0, tempId.find_first_of(' ')).c_str());
}

string& TrapTrackingForm::getUserTrapIdStr()
{
   return m_trapIdStr;
}

void TrapTrackingForm::setTrapHistMod()
{
   const time_t curr_time = time (NULL);     
   const char *timeStr = ctime(&curr_time);
   trapHistGenLabel->setText( timeStr );
}

void TrapTrackingForm::setMigrationHistFileMod()
{
   // set up the file status of the project file
   FileStatus f (projNameLabel->text ().ascii ());
   
   if ( f.error () )
   {
      QString msg ("Error. Incorrect filename: ");
      msg += projNameLabel->text ();
      showMessage (msg);
      return;
   }
   
   // get the date modified
   char* date_name = f.dateMod();
   date_name[strlen(date_name)-1] = '\0';
   
   // set the date modified in the gui string
   cauldronProjectModLabel->setText (date_name);
   update ();
}

void TrapTrackingForm::trapIdEditedSlot()
{
   trapIdUpdate();

   string error = m_kernel.refreshPropertyGraph();
   error += m_kernel.refreshComponentGraph();
   error += m_kernel.refreshSourceGraph();
   
   if ( error.size() > 0 && showErrors () )
   {
      showMessage (error.c_str());
   }
}

void TrapTrackingForm::trapIdUpdate()
{
   m_trapIdStr = trapIdComboBox->currentText().isEmpty() 
                 ? "" 
              : trapIdComboBox->currentText().ascii();
   
   setUserRequestNotUpToDate ();
}

void TrapTrackingForm::ageEditSlot()
{ 
   // hold on to previous trap id
   int previousTrapId = getUserTrapId ();
   
   // get selected age
   m_ageStr = ageComboBox->currentText().isEmpty() ? "" : ageComboBox->currentText().ascii();
   buildTrapIdComboBox ();
   setUserRequestNotUpToDate ();
   
   // find trap corresponding to selected age
   int persisId = m_kernel.getPersisId (m_reservoirStr, m_previousAge, previousTrapId);
   int transId = m_kernel.getTransId (m_reservoirStr, getUserAge(), persisId);
   
   if ( transId == -1 && showErrors () )
   {
      showMessage ("The current trap does not exist at this age. Resetting the trap ID to 1");
   }
   
   // set trap to corresponding trap for selected age
   setTrapIdComboItem (trapIdComboBox, transId);
         
   // update graphs 
   trapIdEditedSlot();
}

void TrapTrackingForm::ageHighlightedSlot()
{
   m_previousAge = atof (ageComboBox->currentText());
}

void TrapTrackingForm::reservoirEditSlot()
{
   m_reservoirStr = reservoirComboBox->currentText().isEmpty() ? "" : reservoirComboBox->currentText().ascii();
   buildAgeComboBox();
   setUserRequestNotUpToDate ();
}

UserSettings* TrapTrackingForm::userSettings()
{
   return m_userSettings;
}

//
// BUTTON FUNCTIONALITY
//
void TrapTrackingForm::refreshInputDataSlot()
{
   refreshAll ();
}

void TrapTrackingForm::closeAllGraphsSlot()
{
   closeGraphs ();
}

void TrapTrackingForm::refreshMigrationHistory()
{
   setMigrationHistFileMod();
   setUserRequestNotUpToDate ();
   update ();   
}

void TrapTrackingForm::generateTrapIdHistory()
{ 
   string errorMsg = m_kernel.generatePersisTraps ();
   
   if ( errorMsg.size () > 0 )
   {
      showMessage (errorMsg.c_str());
   }
   else
   {
      setTrapHistMod();
      setUserRequestNotUpToDate ();
   }
}

void TrapTrackingForm::userSettingsSlot()
{
   m_userSettings->show ();
}

//
// DATA UPDATE FLAG SET AND GET
//
bool TrapTrackingForm::userRequestUpToDate()
{
   return m_userRequestUpToDate;
}

bool TrapTrackingForm::userRequestNotUpToDate()
{
   return ! m_userRequestUpToDate;
}

void TrapTrackingForm::setUserRequestNotUpToDate()
{
   m_userRequestUpToDate = false;
}

void TrapTrackingForm::setUserRequestUpToDate()
{
   m_userRequestUpToDate = true;
}

void TrapTrackingForm::setShowErrors( bool value )
{
   m_showErrors = value;
}

bool TrapTrackingForm::showErrors()
{
   return m_showErrors;
}

//
// GRAPH SELECTION FUNCTIONS
//
bool TrapTrackingForm::openGraphDatabase()
{
   ReadOnlyDatabase *dbase = ReadOnlyDatabase::getReadOnlyDatabase ();
   return dbase->createDatabase (getProjectNameStr ().c_str());
}

void TrapTrackingForm::graphSelectedSlot()
{
   int opt = SelectGraphComboBox->currentItem ();
   
   switch ( opt )
   {
   case PROP_GRAPH:
      displayPropGraph ();
      break;

      // Note; This is a dirty change. I can't find the reference to 
      // the enum COMP_GRAPH entry. So the second element will be 
      // interpreted as SRC_GRAPH
   case SRC_GRAPH:
      displaySrcGraph ();
      break;
   case COMP_GRAPH:
      displayCompGraph ();
      break;
   default:
      break;
   }
}

void TrapTrackingForm::displayPropGraph()
{  
   string error = m_kernel.createPropertyGraph ();
   
   if ( error.size() > 0 )
   {
      showMessage (error.c_str());
   }
}

void TrapTrackingForm::displayCompGraph ()
{
   string error = m_kernel.createComponentsGraph ();
   
   if ( error.size() > 0 )
   {
      showMessage (error.c_str());
   }
}

void TrapTrackingForm::displaySrcGraph()
{   
   string error = m_kernel.createSourceGraph ();
   
   if ( error.size() > 0 )
   {
      showMessage (error.c_str());
   }
}

//
// Overwriting hide rather than close as hide
// is called for every possible exit
//
void TrapTrackingForm::hide()
{
   tidyUp ();
   QWidget::hide();
  
   // brute force as seems to be a problem closing sometimes on the SUN
   // and not exactly sure why
   exit (0);
}

void TrapTrackingForm::closeGraphs ()
{
   // close outstanding graphs
   GraphObjectsList *graphList = GraphObjectsList::getInstance ();
   if ( (graphList->graphCount ()) > 0 ) graphList->deleteAllGraphs ();
}

bool TrapTrackingForm::closeGraphsIfUserRequest ()
{
   bool goahead = true;
   
   GraphObjectsList *graphList = GraphObjectsList::getInstance ();
   
   if ( (graphList->graphCount ()) > 0 )
   {
      bool answer = showQuestion ("All graphs will be closed. Continue?");
      if ( answer )
      {
         graphList->deleteAllGraphs ();
      }
      else
      {
         goahead = false;
      }
   }
   
   return goahead;
}

//
// MENU FUNCTIONS
//
void TrapTrackingForm::createMainMenu()
{
   // create main menu 
   m_mainMenu = new QMenuBar (this);
   
   // create open project menu
   m_openFileMenu = new QPopupMenu (this);
   m_openFileMenu->insertItem ("Open Project", this, SLOT(openProjectFileSlot()), CTRL+Key_O);
   m_openFileMenu->insertItem ("Exit", this, SLOT (close()), CTRL+Key_X);
   
   // add open project menu to main menu
   m_mainMenu->insertItem ("File", m_openFileMenu);
 
   // show menu
   m_mainMenu->show ();
}

void TrapTrackingForm::createHelpMenu ()
{
   // create help menu
   m_helpMenu = new QPopupMenu (this);
   m_helpMenu->insertItem ("Contents", this, SLOT (displayHelp()), CTRL+Key_H);
   
   // add help menu to main menu
   m_mainMenu->insertItem ("Help", m_helpMenu);     
}

void TrapTrackingForm::deleteMainMenu()
{
   // DONT DELETE, AS WIDGETS WITH PARENT (this)
   // delete m_openFileMenu;
   //  delete m_mainMenu;
}

void TrapTrackingForm::openProjectFileSlot()
{
   if ( ! m_openProjectFileDialog )
   {
      m_openProjectFileDialog = initFileDialog ();
   }
   else
   {
      m_openProjectFileDialog->setDir (m_lastOpenFileDir);
   }
   
   m_openProjectFileDialog->exec ();
   
   // get user entered data from dialog
   m_lastOpenFileDir = m_openProjectFileDialog->dirPath();
   
   QString tempFile = m_openProjectFileDialog->selectedFile ();
   if ( tempFile.length () > 1 )
   {
      setProjectName (tempFile);
   }
}

QFileDialog* TrapTrackingForm::initFileDialog()
{
   QString filter = "*.project3d";
   QFileDialog *dialog = new QFileDialog (m_lastOpenFileDir, filter);
   return dialog;
}

QString TrapTrackingForm::createStdOutputDirName (const QString &filename)
{
   string tempStr = filename.ascii();
   tempStr = tempStr.substr (0, tempStr.find_last_of ('.'));
   tempStr  += "_CauldronOutputDir";
   return tempStr.c_str();
}

void TrapTrackingForm::displayHelp()
{
   openInternetHelp ();
}

void TrapTrackingForm::openInternetHelp()
{
   char   command[1024];
   char   commandbase[1024];
   char * dpy   = NULL;
   char * where = NULL;
   char * webbrowser = NULL;

   int status;
 
   webbrowser = getenv ("IBS_WEBBROWSER");
   if (!webbrowser)
   {
      webbrowser = getenv ("WEBBROWSER");
      if (!webbrowser)
      {
	 showMessage ("Neither of the environment variables IBS_WEBBROWSER or WEBBROWSER is set,\n"
	       "trying to use the web browser 'mozilla'");
	 webbrowser = "mozilla";
      }
   }

   dpy = getenv("DISPLAY");

   strcpy(commandbase, webbrowser);
   strcat(commandbase, " ");

   if (dpy)
   {
      strcat(commandbase, "-display ");
      strcat(commandbase, dpy);
      strcat(commandbase, " ");
   }

   strcpy (command, commandbase);
   strcat(command, "-remote 'ping()'");
   strcat(command, " > /dev/null 2>&1 /dev/null");
   status = system(command);

   if (status != 0)
   {
      // not yet running
      strcpy (command, commandbase);
      where = getenv("IBS_NAMES");
      strcat(command, where);
      strcat(command, "/HTML/trapper.html &");
   }
   else
   {
      strcpy (command, commandbase);
      strcat (command, "-remote 'openURL(");
      where = getenv("IBS_NAMES");
      strcat(command, where);
      strcat(command, "/HTML/trapper.html)' &");
   }
      
   system(command);
}

//
// COMBO BOX FUNCTIONALITY
//
void TrapTrackingForm::buildReservoirComboBox()
{
   reservoirComboBox->clear ();
   
   // loop reservoir history adding reservoir names to reservoir combo box
   const TrapInfoHistory &trapHistory = m_kernel.getTrapHistory ();
   const_TrapInfoHistoryIT histIt, endHist = trapHistory.end();
   for ( histIt = trapHistory.begin(); histIt != endHist; ++histIt )
   {
      reservoirComboBox->insertItem ((histIt->first).c_str());
   }  
 
   reservoirEditSlot();
}

void TrapTrackingForm::buildAgeComboBox()
{  
   ageComboBox->clear ();
 
   // find ages relating to current reservoir
   const TrapInfoHistory &trapHistory = m_kernel.getTrapHistory ();
   const_TrapInfoHistoryIT histIt = trapHistory.find (m_reservoirStr);

   if ( histIt != trapHistory.end() )
   {
      // loop age list, adding each age to age combo box
      char tempStr [50];
      const TrapInfoMap &ageMap = (histIt->second);
      const_TrapInfoMapIT ageIt, endAge = ageMap.end();
      for ( ageIt = ageMap.begin(); ageIt != endAge; ++ageIt )
      {
         if ( (ageIt->second)->size() > 0 )
         {
            sprintf(tempStr, "%f", ageIt->first);
            ageComboBox->insertItem (tempStr);
         }
      }
   }

   ageEditSlot ();
}

void TrapTrackingForm::buildTrapIdComboBox()
{
   trapIdComboBox->clear ();
  
   // find current reservoir history
   const TrapInfoHistory &trapHistory = m_kernel.getTrapHistory ();
   const_TrapInfoHistoryIT histIt = trapHistory.find (m_reservoirStr);
 
   if ( histIt != trapHistory.end() )
   { 
      // find current age list for reservoir
      const TrapInfoMap &ageMap = (histIt->second);
      const_TrapInfoMapIT ageIt = ageMap.find (getUserAge ());
      
      if ( ageIt != ageMap.end() )
      {
         // get trap list object for this age
         TrapList *trapList = (ageIt->second);
         
         // get trap location list for this trap list object
         const TrapLocation& locTrap = trapList->getSortedTransIdList ();
         
         // loop trap location list (because its sorted by trap id's)
         // and retrieve trap info node
         const_TrapLocationIT trapIt, endTrap = locTrap.end();
         const TrapInfo *trapInfo;
         char tempStr[100];
         for ( trapIt = locTrap.begin(); trapIt != endTrap; ++trapIt )
         {
            // get the trap info node
            trapInfo = &(*(trapIt->second));
            
            // extract the necessary info from the node
            sprintf(tempStr, "%d  (%2.2f, %2.2f)", 
                    trapInfo->transId,  
                    trapInfo->locTopDepth.i,
                    trapInfo->locTopDepth.j);
            
            // add the trap info to the trap combo list
            trapIdComboBox->insertItem (tempStr);
         }
      }
   }
   
   trapIdUpdate ();
}

void TrapTrackingForm::setComboItem (QComboBox *cb, const QString str)
{
   int size = cb->count();
   for ( int i = 0; i < size; ++i )
   {
      if ( str == cb->text (i) ) 
      {
         cb->setCurrentItem (i);
         return;
      }
   }
   
   cb->setCurrentItem (0);
}

void TrapTrackingForm::setTrapIdComboItem (QComboBox *cb, const QString str)
{
   int size = cb->count();
   QString trapNum;
   for ( int i = 0; i < size; ++i )
   {
      // truncate the combo box trap id option to the trap id 
      // without the coordinates at the end
      trapNum = cb->text(i);
      trapNum.truncate (str.length());
      if ( str == trapNum ) 
      {
         cb->setCurrentItem (i);
         return;
      }
   }
   
   cb->setCurrentItem (0);
}

void TrapTrackingForm::setTrapIdComboItem (QComboBox *cb, int id)
{
   // convert the integer to a string and then find it in the list of trap ids
   char numStr[100];
   sprintf(numStr, "%d", id);
   setTrapIdComboItem (cb, numStr);  
}

void TrapTrackingForm::showMessage(const char *str)
{
   QMessageBox::warning (this, "Trapper Error", str, QMessageBox::Ok, 
                         QMessageBox::NoButton, QMessageBox::NoButton);
   
    // the message box will be deleted when the parent is deleted
}

bool TrapTrackingForm::showQuestion(const char *str)
{
   int answer = QMessageBox::question (this, "Trapper Question", str, QMessageBox::Yes, 
                          QMessageBox::No, QMessageBox::NoButton);
   return answer == QMessageBox::Yes;
   
   // the message box will be deleted when the parent is deleted
}

//
// Continuously monitor pipe commands from 4DViewer
//

void TrapTrackingForm::createCommandThread()
{
   m_commandThread = new CommandThread ();
   m_commandThread->init(this);
   m_commandThread->start();
}

void TrapTrackingForm::closeCommandThread()
{
   if ( m_commandThread )
   {
      m_commandThread->stopRun ();
      m_commandThread->wait();
      delete m_commandThread;
      m_commandThread = 0;
   }
}

bool TrapTrackingForm::event (QEvent *e)
{
   if ( e->type () == TrapChangeEvent::TrapChangeEventType )
   {
      // have received TrapChangeEvent
      TrapChangeEvent *trapChange = dynamic_cast <TrapChangeEvent*>(e);
      executeCommandLineArgs (trapChange->numArgs(), trapChange->getArgs());
      return true;
   }
   else
   {
      return QWidget::event (e);
   }
}

#endif



























