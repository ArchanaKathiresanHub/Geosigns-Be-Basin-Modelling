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

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <string>
#include <vector>
using namespace std;

#include "exportwizard.h"
#include "bpaclient.h"
#include "projectview.h"
#include "misc.h"

#include <qapplication.h>
#include <qcursor.h>
#include <qlistbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qmutex.h>

#include "BPAClientLibrary.hpp"

#include "Interface/ProjectHandle.h"

using namespace DataAccess;

using Interface::PropertyValueList;

/* 
 *  Constructs a ExportWizard which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The wizard will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal wizard.
 */
ExportWizard::ExportWizard( Interface::ProjectHandle * projectHandle, ProjectView * parent,  const char* name, bool modal, WFlags fl )
    : ExportWizardBase( parent, name, modal, fl ), InfoBase (), m_projectHandle (projectHandle),
      m_projectInfo (0), m_highlightCS (false), m_highlightGT (false)
{
   BPAClient *client = BPAClient::instance ();
   setCauldronBPA (client->getCauldronBPA ());

   setNextEnabled (m_namePage, false);
   setNextEnabled (m_basinsPage, false);
   setFinishEnabled (m_geoPage, false);

   connect (m_nameLine, SIGNAL (textChanged (const QString &)),
	 this, SLOT ( evaluateNamePage ()));

   connect (m_orgs, SIGNAL (selectionChanged ()),
	 this, SLOT (displayOwnerOrgs ()));

   connect (m_regions, SIGNAL (selectionChanged ()),
	 this, SLOT (displayRegions ()));

   connect (m_countries, SIGNAL (selectionChanged ()),
	 this, SLOT (displayCountries ()));

   connect (m_basins, SIGNAL (selectionChanged ()),
	 this, SLOT (displayBasins ()));

   connect (m_basins, SIGNAL (selectionChanged ()),
	 this, SLOT (displayBasinSelection ()));

   connect (m_coordinateSystems, SIGNAL (selectionChanged ()),
	 this, SLOT (displayCoordinateSystems ()));

   connect (m_coordinateSystems, SIGNAL (selectionChanged ()),
	 this, SLOT (displayGeoTransformSelection ()));

   connect (m_coordinateSystems, SIGNAL (selectionChanged ()),
	 this, SLOT (displayCoordinateSystemSelection ()));

   connect (m_geoTransforms, SIGNAL (selectionChanged ()),
	 this, SLOT (displayGeoTransforms ()));

   connect (m_geoTransforms, SIGNAL (selectionChanged ()),
	 this, SLOT (displayGeoTransformSelection ()));

   connect (m_coordinateSystemLine, SIGNAL (textChanged (const QString & )), this, SLOT (highlightCoordinateSystem(const QString &)));
   connect (m_coordinateSystemLine, SIGNAL (returnPressed ()), this, SLOT (displayCoordinateSystemSelection()));

   connect (m_geoTransformLine, SIGNAL (textChanged (const QString & )), this, SLOT (highlightGeoTransform(const QString &)));
   connect (m_geoTransformLine, SIGNAL (returnPressed ()), this, SLOT (displayGeoTransformSelection()));

}

/*  
 *  Destroys the object and frees any allocated resources
 */
ExportWizard::~ExportWizard()
{
   deleteGeoTransforms ();
   deleteCoordinateSystems ();
   deleteBasins ();
   deleteCountries ();
   deleteRegions ();
   deleteOwnerOrgs ();
}

/*
 * public slot
 */
void ExportWizard::evaluateNamePage ()
{
   setNextEnabled (m_namePage, m_nameLine->text ().length () != 0);
}

ProjectView * ExportWizard::getProjectView (void)
{
   return (ProjectView *) parentWidget ();
}

string ExportWizard::getName ()
{
   return m_nameLine->text ().ascii ();
}

string ExportWizard::getDescription ()
{
   return m_descriptionText->text ().ascii ();
}

/*
 * public slot
 */
void ExportWizard::startExport()
{
   BPAClient *client = BPAClient::instance ();
   QString server;
   QString name;
   QString password;

   QApplication::setOverrideCursor (QCursor::WaitCursor);
   client->getLoginProperties (server, name, password);

   copyFrom (client);
   loadCoordinateSystems ();
   loadGeoTransforms ();

   displayOwnerOrgs ();

   QApplication::restoreOverrideCursor ();
}

void ExportWizard::startReexport (string ownerOrgName, string projectName)
{
   startExport ();
   m_projectInfo = findProjectInfo (ownerOrgName, projectName);
   if (!m_projectInfo)
      cerr << "Could not find: " << ownerOrgName << "::" << projectName << endl;

   m_nameLine->setText (projectName.c_str ());
}

ListSelectionStatus ExportWizard::displayOwnerOrgs (void)
{
   ListSelectionStatus status = InfoBase::displayOwnerOrgs (m_orgs);
   displayRegions ();
   return status;
}

ListSelectionStatus ExportWizard::displayRegions (void)
{
   ListSelectionStatus status = InfoBase::displayRegions (m_regions);
   displayCountries ();
   return status;
}

ListSelectionStatus ExportWizard::displayCountries (void)
{
   ListSelectionStatus status = InfoBase::displayCountries (m_countries);
   displayBasins ();
   return status;
}

ListSelectionStatus ExportWizard::displayBasins (void)
{
   ListSelectionStatus status = InfoBase::displayBasins (m_basins);
   displayCoordinateSystems ();
   return status;
}

ListSelectionStatus ExportWizard::displayCoordinateSystems (void)
{
   ListSelectionStatus status = InfoBase::displayCoordinateSystems (m_coordinateSystems);
   loadGeoTransforms ();
   displayGeoTransforms ();
   return status;
}

ListSelectionStatus ExportWizard::displayGeoTransforms (void)
{
   ListSelectionStatus status = InfoBase::displayGeoTransforms (m_geoTransforms);
   return status;
}

void ExportWizard::displayBasinSelection (void)
{
   if (m_active) return;
   QString basinList;
   vector<BasinInfo *>::iterator iter;
   bool first = true;

   bool enableNextPage = false;
   for (iter= m_basinInfos.begin (); iter != m_basinInfos.end (); ++iter)
   {
      BasinInfo * basinInfo = * iter;
      if (basinInfo->isSelected ())
      {
	 if (first)
	 {
	    enableNextPage = true;
	    first = false;
	 }
	 else basinList += ", ";

	 basinList += basinInfo->getName ().c_str ();
      }
   }
   setNextEnabled (m_basinsPage, enableNextPage);

   m_basinsLine->setText (basinList);
}

void ExportWizard::highlightCoordinateSystem (const QString & partialName)
{
   // cerr << "start highlighting\n";
   vector<CoordinateSystemInfo *>::iterator iter;
   for (iter= m_coordinateSystemInfos.begin (); iter != m_coordinateSystemInfos.end (); ++iter)
   {
      CoordinateSystemInfo * coordinateSystemInfo = * iter;
      if (strncmp (coordinateSystemInfo->getName ().c_str (), partialName.ascii (), strlen (partialName.ascii ())) == 0)
      {
	 m_highlightCS = true;
	 coordinateSystemInfo->setCurrent ();
	 m_highlightCS = false;
	 break;
      }
   }
   // cerr << "finish highlighting\n";
}

void ExportWizard::displayCoordinateSystemSelection (void)
{
   // cerr << "start displaying\n";
   if (m_active) return;
   if (m_highlightCS)
   {
      // cerr << "stop displaying\n";
      return;
   }
   QString coordinateSystemList;
   vector<CoordinateSystemInfo *>::iterator iter;
   bool first = true;

   for (iter= m_coordinateSystemInfos.begin (); iter != m_coordinateSystemInfos.end (); ++iter)
   {
      CoordinateSystemInfo * coordinateSystemInfo = * iter;
      if (coordinateSystemInfo->isSelected ())
      {
	 if (first)
	 {
	    first = false;
	 }
	 else coordinateSystemList += ", ";

	 coordinateSystemList += coordinateSystemInfo->getName ().c_str ();
      }
   }

   m_coordinateSystemLine->setText (coordinateSystemList);
   // cerr << "finish displaying\n";
}

void ExportWizard::highlightGeoTransform (const QString & partialName)
{
   vector<GeoTransformInfo *>::iterator iter;
   for (iter= m_geoTransformInfos.begin (); iter != m_geoTransformInfos.end (); ++iter)
   {
      GeoTransformInfo * geoTransformInfo = * iter;
      if (strncmp (geoTransformInfo->getName ().c_str (), partialName.ascii (), strlen (partialName.ascii ())) == 0)
      {
	 m_highlightGT = true;
	 geoTransformInfo->setCurrent ();
	 m_highlightGT = false;
	 break;
      }
   }
}

void ExportWizard::displayGeoTransformSelection (void)
{
   if (m_active) return;
   if (m_highlightGT) return;

   QString geoTransformList;
   vector<GeoTransformInfo *>::iterator iter;
   bool first = true;

   bool enableFinish = false;
   for (iter= m_geoTransformInfos.begin (); iter != m_geoTransformInfos.end (); ++iter)
   {
      GeoTransformInfo * geoTransformInfo = * iter;
      if (geoTransformInfo->isSelected ())
      {
	 if (first)
	 {
	    enableFinish = true;
	    first = false;
	 }
	 else geoTransformList += ", ";

	 geoTransformList += geoTransformInfo->getName ().c_str ();
      }
   }
   setFinishEnabled (m_geoPage, enableFinish);

   m_geoTransformLine->setText (geoTransformList);
}

OwnerOrgInfo * ExportWizard::getOwnerOrgInfoSelected (void)
{
   vector<OwnerOrgInfo *>::iterator iter;
   for (iter= m_ownerOrgInfos.begin (); iter != m_ownerOrgInfos.end (); ++iter)
   {
      OwnerOrgInfo * ownerOrgInfo = * iter;
      if (ownerOrgInfo->isSelected ())
      {
	 return ownerOrgInfo;
      }
   }
   return 0;
}

const BPA::CoordinateSystem * ExportWizard::getCoordinateSystemSelected (void)
{
   vector<CoordinateSystemInfo *>::iterator iter;
   for (iter= m_coordinateSystemInfos.begin (); iter != m_coordinateSystemInfos.end (); ++iter)
   {
      CoordinateSystemInfo * coordinateSystemInfo = * iter;
      if (coordinateSystemInfo->isSelected ())
      {
	 const BPA::CoordinateSystem * coordinateSystem = coordinateSystemInfo->getItem ();
	 return coordinateSystem;
      }
   }
   return 0;
}

const BPA::GeoTransform * ExportWizard::getGeoTransformSelected (void)
{
   vector<GeoTransformInfo *>::iterator iter;
   for (iter= m_geoTransformInfos.begin (); iter != m_geoTransformInfos.end (); ++iter)
   {
      GeoTransformInfo * geoTransformInfo = * iter;
      if (geoTransformInfo->isSelected ())
      {
	 const BPA::GeoTransform * geoTransform = geoTransformInfo->getItem ();
	 return geoTransform;
      }
   }
   return 0;
}


vector<const BPA::Basin *> * ExportWizard::getBasinsSelected (void)
{
   vector<const BPA::Basin *> * basinsSelected = new vector<const BPA::Basin *>;

   vector<BasinInfo *>::iterator iter;
   for (iter= m_basinInfos.begin (); iter != m_basinInfos.end (); ++iter)
   {
      BasinInfo * basinInfo = * iter;
      if (basinInfo->isSelected ())
      {
	 const BPA::Basin * basin = basinInfo->getItem ();
	 basinsSelected->push_back (basin);
      }
   }
   return basinsSelected;
}

/*
 * public slot
 */
void ExportWizard::finishExport()
{
   hide ();
}

void ExportWizard::accept()
{
   char hostNameStr[256];

   gethostname (hostNameStr, 256);
   string hostName = hostNameStr;

   string projectName = getName ();
   string description = getDescription ();
   const BPA::OwnerOrg * ownerOrg = getOwnerOrgInfoSelected ()->getItem ();
   const BPA::CoordinateSystem * coordinateSystem = getCoordinateSystemSelected ();
   const BPA::GeoTransform * geoTransform = getGeoTransformSelected ();
   vector < const BPA::Basin * >*basins = getBasinsSelected ();
   string fullName = m_projectHandle->getName ();

   const char * dir = createTmpDir ();

   if (!dir) return;

   string fullPath = dir;

   fullPath += "/";
   fullPath += projectName;
   fullPath += ".project3d";

   PropertyValueList *propertyValuesToUpload = getProjectView ()->getPropertyValuesToUpload ();

   vector < string > inputMapFileNames;
   vector < string > outputMapFileNames;
#ifdef PRODUCELEADS
   ((Leads::ProjectHandle *) m_projectHandle)->createLeads ();
#endif
   if (m_projectHandle->saveForBPA (fullPath, propertyValuesToUpload, inputMapFileNames, outputMapFileNames))
   {
      lockServer ();
      char * login = getlogin ();
      if (!login)
      {
	 cerr << "ExportWizard::accept (): could not retrieve user name" << endl;
	 login = "nouser";
      }

      BPA::CauldronProject * project = 0;
      if (m_projectInfo) project = m_projectInfo->getItem ();

      bool result = m_cauldronBPA->UploadCauldronProject (projectName, description,
	       coordinateSystem, geoTransform, *basins,
	       hostName, login, ownerOrg, fullPath,
	       inputMapFileNames, outputMapFileNames, project);
      unlockServer ();

      if (result)
      {
         BPAClient *client = BPAClient::instance ();
         QString server;
         QString name;
         QString password;

         client->getLoginProperties (server, name, password);

         string checksum = m_projectHandle->computeChecksum (fullPath);

         m_projectHandle->updateOriginal (server.ascii (), projectName,
	       ownerOrg->getName (), checksum, propertyValuesToUpload);
      }
   }

   if (propertyValuesToUpload)
      delete propertyValuesToUpload;

   unlink (fullPath.c_str ());

   int i;

   for (i = 0; i < inputMapFileNames.size (); ++i)
   {
      string path = dir;
      path += "/" + inputMapFileNames[i];
      unlink (path.c_str ());
   }

   for (i = 0; i < outputMapFileNames.size (); ++i)
   {
      string path = dir;
      path += "/" + outputMapFileNames[i];
      unlink (path.c_str ());
   }

   if (outputMapFileNames.size () > 0)
   {
      string path = dir;
      size_t pos = outputMapFileNames[0].rfind ('/');
      if (pos != string::npos)
      {
	 path += '/';
	 path += outputMapFileNames[0].substr (0, pos);
      }
      if (rmdir (path.c_str ()) != 0)
      {
	 perror (path.c_str ());
      }
   }

   rmdir (dir);

   BPAClient *client = BPAClient::instance ();

   client->reloadProjects ();
   client->displayProjects ();
   ExportWizardBase::accept ();
   emit finished ();
}

void ExportWizard::reject()
{
   ExportWizardBase::reject ();
   emit finished();
}
