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

#include <unistd.h>
#include <assert.h>


#include <qapplication.h>
#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qwidgetlist.h>
#include <qlineedit.h>
#include <qfiledialog.h>
#include <qlistview.h>
#include <qstatusbar.h>
#include <qmessagebox.h>

#include "rasterplot.h"
#include <qwt_plot.h>

#include "Interface/ProjectHandle.h"
#include "Interface/PropertyValue.h"
#include "Interface/Property.h"
#include "Interface/Snapshot.h"
#include "Interface/Reservoir.h"
#include "Interface/Formation.h"
#include "Interface/Surface.h"
#include "Interface/InputValue.h"
#include "Interface/GridMap.h"
#include "Interface/Grid.h"

#ifdef CREATELEADS
#include "LeadFactory.h"
#include "Implementation/ProjectHandle.h"
#endif

#include "gridmapview.h"
#include "bpaclient.h"
#include "exportwizard.h"
#include "misc.h"

#include "array.h"

using namespace DataAccess;

using Interface::OpenCauldronProject;
using Interface::CloseCauldronProject;
using Interface::PropertyValue;
using Interface::PropertyValueList;
using Interface::Property;
using Interface::Snapshot;
using Interface::Reservoir;
using Interface::Formation;
using Interface::Surface;
using Interface::InputValue;
using Interface::InputValueList;
using Interface::GridMap;
using Interface::Grid;

using Interface::SURFACE;
using Interface::FORMATION;
using Interface::FORMATIONSURFACE;
using Interface::RESERVOIR;

extern void IncrementNumWindows ();
extern void DecrementNumWindows ();
extern unsigned int NumWindows ();

#include "maplistviewitem.h"
#include "projectview.h"

/* 
 *  Constructs a ProjectView which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
ProjectView::ProjectView( QWidget* parent,  const char* name, WFlags fl )
    : ProjectViewBase( parent, name, fl )
{
   static bool first = true;

   if (first)
   {
#ifdef CREATELEADS
      Implementation::ProjectHandle::UseFactory (new Leads::LeadFactory);
#endif
   }

   m_projectHandle = 0;
   m_onServer = false;

   m_gridMapViews.setAutoDelete (true);

   m_gridMapView->setTitle ("");

   connect (m_exportButton, SIGNAL (clicked ()), this, SLOT (startExport ()));
   connect (m_reexportButton, SIGNAL (clicked ()), this, SLOT (startReexport ()));

   m_exportWizard = 0;

   for (int i = 0; i < MaxRecentProjects; ++i)
      m_recentProjectIds[i] = -1;

   m_inputMapActionsMenu = new QPopupMenu (m_inputMapListView);
   m_inputMapActionsMenu->insertItem (tr ("Display Map"), this, SLOT (displaySelectedInputMap ()));
   m_inputMapActionsMenu->insertItem (tr ("Display Map in Separate Window"),this, SLOT (displaySelectedInputMapSeparately ()));
   m_inputMapActionsMenu->insertSeparator ();
   m_inputMapActionsMenu->insertItem (tr ("Map Properties"), this , SLOT (displaySelectedInputMapProperties ()));

   connect (m_inputMapListView, SIGNAL (contextMenuRequested (QListViewItem *, const QPoint &, int)),
            this, SLOT (popupInputMapActionsMenu (QListViewItem *, const QPoint &, int)));

   m_outputMapActionsMenu = new QPopupMenu (m_outputMapListView);
   m_outputMapActionsMenu->insertItem (tr ("Display Map"), this, SLOT (displaySelectedOutputMap ()));
   m_outputMapActionsMenu->insertItem (tr ("Display Map in Separate Window"),this, SLOT (displaySelectedOutputMapSeparately ()));
   m_outputMapActionsMenu->insertSeparator ();
   m_outputMapActionsMenu->insertItem (tr ("Map Properties"), this , SLOT (displaySelectedOutputMapProperties ()));
   m_outputMapActionsMenu->insertSeparator ();
   m_uploadId = m_outputMapActionsMenu->insertItem (tr ("Upload to Server"), this, SLOT (uploadToServer ()));
   m_cancelUploadId = m_outputMapActionsMenu->insertItem (tr ("Cancel Upload to Server"), this, SLOT (cancelUploadToServer ()));
   m_removeId = m_outputMapActionsMenu->insertItem (tr ("Remove from Server"), this, SLOT (removeFromServer ()));
   m_cancelRemoveId = m_outputMapActionsMenu->insertItem (tr ("Cancel Remove from Server"), this, SLOT (cancelRemoveFromServer ()));

   connect (m_outputMapListView, SIGNAL (contextMenuRequested (QListViewItem *, const QPoint &, int)),
            this, SLOT (popupOutputMapActionsMenu (QListViewItem *, const QPoint &, int)));


   BPAClient::instance ()->addProjectView (this);

}

/*  
 *  Destroys the object and frees any allocated resources
 */
ProjectView::~ProjectView()
{
   BPAClient::instance ()->removeProjectView (this);

   delete m_outputMapActionsMenu;

   if (m_projectHandle)
      CloseCauldronProject (m_projectHandle);
}

void ProjectView::show ()
{
   if (isHidden ())
   {
      ProjectViewBase::show ();
      IncrementNumWindows ();
   }
   else
   {
      raise ();
   }
}

void ProjectView::fileClose ()
{
   if (!isHidden ())
   {
      close (true);
      DecrementNumWindows ();
   }
}

void ProjectView::popupInputMapActionsMenu( QListViewItem* item, const QPoint & point, int )
{
   if (item)
   {
      m_selectedInputItem = item;
      m_inputMapActionsMenu->popup (point);
   }
}

void ProjectView::displaySelectedInputMap (void)
{
   InputMapListViewItem * mvi = (InputMapListViewItem *) m_selectedInputItem;
   assert (mvi);
   const InputValue * iv = (const InputValue *) mvi->getAttribute ();
   displayInputValue (iv);
}

void ProjectView::displaySelectedInputMapSeparately (void)
{
   InputMapListViewItem * mvi = (InputMapListViewItem *) m_selectedInputItem;
   assert (mvi);
   const InputValue * iv = (const InputValue *) mvi->getAttribute ();

   GridMapView * gmv = new GridMapView;
   m_gridMapViews.append (gmv);

   gmv->show ();
   displayInputValue (iv, gmv);
}

void ProjectView::displaySelectedInputMapProperties (void)
{
   InputMapListViewItem * mvi = (InputMapListViewItem *) m_selectedInputItem;
   assert (mvi);
   const InputValue * iv = (const InputValue *) mvi->getAttribute ();
   displayInputValueProperties (iv);
}

void ProjectView::popupOutputMapActionsMenu( QListViewItem* item, const QPoint & point, int )
{
   if (!item) return;
   
   m_selectedOutputItem = item;

   OutputMapListViewItem * mvi = (OutputMapListViewItem *) m_selectedOutputItem;
   assert (mvi);

   m_outputMapActionsMenu->setItemVisible (m_uploadId,
	 !mvi->uploaded () && !mvi->selectedForUpload ());
   m_outputMapActionsMenu->setItemVisible (m_cancelUploadId,
	 !mvi->uploaded () && mvi->selectedForUpload ());
   
   m_outputMapActionsMenu->setItemVisible (m_removeId,
	 mvi->uploaded () && !mvi->selectedForRemoval ());
   m_outputMapActionsMenu->setItemVisible (m_cancelRemoveId,
	 mvi->uploaded () && mvi->selectedForRemoval ());

   m_outputMapActionsMenu->popup (point);

}

void ProjectView::displaySelectedOutputMap (void)
{
   OutputMapListViewItem * mvi = (OutputMapListViewItem *) m_selectedOutputItem;
   assert (mvi);
   const PropertyValue * pv = (const PropertyValue *) mvi->getAttribute ();
   displayPropertyValue (pv);
}

void ProjectView::displaySelectedOutputMapSeparately (void)
{
   OutputMapListViewItem * mvi = (OutputMapListViewItem *) m_selectedOutputItem;
   assert (mvi);
   const PropertyValue * pv = (const PropertyValue *) mvi->getAttribute ();

   GridMapView * gmv = new GridMapView;
   m_gridMapViews.append (gmv);

   gmv->show ();
   displayPropertyValue (pv, gmv);
}

void ProjectView::displaySelectedOutputMapProperties (void)
{
   OutputMapListViewItem * mvi = (OutputMapListViewItem *) m_selectedOutputItem;
   assert (mvi);
   const PropertyValue * pv = (const PropertyValue *) mvi->getAttribute ();
   displayPropertyValueProperties (pv);
}

void ProjectView::updateOutputMapListView (void)
{
   QListViewItemIterator it (m_outputMapListView);

   while (it.current ())
   {
      OutputMapListViewItem * mvi = (OutputMapListViewItem *) it.current ();
      assert (mvi);
      const PropertyValue * pv = (const PropertyValue *) mvi->getAttribute ();
      assert (pv);
      mvi->setSelectForUpload (false);
      if (existsOnServer () && pv->isUploadedToBPA ())
      {
	 mvi->setUploaded (true);
      }

      ++it;
   }
}

void ProjectView::uploadToServer (void)
{
   QListViewItemIterator it (m_outputMapListView);

   while (it.current ())
   {
      OutputMapListViewItem * mvi = (OutputMapListViewItem *) it.current ();
      assert (mvi);
      if (!mvi->uploaded () && mvi->isSelected ())
	 mvi->setSelectForUpload (true);
      ++it;
   }
}

void ProjectView::cancelUploadToServer (void)
{
   QListViewItemIterator it (m_outputMapListView);

   while (it.current ())
   {
      OutputMapListViewItem * mvi = (OutputMapListViewItem *) it.current ();
      assert (mvi);
      if (!mvi->uploaded () && mvi->isSelected ())
	 mvi->setSelectForUpload (false);
      ++it;
   }
}

void ProjectView::removeFromServer (void)
{
   QListViewItemIterator it (m_outputMapListView);

   while (it.current ())
   {
      OutputMapListViewItem * mvi = (OutputMapListViewItem *) m_selectedOutputItem;
      assert (mvi);
      if (mvi->uploaded () && mvi->isSelected ())
      mvi->setSelectForRemoval (true);
      ++it;
   }
}

void ProjectView::cancelRemoveFromServer (void)
{
   QListViewItemIterator it (m_outputMapListView);

   while (it.current ())
   {
      OutputMapListViewItem * mvi = (OutputMapListViewItem *) m_selectedOutputItem;
      assert (mvi);
      if (mvi->uploaded () && mvi->isSelected ())
      mvi->setSelectForRemoval (false);
      ++it;
   }
}

PropertyValueList * ProjectView::getPropertyValuesToUpload (void)
{
   PropertyValueList * pvl = new PropertyValueList;

   QListViewItemIterator it (m_outputMapListView);

   while (it.current ())
   {
      OutputMapListViewItem * mvi = (OutputMapListViewItem *) it.current ();
      if (mvi->selectedForUpload () ||
	 (existsOnServer () && mvi->uploaded () && !mvi->selectedForRemoval ()))
      {
	 const PropertyValue * pv = mvi->getAttribute ();
	 QString title;

	 getPropertyValueTitle (pv, title);
	 cerr << "PropertyValue to upload: " << title << endl;

	 pvl->push_back (pv);
      }
      ++it;
   }

   return pvl;
}


/*
 * public slot
 */
void ProjectView::connectToBPA()
{
   BPAClient * client = BPAClient::instance ();
   client->showNormal ();
   if (!client->isLoggedIn ())
   {
      client->startLogin ();
   }
}

/*
 * public slot
 */
void ProjectView::startExport()
{
   BPAClient * client = BPAClient::instance ();

   if (!client->isLoggedIn ())
   {
      client->startLogin ();
      connect (client, SIGNAL (loginSucceeded ()), this, SLOT (continueExport ()));
      connect (client, SIGNAL (loginFailed ()), this, SLOT (finishExport ()));
   }
   else
   {
      continueExport ();
   }
}

/*
 * public slot
 */
void ProjectView::startReexport()
{
   BPAClient * client = BPAClient::instance ();

   assert (client->isLoggedIn ());
   continueReexport ();
}

/*
 * public slot
 */
void ProjectView::continueExport()
{
   BPAClient * client = BPAClient::instance ();
   disconnect (client, SIGNAL (loginSucceeded ()), this, SLOT (continueExport ()));

   m_exportWizard = new ExportWizard (m_projectHandle, this);
   m_exportWizard->setModal (true);
   m_exportWizard->show ();

   connect (m_exportWizard, SIGNAL (finished ()), this, SLOT (finishExport()));

   m_exportWizard->startExport ();
}

/*
 * public slot
 */
void ProjectView::continueReexport()
{
   BPAClient * client = BPAClient::instance ();
   disconnect (client, SIGNAL (loginSucceeded ()), this, SLOT (continueExport ()));

   m_exportWizard = new ExportWizard (m_projectHandle, this);
   m_exportWizard->setModal (true);
   m_exportWizard->show ();

   connect (m_exportWizard, SIGNAL (finished ()), this, SLOT (finishExport()));

   const string & projectName = m_projectHandle->getBPAName ();
   const string & ownerOrgName = m_projectHandle->getBPAOwnerOrg ();

   m_exportWizard->startReexport (ownerOrgName, projectName);
}

/*
 * public slot
 */
void ProjectView::finishExport()
{
   BPAClient * client = BPAClient::instance ();
   disconnect (client, SIGNAL (loginFailed ()), this, SLOT (finishExport ()));

   if (m_exportWizard)
   {
      delete m_exportWizard;
      m_exportWizard = 0;
   }

   updateOutputMapListView ();
}

/*
 * public slot
 */
void ProjectView::projectOpen()
{
   QString fn = QFileDialog::getOpenFileName (QString::null, "Project files (*.project[13]d)", this);

   if (!fn.isEmpty ())
   {
      loadProject (fn);
   }
}

/*
 * public slot
 */
void ProjectView::filePrint()
{
    qWarning( "ProjectView::filePrint() not yet implemented!" );
}

extern QApplication * app;
/*
 * public slot
 */
void ProjectView::fileExit()
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
void ProjectView::helpIndex()
{
    qWarning( "ProjectView::helpIndex() not yet implemented!" );
}

/*
 * public slot
 */
void ProjectView::helpContents()
{
    qWarning( "ProjectView::helpContents() not yet implemented!" );
}

/*
 * public slot
 */
void ProjectView::helpAbout()
{
   BPAClient::instance ()->helpAbout ();
}

/*
 * public slot
 */
void ProjectView::ProjectViewBase_toolBarPositionChanged( QToolBar * )
{
    qWarning( "ProjectView::ProjectViewBase_toolBarPositionChanged( QToolBar * ) not yet implemented!" );
}

/*
 * public slot
 */
void ProjectView::ProjectViewBase_dockWindowPositionChanged( QDockWindow * )
{
    qWarning( "ProjectView::ProjectViewBase_dockWindowPositionChanged( QDockWindow * ) not yet implemented!" );
}

/*
 * public slot
 */
void ProjectView::inputItemSelected( QListViewItem * item )
{
   InputMapListViewItem * mvi = (InputMapListViewItem *) item;
   const InputValue * iv = (const InputValue *) mvi->getAttribute ();
   displayInputValue (iv);
}

void ProjectView::displayInputValue (const InputValue * iv, GridMapView * gmv)
{
   if (!iv) return;

   if (gmv == 0) gmv = m_gridMapView;
   assert (gmv);
   
   QString title;
   getInputValueTitle (iv, title);

   const GridMap * gm = iv->getGridMap ();

   displayGridMap (gm, title, gmv);
}

void ProjectView::getInputValueTitle (const InputValue * iv, QString & title)
{
   if (!iv) return;

   bool commaNeeded = false;

   title = title.append ("Input: %2, Age: %1\n");
   title = title.arg (iv->getEventAge ()).arg (iv->getPropertyName ().c_str ());

   if (iv->getSurfaceName ().size () != 0)
   {
      if (commaNeeded) title = title.append (", ");
      title = title.append ("Surface: %1");
      title = title.arg ( iv->getSurfaceName ().c_str ());
      commaNeeded = true;
   }

   if (iv->getFormationName ().size () != 0)
   {
      if (commaNeeded) title = title.append (", ");
      title = title.append ("Formation: %1");
      title = title.arg ( iv->getFormationName ().c_str ());
      commaNeeded = true;
   }

   if (iv->getReservoirName ().size () != 0)
   {
      if (commaNeeded) title = title.append (", ");
      title = title.append ("Reservoir: %1");
      title = title.arg ( iv->getReservoirName ().c_str ());
      commaNeeded = true;
   }

}

void ProjectView::displayInputValueProperties (const InputValue * iv)
{
   if (!iv) return;

   QString title;
   getInputValueTitle (iv, title);
   title = title.append ("\n");
   getGridMapDescription (iv->getGridMap (), title);

   QMessageBox::information (this, "Input Value Properties", title, QMessageBox::Ok);
}


/*
 * public slot
 */
void ProjectView::outputItemSelected( QListViewItem * item )
{
   OutputMapListViewItem * mvi = (OutputMapListViewItem *) item;
   const PropertyValue * pv = (const PropertyValue *) mvi->getAttribute ();
   displayPropertyValue (pv);
}


void ProjectView::displayPropertyValue (const PropertyValue * pv, GridMapView * gmv)
{
   if (!pv) return;

   if (gmv == 0) gmv = m_gridMapView;
   assert (gmv);

   QString title;
   getPropertyValueTitle (pv, title);

   const GridMap * gm = pv->getGridMap ();

   displayGridMap (gm, title, gmv);
}

void ProjectView::getPropertyValueTitle (const PropertyValue * pv, QString & title)
{
   if (!pv) return;

   bool commaNeeded = false;

   title = title.append ("Output: %2 (%3), Age: %1\n");
   title = title.arg (pv->getSnapshot ()->getTime ()).arg (pv->getProperty ()->getName ().c_str ()).arg (pv->getProperty ()->getUnit ().c_str ());

   if (pv->getSurface () != 0)
   {
      if (commaNeeded) title = title.append (", ");
      title = title.append ("Surface: %1");
      title = title.arg ( pv->getSurface ()->getName ().c_str ());
      commaNeeded = true;
   }

   if (pv->getFormation () != 0)
   {
      if (commaNeeded) title = title.append (", ");
      title = title.append ("Formation: %1");
      title = title.arg ( pv->getFormation ()->getName ().c_str ());
      commaNeeded = true;
   }

   if (pv->getReservoir () != 0)
   {
      if (commaNeeded) title = title.append (", ");
      title = title.append ("Reservoir: %1");
      title = title.arg ( pv->getReservoir ()->getName ().c_str ());
      commaNeeded = true;
   }
}

void ProjectView::displayPropertyValueProperties (const PropertyValue * pv)
{
   if (!pv) return;

   QString title;
   getPropertyValueTitle (pv, title);
   title = title.append ("");
   getGridMapDescription (pv->getGridMap (), title);

   QMessageBox::information (this, "Output Value Properties", title, QMessageBox::Ok);
}

void ProjectView::getGridMapDescription (const GridMap * gm , QString & description)
{
   if (gm)
   {
      const Grid *g = gm->getGrid ();

      description = description.append ("GridMap: NumX = %1, NumY = %2,\n"
	 "MinX = %3, MinY = %4,\n" "MaxX = %5, MaxY = %6,\nMin = %7, Max = %8");
      description = description.arg (g->numI ());
      description = description.arg (g->numJ ());
      description = description.arg (g->minI ());
      description = description.arg (g->minJ ());
      description = description.arg (g->maxI ());
      description = description.arg (g->maxJ ());

      double min;
      double max;

      gm->getMinMaxValue (min, max);
      description = description.arg (min);
      description = description.arg (max);
   }
   else
   {
      description = description.append ("GridMap: Unable to retrieve");
   }
}
void ProjectView::displayGridMap(const GridMap * gm, const QString & title, GridMapView * gmv )
{
   if (gmv == 0)
      gmv = m_gridMapView;
   assert (gmv);

   gmv->setGridMap (gm);

   if (gmv == m_gridMapView)
   {
      gmv->setTitle (title);
      QString status;
      getGridMapDescription (gm, status);
      statusBar ()->message (status);
   }
   else
   {
      gmv->hideTitle ();
      gmv->setCaption (title);
   }
}


void ProjectView::loadProject (const QString & fileName)
{
   QApplication::setOverrideCursor (QCursor::WaitCursor);
   m_inputMapListView->clear ();
   m_outputMapListView->clear ();
   m_gridMapViews.clear ();
   // m_outputMapListView->setShowSortIndicator( false );
   // m_outputMapListView->setSorting ( -1 );
   displayGridMap (0, "");

   if (m_projectHandle)
      CloseCauldronProject (m_projectHandle);

   m_projectHandle = OpenCauldronProject (fileName.ascii (), "rw");

   QString status = "Opening Project %1 %2";
   status = status.arg (fileName);

   QString caption = "Cauldron Project: ";

   if (m_projectHandle)
   {
      status = status.arg ("succeeded");
      caption += fileName;
      statusBar ()->message (status);
      setCaption(caption);

      QString fullFileName = QFileInfo (fileName).absFilePath ();
      BPAClient::instance ()->addRecentProject (fullFileName);
   }
   else
   {
      status = status.arg ("failed");
      caption += "None";
      statusBar ()->message (status);
      setCaption(caption);
      QApplication::restoreOverrideCursor ();
      return;
   }

   checkServerPresence ();

   if (!existsOnServer ())
   {
      m_reexportButton->setEnabled (false);
   }
   else
   {
      m_reexportButton->setEnabled (true);
   }

   /// processing input maps
   InputValueList *ivList = m_projectHandle->getInputValues ();

   InputValueList::iterator ivIter;

   for (ivIter = ivList->begin (); ivIter != ivList->end (); ++ivIter)
   {
      const InputValue *iv = *ivIter;

      InputMapListViewItem *item = new InputMapListViewItem (m_inputMapListView);
      item->setAttribute (iv);

      char eventAgeStr[20];
      sprintf (eventAgeStr,"%8.3lf ", iv->getEventAge ());
      QString eventAgeValue;
      eventAgeValue.setNum (iv->getEventAge ());
      item->setText (InputMapListViewItem::AgeColumn, eventAgeStr);
      item->setText (InputMapListViewItem::PropertyColumn, iv->getPropertyName ().c_str ());

      item->setText (InputMapListViewItem::SurfaceColumn, iv->getSurfaceName ().c_str ());
      item->setText (InputMapListViewItem::FormationColumn, iv->getFormationName ().c_str ());
      item->setText (InputMapListViewItem::ReservoirColumn, iv->getReservoirName ().c_str ());
   }


   /// processing output maps
   PropertyValueList *pvList = m_projectHandle->getPropertyValues (SURFACE | FORMATION |
                                                                 FORMATIONSURFACE | RESERVOIR, 0, 0, 0, 0, 0, SURFACE);

   PropertyValueList::iterator pvIter;

   QString snapshotValue;
   char snapshotStr[20];

#ifdef USESUBLISTS
   const Snapshot *prevSnapshot = 0;

   OutputMapListViewItem *snapshotItem = 0;
#endif

   for (pvIter = pvList->begin (); pvIter != pvList->end (); ++pvIter)
   {
      OutputMapListViewItem *item = 0;

      const PropertyValue *propval = *pvIter;

      const Snapshot *curSnapshot = propval->getSnapshot ();

      snapshotValue.setNum (curSnapshot->getTime ());
      sprintf (snapshotStr,"%8.3lf ", curSnapshot->getTime ());

#ifdef USESUBLISTS
      if (prevSnapshot != curSnapshot)
      {
         snapshotItem = new OutputMapListViewItem (m_outputMapListView);
         snapshotItem->setExpandable (true);
         snapshotItem->setOpen (true);
         snapshotItem->setText (OutputMapListViewItem::AgeColumn, snapshotStr);
	 snapshotItem->setAttribute (0);
         prevSnapshot = curSnapshot;
      }

      item = new OutputMapListViewItem (snapshotItem);
#else
      item = new OutputMapListViewItem (m_outputMapListView);
#endif

      item->setAttribute (propval);

      item->setText (OutputMapListViewItem::AgeColumn, snapshotStr);

      item->setText (OutputMapListViewItem::PropertyColumn, propval->getProperty ()->getName ().c_str ());

      const Surface *surface = propval->getSurface ();

      if (surface)
      {
         item->setText (OutputMapListViewItem::SurfaceColumn, surface->getName ().c_str ());
      }

      const Formation *formation = propval->getFormation ();

      if (formation)
      {
         item->setText (OutputMapListViewItem::FormationColumn, formation->getName ().c_str ());
      }

      const Reservoir *reservoir = propval->getReservoir ();

      if (reservoir)
      {
         item->setText (OutputMapListViewItem::ReservoirColumn, reservoir->getName ().c_str ());
      }
   }

   updateOutputMapListView ();

   QApplication::restoreOverrideCursor ();
}

void ProjectView::checkServerPresence ()
{
   m_onServer = false;

   BPAClient *client = BPAClient::instance ();

   const string & projectName = m_projectHandle->getBPAName ();
   const string & ownerOrgName = m_projectHandle->getBPAOwnerOrg ();

   if (ownerOrgName.length () != 0 && projectName.length () != 0)
   {
      const char * tmpDir = createTmpDir ();
      if (!tmpDir) return;

      char projName[128];
      char fullProjName[256];

      sprintf (projName, "tmp%ld.project3d", getpid ());
      sprintf (fullProjName, "%s/%s", tmpDir, projName);

      if (client->downloadProject (ownerOrgName, projectName, tmpDir, projName, true))
      {
	 string checksum = m_projectHandle->computeChecksum (fullProjName);
	 if (checksum == m_projectHandle->getBPAChecksum ())
	 {
	    m_onServer = true;
	 }
	 unlink (fullProjName);
      }
      rmdir (tmpDir);
   }
}

bool ProjectView::existsOnServer ()
{
   return m_onServer;
}

void ProjectView::openRecentProject(int param)
{
   BPAClient * client = BPAClient::instance ();
   loadProject(client->getRecentProject(param));
}

void ProjectView::updateRecentProjects (const QStringList & projectList)
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

