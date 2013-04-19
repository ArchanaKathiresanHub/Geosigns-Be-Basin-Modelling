#ifndef _SSH_SESSION_H_
#define _SSH_SESSION_H_

#include <windows.h>
#include <QString>
#include <QObject>
#include <MyDefines.h>

#ifdef USE_CHILKAT

#include <CkSsh.h>

class SSHSession : public QObject
{
	Q_OBJECT;
public:
	SSHSession();
	~SSHSession();

	bool Connect();

	virtual bool ExecuteCommand(const QString& command,QString& result);

protected:
	CkSsh _ssh;
	bool _bIsConnected;
};

#else // USE_CHILKAT


// Use plink

class SSHSession : public QObject
{
	Q_OBJECT;
public:
	SSHSession();
	~SSHSession();

	bool Connect();

	virtual bool ExecuteCommand(const QString& command,QString& result);

protected:
	QString CreateConnectionString() const;
	void ExecutePLinkCommand(const QString& command,QString& result);

protected:
	bool _bIsConnected;
};

#endif // USE_CHILKAT

#endif
