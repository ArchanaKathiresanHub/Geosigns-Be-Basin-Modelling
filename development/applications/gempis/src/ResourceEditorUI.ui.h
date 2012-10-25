/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include "includestreams.h"
#include <ResourceManagerController.h>
#include <ResourceEditorController.h>
#include <qmessagebox.h> 

void ResourceEditorUI::init()
{
#ifdef DEBUG
    cout << this << " ResourceEditorUI::init()" << endl;
#endif
    
    m_controller = new ResourceEditorController( this );
}


int ResourceEditorUI::askQuestion( const char * str )
{
#ifdef DEBUG
    cout << this << " ResourceEditorUI::askQuestion()" << endl;
#endif

    return QMessageBox::question( this, "Resource Editor", 
                            str, "Yes", "No", 0, 1 );
}


void ResourceEditorUI::showMessage( const char * str )
{
#ifdef DEBUG
    cout << this << " ResourceEditorUI::showMessage()" << endl;
#endif
    
    QMessageBox::warning (this, "Resource Editor", str, QMessageBox::Ok, 
                         QMessageBox::NoButton, QMessageBox::NoButton);
}


void ResourceEditorUI::destroy()
{
#ifdef DEBUG
     cout << this << " ResourceEditorUI::destroy()" << endl;
#endif

    delete m_controller;
}


