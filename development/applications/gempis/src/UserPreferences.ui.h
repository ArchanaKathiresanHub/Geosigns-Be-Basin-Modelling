/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include "includestreams.h"
#include <JobLauncherController.h>
//#include <UserPreferencesController.h>
#include <qmessagebox.h> 

void UserPreferences::init()
{
#ifdef DEBUG
    cout << this << " UserPreferencesUI::init()" << endl;
#endif
    
//    m_controller = new UserPreferencesController( this );
}


void UserPreferences::destroy()
{
#ifdef DEBUG
     cout << this << " UserPreferencesUI::destroy()" << endl;
#endif

// delete m_controller;
}
