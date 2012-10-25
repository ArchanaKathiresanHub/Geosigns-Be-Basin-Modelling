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
#include <LocalCommandController.h>
#include <qmessagebox.h> 


void LocalCommandUI::init()
{
#ifdef DEBUG
    cout << this << " LocalCommandUI::init()" << endl;
#endif

    m_controller = new LocalCommandController( this );
}

void LocalCommandUI::DoLocalCommand( JobLauncherController * ptr )
{
#ifdef DEBUG
    cout << this << " LocalCommandUI::DoLocalCommand()" << endl;
#endif
    
    m_controller->StartLocalCommand( ptr );
}



void LocalCommandUI::destroy()
{
#ifdef DEBUG
    cout << this << " LocalCommandUI::destroy()" << endl;
#endif

    delete m_controller;
}


void LocalCommandUI::closeEvent( QCloseEvent* ce )
{
#ifdef DEBUG
    cout << this << " LocalCommandUI::closeEvent()" << endl;
#endif
 
    m_controller->KillChildExecutable();
    
    emit jobComplete();
    
    ce->accept();
}
