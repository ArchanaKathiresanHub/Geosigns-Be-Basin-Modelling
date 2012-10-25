#ifndef ResourceManagerController_H
#define ResourceManagerController_H _ResourceManagerController_H

#include <qobject.h>
#include <qmenubar.h>
#include <qpopupmenu.h> 
#include <qlistview.h>
#include <qpopupmenu.h>

#include <AppTimer.h>

#define MAX_COMMANDS 1024

class ResourceManagerUI;
class GempisThread;

class ResourceManagerController :
        public QObject
{

    Q_OBJECT

    public: 
        ResourceManagerController( ResourceManagerUI * );
        ~ResourceManagerController();
	void DisplayWindow ( void );
        QString GetResourceName() const;

    public slots:
        void nameSelected( const QString & );
        void closeClicked( void );
	void updateWindow( void );
	int SendEmail( void );
	void listViewItemSelected( QListViewItem*, const QPoint &point, int );
	void killSelected();

    private:
	void scheduleResourceUpdate( void );

	void enableResourceInfo( bool );
	void clearResourceInfo( void );
	void parseUpdateResourceInfo( void );

        void updateProcessorInfo( void );
	void startResourceRequest( void );
	void stopResourceRequest( void );
	void spawnKillRequest( const QString & );

	bool m_blockOn;
	bool m_resource_error;

	QString m_resourceName;
	QString m_resourceNameSelected;
	QString m_resourceInfoName;
	QString m_processorsTotal;
	QString m_processorsAvailable;
 	QString m_processorsMemoryper;
	QString m_envstring;
	QString m_resourceLog;
	QString m_resourceInfoRequested;
	QString m_errorMessage;
	QString m_selectedJobid;

	QStringList m_resourceList;

	ResourceManagerUI * m_select;

	QMenuBar *m_mainMenu;
	QPopupMenu *m_openFileMenu;
	QPopupMenu *m_inUseByActionsMenu;

	GempisThread * m_resourceThread;
	AppTimer * m_appTimer;

    private slots:
    
    protected:
};

#endif
