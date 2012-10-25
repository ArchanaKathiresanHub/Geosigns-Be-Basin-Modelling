#include <MainWindow.h>
#include <PasswordDialog.h>
#include <SSHSession.h>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags )
: QMainWindow(parent,flags)
{
	_backgroundSSHSession=0;

	setupUi(this);

	QObject::connect(actionSet_password,SIGNAL(triggered(bool)),this,SLOT(ChangePasswordSlot()));
	QObject::connect(actionSubmit,SIGNAL(triggered(bool)),this,SLOT(JobSubmitSlot()));
	QObject::connect(&_timer,SIGNAL(timeout()),this,SLOT(TimerSlot()));

	_timer.start(1000);
}

void MainWindow::ChangePasswordSlot()
{
	PasswordDialog pwd;
	pwd.exec();
}

void MainWindow::JobSubmitSlot()
{
#if 0
	SSHSession session;
	if (! session.Connect())
	{
		QMessageBox::critical(this,"Error","Unable to connect");
	}

	QString result;
	// session.ExecuteCommand("/bin/ls -lsa",result);
	// session.ExecuteCommand("/glb/home/nlhkn3/test_script Darwin2 2 /bin/hostname",result);
	session.ExecuteCommand("/glb/home/nlhkn3/test_script res Darwin2",result);

	_textEdit->setText(result);
#else
	if (_backgroundSSHSession)
		return;

	_waitWidget.Show(this);

	_backgroundSSHSession=new BackgroundSSHSession();

	// _backgroundSSHSession->ExecuteCommand("/bin/ls -lsa");
	_backgroundSSHSession->ExecuteCommand("/glb/home/nlhkn3/test_script res Darwin2");
	// _backgroundSSHSession->ExecuteCommand("/bin/ls");
	// _backgroundSSHSession->ExecuteCommand(". /etc/profile ; /bin/ksh");

#endif
}

void MainWindow::TimerSlot()
{
	if (!_backgroundSSHSession)
		return;

	QString result=_backgroundSSHSession->GetCommandResult();

	if (result!="")
	{
		_waitWidget.Hide();
		_textEdit->setText(result);

		delete _backgroundSSHSession; _backgroundSSHSession=0;
	}
}

void MainWindow::CancelPressed()
{
	if (_backgroundSSHSession)
	{
		_backgroundSSHSession->Cancel();
		delete _backgroundSSHSession; _backgroundSSHSession=0;
	}
	_waitWidget.Hide();
}
