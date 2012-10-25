#include <BackgroundSSHSession.h>

#define INTERVAL 1000

//////////////////////////////////////////////////////////////////////////////
void SSHSessionThread::run()
{
	_sshSession->Run();
}
//////////////////////////////////////////////////////////////////////////////


BackgroundSSHSession::BackgroundSSHSession()
{
}

BackgroundSSHSession::~BackgroundSSHSession()
{
	if (_thread)
	{
		delete _thread;
		_thread=0;
	}
}

bool BackgroundSSHSession::ExecuteCommand(const QString& command)
{
	_command=command;
	_result="";

	_thread=new SSHSessionThread(this);
	_thread->start();

	return true;
}

void BackgroundSSHSession::Run()
{
	SSHSession sshSession;

	sshSession.Connect();
	QString result;
	sshSession.ExecuteCommand(_command,result);

	QMutexLocker locker(&_resultMutex);
	_result=result;
}

QString BackgroundSSHSession::GetCommandResult()
{
	QString result;
	{
		QMutexLocker locker(&_resultMutex);
		result=_result;
		if (_result!="")
			_result="";
	}
	return result;
}

void BackgroundSSHSession::Cancel()
{
	if (!_thread)
		return;

	_thread->terminate();
	_thread->wait();
}
