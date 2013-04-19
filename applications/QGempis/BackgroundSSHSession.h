#ifndef _BACKGROUND_SSH_SESSION_H_
#define _BACKGROUND_SSH_SESSION_H_

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QMutex>
#include <SSHSession.h>

class BackgroundSSHSession;

class SSHSessionThread : public QThread
{
public:
	SSHSessionThread(BackgroundSSHSession* sshSession)
	{
		_sshSession=sshSession;
	}

protected:
	virtual void run();

protected:
	BackgroundSSHSession* _sshSession;
};

class BackgroundSSHSession : public QObject
{
	Q_OBJECT;

public:
	BackgroundSSHSession();
	~BackgroundSSHSession();

	virtual bool ExecuteCommand(const QString& command);

	friend class SSHSessionThread;

	QString GetCommandResult();

	void Cancel();

protected:
	void Run();

protected:
	QString _command;

	QMutex _resultMutex;
	QString _result;
	SSHSessionThread* _thread;
};

#endif
