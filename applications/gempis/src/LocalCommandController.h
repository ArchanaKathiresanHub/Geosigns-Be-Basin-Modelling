#ifndef LocalCommandController_H
#define LocalCommandController_H _LocalCommandController_H

#include <qobject.h>
#include <JobLauncherController.h>

class LocalCommandUI;

class LocalCommandController :
        public QObject
{

    Q_OBJECT

    public: 
        LocalCommandController( LocalCommandUI * );
        ~LocalCommandController();
        void StartLocalCommand( JobLauncherController * );
	void KillChildExecutable( void );
        
    public slots:
        void refreshWindow( void );
        void commandComplete( void );
        void cancelClicked( void );

    private:
	void updateDisplay( void );

	int cancelActive;
	long m_result;

	LocalCommandUI * m_select;
	JobLauncherController * m_controller;

	GempisThread * m_gempisThread;
	QTimer *internalLCTimer;
};

#endif




