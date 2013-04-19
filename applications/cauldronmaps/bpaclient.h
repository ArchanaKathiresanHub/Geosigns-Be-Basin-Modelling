#ifndef BPACLIENT_H
#define BPACLIENT_H
#include "bpaclientbase.h"
#include "infobase.h"

#include <vector>
#include <string>

#include <qstringlist.h>
#include <qptrlist.h>

class QFileDialog;
class QPopupMenu;

class LoginDialog;
class ProjectView;
class LoaderThread;

class BPAClient : public BPAClientBase
#ifdef BPACONNECT
   , public InfoBase
#endif
{
   Q_OBJECT

public slots:
   virtual void show ();
   virtual void fileClose ();
   void fileExit();
   void fileNew();
   void fileOpen();
   void fileSave();
   void fileSaveAs();
   void filePrint();
   void helpIndex();
   void helpContents();
   void helpAbout();
   void fileRefresh ();

   void openRecentProject (int index);

   void startLogin();
   void doLogin (const QString & server, const QString & name, const QString & password);
   void cancelLogin ();
   void projectOpen();

signals:
    void loginSucceeded();
    void loginFailed();

public:
   ~BPAClient();

   static BPAClient * instance (void);

   void addRecentProject (const QString & fileName);
   const QString & getRecentProject (int param);

   void addProjectView (ProjectView * view);
   void removeProjectView (ProjectView * view);

   bool isLoggedIn (void);

   bool getLoginProperties (QString & server, QString & name, QString & password);

   void loadProject (const QString &fileName);

   void updateRecentProjects ();
   void updateRecentProjects (const QStringList & projectList);

public slots:
   virtual ListSelectionStatus displayOwnerOrgs (void);
   virtual ListSelectionStatus displayRegions (void);
   virtual ListSelectionStatus displayCountries (void);
   virtual ListSelectionStatus displayBasins (void);
   virtual ListSelectionStatus displayProjects (void);

protected slots:
   void popupProjectActionsMenu (QListViewItem* item, const QPoint & point, int);
   void displaySelectedProjectProperties (void);
   void displaySelectedProjectDescription (void);
   void displaySelectedProjectContext (void);

   void deleteMyItemInfos (void);

   void selectOwnerOrgs (void);
   void selectRegions (void);
   void selectCountries (void);
   void selectBasins (void);

   void evaluateSelectButton (QPushButton * button, ListSelectionStatus status);
   void evaluateProjectSelection ();

   void displayMyProjects (void);
   void displayAllProjects (void);
   void highlightSelection (void);

   void importProject (void);
   void deleteProject (void);

   void startLoaderThread (void);
   
protected:
   BPAClient( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );

   ProjectInfo * findSelectedProjectInfo (void);

   static BPAClient * s_instance;

   QPtrList<ProjectView> m_projectViews;

   LoginDialog* m_loginDialog;

   QFileDialog * m_importDialog;

   QString m_server;
   QString m_name;
   QString m_password;

   void readSettings();
   void writeSettings();

   QPopupMenu * m_recentProjectsMenu;
   QStringList m_recentProjects;

   QPopupMenu * m_projectActionsMenu;

   enum { MaxRecentProjects = 10 };
   int m_recentProjectIds[MaxRecentProjects];
private:
   LoaderThread * m_loaderThread;
    QListViewItem * m_selectedProjectItem;
};

#endif // BPACLIENT_H
