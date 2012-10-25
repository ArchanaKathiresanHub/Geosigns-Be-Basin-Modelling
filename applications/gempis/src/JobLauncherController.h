#ifndef JobLauncherController_H
#define JobLauncherController_H _JobLauncherController_H

#include <AppTimer.h>
#include <ResourceLimits.h>

#include <qobject.h>
#include <qmenubar.h>
#include <qpopupmenu.h> 
#include <qstatusbar.h> 


#define PARALLEL 1
#define LOCAL 2

#define MAX_COMMANDS 1024

class JobLauncherUI;
class LocalCommandUI;
class SubmitStatusUI;
class GempisThread;

class JobLauncherController :
        public QObject
{

    Q_OBJECT

    public: 
        JobLauncherController( JobLauncherUI * );
        ~JobLauncherController();

	// these 3 are used by Qt
	void SetJobFilename ( const char * );
	void ReadJobFile ( void );
	void DisplayWindow ( void );

	void ReadResourceConfig ( void );

        QString GetProjectName() const;
        QString GetResourceName() const;
        QString GetNumberProcessors() const;
        QString GetCommand() const;
        QString GetCommandType() const;
        QString GetProjectLogFileName() const;
        long GetProjectCommandIndex() const;
	void SetCommandResult( long );

    public slots:
        void resourceListSelect( const QString & );
        void processorListSelect( const QString & );
        void terminateApplication( void );
        void enableGempir( void );
        void launchGempir( void );
	void jobStart( void );
	void jobNext( void );
	void jobComplete( void );
	void jobInterrupt( void );
	int SendEmail( void );
	void childLauncher( void );

    private:
	void getComputerResources( void );
	void getMemoryRequirements( void );

	// window display 
	void setWindowTitle( void );

	void enableSubmitButton( bool );
	void setTextSubmitButton( void );
	void enableCloseButton( bool );

	void projectInfoEnable( bool );
	void projectInfoDisplay( void );

	void commandsEnable( bool );
	void commandsDisplay( void );
	void commandsDisplayResult( void );
	void commandsDisplayCursor( void );

	void resourceListEnable( bool );
	void resourceListBuild( void );
	void resourceListAdjust( void );

	void resourceLimitsCreate( void );
	void resourceLimitsRead( void );

	void processorListEnable( bool );
        void processorListBuild( void );
	void processorListAdjust( void );

	void memoryAllocatedEnable( bool );
        void memoryAllocatedUpdate( void );

	bool m_autoClose;

	bool m_jobFileRead;           // true = GJF file read
	bool m_memoryRequiredDefined; // true = have memory required value
	bool m_resourceConfigLoaded;  // true = gempis resource configuration loaded
	bool m_resourceLimitsDefined; // true = resource limits have been defined

	bool m_submitButtonActive;    // true = submit, false = terminate
	bool m_launchGempir;

	volatile long m_commandIndex;

	QString m_jobFileName;
	QString m_requirementsCommand;
	QString m_memoryRequired;
	QString m_projectName;
	QString m_resourceName;
	QString m_processorsTotal;
 	QString m_processorsMemoryper;
	QString m_processorsSelected;
	QString m_envstring;
	QString m_projectLog;
	QString m_memoryAllocated;

	QStringList m_resourceList;

	JobLauncherUI * m_select;

	// job control info
	long m_commandResult[ MAX_COMMANDS ];
	QString m_commandType[ MAX_COMMANDS ];
	QString m_description[ MAX_COMMANDS ];
	QString m_commandString[ MAX_COMMANDS ];

	QMenuBar *m_mainMenu;
	QPopupMenu *m_openFileMenu;
	LocalCommandUI * m_lcDialog;
	SubmitStatusUI * m_ssDialog;
	AppTimer * m_appTimer;
	ResourceLimits * m_resourceLimits;

    private slots:
    
    protected:
};

#endif
