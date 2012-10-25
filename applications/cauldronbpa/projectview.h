#ifndef PROJECTVIEW_H
#define PROJECTVIEW_H
#include "projectviewbase.h"

class ExportWizard;

class QStringList;
class QPopupMenu;

#include "Interface/Interface.h"

class InputMapListViewItem;
class OutputMapListViewItem;

class ProjectView : public ProjectViewBase
{
    Q_OBJECT

public:
    ProjectView( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~ProjectView();

    void loadProject (const QString & fileName);
    void updateRecentProjects (const QStringList & projectList);

    void checkServerPresence ();
    bool existsOnServer ();

    DataAccess::Interface::PropertyValueList * getPropertyValuesToUpload (void);

public slots:
    virtual void show ();
    virtual void fileClose ();
    void connectToBPA();
    void projectOpen();
    void startExport();
    void continueExport();
    void startReexport();
    void continueReexport();
    void finishExport();
    void filePrint();
    void fileExit();
    void helpIndex();

    void openRecentProject (int index);

    void helpContents();
    void helpAbout();

    void inputItemSelected( QListViewItem * );
    void outputItemSelected( QListViewItem * );

    void displayGridMap( const DataAccess::Interface::GridMap * gm, const QString & title,
	  GridMapView * gmv = 0 );
    void displayInputValue (const DataAccess::Interface::InputValue * iv, GridMapView * gmv = 0);
    void displayPropertyValue (const DataAccess::Interface::PropertyValue * pv, GridMapView * gmv = 0);

    void getInputValueTitle (const DataAccess::Interface::InputValue * iv, QString & title);
    void displayInputValueProperties (const DataAccess::Interface::InputValue * iv);

    void getPropertyValueTitle (const DataAccess::Interface::PropertyValue * pv, QString & title);
    void displayPropertyValueProperties (const DataAccess::Interface::PropertyValue * pv);

    void getGridMapDescription (const DataAccess::Interface::GridMap * gm, QString & description);

    void ProjectViewBase_toolBarPositionChanged( QToolBar * );
    void ProjectViewBase_dockWindowPositionChanged( QDockWindow * );

protected:

   QPopupMenu * m_recentProjectsMenu;
   QPopupMenu * m_inputMapActionsMenu;
   QPopupMenu * m_outputMapActionsMenu;

   enum { MaxRecentProjects = 10 };
   int m_recentProjectIds[MaxRecentProjects];
   int m_uploadId;
   int m_cancelUploadId;
   int m_removeId;
   int m_cancelRemoveId;

protected slots:
   void popupInputMapActionsMenu (QListViewItem* item, const QPoint & point, int);
   void displaySelectedInputMap (void);
   void displaySelectedInputMapSeparately (void);
   void displaySelectedInputMapProperties (void);

   void popupOutputMapActionsMenu (QListViewItem* item, const QPoint & point, int);
   void displaySelectedOutputMap (void);
   void displaySelectedOutputMapSeparately (void);
   void displaySelectedOutputMapProperties (void);

   void updateOutputMapListView (void);

   void uploadToServer (void);

   void cancelUploadToServer (void);
   void removeFromServer (void);
   void cancelRemoveFromServer (void);

private:
    ExportWizard * m_exportWizard;

    QListViewItem * m_selectedInputItem;
    QListViewItem * m_selectedOutputItem;

    QPtrList<GridMapView> m_gridMapViews;

    DataAccess::Interface::ProjectHandle * m_projectHandle;


    bool m_onServer;
};

#endif // PROJECTVIEW_H
