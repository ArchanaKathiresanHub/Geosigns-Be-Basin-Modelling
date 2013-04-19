#include <SSHSession.h>
#include <GlobalSettings.h>
#include <MyDefines.h>
#include <iostream>

#ifdef USE_CHILKAT

SSHSession::SSHSession()
{
	bool bOk=_ssh.UnlockComponent("30-day trial");
	if (!bOk)
		THROW_ERROR("Unable to use Chilkat SSH");

	_bIsConnected=false;
}

SSHSession::~SSHSession()
{
	if (_bIsConnected)
	{
		_ssh.Disconnect();
	}
}

bool SSHSession::Connect()
{
	bool bOk=_ssh.Connect(REMOTE_HOSTNAME,22);
	if (!bOk)
		return false;

	QString username=GlobalSettings::GetUsername();
	QString password=GlobalSettings::GetPassword();

	bOk=_ssh.AuthenticatePw(username.toAscii(),password.toAscii());
	if (!bOk)
		return false;

	_bIsConnected=true;

	return true;
}

bool SSHSession::ExecuteCommand(const QString& command,QString& result)
{
	long channelNumber=_ssh.OpenSessionChannel();
	if (channelNumber<0)
		return false;

	bool bOk=_ssh.SendReqExec(channelNumber,command.toAscii());
	if (! bOk)
		return false;

	result="";

	long n=_ssh.ChannelReadAndPoll(channelNumber,20000);
	if (n<0)
		return false;

	// Close the channel
	bOk=_ssh.ChannelSendClose(channelNumber);

	// Accumulate any remaining output
	bOk=_ssh.ChannelReceiveToClose(channelNumber);
	if (!bOk)
		return false;

	// Get all remaining output
	const char* cmdOutput=_ssh.getReceivedText(channelNumber,"ansi");
	if (! cmdOutput)
		return false;

	result=QString(cmdOutput);

	// Pick up accumulated output of the command

	return true;
}

#else // USE_CHILKAT
// Use plink

SSHSession::SSHSession()
{
	_bIsConnected=false;
}

SSHSession::~SSHSession()
{
	if (_bIsConnected)
	{
		// _ssh.Disconnect();
	}
}

bool SSHSession::Connect()
{
	return true;
}

bool SSHSession::ExecuteCommand(const QString& command,QString& result)
{
	QString connectionString=CreateConnectionString();

	connectionString+=" \". /etc/profile ; "+command+" ; echo FiNiShEd\"";

 	ExecutePLinkCommand(connectionString,result);

	return true;
}

QString SSHSession::CreateConnectionString() const
{
	QString username=GlobalSettings::GetUsername();
	QString password=GlobalSettings::GetPassword();

	QString result="-ssh -l "+username+" -pw " +password+" "+QString(REMOTE_HOSTNAME);
	return result;
}

// Big chunks from function below are taken from Dynamo (file Process.cpp)
void SSHSession::ExecutePLinkCommand(const QString& command,QString& result)
{
	result="";

	static bool firstTime = true;
	if (firstTime)
	{
		// Start WinSock
		WSADATA ws;
		WSAStartup(0x101,&ws);
		firstTime = false;
	}

	SECURITY_ATTRIBUTES sa;
	memset(&sa,0,sizeof(sa));
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	HANDLE outRead, outWrite;
	HANDLE inRead, inWrite;
	if (!CreatePipe(&outRead,&outWrite,&sa,0))
	{
		DWORD err = GetLastError();
	}

	if (!CreatePipe(&inRead,&inWrite,&sa,0))
	{
		DWORD err = GetLastError();
	}

	STARTUPINFO si;
	memset(&si,0,sizeof(si));
	si.cb=sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
	si.hStdInput = outRead;
	si.hStdOutput = inWrite;
	//si.hStdError = errorWrite;
	si.hStdError = inWrite;
	si.wShowWindow = SW_HIDE;

	PROCESS_INFORMATION pi;
	memset(&pi,0,sizeof(pi));

	// CreateProcess needs writable buffer
	QByteArray tmpCommand=command.toAscii();

	char* params=tmpCommand.data();

	if (CreateProcess(PLINK_PROGRAM,
		params,
		NULL, NULL, TRUE, NULL,
		NULL, NULL, &si, &pi) )
	{
		result="";
		DWORD bytesRead;
		char buf[10];
		do
		{

			if (ReadFile(inRead, buf, sizeof(buf)-1, &bytesRead, NULL))
			{
				DWORD err = GetLastError();
				buf[bytesRead]='\0';
				result+=QString(buf);
			}
			else
			{
				// Is EOF?
				DWORD err = GetLastError();
				if (err==ERROR_HANDLE_EOF)
				{
					return;
				}
			}
			if (result.contains("FiNiShEd"))
			{
				result.remove("FiNiShEd");
				return;
			}
		} while(true /* bytesRead==sizeof(buf)-1 */);
	}
	else
	{
		// Error launching process
	}
}

#endif // USE_CHILKAT
