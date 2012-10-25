#ifndef INFOBASE_H
#define INFOBASE_H

#include "iteminfo.h"

#include <vector>

class QListBox;
class QListView;
class QString;
class QMutex;

namespace BPA
{
   class CauldronBPA;
   class OwnerOrg;
   class Country;
   class Region;
   class Basin;
   class CauldronProject;
   class CoordinateSystem;
   class GeoTransform;
};

typedef ItemInfo<const BPA::OwnerOrg> OwnerOrgInfo;
typedef ItemInfo<const BPA::Country> CountryInfo;
typedef ItemInfo<const BPA::Region> RegionInfo;
typedef ItemInfo<const BPA::Basin> BasinInfo;
typedef ItemInfo<BPA::CauldronProject> ProjectInfo;
typedef ItemInfo<const BPA::CoordinateSystem> CoordinateSystemInfo;
typedef ItemInfo<const BPA::GeoTransform> GeoTransformInfo;

#define OwnerOrgType (1 << 1)
#define CountryType (1 << 2)
#define RegionType (1 << 3)
#define BasinType (1 << 4)
#define ProjectType (1 << 5)
#define CoordinateSystemType (1 << 6)
#define GeoTransformType (1 << 7)

enum ListSelectionStatus
{
   EmptyList, NoneSelected, SomeSelected, AllSelected
};

class InfoBase
{
public:
   InfoBase (void);
   virtual ~InfoBase (void);

   void createCauldronBPA (void);
   BPA::CauldronBPA * getCauldronBPA (void);
   void setCauldronBPA (BPA::CauldronBPA * cauldronBPA);
   void deleteCauldronBPA (void);

   virtual void copyFrom (InfoBase * infoBase);

   virtual bool login (const QString & server, const QString & name, const QString & password);
   int loadOwnerOrgs ();
   void loadRegions (void);
   int loadRegions (OwnerOrgInfo * ownerOrgInfo);
   void loadCountries (void);
   int loadCountries (RegionInfo * regionInfo);
   void loadBasins (void);
   int loadBasins (CountryInfo * country);
   void reloadProjects ();
   void loadProjects ();
   int loadProjects (OwnerOrgInfo * ownerOrgInfo);
   void connectProjects (void);
   int loadCoordinateSystems (void);
   void loadGeoTransforms (void);
   int loadGeoTransforms (CoordinateSystemInfo * coordinateSystemInfo);

   OwnerOrgInfo * findOwnerOrgInfo (long id);
   CountryInfo * findCountryInfo (long id);
   RegionInfo * findRegionInfo (long id);
   BasinInfo * findBasinInfo (long id);
   ProjectInfo * findProjectInfo (long id);
   CoordinateSystemInfo * findCoordinateSystemInfo (long id);
   GeoTransformInfo * findGeoTransformInfo (long id);

   bool downloadProject (const std::string & ownerOrgName, const std::string & projectName, const std::string & dir, const std::string & file, bool projectOnly = false);
   bool downloadProject (ProjectInfo *projectInfo, const std::string & dir, const std::string & file, bool projectOnly = false);

   ProjectInfo * findProjectInfo (const std::string & ownerOrgName, const std::string & projectName);
   ProjectInfo * findProjectInfo (OwnerOrgInfo * ownerOrgInfo, const std::string & projectName);

   OwnerOrgInfo * findOwnerOrgInfo (const std::string & name);
   CountryInfo * findCountryInfo (const std::string & name);
   RegionInfo * findRegionInfo (const std::string & name);
   BasinInfo * findBasinInfo (const std::string & name);
   ProjectInfo * findProjectInfo (const std::string & name);
   CoordinateSystemInfo * findCoordinateSystemInfo (const std::string & name);
   GeoTransformInfo * findGeoTransformInfo (const std::string & name);

   void deleteOwnerOrgs (void);
   void deleteRegions (void);
   void deleteCountries (void);
   void deleteBasins (void);
   void deleteProjects (void);
   void deleteCoordinateSystems (void);
   void deleteGeoTransforms (void);

   virtual ListSelectionStatus displayOwnerOrgs (QListBox * listBox);
   virtual ListSelectionStatus displayRegions (QListBox * listBox);
   virtual ListSelectionStatus displayCountries (QListBox * listBox);
   virtual ListSelectionStatus displayBasins (QListBox * listBox, bool selectAlways = false);
   virtual ListSelectionStatus displayProjects (QListView * listView);
   virtual ListSelectionStatus displayCoordinateSystems (QListBox * listBox);
   virtual ListSelectionStatus displayGeoTransforms (QListBox * listBox);

   virtual void stayConnected (void);

   virtual ListSelectionStatus displayCoordinateSystems ()
   {
      return NoneSelected;
   }

protected:
   BPA::CauldronBPA* m_cauldronBPA;

   int m_active;

   std::vector<OwnerOrgInfo *> m_ownerOrgInfos;
   std::vector<CountryInfo *> m_countryInfos;
   std::vector<RegionInfo *> m_regionInfos;
   std::vector<BasinInfo *> m_basinInfos;
   std::vector<ProjectInfo *> m_projectInfos;
   std::vector<CoordinateSystemInfo *> m_coordinateSystemInfos;
   std::vector<GeoTransformInfo *> m_geoTransformInfos;

   /// Guard to disable multiple concurrent access to m_CauldronBPA and to objects
   /// returned by it that are subject to change.
   static QMutex s_serverLock;
   /// Counter that keeps count of the number of re-entrancies into the guarded area
   /// by the same thread.
   static int s_numServerLocks;

   /// Acquire access to the guarded area and lock access by other threads.
   void lockServer (void);
   /// Leave the guarded area to allow access by other threads if left completely.
   void unlockServer (void);
};

template < class ItemInfoType> 
ListSelectionStatus DisplayItems (std::vector<ItemInfoType *> & list, QListBox * listBox, bool showIfNoChildren = true, bool selectAlways = false);

template < class ItemInfoType> 
void DeleteItems (std::vector<ItemInfoType *> & list);

template < class ItemInfoType> 
ItemInfoType * FindItemInfo (std::vector<ItemInfoType *> &list,long id);

template < class ItemInfoType> 
ItemInfoType * FindItemInfo (std::vector<ItemInfoType *> &list,const std::string & name);

template < class ItemInfoType> 
void ReplaceByItemCopies (std::vector<ItemInfoType *> &list);

template < class ItemInfoType> 
ItemInfoType * ClearItemCopies (std::vector<ItemInfoType *> &list);

#endif // INFOBASE_H
