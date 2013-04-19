#include <unistd.h>

#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
      using namespace std;
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   using namespace std;
#endif // sgi

#include <qvariant.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

#include "logindialog.h"

LoginDialog::LoginDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : LoginDialogBase( parent, name, modal, fl )
{
   buttonOk->setEnabled (false);
   serverBox->setDuplicatesEnabled (false);
   serverBox->setEditable (false);
}

LoginDialog::~LoginDialog()
{
}

void LoginDialog::enableOkButton()
{
   buttonOk->setEnabled (!serverBox->currentText ().isEmpty() && !nameEdit->text ().isEmpty () && !passwordEdit->text ().isEmpty ());
}

void LoginDialog::show ()
{
   passwordEdit->setText ("");
   LoginDialogBase::show ();
}

void LoginDialog::addServer (const QString & server, const QString & label)
{
   m_servers[label] = server;
   serverBox->insertItem (label);
}

void LoginDialog::setName (const QString & name)
{
   nameEdit->setText (name);
}

void LoginDialog::setServer (const QString & server)
{

   if (!server.isEmpty ())
   {
      QMap<QString, QString>::Iterator it;
      for ( it = m_servers.begin(); it != m_servers.end(); ++it )
      {
	 if (it.data () == server)
	 {
	    serverBox->setCurrentText (it.key ());
	    break;
	 }
      }
   }
}


void LoginDialog::accept()
{
   LoginDialogBase::accept ();
   QString serverLabel = serverBox->currentText ();
   QString serverName = m_servers[serverLabel];
   emit login (serverName, nameEdit->text (), passwordEdit->text ());
}

void LoginDialog::reject()
{
   LoginDialogBase::reject ();
   emit loginCanceled ();
}
