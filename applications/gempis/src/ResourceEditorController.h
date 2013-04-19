#ifndef ResourceEditorController_H
#define ResourceEditorController_H _ResourceEditorController_H

#include <qobject.h>
#include <ResourceManagerController.h>
#include <ResourceFile.h>

class ResourceEditorUI;

class ResourceEditorController :
        public QObject
{

    Q_OBJECT

    public: 
        ResourceEditorController( ResourceEditorUI * );
        ~ResourceEditorController();
        
    public slots:
        void addClicked( void );
        void saveClicked( void );
        void deleteClicked( void );
        void closeClicked( void );
        void resourceSelected( const QString & );

    private:
	void updateResourceEditorWindow();
	void getResourceFromScreen();
	
	// data variables
        long m_contentsChanged;
        long m_index;
	ResourceEditorUI * m_select;
	ResourceFile * m_resourceFile;
	t_resourceEntry m_resourceEntry;
};

#endif
