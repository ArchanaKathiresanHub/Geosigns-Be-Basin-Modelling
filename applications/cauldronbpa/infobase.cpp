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
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include <string>
#include <vector>
using namespace std;

#include <qapplication.h>
#include <qmessagebox.h>
#include <qcursor.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qdatetime.h>
#include <qprogressdialog.h>
#include <qmutex.h>

#include "BPAClientLibrary.hpp"

#include "infobase.h"
#include "bpaclient.h"

extern bool cbTiming;
extern bool cbVerbose;

static QTime timer;

extern QApplication * app;

static char *time2string (const time_t * time);


InfoBase::InfoBase (void)
{
   m_cauldronBPA = 0;
   m_active = 0;

}

InfoBase::~InfoBase (void)
{
}

void InfoBase::copyFrom (InfoBase * infoBase)
{
   setCauldronBPA (infoBase->getCauldronBPA ());

   m_regionInfos = infoBase->m_regionInfos;
   m_ownerOrgInfos = infoBase->m_ownerOrgInfos;
   m_countryInfos = infoBase->m_countryInfos;
   m_basinInfos = infoBase->m_basinInfos;
   m_projectInfos = infoBase->m_projectInfos;
   m_coordinateSystemInfos = infoBase->m_coordinateSystemInfos;
   m_geoTransformInfos = infoBase->m_geoTransformInfos;

   ReplaceByItemCopies (m_regionInfos);
   ReplaceByItemCopies (m_ownerOrgInfos);
   ReplaceByItemCopies (m_countryInfos);
   ReplaceByItemCopies (m_basinInfos);
   ReplaceByItemCopies (m_projectInfos);
   ReplaceByItemCopies (m_coordinateSystemInfos);
   ReplaceByItemCopies (m_geoTransformInfos);

   ClearItemCopies (infoBase->m_ownerOrgInfos);
   ClearItemCopies (infoBase->m_regionInfos);
   ClearItemCopies (infoBase->m_countryInfos);
   ClearItemCopies (infoBase->m_basinInfos);
   ClearItemCopies (infoBase->m_projectInfos);
   ClearItemCopies (infoBase->m_coordinateSystemInfos);
   ClearItemCopies (infoBase->m_geoTransformInfos);
}
void InfoBase::createCauldronBPA (void)
{
   m_cauldronBPA = new BPA::CauldronBPA ();
}

BPA::CauldronBPA * InfoBase::getCauldronBPA (void)
{
   return m_cauldronBPA;
}

void InfoBase::setCauldronBPA (BPA::CauldronBPA * cauldronBPA)
{
   m_cauldronBPA = cauldronBPA;
}

void InfoBase::deleteCauldronBPA (void)
{
   if (m_cauldronBPA) delete m_cauldronBPA;
}

void InfoBase::stayConnected (void)
{
   QString server;
   QString name;
   QString password;

   BPAClient *client = BPAClient::instance ();
   if (cbVerbose)
      cerr << "InfoBase::stayConnected (): started\n";

   if (!client->getLoginProperties (server, name, password))
   {
      return;
   }

   if (cbVerbose)
      cerr << "InfoBase::stayConnected (): Reloading projects\n";
   reloadProjects ();
   if (cbVerbose)
      cerr << "InfoBase::stayConnected (): Connecting projects\n";
   connectProjects ();
   // displayProjects ();

   if (cbVerbose)
      cerr << "InfoBase::stayConnected (): reloading projects succeeded\n";
   return;
}


bool InfoBase::login (const QString & server, const QString & name, const QString & password)
{
   lockServer ();
   bool result = m_cauldronBPA->Login (server.ascii (), name.ascii (), password.ascii ());
   unlockServer ();

   return result;
}

int InfoBase::loadOwnerOrgs ()
{
   int count = 0;
   if (m_active) return 0;

   lockServer ();
   const std::vector<const BPA::OwnerOrg *> * ownerOrgs = m_cauldronBPA->GetOwnerOrgs ();
   unlockServer ();

   if (!ownerOrgs) return false;

   vector<const BPA::OwnerOrg *>::const_iterator iter;

   for (iter = ownerOrgs->begin (); iter != ownerOrgs->end (); ++iter)
   {
      const BPA::OwnerOrg * ownerOrg = * iter;

      OwnerOrgInfo * ownerOrgInfo;
      if ((ownerOrgInfo = findOwnerOrgInfo (ownerOrg->getId ())) == 0)
      {
	 ownerOrgInfo = new OwnerOrgInfo (OwnerOrgType);
	 ownerOrgInfo->setItem (ownerOrg);
	 m_ownerOrgInfos.push_back (ownerOrgInfo);
	 count++;
      }
   }

   return count;
}

void InfoBase::loadRegions (void)
{
   int count = 0;
   if (m_active) return;

   if (cbTiming)
   {
      cerr << "loading regions\n";
      timer.start ();
   }

   QApplication::setOverrideCursor (QCursor::WaitCursor);
   vector<OwnerOrgInfo *>::iterator iter;
   lockServer ();
   for (iter= m_ownerOrgInfos.begin (); iter != m_ownerOrgInfos.end (); ++iter)
   {
      OwnerOrgInfo * ownerOrgInfo = * iter;
      count += loadRegions (ownerOrgInfo);
   }
   unlockServer ();

   QApplication::restoreOverrideCursor ();

   if (cbTiming)
   {
      cerr << "loading " << count << " regions finished\n";
      int duration = timer.elapsed ();
      cerr << "Duration: " << duration << " milliseconds" << endl;
   }
}

int InfoBase::loadRegions (OwnerOrgInfo * ownerOrgInfo)
{
   int count = 0;

   // if (!ownerOrgInfo->isSelected () || ownerOrgInfo->BitValueIsTrue (RegionsDownloaded)) return count;

   ownerOrgInfo->SetBitValue (RegionsDownloaded, true);
   // cerr << "Loading Regions for OwnerOrg: " << ownerOrgInfo->getName () << endl;

   const BPA::OwnerOrg * ownerOrg = ownerOrgInfo->getItem ();

   const std::vector < const BPA::Region * >* regions = m_cauldronBPA->GetRegions (ownerOrg);
   if (!regions) return count;

   vector < const BPA::Region * >::const_iterator iter;

   for (iter = regions->begin (); iter != regions->end (); ++iter)
   {
      const BPA::Region * region = *iter;

      RegionInfo *regionInfo;

      if ((regionInfo = findRegionInfo (region->getId ())) == 0)
      {
         regionInfo = new RegionInfo (RegionType);
         regionInfo->setItem (region);
         m_regionInfos.push_back (regionInfo);
	 count++;
      }
      ownerOrgInfo->addChild (regionInfo);
   }

   return count;
}

void InfoBase::loadCountries (void)
{
   int count = 0;
   if (m_active) return;

   if (cbTiming)
   {
      cerr << "Loading countries\n";
      timer.start ();
   }

   QApplication::setOverrideCursor (QCursor::WaitCursor);
   vector<RegionInfo *>::iterator iter;

   lockServer ();
   for (iter= m_regionInfos.begin (); iter != m_regionInfos.end (); ++iter)
   {
      RegionInfo * regionInfo = * iter;
      count += loadCountries (regionInfo);
   }
   unlockServer ();
   QApplication::restoreOverrideCursor ();

   if (cbTiming)
   {
      cerr << "Loading " << count << " countries finished\n";
      int duration = timer.elapsed ();
      cerr << "Duration: " << duration << " milliseconds" << endl;
   }
}

int InfoBase::loadCountries (RegionInfo * regionInfo)
{
   int count = 0;
   // if (!regionInfo->isSelected () || regionInfo->BitValueIsTrue (CountriesDownloaded)) return count;
   regionInfo->SetBitValue (CountriesDownloaded, true);

   // cerr << "Loading Countries for Region: " << regionInfo->getName () << endl;
   const BPA::Region * region = regionInfo->getItem ();
   const std::vector < const BPA::Country * > * countries;

   countries = m_cauldronBPA->GetCountries (region);

   if (!countries)
      return count;

   vector < const BPA::Country * >::const_iterator iter;

   for (iter = countries->begin (); iter != countries->end (); ++iter)
   {
      const BPA::Country * country = *iter;

      CountryInfo *countryInfo;

      if ((countryInfo = findCountryInfo (country->getId ())) == 0)
      {
         countryInfo = new CountryInfo (CountryType);
         countryInfo->setItem (country);
         m_countryInfos.push_back (countryInfo);
	 count++;
      }
      regionInfo->addChild (countryInfo);
   }
   return count;
}

void InfoBase::loadBasins (void)
{
   if (m_active) return;

   if (cbTiming)
   {
      cerr << "Loading basins\n";
      timer.start ();
   }

   int count = 0;

   QApplication::setOverrideCursor (QCursor::WaitCursor);
   vector<CountryInfo *>::iterator iter;

   BPAClient *client = BPAClient::instance ();
   QProgressDialog progress ("Downloading data ...", "Cancel", m_countryInfos.size (),
	 client, "progressBar", true);
   progress.setTotalSteps (m_countryInfos.size ());
   progress.setMinimumDuration (1000);
   progress.setCancelButton (0);

   int countryIndex = 0;

   lockServer ();
   for (iter= m_countryInfos.begin (); iter != m_countryInfos.end (); ++iter)
   {
      progress.setProgress (countryIndex);
      app->processEvents ();
      CountryInfo * countryInfo = * iter;
      count += loadBasins (countryInfo);

      countryIndex++;
   }
   unlockServer ();

   if (cbTiming)
   {
      cerr << "Loading " << count << " basins finished\n";
      int duration = timer.elapsed ();
      cerr << "Duration: " << duration << " milliseconds" << endl;
   }

   if (count)
   {
      connectProjects ();
   }
   QApplication::restoreOverrideCursor ();

}

int InfoBase::loadBasins (CountryInfo * countryInfo)
{
   int count = 0;
   // if (!countryInfo->isSelected () || countryInfo->BitValueIsTrue (BasinsDownloaded)) return count;
   countryInfo->SetBitValue (BasinsDownloaded, true);

   const BPA::Country * country = countryInfo->getItem ();
   const std::vector < const BPA::Basin * >*basins;

   basins = m_cauldronBPA->GetBasinsForCountry (country);

   if (!basins)
      return count;

   vector < const BPA::Basin * >::const_iterator iter;

   for (iter = basins->begin (); iter != basins->end (); ++iter)
   {
      const BPA::Basin * basin = *iter;

      BasinInfo *basinInfo;

      if ((basinInfo = findBasinInfo (basin->getId ())) == 0)
      {
         basinInfo = new BasinInfo (BasinType);
         basinInfo->setItem (basin);
         m_basinInfos.push_back (basinInfo);
	 count++;
      }
      countryInfo->addChild (basinInfo);
   }
   return count;
}

void InfoBase::reloadProjects(void)
{
   vector<OwnerOrgInfo *>::iterator iter;
   for (iter= m_ownerOrgInfos.begin (); iter != m_ownerOrgInfos.end (); ++iter)
   {
      OwnerOrgInfo * ownerOrgInfo = * iter;
      ownerOrgInfo->SetBitValue (ProjectsDownloaded, false);
   }
   loadProjects ();
}

void InfoBase::loadProjects(void)
{
   int count = 0;
   if (m_active) return;

   if (cbTiming)
   {
      cerr << "Loading projects\n";
      timer.start ();
   }

   vector<OwnerOrgInfo *>::iterator iter;
   lockServer ();
   for (iter= m_ownerOrgInfos.begin (); iter != m_ownerOrgInfos.end (); ++iter)
   {
      OwnerOrgInfo * ownerOrgInfo = * iter;
      // cerr << "Starting to load Projects for OwnerOrgInfo: " << ownerOrgInfo->getName () << endl;
      count += loadProjects (ownerOrgInfo);
   }
   unlockServer ();

   if (cbTiming)
   {
      cerr << "Loading " << count << " projects finished\n";
      int duration = timer.elapsed ();
      cerr << "Duration: " << duration << " milliseconds" << endl;
   }
}

int InfoBase::loadProjects (OwnerOrgInfo * ownerOrgInfo)
{
   int count = 0;
   if (ownerOrgInfo->BitValueIsTrue (ProjectsDownloaded))
   {
      // cerr << "Already loaded Projects for OwnerOrg: " << ownerOrgInfo->getName () << endl;
      return count;
   }
   ownerOrgInfo->SetBitValue (ProjectsDownloaded, true);

   // cerr << "Loading Projects for OwnerOrg: " << ownerOrgInfo->getName () << endl;

   const BPA::OwnerOrg * ownerOrg = ownerOrgInfo->getItem ();
   std::vector < BPA::CauldronProject * > * projects;

   projects = m_cauldronBPA->GetCauldronProjects (ownerOrg);

   if (!projects)
      return count;

   vector < BPA::CauldronProject * >::iterator iter;


   for (iter = projects->begin (); iter != projects->end (); ++iter)
   {
      BPA::CauldronProject * project = *iter;

      ProjectInfo *projectInfo;

      // cerr << "Obtained Project: " << project->getName () << endl;
      if ((projectInfo = findProjectInfo (project->getId ())) == 0)
      {
	 projectInfo = new ProjectInfo (ProjectType);
	 projectInfo->setItem (project);
	 m_projectInfos.push_back (projectInfo);
	 // cerr << "Added project: " << project->getName () << endl;
	 count++;
      }
      else
      {
	 BPA::CauldronProject * oldProject = projectInfo->getItem ();
	 projectInfo->setItem (project);

	 if (oldProject && oldProject != project) delete oldProject;
      }

      ownerOrgInfo->addChild (projectInfo);
   }

   delete projects;

   return count;
}

void InfoBase::connectProjects (void)
{
   if (cbTiming)
   {
      cerr << "Connecting projects\n";
      timer.start ();
   }

   vector<ProjectInfo *>::iterator iter;

   lockServer ();
   for (iter= m_projectInfos.begin (); iter != m_projectInfos.end (); ++iter)
   {
      ProjectInfo * projectInfo = * iter;
      BPA::CauldronProject * project = projectInfo->getItem ();

      string tail = project->getBasinNames ();
      string basinName;
      size_t comma;

      do
      {
         comma = tail.find (',');
         if (comma != string::npos)
         {
            basinName = tail.substr (0, comma);
            tail = tail.substr (comma + 1, string::npos);
         }
         else
         {
            basinName = tail;
         }
         BasinInfo *basinInfo = findBasinInfo (basinName);

         if (basinInfo && !basinInfo->hasChild (projectInfo))
         {
            basinInfo->addChild (projectInfo);
         }
      }
      while (comma != string::npos);
   }
   unlockServer ();

   if (cbTiming)
   {
      cerr << "Connecting projects finished\n";
      int duration = timer.elapsed ();
      cerr << "Duration: " << duration << " milliseconds" << endl;
   }
}

int InfoBase::loadCoordinateSystems (void)
{
   int count = 0;
   if (m_active) return count;

   if (cbTiming)
   {
      cerr << "loading coordinate systems\n";
      timer.start ();
   }

   QApplication::setOverrideCursor (QCursor::WaitCursor);
   const std::vector < const BPA::CoordinateSystem * >*coordinateSystems;

   lockServer ();
   coordinateSystems = m_cauldronBPA->GetCoordinateSystems ();
   unlockServer ();

   if (!coordinateSystems)
   {
      cerr << "No coordinate systems downloaded\n";
      return count;
   }

   vector < const BPA::CoordinateSystem * >::const_iterator iter;

   for (iter = coordinateSystems->begin (); iter != coordinateSystems->end (); ++iter)
   {
      const BPA::CoordinateSystem * coordinateSystem = *iter;

      CoordinateSystemInfo *coordinateSystemInfo;

      if ((coordinateSystemInfo = findCoordinateSystemInfo (coordinateSystem->getId ())) == 0)
      {
         coordinateSystemInfo = new CoordinateSystemInfo (CoordinateSystemType);
         coordinateSystemInfo->setItem (coordinateSystem);
         m_coordinateSystemInfos.push_back (coordinateSystemInfo);
	 count++;
      }
   }
   QApplication::restoreOverrideCursor ();

   if (cbTiming)
   {
      cerr << "loading << " << count << " coordinate systems finished\n";
      int duration = timer.elapsed ();
      cerr << "Duration: " << duration << " milliseconds" << endl;
   }

   return count;
}

void InfoBase::loadGeoTransforms (void)
{
   int count = 0;
   if (m_active) return;

   if (cbTiming)
   {
      cerr << "loading geotransforms\n";
      timer.start ();
   }

   QApplication::setOverrideCursor (QCursor::WaitCursor);
   vector<CoordinateSystemInfo *>::iterator iter;

   lockServer ();
   for (iter= m_coordinateSystemInfos.begin (); iter != m_coordinateSystemInfos.end (); ++iter)
   {
      CoordinateSystemInfo * coordinateSystemInfo = * iter;
      count += loadGeoTransforms (coordinateSystemInfo);
   }
   unlockServer ();
   QApplication::restoreOverrideCursor ();

   if (cbTiming)
   {
      cerr << "loading " << count << " geotransforms finished\n";
      int duration = timer.elapsed ();
      cerr << "Duration: " << duration << " milliseconds" << endl;
   }
}

int InfoBase::loadGeoTransforms (CoordinateSystemInfo * coordinateSystemInfo)
{
   int count = 0;
   if (!coordinateSystemInfo->isSelected () || coordinateSystemInfo->BitValueIsTrue (GeoTransformsDownloaded)) return count;
   coordinateSystemInfo->SetBitValue (GeoTransformsDownloaded, true);

   // cerr << "Loading GeoTransform for CoordinateSystem: " << coordinateSystemInfo->getName () << endl;
   const BPA::CoordinateSystem * coordinateSystem = coordinateSystemInfo->getItem ();

   const std::vector < const BPA::GeoTransform * > * geoTransforms =
      m_cauldronBPA->GetGeoTransforms (coordinateSystem);

   if (!geoTransforms)
      return count;

   vector < const BPA::GeoTransform * >::const_iterator iter;

   for (iter = geoTransforms->begin (); iter != geoTransforms->end (); ++iter)
   {
      const BPA::GeoTransform * geoTransform = *iter;

      GeoTransformInfo *geoTransformInfo;

      if ((geoTransformInfo = findGeoTransformInfo (geoTransform->getId ())) == 0)
      {
         geoTransformInfo = new GeoTransformInfo (GeoTransformType);
         geoTransformInfo->setItem (geoTransform);
         m_geoTransformInfos.push_back (geoTransformInfo);
	 /*
         cerr << "Loaded GeoTransform: " << geoTransformInfo->
	 getName () << " for CoordinateSystem: " << coordinateSystemInfo->getName () << endl;
	  */
	 count++;
      }
      coordinateSystemInfo->addChild (geoTransformInfo);
   }

   return count;
}

bool InfoBase::downloadProject (const string & ownerOrgName, const string & projectName, const std::string & dir, const std::string & file, bool projectOnly)
{
   ProjectInfo *projectInfo = findProjectInfo (ownerOrgName, projectName);
   return downloadProject (projectInfo, dir, file, projectOnly);
}

bool InfoBase::downloadProject (ProjectInfo *projectInfo, const std::string & dir, const std::string & file, bool projectOnly)
{
   if (!projectInfo) return false;

   lockServer ();
   bool result = m_cauldronBPA->DownloadCauldronProject (projectInfo->getItem (), dir, file, projectOnly);
   unlockServer ();

   return result;
}

ProjectInfo * InfoBase::findProjectInfo (const string & ownerOrgName, const string & projectName)
{
   OwnerOrgInfo * ownerOrgInfo = findOwnerOrgInfo (ownerOrgName);
   if (!ownerOrgInfo) return 0;

   ProjectInfo *projectInfo = findProjectInfo (ownerOrgInfo, projectName);
   return projectInfo;
}

ProjectInfo * InfoBase::findProjectInfo (OwnerOrgInfo * ownerOrgInfo, const string & projectName)
{
   ProjectInfo * projectInfo = findProjectInfo (projectName);

   if (!projectInfo) return 0;
   if (!ownerOrgInfo->hasChild (projectInfo)) return 0;

   return projectInfo;
}

OwnerOrgInfo * InfoBase::findOwnerOrgInfo (long id)
{
   return (OwnerOrgInfo *) FindItemInfo (m_ownerOrgInfos, id);
}

RegionInfo * InfoBase::findRegionInfo (long id)
{
   return (RegionInfo *) FindItemInfo (m_regionInfos, id);
}

CountryInfo * InfoBase::findCountryInfo (long id)
{
   return (CountryInfo *) FindItemInfo (m_countryInfos, id);
}

BasinInfo * InfoBase::findBasinInfo (long id)
{
   return (BasinInfo *) FindItemInfo (m_basinInfos, id);
}

ProjectInfo * InfoBase::findProjectInfo (long id)
{
   ProjectInfo * projectInfo = (ProjectInfo *) FindItemInfo (m_projectInfos, id);
   return projectInfo;
}


CoordinateSystemInfo * InfoBase::findCoordinateSystemInfo (long id)
{
   return (CoordinateSystemInfo *) FindItemInfo (m_coordinateSystemInfos, id);
}

GeoTransformInfo * InfoBase::findGeoTransformInfo (long id)
{
   return (GeoTransformInfo *) FindItemInfo (m_geoTransformInfos, id);
}

OwnerOrgInfo * InfoBase::findOwnerOrgInfo (const std::string & name)
{
   return (OwnerOrgInfo *) FindItemInfo (m_ownerOrgInfos, name);
}

RegionInfo * InfoBase::findRegionInfo (const std::string & name)
{
   return (RegionInfo *) FindItemInfo (m_regionInfos, name);
}

CountryInfo * InfoBase::findCountryInfo (const std::string & name)
{
   return (CountryInfo *) FindItemInfo (m_countryInfos, name);
}

BasinInfo * InfoBase::findBasinInfo (const std::string & name)
{
   return (BasinInfo *) FindItemInfo (m_basinInfos, name);
}

ProjectInfo * InfoBase::findProjectInfo (const std::string & name)
{
   lockServer ();
   ProjectInfo * projectInfo = (ProjectInfo *) FindItemInfo (m_projectInfos, name);
   unlockServer ();
   return projectInfo;
}

CoordinateSystemInfo * InfoBase::findCoordinateSystemInfo (const std::string & name)
{
   return (CoordinateSystemInfo *) FindItemInfo (m_coordinateSystemInfos, name);
}

GeoTransformInfo * InfoBase::findGeoTransformInfo (const std::string & name)
{
   return (GeoTransformInfo *) FindItemInfo (m_geoTransformInfos, name);
}


void InfoBase::deleteOwnerOrgs (void)
{
   m_active++;
   DeleteItems (m_ownerOrgInfos);
   m_active--;
}

void InfoBase::deleteRegions (void)
{
   m_active++;
   DeleteItems (m_regionInfos);
   m_active--;
}

void InfoBase::deleteCountries (void)
{
   m_active++;
   DeleteItems (m_countryInfos);
   m_active--;
}

void InfoBase::deleteBasins (void)
{
   m_active++;
   DeleteItems (m_basinInfos);
   m_active--;
}

void InfoBase::deleteProjects (void)
{
   m_active++;
   DeleteItems (m_projectInfos);
   m_active--;
}

void InfoBase::deleteCoordinateSystems (void)
{
   m_active++;
   DeleteItems (m_coordinateSystemInfos);
   m_active--;
}

void InfoBase::deleteGeoTransforms (void)
{
   m_active++;
   DeleteItems (m_geoTransformInfos);
   m_active--;
}

ListSelectionStatus InfoBase::displayOwnerOrgs (QListBox * listBox)
{
   if (m_active) return NoneSelected;
   m_active++;
   ListSelectionStatus status = DisplayItems (m_ownerOrgInfos, listBox);
   m_active--;
   return status;
}

ListSelectionStatus InfoBase::displayRegions (QListBox * listBox)
{
   if (m_active) return NoneSelected;
   m_active++;
   ListSelectionStatus status = DisplayItems (m_regionInfos, listBox);
   m_active--;
   return status;
}

ListSelectionStatus InfoBase::displayCountries (QListBox * listBox)
{
   if (m_active) return NoneSelected;
   m_active++;
   ListSelectionStatus status = DisplayItems (m_countryInfos, listBox);
   m_active--;
   return status;
}

ListSelectionStatus InfoBase::displayBasins (QListBox * listBox, bool selectAlways)
{
   if (m_active) return NoneSelected;
   m_active++;
   ListSelectionStatus status = DisplayItems (m_basinInfos, listBox, true, selectAlways);
   m_active--;
   return status;
}

ListSelectionStatus InfoBase::displayProjects (QListView * listView)
{
   if (m_active) return NoneSelected;
   m_active++;

   vector<ProjectInfo *>::iterator iter;

   lockServer ();
   for (iter= m_projectInfos.begin (); iter != m_projectInfos.end (); ++iter)
   {
      ProjectInfo * projectInfo = * iter;
      BPA::CauldronProject * project = projectInfo->getItem ();
      QListViewItem * lvi;
      if ((lvi = projectInfo->getListViewItem ()) == 0)
      {
	 lvi = new QListViewItem (listView);
	 lvi->setText (0, project->getName ().c_str ());
	 lvi->setText (1, project->getBasinNames ().c_str ());
	 lvi->setText (2, project->getDescription ().c_str ());
	 // lvi->setText (3, project->getCoordinateSystemName ().c_str ());
	 // lvi->setText (4, project->getGeoTransformName ().c_str ());
	 lvi->setText (3, project->getBpaUserId ().c_str ());

	 const time_t creationTime = project->getCreateTime ();
	 lvi->setText (4, time2string (&creationTime));

	 projectInfo->setListViewItem (lvi);
	 listView->setSelected (lvi, false);
      }

      if (!projectInfo->parentsSelected (BasinType) || !projectInfo->parentsSelected (OwnerOrgType))
      {
	 if (lvi->listView ())
	 {
	    assert (listView == lvi->listView ());
	    listView->setSelected (lvi, false);
	    listView->takeItem (lvi);
	 }
      }
      else
      {
	 if (!lvi->listView ())
	 {
	    listView->insertItem (lvi);
	    listView->setSelected (lvi, false);
	 }
      }
   }
   unlockServer ();
   
   m_active--;
   return NoneSelected;
}

ListSelectionStatus InfoBase::displayCoordinateSystems (QListBox * listBox)
{
   if (m_active) return NoneSelected;
   m_active++;
   ListSelectionStatus status = DisplayItems (m_coordinateSystemInfos, listBox);
   m_active--;
   return status;
}

ListSelectionStatus InfoBase::displayGeoTransforms (QListBox * listBox)
{
   if (m_active) return NoneSelected;
   m_active++;
   ListSelectionStatus status = DisplayItems (m_geoTransformInfos, listBox);
   m_active--;
   return status;
}

QMutex InfoBase::s_serverLock (true);
int InfoBase::s_numServerLocks = 0;

void InfoBase::lockServer (void)
{
   s_serverLock.lock ();
   if (cbVerbose) cerr << "Server access locked at depth " << s_numServerLocks << endl;
   s_numServerLocks++;
}

void InfoBase::unlockServer (void)
{
   if (cbVerbose) cerr << "Server access unlocked at depth " << s_numServerLocks - 1 << endl;
   if (--s_numServerLocks == 0)
   {
      s_serverLock.unlock ();
   }
}

template < class ItemInfoType> 
ListSelectionStatus DisplayItems (std::vector<ItemInfoType *> & list, QListBox * listBox,
      bool showIfNoChildren, bool selectAlways)
{
   ListSelectionStatus status = EmptyList;

   int index = -1;
   vector<ItemInfoType *>::iterator iter;

   for (iter= list.begin (); iter != list.end (); ++iter)
   {
      ItemInfoType * itemInfo = * iter;
      QListBoxItem * lbi;
      if ((lbi = itemInfo->getListBoxItem ()) == 0)
      {
	 lbi = new QListBoxText (listBox, itemInfo->getName ().c_str ());
	 itemInfo->setListBoxItem (lbi);
	 // listBox->setSelected (lbi, true);
      }

      if (!itemInfo->parentsSelected () || (!showIfNoChildren && !itemInfo->hasChildren ()))
      {
	 if (lbi->listBox ())
	 {
	    assert (listBox == lbi->listBox ());
	    listBox->setSelected (lbi, false);
	    listBox->takeItem (lbi);
	 }
      }
      else
      {
	 if (!lbi->listBox ())
	 {
	    listBox->insertItem (lbi, ++index);
	    if (selectAlways)
	       listBox->setSelected (lbi, true);
	    else
	       listBox->setSelected (lbi, false);
	 }
      }

      if (itemInfo->isSelected ())
      {
	 switch (status)
	 {
	    case EmptyList:
	       status = AllSelected;
	       break;
	    case NoneSelected:
	       status = SomeSelected;
	       break;
	    default:
	       break;
	 }
      }
      else if (itemInfo->isVisible ())
      {
	 switch (status)
	 {
	    case EmptyList:
	       status = NoneSelected;
	       break;
	    case AllSelected:
	       status = SomeSelected;
	       break;
	    default:
	       break;
	 }
      }
   }
   return status;
}


template < class ItemInfoType> 
void DeleteItems (std::vector<ItemInfoType *> & list)
{
   vector<ItemInfoType *>::iterator iter;

   for (iter= list.begin (); iter != list.end (); ++iter)
   {
      ItemInfoType * itemInfo = * iter;
      delete itemInfo;
   }
   list.clear ();
}

template < class ItemInfoType> 
ItemInfoType * FindItemInfo (std::vector<ItemInfoType *> &list,long id)
{
   vector<ItemInfoType *>::iterator iter;
   for (iter= list.begin (); iter != list.end (); ++iter)
   {
      ItemInfoType * itemInfo = * iter;
      if (itemInfo->getId () == id)
      {
	 return itemInfo;
      }
   }
   return 0;
}

template < class ItemInfoType> 
ItemInfoType * FindItemInfo (std::vector<ItemInfoType *> &list, const std::string & name)
{
   vector<ItemInfoType *>::iterator iter;
   for (iter= list.begin (); iter != list.end (); ++iter)
   {
      ItemInfoType * itemInfo = * iter;

      if (itemInfo->getName () == name)
      {
	 return itemInfo;
      }
   }
   return 0;
}

template < class ItemInfoType> 
void ReplaceByItemCopies (std::vector<ItemInfoType *> &list)
{
   vector<ItemInfoType *>::iterator iter;
   for (iter= list.begin (); iter != list.end (); ++iter)
   {
      ItemInfoType * itemInfo = * iter;

      * iter = (ItemInfoType *) itemInfo->deepCopy ();

      // cerr << "Replaced " << itemInfo->getName () << " by " << (* iter)->getName () << endl;
   }
}

template < class ItemInfoType> 
ItemInfoType * ClearItemCopies (std::vector<ItemInfoType *> &list)
{
   vector<ItemInfoType *>::iterator iter;
   for (iter= list.begin (); iter != list.end (); ++iter)
   {
      (* iter)->clearCopy ();
   }
   return 0;
}

static char *time2string (const time_t * time)
{
   struct tm *timeptr = localtime (time);

   static char mon_name[12][3] = {
      "01", "02", "03", "04", "05", "06",
      "07", "08", "08", "10", "11", "12"
   };

   static char result[26];

   sprintf (result, "%d-%.2s-%.2d %.2d:%.2d:%.2d",
	    1900 + timeptr->tm_year,
            mon_name[timeptr->tm_mon],
            timeptr->tm_mday,
	    timeptr->tm_hour,
	    timeptr->tm_min,
	    timeptr->tm_sec);
   return result;
}
