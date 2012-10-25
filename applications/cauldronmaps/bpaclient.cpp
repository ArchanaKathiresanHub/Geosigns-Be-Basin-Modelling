#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
      using namespace std;
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   using namespace std;
#endif // sgi

#include <string>
#include <algorithm>
using namespace std;

#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <assert.h>

#ifdef BPACONNECT
#include "BPAClientLibrary.hpp"
#endif
#include "bpaclient.h"
#include "loaderthread.h"

#include <qapplication.h>
#include <qsettings.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qprocess.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qtimer.h>
#include <qmutex.h>

#include "logindialog.h"

#include "iteminfo.h"

#include "projectview.h"

extern bool cbVerbose;
extern bool cbTesting;

static int s_NumWindows = 0;

unsigned int NumWindows ()
{
   return s_NumWindows;
}

void IncrementNumWindows ()
{
   ++s_NumWindows;
}

void DecrementNumWindows ()
{
   --s_NumWindows;
}


/*
 *  Constructs a BPAClient which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 */
BPAClient::BPAClient( QWidget* parent,  const char* name, WFlags fl )
    : BPAClientBase( parent, name, fl ),
#ifdef BPACONNECT
    InfoBase (),
#endif
    m_server (""), m_name (""), m_password ("")
{
#ifdef ISBPACLIENT
   m_listsLayout->removeItem (m_basinsLayout);
   m_basins->hide ();
   m_basinsLabel->hide ();
   m_basinsSelectButton->hide ();
   createCauldronBPA ();
   m_loginDialog = new LoginDialog (this, "Log into BPA", true);

   m_importDialog = new QFileDialog (this, "Select Directory to store project", true);
   m_importDialog->setMode (QFileDialog::Directory);

   m_projectActionsMenu = new QPopupMenu (m_projects);
   m_projectActionsMenu->insertItem (tr ("Properties"), this, SLOT (displaySelectedProjectProperties ()));
   m_projectActionsMenu->insertItem (tr ("Description"), this, SLOT (displaySelectedProjectDescription ()));
   m_projectActionsMenu->insertItem (tr ("Highlight context"), this, SLOT (displaySelectedProjectContext ()));

   connect (m_projects, SIGNAL (contextMenuRequested (QListViewItem *, const QPoint &, int)),
            this, SLOT (popupProjectActionsMenu (QListViewItem *, const QPoint &, int)));

   m_orgsSelectButton->setEnabled (false);
   m_regionsSelectButton->setEnabled (false);
   m_countriesSelectButton->setEnabled (false);
   m_basinsSelectButton->setEnabled (false);

   m_importButton->setEnabled (false);
   m_deleteButton->setEnabled (false);

   connect (m_loginDialog, SIGNAL (login (const QString &, const QString &, const QString &)),
   this, SLOT (doLogin (const QString &, const QString &, const QString &)));

   connect (m_loginDialog, SIGNAL (loginCanceled ()),
	 this, SLOT (cancelLogin ()));

   connect (m_orgs, SIGNAL (selectionChanged ()),
	 this, SLOT (displayOwnerOrgs ()));

   connect (m_regions, SIGNAL (selectionChanged ()),
	 this, SLOT (displayRegions ()));

   connect (m_countries, SIGNAL (selectionChanged ()),
	 this, SLOT (displayCountries ()));

   connect (m_projects, SIGNAL (selectionChanged ()),
	 this, SLOT (evaluateProjectSelection ()));

   connect (m_basins, SIGNAL (selectionChanged ()),
	 this, SLOT (displayBasins ()));

   connect (m_orgsSelectButton, SIGNAL (clicked ()),
	 this, SLOT (selectOwnerOrgs ()));

   connect (m_regionsSelectButton, SIGNAL (clicked ()),
	 this, SLOT (selectRegions ()));

   connect (m_countriesSelectButton, SIGNAL (clicked ()),
	 this, SLOT (selectCountries ()));

   connect (m_basinsSelectButton, SIGNAL (clicked ()),
	 this, SLOT (selectBasins ()));

   connect (m_projectsDisplayAllButton, SIGNAL (clicked ()),
	 this, SLOT (displayAllProjects ()));

   connect (m_projectsDisplayMineButton, SIGNAL (clicked ()),
	 this, SLOT (displayMyProjects ()));

   /*
   connect (m_projects, SIGNAL (selectionChanged ()),
	 this, SLOT (highlightSelection ()));
   */

   connect (m_importButton, SIGNAL (clicked ()),
	 this, SLOT (importProject ()));

   connect (m_deleteButton, SIGNAL (clicked ()),
	 this, SLOT (deleteProject ()));

   if (cbTesting)
   {
      m_loginDialog->addServer ("http://sww-bpa-am-accept.siep.shell.com/Bpa", "US Acceptance Server");
      m_loginDialog->addServer ("http://sww-bpa-eu-accept.siep.shell.com/Bpa", "EU Acceptance Server");
   }

   m_loginDialog->addServer ("http://sww-bpa-am.siep.shell.com/Bpa", "US Server");
   m_loginDialog->addServer ("http://sww-bpa-eu.siep.shell.com/Bpa", "EU Server");

   m_loaderThread = new LoaderThread (this, 0);

   QTimer * timer = new QTimer (this);
   connect (timer, SIGNAL (timeout ()), this, SLOT (startLoaderThread ()));

   if (timer->start (300000) == 0)
   {
      cerr << "BPAClient::Could not start timer to keep up connections\n";
   }
#endif

   for (int i = 0; i < MaxRecentProjects; ++i)
      m_recentProjectIds[i] = -1;

   readSettings ();
}

/*
 *  Destroys the object and frees any allocated resources
 */
BPAClient::~BPAClient()
{
#ifdef BPACONNECT
   deleteCauldronBPA ();
#endif
   writeSettings ();
   s_instance = 0;
   deleteMyItemInfos ();

   delete m_loaderThread;
}

void BPAClient::show ()
{
   if (isHidden ())
   {
      BPAClientBase::show ();
      IncrementNumWindows ();
   }
   else
   {
      raise ();
   }
}

void BPAClient::fileClose ()
{
   if (!isHidden ())
   {
      close ();
      DecrementNumWindows ();
   }
}


void BPAClient::popupProjectActionsMenu( QListViewItem* item, const QPoint & point, int )
{
   if (item)
   {
      m_selectedProjectItem = item;
      m_projectActionsMenu->popup (point);
   }
}

ProjectInfo * BPAClient::findSelectedProjectInfo (void)
{
#ifdef BPACONNECT
   vector < ProjectInfo * >::iterator iter;
   for (iter = m_projectInfos.begin (); iter != m_projectInfos.end (); ++iter)
   {
      ProjectInfo *projectInfo = *iter;
      if (projectInfo->getListViewItem () == m_selectedProjectItem)
      {
	 return projectInfo;
      }
   }
#endif
   return 0;
}

void BPAClient::displaySelectedProjectProperties (void)
{
#ifdef BPACONNECT
   ProjectInfo *projectInfo = findSelectedProjectInfo ();

   if (!projectInfo)
      return;

   lockServer ();
   const BPA::CauldronProject * project = projectInfo->getItem ();

   QString contents;
   contents = contents.append ("Name = ");
   contents = contents.append (projectInfo->getName ().c_str ());

   contents = contents.append ("\nProject Name = ");
   contents = contents.append (project->getProjectName ().c_str ());

   OwnerOrgInfo *ownerOrgInfo = findOwnerOrgInfo (project->getOwnerOrgId ());
   if (ownerOrgInfo)
   {
      contents = contents.append ("\nOwnerOrg = ");
      contents = contents.append (ownerOrgInfo->getName ().c_str ());
   }

   contents = contents.append ("\nBasins = ");
   contents = contents.append (project->getBasinNames ().c_str ());

   contents = contents.append ("\nCoordinate System = ");
   contents = contents.append (project->getCoordinateSystemName ().c_str ());

   contents = contents.append ("\nGeoTransform = ");
   contents = contents.append (project->getGeoTransformName ().c_str ());

   contents = contents.append ("\nUnix Path = ");
   contents = contents.append (project->getUnixHostAndPath ().c_str ());

   contents = contents.append ("\nUnix User Name = ");
   contents = contents.append (project->getUnixUserId ().c_str ());

   contents = contents.append ("\nBPA User Name = ");
   contents = contents.append (project->getBpaUserId ().c_str ());

   time_t creationTime = project->getCreateTime ();
   contents = contents.append ("\nCreation Time = ");
   contents = contents.append (ctime (&creationTime));

   unlockServer ();

   QMessageBox::information (this, "Project Properties", contents, QMessageBox::Ok);

#endif
}

void BPAClient::displaySelectedProjectDescription (void)
{
#ifdef BPACONNECT
   ProjectInfo *projectInfo = findSelectedProjectInfo ();

   if (!projectInfo)
      return;

   lockServer ();
   const BPA::CauldronProject * project = projectInfo->getItem ();

   QString contents;

   contents = contents.append ("\nDescription:\n");
   contents = contents.append (project->getDescription ().c_str ());
   unlockServer ();

   QMessageBox::information (this, "Project Description", contents, QMessageBox::Ok);
#endif
}

BPAClient * BPAClient::instance(void)
{
   if (!s_instance) s_instance = new BPAClient;
   return s_instance;
}

void BPAClient::startLogin()
{
   deleteMyItemInfos ();
#ifdef ISBPACLIENT
   m_loginDialog->show ();
   m_loginDialog->setName (m_name);
   m_loginDialog->setServer (m_server);
#endif
}

bool BPAClient::isLoggedIn (void)
{
   return m_name != "" && m_password != "";
}

void BPAClient::cancelLogin ()
{
   emit loginFailed ();
}

void BPAClient::doLogin (const QString & server, const QString & name, const QString & password)
{

#ifdef BPACONNECT
   QApplication::setOverrideCursor (QCursor::WaitCursor);

   if (cbVerbose)
   {
      cerr << "Logging on to server: " << server.ascii () << endl;
      cerr << "                name: " << name.ascii () << endl;
      cerr << "            password: ";

      for (int i = 0; i < password.length (); ++i)
      {
	 cerr << "x";
      }
      cerr << endl;
   }

   if (!InfoBase::login (server, name, password))
   {
      m_server = server;
      m_name = name;
      m_password = "";

      QApplication::restoreOverrideCursor ();
      startLogin ();
   }
   else
   {
      m_server = server;
      m_name = name;
      m_password = password;

      loadOwnerOrgs ();
      loadProjects ();
      loadRegions ();
      loadCountries ();
      loadBasins ();

      displayOwnerOrgs ();


      QApplication::restoreOverrideCursor ();
      emit loginSucceeded ();
   }
#endif
}

void BPAClient::startLoaderThread (void)
{

   if (m_loaderThread->running ())
   {
      if (cbVerbose)
	 cerr << "LoaderThread still running" << endl;
   }
   else
   {
      if (cbVerbose)
	 cerr << "LoaderThread starting" << endl;
      m_loaderThread->start ();
   }
}

void BPAClient::fileRefresh ()
{
#ifdef BPACONNECT
#ifdef OLD
   deleteMyItemInfos ();
   if (m_name != "")
   {
      doLogin (m_server, m_name, m_password);
   }
#else
   deleteProjects ();
   reloadProjects ();
   connectProjects ();
   displayProjects ();
#endif
#endif
}

bool BPAClient::getLoginProperties (QString & server, QString & name, QString & password)
{
   if (!isLoggedIn ()) return false;
   server = m_server;
   name = m_name;
   password = m_password;
   return true;
}

void BPAClient::deleteMyItemInfos (void)
{
#ifdef BPACONNECT
   deleteProjects ();
   deleteBasins ();
   deleteCountries ();
   deleteRegions ();
   deleteOwnerOrgs ();
#endif
}

ListSelectionStatus BPAClient::displayOwnerOrgs (void)
{
#ifdef BPACONNECT
   ListSelectionStatus status = InfoBase::displayOwnerOrgs (m_orgs);
   evaluateSelectButton (m_orgsSelectButton, status);
   // loadProjects ();
   // loadRegions ();
   displayRegions ();
   return status;
#endif
   return EmptyList;
}

ListSelectionStatus BPAClient::displayRegions (void)
{
#ifdef BPACONNECT
   ListSelectionStatus status = InfoBase::displayRegions (m_regions);
   evaluateSelectButton (m_regionsSelectButton, status);
   // loadCountries ();
   displayCountries ();
   return status;
#endif
   return EmptyList;
}

ListSelectionStatus BPAClient::displayCountries (void)
{
#ifdef BPACONNECT
   ListSelectionStatus status = InfoBase::displayCountries (m_countries);
   evaluateSelectButton (m_countriesSelectButton, status);
   // loadBasins ();
   displayBasins ();
   return status;
#endif
   return EmptyList;
}

ListSelectionStatus BPAClient::displayBasins (void)
{
#ifdef BPACONNECT
   ListSelectionStatus status = InfoBase::displayBasins (m_basins, true);
   evaluateSelectButton (m_basinsSelectButton, status);
   displayProjects ();
   return status;
#endif
   return EmptyList;
}

ListSelectionStatus BPAClient::displayProjects (void)
{
#ifdef BPACONNECT
   ListSelectionStatus status = InfoBase::displayProjects (m_projects);
   return status;
#endif
   return EmptyList;
}

void BPAClient::selectOwnerOrgs (void)
{
   if (m_orgsSelectButton->text () == "Select All")
   {
      m_orgsSelectButton->setText ("Deselect All");
      m_orgs->selectAll (true);
   }
   else
   {
      m_orgsSelectButton->setText ("Select All");
      m_orgs->selectAll (false);
   }

}

void BPAClient::selectRegions (void)
{
   if (m_regionsSelectButton->text () == "Select All")
   {
      m_regionsSelectButton->setText ("Deselect All");
      m_regions->selectAll (true);
   }
   else
   {
      m_regionsSelectButton->setText ("Select All");
      m_regions->selectAll (false);
   }
}


void BPAClient::selectCountries (void)
{
   if (m_countriesSelectButton->text () == "Select All")
   {
      m_countriesSelectButton->setText ("Deselect All");
      m_countries->selectAll (true);
   }
   else
   {
      m_countriesSelectButton->setText ("Select All");
      m_countries->selectAll (false);
   }
}

void BPAClient::selectBasins (void)
{
   if (m_basinsSelectButton->text () == "Select All")
   {
      m_basinsSelectButton->setText ("Deselect All");
      m_basins->selectAll (true);
   }
   else
   {
      m_basinsSelectButton->setText ("Select All");
      m_basins->selectAll (false);
   }
}

void BPAClient::evaluateSelectButton (QPushButton * button, ListSelectionStatus status)
{
   switch (status)
   {
      case EmptyList:
         button->setEnabled (false);
         break;
      case NoneSelected:
         button->setText ("Select All");
         button->setEnabled (true);
         break;
      case AllSelected:
         button->setText ("Deselect All");
         button->setEnabled (true);
         break;
      case SomeSelected:
         button->setEnabled (true);
         break;
   }
}



void BPAClient::displaySelectedProjectContext (void)
{
#ifdef BPACONNECT
   ProjectInfo *projectInfo = findSelectedProjectInfo ();

   if (!projectInfo)
      return;

   vector < ProjectInfo * >::iterator iter;
   for (iter = m_projectInfos.begin (); iter != m_projectInfos.end (); ++iter)
   {
      (* iter)->deselectParents ();
   }

   projectInfo->selectParents (ItemInfoBase::Including, OwnerOrgType);
   projectInfo->selectParents (ItemInfoBase::Excluding, OwnerOrgType);
   projectInfo->setSelected (true);
#endif
}

void BPAClient::displayMyProjects (void)
{
#ifdef BPACONNECT
   vector < ProjectInfo * >::iterator iter;
   for (iter = m_projectInfos.begin (); iter != m_projectInfos.end (); ++iter)
   {
      ProjectInfo *projectInfo = *iter;

      projectInfo->deselectParents ();
   }

#ifdef BPACONNECT
   lockServer ();
   for (iter = m_projectInfos.begin (); iter != m_projectInfos.end (); ++iter)
   {
      ProjectInfo *projectInfo = *iter;
      int (* pf) (int) = tolower;

      const BPA::CauldronProject * project = projectInfo->getItem ();
      string uid = project->getBpaUserId ();

      transform (uid.begin (), uid.end (), uid.begin (), pf);

      if (uid == m_name.lower ().ascii ())
      {
         projectInfo->selectParents (ItemInfoBase::Including, OwnerOrgType);
         projectInfo->selectParents (ItemInfoBase::Excluding, OwnerOrgType);
      }
   }

   for (iter = m_projectInfos.begin (); iter != m_projectInfos.end (); ++iter)
   {
      QListViewItem *lvi;
      ProjectInfo *projectInfo = *iter;
       int (* pf) (int) = tolower;

      const BPA::CauldronProject * project = projectInfo->getItem ();
      string uid = project->getBpaUserId ();

      transform (uid.begin (), uid.end (), uid.begin (), pf);

      if (uid != m_name.lower ().ascii () &&
          (lvi = projectInfo->getListViewItem ()) != 0 && lvi->listView ())
      {
         assert (m_projects == lvi->listView ());
         m_projects->setSelected (lvi, false);
         m_projects->takeItem (lvi);
      }
   }
   unlockServer ();
#endif
#endif
}

void BPAClient::displayAllProjects (void)
{
#ifdef BPACONNECT
   vector<ProjectInfo *>::iterator iter;
   for (iter= m_projectInfos.begin (); iter != m_projectInfos.end (); ++iter)
   {
      ProjectInfo * projectInfo = * iter;
      projectInfo->deselectParents ();
   }

   for (iter= m_projectInfos.begin (); iter != m_projectInfos.end (); ++iter)
   {
      ProjectInfo * projectInfo = * iter;

      projectInfo->selectParents (ItemInfoBase::Excluding, OwnerOrgType);
      projectInfo->selectParents (ItemInfoBase::Including, OwnerOrgType);
   }
   // displayOwnerOrgs ();
#endif
}

void BPAClient::highlightSelection (void)
{
#ifdef BPACONNECT
   vector<ProjectInfo *>::iterator iter;
   for (iter= m_projectInfos.begin (); iter != m_projectInfos.end (); ++iter)
   {
      ProjectInfo * projectInfo = * iter;
      if (projectInfo->isSelected ())
      {
	 projectInfo->highlight ();
	 projectInfo->highlightParents ();
      }
      else
      {
	 projectInfo->dehighlight ();
	 projectInfo->dehighlightParents ();
      }
   }
#endif
}

void BPAClient::evaluateProjectSelection (void)
{
#ifdef BPACONNECT
   bool selectionMade = false;
   vector<ProjectInfo *>::iterator iter;
   for (iter= m_projectInfos.begin (); iter != m_projectInfos.end (); ++iter)
   {
      // cerr << "Evaluating project: " << *iter << " from list: " << & m_projectInfos << endl;

      if ((* iter)->isSelected ())
      {
	 selectionMade = true;
	 break;
      }
   }
   m_importButton->setEnabled (selectionMade);
   m_deleteButton->setEnabled (selectionMade);
#endif
}

void BPAClient::importProject (void)
{
#ifdef BPACONNECT
   ProjectInfo * projectInfo = 0;
   vector<ProjectInfo *>::iterator iter;
   for (iter= m_projectInfos.begin (); iter != m_projectInfos.end (); ++iter)
   {
      if ((* iter) ->isSelected ())
      {
	 projectInfo = * iter;
	 break;
      }
   }

   if (projectInfo && m_importDialog->exec () == QDialog::Accepted)
   {
      QString directory = m_importDialog->selectedFile ();
      string projectFile;
      projectFile = projectInfo->getName ();
      projectFile += ".project3d";

      bool result = downloadProject (projectInfo, directory.ascii (), projectFile);

      if (result)
      {
	 string fullFileName;
	 fullFileName = directory.ascii ();
	 fullFileName += "/";
	 fullFileName += projectFile;

	 loadProject (fullFileName.c_str ());
      }
      else
      {
	 QMessageBox::information (this, QString (projectFile.c_str ()), QString ("Download failed"));
      }
   }
#endif
}

void BPAClient::deleteProject (void)
{
#ifdef BPACONNECT
   ProjectInfo * projectInfo = 0;
   vector<ProjectInfo *>::iterator iter;
   for (iter= m_projectInfos.begin (); iter != m_projectInfos.end (); ++iter)
   {
      if ((* iter) ->isSelected ())
      {
	 projectInfo = * iter;
	 break;
      }
   }

   if (projectInfo)
   {
      string projectFile;
      projectFile = projectInfo->getName ();

      if (QMessageBox::question(
            this,
            tr("Delete project"),
            tr("Delete project %1\nAre you sure?").arg (projectFile.c_str ()),
            tr("&Yes"), tr("&No"),
            QString::null, 1, 1 ) == 0)
      {
	 lockServer ();
	 bool result = m_cauldronBPA->DeleteCauldronProject (projectInfo->getItem ());
	 unlockServer ();
	 if (result)
	 {
	    m_projectInfos.erase (iter);
	    delete projectInfo;
	    QMessageBox::information (this, tr ("Delete project"),
		  tr ("Project %1 Deleted").arg (projectFile.c_str ()));
	 }
	 else
	 {
	    QMessageBox::information (this, tr ("Delete project failed"),
		  tr ("Deletion of Project %1 failed").arg (projectFile.c_str ()));
	 }
      }
   }
#endif
}

/*
 * public slot
 */
void BPAClient::projectOpen()
{
   QString fn = QFileDialog::getOpenFileName (QString::null, "Project files (*.project[13]d)", this);

   if (!fn.isEmpty ())
   {
      loadProject (fn);
   }
}


void BPAClient::loadProject (const QString &fileName)
{
   ProjectView * pv = new ProjectView;
   pv->show ();
   pv->loadProject (fileName);
}

void BPAClient::addProjectView (ProjectView * view)
{
   m_projectViews.append (view);
}

void BPAClient::removeProjectView (ProjectView * view)
{
   m_projectViews.remove (view);
}

void BPAClient::openRecentProject(int param)
{
   loadProject(m_recentProjects[param]);
}


void BPAClient::addRecentProject (const QString & fileName)
{
   QString projectName = fileName;
   if (!projectName.isEmpty())
   {
      m_recentProjects.remove (fileName);
      m_recentProjects.push_front (projectName);
      updateRecentProjects ();
   }
}

const QString & BPAClient::getRecentProject (int param)
{
   return m_recentProjects[param];
}

void BPAClient::updateRecentProjects ()
{
   while ((int) m_recentProjects.size () > MaxRecentProjects)
      m_recentProjects.pop_back ();

   updateRecentProjects (m_recentProjects);

   for (int i = 0; i < m_projectViews.count (); ++i)
   {
      m_projectViews.at (i)->updateRecentProjects (m_recentProjects);
   }
}

void BPAClient::updateRecentProjects (const QStringList & projectList)
{
   for (int i = 0; i < (int) projectList.size (); ++i)
   {
      QFileInfo fileInfo (projectList[i]);
      QString fileName = fileInfo.absFilePath ();
      QString text ="&%1 %2";
      text = text.arg (i + 1).arg (fileName);


      if (m_recentProjectIds[i] == -1)
      {
         if (i == 0)
	 {
            fileMenu->insertSeparator (fileMenu->count ());
	    m_recentProjectsMenu = new QPopupMenu( this );
	    fileMenu->insertItem(tr( "Recently Opened &Projects" ), m_recentProjectsMenu, -1,  fileMenu->count ());
	 }

         m_recentProjectIds[i] =
               m_recentProjectsMenu->insertItem (text, this, SLOT (openRecentProject (int)),
	       0, -1, m_recentProjectsMenu->count ());
         m_recentProjectsMenu->setItemParameter (m_recentProjectIds[i], i);
      }
      else
      {
         m_recentProjectsMenu->changeItem (m_recentProjectIds[i], text);
      }
      m_recentProjectsMenu->setItemEnabled (m_recentProjectIds[i], fileInfo.exists ());
   }
}

void BPAClient::writeSettings()
{
    QSettings settings;
    settings.setPath("SIEP", "CauldronBPA");

    settings.beginGroup("/CauldronBPA");

    settings.writeEntry("/recentProjects", m_recentProjects);
    settings.writeEntry("/name", m_name);
    settings.writeEntry("/server", m_server);

    settings.endGroup();
}

void BPAClient::readSettings()
{
    QSettings settings;
    settings.setPath("SIEP", "CauldronBPA");

    settings.beginGroup("/CauldronBPA");

    m_recentProjects = settings.readListEntry("/recentProjects");
    m_name = settings.readEntry("/name");
    m_server = settings.readEntry("/server");

    updateRecentProjects();
    settings.endGroup();
}


/*
 * public slot
 */
void BPAClient::fileNew()
{
    qWarning( "BPAClient::fileNew() not yet implemented!" );
}

/*
 * public slot
 */
void BPAClient::fileOpen()
{
    qWarning( "BPAClient::fileOpen() not yet implemented!" );
}

/*
 * public slot
 */
void BPAClient::fileSave()
{
    qWarning( "BPAClient::fileSave() not yet implemented!" );
}

/*
 * public slot
 */
void BPAClient::fileSaveAs()
{
    qWarning( "BPAClient::fileSaveAs() not yet implemented!" );
}

/*
 * public slot
 */
void BPAClient::filePrint()
{
    qWarning( "BPAClient::filePrint() not yet implemented!" );
}

/*
 * public slot
 */
extern QApplication * app;

void BPAClient::fileExit()
{
   if (NumWindows () <= 1 || QMessageBox::question(
	    this,
	    tr("Exit Application"),
	    tr("Close all %1 open windows\nAre you sure?").arg (NumWindows ()),
	    tr("&Yes"), tr("&No"),
	    QString::null, 1, 1 ) == 0)
   {
      app->closeAllWindows ();
   }
}

/*
 * public slot
 */
void BPAClient::helpIndex()
{
    qWarning( "BPAClient::helpIndex() not yet implemented!" );
}

/*
 * public slot
 */
void BPAClient::helpContents()
{
    qWarning( "BPAClient::helpContents() not yet implemented!" );
}

/*
 * public slot
 */
void BPAClient::helpAbout()
{
   QMessageBox::information (this, "About CauldronMaps",
	 "CauldronMaps is a graphical tool for the display of input and output maps of a cauldron 3D project\n"
	 "Copyright Shell International Exploration and Production B.V., The Netherlands\n"
	 "Written by Alfred van der Hoeven", QMessageBox::Ok);
}

BPAClient * BPAClient::s_instance = 0;

