#include <iostream>

#include <QApplication>
#include <MainWindow.h>
#include <SSHSession.h>

int main(int argc,char* argv[])
{
	QApplication app(argc,argv);

	if (argc==1)
	{
		// GUI
		MainWindow mw;
		mw.show();

		return app.exec();
	}
	else
	{
		// Command line
		SSHSession sshSession;
		if (!sshSession.Connect())
			std::cerr << "Unable to connect\n";

		QString result;
		sshSession.ExecuteCommand(argv[1],result);

		QByteArray byteArray=result.toAscii();
		std::cout << byteArray.data() << std::endl;
	}
}
