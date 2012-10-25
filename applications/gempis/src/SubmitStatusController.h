#ifndef SubmitStatusController_H
#define SubmitStatusController_H _SubmitStatusController_H

#include <qobject.h>
#include <qmessagebox.h> 
#include <JobLauncherController.h>

class GempisThread;
class SubmitStatusUI;

class SubmitStatusController :
        public QObject
{

    Q_OBJECT

    public: 
        SubmitStatusController( SubmitStatusUI * );
        ~SubmitStatusController();
        void StartGempisCommand( JobLauncherController * );
	void KillChildExecutable( void );
	bool GetJobCompleteSent();
	void SetJobCompleteSent();
        
    public slots:
        void refreshWindow( void );
        void commandComplete( void );
        void terminateClicked( void );

    private:
	void updateDisplay( void  );

	bool m_jobCompleteSent;
	bool m_errorDetected;
	bool m_warningDetected;
	bool m_childrenActive;
	volatile bool terminateActive;

	long m_result;
	int m_lastLineNumber;

	QString m_error;
	
	QStringList m_skipWarnings;
	QStringList m_skipErrors;

	SubmitStatusUI * m_select;
	JobLauncherController * m_controller;

	GempisThread * m_gempisThread;
	QTimer *internalSSTimer;
};

#endif
