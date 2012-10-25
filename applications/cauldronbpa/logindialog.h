#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include "logindialogbase.h"

#include <qstring.h>
#include <qmap.h>


class LoginDialog : public LoginDialogBase
{
    Q_OBJECT

public:
    LoginDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~LoginDialog();
    virtual void show ();
    void addServer (const QString & server, const QString & label);
    void setName (const QString & name);
    void setServer (const QString & server);

private slots:
    virtual void enableOkButton();
    virtual void accept();
    virtual void reject();

protected:
    QMap<QString, QString> m_servers;

};

#endif // LOGINDIALOG_H
