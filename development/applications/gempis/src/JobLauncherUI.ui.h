/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

#include "includestreams.h"
#include <JobLauncherController.h>

#include <qstring.h>
#include <qmessagebox.h> 
#include <qfiledialog.h> 

#include <stdlib.h> // getenv(), system(), 
#include <string.h> // strcpy(), strcat()


// name of web brouwser to use for help documentationm_mainMenu
#define WEB_BROUWSER "mozilla"


void JobLauncherUI::init()
{
#ifdef DEBUG
    cout << this << " JobLauncherUI::init()" << endl;
#endif
    
    controller = new JobLauncherController( this );
}


void JobLauncherUI::showError( const char * str )
{
#ifdef DEBUG
    cout << this << " JobLauncherUI::showError()" << endl;
#endif
    
    QMessageBox::warning (this, "Gempis Error", str, QMessageBox::Ok, 
                         QMessageBox::NoButton, QMessageBox::NoButton);
   
    // the message box will be deleted when the parent is deleted
}


void JobLauncherUI::helpAbout()
{
#ifdef DEBUG
  cout << this << " JobLauncherUI::helpAbout()" << endl;
#endif

  QString msg;
  msg = "Version ";

  char * cptr;
  cptr = getenv( "IBS_VERSION" );
  if ( cptr != NULL )
    {
      msg += cptr;
    }
  else
   {
       msg += "Unknown";
   }

#ifdef SVNREVISION
  msg += " (revision ";
  msg += SVNREVISION;
  msg += ")";
#endif

   msg += "\n";
   msg += "Copyright 2006 - 2008\n";
   msg += "Shell International E&P \n ";
    
    QMessageBox::about( this, "Gempil Job Launcher", msg );
}


void JobLauncherUI::fileNew()
{
#ifdef DEBUG
    cout << this << " JobLauncherUI::fileNew()" << endl;
#endif
}


void JobLauncherUI::fileOpen()
{
#ifdef DEBUG
    cout << this << " JobLauncherUI::fileOpen()" << endl;
#endif
    
    QString fn = QFileDialog::getOpenFileName( QString::null, "Gempis Job Files (*.gjf)",
            this);
    if ( !fn.isEmpty() )
    {
      controller->SetJobFilename( fn );
      controller->ReadJobFile();
      controller->DisplayWindow();
    }
}


void JobLauncherUI::fileSave()
{
#ifdef DEBUG
    cout << this << " JobLauncherUI::fileSave()" << endl;
#endif
}


void JobLauncherUI::fileSaveAs()
{
#ifdef DEBUG
    cout << this << " JobLauncherUI::fileSaveAs()" << endl;
#endif
}


void JobLauncherUI::editPreferences()
{
#ifdef DEBUG
  cout << this << " JobLauncherUI::editPreferences()" << endl;
#endif

  QString msg;
  msg = "This feature has not been enabled.";

  QMessageBox::information( this, "Edit user preferences", msg );
}


void JobLauncherUI::helpContents()
{
#ifdef DEBUG
   cout << this << " JobLauncherUI::helpContents()" << endl;
#endif

   char   command[1024];
   char * dpy   = NULL;
   char * where = NULL;
 
   // Build the command line to launch netscape for the first time and do it.
   dpy = getenv( "DISPLAY" );
   strcpy( command, WEB_BROUWSER );
   if ( dpy )
   {
     strcat( command, " -display " );
     strcat( command, dpy );
   }
   strcat( command, " & sleep 5" );
   system( command );
   
   //  Now fill netscape with the user documentation.
   strcpy( command, WEB_BROUWSER );
   strcat ( command, " -remote 'openURL(" );
   where = getenv( "IBS_NAMES" );
   strcat( command, where );
   strcat( command, "/HTML/gempil.html)' " );
   strcat( command, "> /dev/null 2>&1 /dev/null &" );
   system( command );
}


void JobLauncherUI::destroy()
{
#ifdef DEBUG
  cout << this << " JobLauncherUI::destroy()" << endl;
#endif

  delete controller;
}


void JobLauncherUI::helpEmailLog()
{
    int res =  QMessageBox::question( 0, "Send Project Log", 
                            "Do you want to send the project log file to the helpdesk (no confidential infomation is contained in this file)?", 
                            "Yes", "No", QString::null,  0, 1 );

    if ( res == 0 )
    {
 res = controller->SendEmail();
 if ( res == 0 )
 {
     QMessageBox::information( 0, "Project Log Sent", 
  "The project log file has been passed to your computer's mail server. If mail is not working on this computer the file must be sent by another means", 
  QMessageBox::Ok );
 }
 else
 {
     QMessageBox::warning( 0, "Project Log Send Failed", 
                            "The project log could not be sent by your computer. Check the contents of Email.log for details and resubmit, otherwise you must send this file by another means.", 
  QMessageBox::Ok, QMessageBox::NoButton );
             }
     }
}


void JobLauncherUI::helpEmail_Log_FileAction_activated()
{
#ifdef DEBUG
    cout << this << " JobLauncherUI::helpEmail_Log_FileAction()" << endl;
#endif

    int res =  QMessageBox::question( 0, "Send Project Log", 
                            "Do you want to send the log file to the helpdesk (no confidential infomation will be sent)?", 
                            "Yes", "No", QString::null,  0, 1 );

    if ( res == 0 )
    {
 res = controller->SendEmail();
 if ( res == 0 )
 {
     QMessageBox::information( 0, "Project Log Sent", 
  "The project log file has been passed to your computer's mail server. If mail is not working on this computer the file must be sent by hand", 
  QMessageBox::Ok );
 }
 else
 {
     QMessageBox::warning( 0, "Project Log Send Failed", 
                            "The project log could not be sent by your computer. Check the contents of Email.log for details and resubmit, otherwise you must send this file by hand.", 
  QMessageBox::Ok, QMessageBox::NoButton );
             }
     }
}
