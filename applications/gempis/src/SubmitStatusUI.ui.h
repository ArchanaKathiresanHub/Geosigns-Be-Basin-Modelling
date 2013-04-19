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
#include <SubmitStatusController.h>
#include <qmessagebox.h> 


void SubmitStatusUI::init()
{
#ifdef DEBUG
    cout << this << " SubmitStatusUI::init()" << endl;
#endif

    m_controller = new SubmitStatusController( this );
    m_box = 0;
}


void SubmitStatusUI::destroy()
{
#ifdef DEBUG
    cout << this << " SubmitStatusUI::destroy()" << endl;
#endif

    if ( m_box )
      {
	m_box->close();
	m_box = 0;
      }

    delete m_controller;

    // set value to undefined for any children that may be still active
    m_controller = NULL;
}


void SubmitStatusUI::DoParallelCommand( JobLauncherController * ptr )
{
#ifdef DEBUG
    cout << this << " SubmitStatusUI::DoParallelCommand()" << endl;
#endif
    
    m_controller->StartGempisCommand( ptr );
}


int SubmitStatusUI::QuitJobMessage()
{
#ifdef DEBUG
    cout << this << " SubmitStatusUI::QuitJobMessage()" << endl;
#endif

    m_box = new QMessageBox( "Terminate job", 
			     "Do you want to terminate this job?", 
			     QMessageBox::Question, 
			     QMessageBox::Yes | QMessageBox::Default,
			     QMessageBox::No | QMessageBox::Escape,
			     QMessageBox::NoButton );
    int res;
    res = m_box->exec();

    return res == QMessageBox::Yes;
}

void SubmitStatusUI::closeEvent( QCloseEvent* ce )
{
#ifdef DEBUG
    cout << this << " SubmitStatusUI::closeEvent()" << endl;
#endif
 
    m_controller->KillChildExecutable();

    if ( m_controller->GetJobCompleteSent() == false )
      {
	m_controller->SetJobCompleteSent();
	emit jobComplete();
      }

    ce->accept();
}


void SubmitStatusUI::Continue2NextJob()
{
#ifdef DEBUG
    cout << this << " SubmitStatusUI::Continue2NextJob()" << endl;
#endif

    m_controller->SetJobCompleteSent();
    emit jobComplete();
}

