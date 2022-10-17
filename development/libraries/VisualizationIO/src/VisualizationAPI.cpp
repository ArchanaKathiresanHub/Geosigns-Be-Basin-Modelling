//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "VisualizationAPI.h"
#include <assert.h>
#include <new>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <math.h>

#include "LogHandler.h"

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4290)
#endif

using namespace CauldronIO;
using namespace std;

/// Project Implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Project::Project(const string& name, const string& description, const string& version, int xmlVersionMajor, int xmlVersionMinor)
{
    m_name = name;
    m_description = description;
    m_version = version;
    m_xmlVersionMajor = xmlVersionMajor;
    m_xmlVersionMinor = xmlVersionMinor;
}

CauldronIO::Project::~Project()
{
    release();
}

void CauldronIO::Project::addSnapShot(std::shared_ptr<SnapShot>& newSnapShot)
{
    if (!newSnapShot) throw CauldronIOException("Cannot add empty snapshot");

    m_snapShotList.push_back(newSnapShot);
}

const string& CauldronIO::Project::getName() const
{
    return m_name;
}

const string& CauldronIO::Project::getDescription() const
{
    return m_description;
}

const string& CauldronIO::Project::getProgramVersion() const
{
    return m_version;
}

bool CauldronIO::Project::operator==(const Project& other) const
{
    return
        m_version == other.m_version &&
        m_description == other.m_description &&
        m_name == other.m_name;
}

const SnapShotList& CauldronIO::Project::getSnapShots() const
{
    return m_snapShotList;
}


const PropertyList& CauldronIO::Project::getProperties() const
{
    return m_propertyList;
}


const GeometryList& CauldronIO::Project::getGeometries() const
{
    return m_geometries;
}

std::shared_ptr<const Property> CauldronIO::Project::findProperty(const std::string &propertyName) const
{
    for(const std::shared_ptr<const Property>& property: m_propertyList)
    {
        if (property->getName() == propertyName) return property;
    }

    return std::shared_ptr<const Property>();
}

const FormationList& CauldronIO::Project::getFormations() const
{
    return m_formationList;
}

std::shared_ptr<Formation> CauldronIO::Project::findFormation(const std::string &formationName) const
{
    for(const std::shared_ptr<Formation>& formation: m_formationList)
    {
        if (formation->getName() == formationName) return formation;
    }

    return std::shared_ptr<Formation>();
}

int CauldronIO::Project::getXmlVersionMajor() const
{
    return m_xmlVersionMajor;
}

int CauldronIO::Project::getXmlVersionMinor() const
{
    return m_xmlVersionMinor;
}

const StratigraphyTableEntryList& CauldronIO::Project::getStratigraphyTable() const
{
    return m_stratTable;
}

void CauldronIO::Project::addStratigraphyTableEntry(StratigraphyTableEntry entry)
{
    m_stratTable.push_back(entry);
}


void CauldronIO::Project::retrieveStratigraphyTable()
{
    for (auto& entry : m_stratTable)
    {
        if (entry.getFormation())
        {
            entry.getFormation()->retrieve();
        }
        if (entry.getSurface())
        {
            entry.getSurface()->retrieve();
        }
    }
}


const CauldronIO::MigrationEventList& CauldronIO::Project::getMigrationEventsTable() const
{
    return m_migrationEventList;
}

void CauldronIO::Project::clearMigrationEventsTable()
{
   m_migrationEventList.clear();
}


void CauldronIO::Project::addMigrationEvent(std::shared_ptr<MigrationEvent> event)
{
    m_migrationEventList.push_back(event);
}

const CauldronIO::TrapperList& CauldronIO::Project::getTrapperTable() const
{
    return m_trapperList;
}

void CauldronIO::Project::clearTrapperTable()
{
   m_trapperList.clear();
}

void CauldronIO::Project::addTrapper(std::shared_ptr<Trapper>& newTrapper)
{
	if (!newTrapper) throw CauldronIOException("Cannot add empty trapper");

	m_trapperList.push_back(newTrapper);
}


const CauldronIO::TrapList& CauldronIO::Project::getTrapTable() const
{
    return m_trapList;
}

void CauldronIO::Project::clearTrapTable()
{
   m_trapList.clear();
}

void CauldronIO::Project::addTrap(std::shared_ptr<Trap>&  newTrap)

{
   if (!newTrap) throw CauldronIOException("Cannot add empty trap");

   m_trapList.push_back(newTrap);
}

std::shared_ptr<const Trapper>  CauldronIO::Project::findTrapper(int trapId, float snapsotAge) const

{
   for(auto & trapper : m_trapperList) {
      if (trapper->getID() == trapId and trapper->getAge() == snapsotAge) {
         return trapper;
      }
   }

   return std::shared_ptr<const Trapper>();
}

const std::vector<std::string>& CauldronIO::Project::getGenexHistoryList()
{
    return m_genexHistoryList;
}

void CauldronIO::Project::addGenexHistoryRecord(std::string historyRecord)
{
   m_genexHistoryList.push_back( historyRecord );
}

const std::vector<std::string>& CauldronIO::Project::getBurialHistoryList()
{
    return m_burialHistoryList;
}

void CauldronIO::Project::addBurialHistoryRecord(std::string historyRecord)
{
   m_burialHistoryList.push_back( historyRecord );
}

const std::string& CauldronIO::Project::getMassBalance()
{
   return m_massBalance;
}

void  CauldronIO::Project::setMassBalance(const std::string name)
{
   m_massBalance = name;
}

const ReservoirList& CauldronIO::Project::getReservoirs() const
{
    return m_reservoirList;
}

std::shared_ptr<const Reservoir> CauldronIO::Project::findReservoir(const std::string &reservoirName) const
{
    for(const std::shared_ptr<const Reservoir>& reservoir: m_reservoirList)
    {
        if (reservoir->getName() == reservoirName) return reservoir;
    }

    return std::shared_ptr<const Reservoir>();
}

void CauldronIO::Project::addProperty(std::shared_ptr<const Property>& newProperty)
{
    for(std::shared_ptr<const Property>& property: m_propertyList)
    {
        if (*property == *newProperty) return;
    }

    m_propertyList.push_back(newProperty);
}


void CauldronIO::Project::addFormation(std::shared_ptr<Formation>& newFormation)
{
    if (!newFormation) throw CauldronIOException("Cannot add empty formation");

    m_formationList.push_back(newFormation);
}


void CauldronIO::Project::addReservoir(std::shared_ptr<const Reservoir>& newReservoir)
{
    if (!newReservoir) throw CauldronIOException("Cannot add empty reservoir");

    m_reservoirList.push_back(newReservoir);
}


void CauldronIO::Project::addGeometry(const std::shared_ptr<const Geometry2D>& newGeometry)
{
    if (!newGeometry) throw CauldronIOException("Cannot add empty geometry");

    // Check if exists
    for(const std::shared_ptr<const Geometry2D>& geometry: m_geometries)
        if (*geometry == *newGeometry) return;

    m_geometries.push_back(newGeometry);
}

size_t CauldronIO::Project::getGeometryIndex(const std::shared_ptr<const Geometry2D>& newGeometry, bool addWhenNotFound)
{
    if (!newGeometry) throw CauldronIOException("Cannot find empty geometry");

    // Check if exists
    for (size_t i = 0; i < m_geometries.size(); i++)
    {
        if (*m_geometries.at(i) == *newGeometry) return i;
    }

    if (addWhenNotFound)
    {
        addGeometry(newGeometry);
        return getGeometryIndex(newGeometry);
    }

    throw CauldronIOException("Geometry not found");
}

void CauldronIO::Project::release()
{
    for(std::shared_ptr<SnapShot>& snapShot: m_snapShotList)
        snapShot->release();
    m_snapShotList.clear();

    // Release strat-table
    for (auto& entry : m_stratTable)
    {
        if (entry.getSurface())
        {
            entry.getSurface()->release();
        }
        if (entry.getFormation())
        {
            entry.getFormation()->release();
        }
    }

    m_stratTable.clear();
    m_geometries.clear();
    m_migrationEventList.clear();
    m_trapperList.clear();
    m_trapList.clear();
    m_formationList.clear();
    m_propertyList.clear();
}

float CauldronIO::Project::getPropertyAtLocation(double snapshotTime, const std::string & propertyName,
                                                 double xCoord, double yCoord, double zCoord,
                                                 const std::string& reservoirName, const std::string& surfaceName,
                                                 const std::string& formationName ) const
{
   float value = DefaultUndefinedValue;
   const SnapShotList& snapshots = getSnapShots();
   const FormationList& formations = getFormations();

   for( auto &snapshot : snapshots) {
      if(snapshot->getAge() == snapshotTime) {

         if(reservoirName != "") {
            std::shared_ptr<const Reservoir> reservoirFormation = findReservoir(reservoirName);

            if(not reservoirFormation)  throw CauldronIO::CauldronIOException("Reservoir not found");
            std::string fname = reservoirFormation->getFormation()->getName();
            std::shared_ptr<const Property> property = findProperty("ResRockTrapId");
            if(property != 0) {
               float trapId = DefaultUndefinedValue;
               trapId = snapshot->getPropertyAtLocation(formations, xCoord, yCoord, zCoord, property, reservoirName, surfaceName, fname);
               if(trapId != DefaultUndefinedValue) {
                  std::shared_ptr<const Trapper> trapFound = findTrapper(static_cast<int>(trapId), static_cast<float>(snapshotTime));
                  if( trapFound != 0 ) {
                     value = trapFound->getValue(propertyName);
                  }
               }
            }
         } else {

            std::shared_ptr<const Property> property = findProperty(propertyName);
            if(property != 0) {

               value = snapshot->getPropertyAtLocation(formations, xCoord, yCoord, zCoord, property, reservoirName, surfaceName, formationName);
               break;
            }
         }
      }
   }
   return value;
}

/// SnapShot implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::SnapShot::SnapShot(double age, SnapShotKind kind, bool isMinorShapshot)
{
    if (age < 0) throw CauldronIO::CauldronIOException("SnapShot age cannot be negative");
    m_age = age;
    m_kind = kind;
    m_isMinor = isMinorShapshot;
}

CauldronIO::SnapShot::~SnapShot()
{
    // Delete all snapshots
    m_volume.reset();
    m_formationVolumeList.clear();
    m_surfaceList.clear();
    m_trapperList.clear();
}

void CauldronIO::SnapShot::setVolume(std::shared_ptr<Volume>& volume)
{
    m_volume = volume;
}

void CauldronIO::SnapShot::addSurface(std::shared_ptr<Surface>& newSurface)
{
    if (!newSurface) throw CauldronIOException("Cannot add empty surface");

    m_surfaceList.push_back(newSurface);
}

void CauldronIO::SnapShot::addFormationVolume(FormationVolume& formVolume)
{
    m_formationVolumeList.push_back(formVolume);
}


void CauldronIO::SnapShot::addTrapper(std::shared_ptr<Trapper>& newTrapper)
{
    if (!newTrapper) throw CauldronIOException("Cannot add empty trapper");

    m_trapperList.push_back(newTrapper);
}

double CauldronIO::SnapShot::getAge() const
{
    return m_age;
}

CauldronIO::SnapShotKind CauldronIO::SnapShot::getKind() const
{
    return m_kind;
}

bool CauldronIO::SnapShot::isMinorShapshot() const
{
    return m_isMinor;
}

const SurfaceList& CauldronIO::SnapShot::getSurfaceList() const
{
    return m_surfaceList;
}

const std::shared_ptr<Volume>& CauldronIO::SnapShot::getVolume() const
{
    return m_volume;
}

const FormationVolumeList& CauldronIO::SnapShot::getFormationVolumeList() const
{
    return m_formationVolumeList;
}

const TrapperList& CauldronIO::SnapShot::getTrapperList() const
{
    return m_trapperList;
}

std::vector < VisualizationIOData* > CauldronIO::SnapShot::getAllRetrievableData() const
{
    // Collect all data to retrieve
    //////////////////////////////////////////////////////////////////////////
    std::vector < VisualizationIOData* > allReadData;
    for (const std::shared_ptr<Surface>& surfaceIO: getSurfaceList())
    {
        for (const PropertySurfaceData& propertySurfaceData : surfaceIO->getPropertySurfaceDataList())
        {
            VisualizationIOData* surfaceData = propertySurfaceData.second.get();
            if (!surfaceData->isRetrieved())
                allReadData.push_back(surfaceData);
        }
    }

    if (getVolume())
    {
        for (const PropertyVolumeData& propVolume: getVolume()->getPropertyVolumeDataList())
        {
            VisualizationIOData* data = propVolume.second.get();
            if (!data->isRetrieved())
                allReadData.push_back(data);
        }
    }

    for (const FormationVolume& formVolume: getFormationVolumeList())
    {
        const std::shared_ptr<Volume> subVolume = formVolume.second;
        for (const PropertyVolumeData& propVolume : subVolume->getPropertyVolumeDataList())
        {
            VisualizationIOData* data = propVolume.second.get();
            if (!data->isRetrieved())
                allReadData.push_back(data);
        }
    }

    return allReadData;
}

float CauldronIO::SnapShot::getPropertyAtLocation(const FormationList& formations,
                                                  double xCoord, double yCoord, double zCoord, std::shared_ptr<const Property>& property,
                                                  const std::string& reservoirName, const std::string& surfaceName,
                                                  const std::string& formationName ) const
{

   if(property == 0)  throw  CauldronIOException ("Property not found");
   if(zCoord == DefaultUndefinedScalarValue and formationName == "" and surfaceName == "") {
      return DefaultUndefinedScalarValue;
   }
   float value = DefaultUndefinedValue;
   bool highRes = reservoirName != "";

   std::shared_ptr<CauldronIO::Element> element = getDepthElementAtLocation(xCoord, yCoord, zCoord, highRes);
   if(element != 0) {
      if(zCoord != DefaultUndefinedScalarValue) {
         // Extract value at zCoord
         value = getValueAtLocation(property, element);
      } else if((property->getAttribute() == CauldronIO::Surface2DProperty or property->getAttribute() == CauldronIO::Formation2DProperty) and
                (surfaceName != "" or formationName != "")) {
         // Extract value at surface or formation map
         value = getValueAtLocation(property, surfaceName, formationName, element);
      } else if(surfaceName != "" or formationName != "") {
         // Extract continous or discontinous value at surface

         // find a formation at the bottom of the surface and set zCoord
         if(findFormationForSurface(formations, property, surfaceName, formationName, element)) {
            value = getValueAtLocation(property, element);
         }
      }

   }
   return value;
}

std::shared_ptr<CauldronIO::Element> CauldronIO::SnapShot::getDepthElementAtLocation(double xCoord, double yCoord, double zCoord, const bool highRes) const
{
   if (xCoord == DefaultUndefinedScalarValue) throw  CauldronIOException ("Undefined XCoord value");
   if (yCoord == DefaultUndefinedScalarValue) throw  CauldronIOException ("Undefined YCoord value");

   if(not m_volume)  return std::shared_ptr<CauldronIO::Element> ();

   std::shared_ptr<CauldronIO::Element> element(new CauldronIO::Element);

   if(highRes) {
      const SurfaceList surfaces = getSurfaceList();
      bool depthFound = false;
      if( surfaces.size() > 0 ) {
         for( auto&  surfaceIO : surfaces) {
            const PropertySurfaceDataList valueMaps = surfaceIO->getPropertySurfaceDataList();
            if (valueMaps.size() > 0) {
               for( auto& propertySurfaceData : valueMaps)  {
                  if(propertySurfaceData.first->getName() == "DepthHighRes" ) {
                     depthFound = true;
                     std::shared_ptr< CauldronIO::SurfaceData> valueMap = propertySurfaceData.second;
                     if (not valueMap->isRetrieved()) {
                        valueMap->retrieve();
                     }
                     const bool planeElementFound = valueMap->findPlaneLocation(xCoord, yCoord, element);
                     if(planeElementFound) {
                        return element;
                     } else {
                        return std::shared_ptr<CauldronIO::Element> ();
                     }
                  }
               }
            }
         }
      }
      if(not depthFound) throw CauldronIOException ("DepthHighRes  property is not found");
   } else {
      if(m_volume->hasDepthVolume()) {
         std::shared_ptr<VolumeData> depth = m_volume->getDepthVolume();

         if(not depth) throw CauldronIOException ("Depth property is not found");

         if (not depth->isRetrieved()) {
            depth->retrieve();
         }
         const bool planeElementFound = depth->findPlaneLocation(xCoord, yCoord, element);
         if(planeElementFound) {
            if(zCoord != DefaultUndefinedScalarValue) {
               float topDepth = depth->interpolate(element, 0);
               float bottomDepth = depth->interpolate(element, depth->getGeometry()->getNumK() - 1);

               if (topDepth == DefaultUndefinedValue or bottomDepth == DefaultUndefinedValue) {
                  // depth->release();
                  return std::shared_ptr<CauldronIO::Element> ();
               }
               if(zCoord <= bottomDepth and topDepth <= zCoord) {

                  for(unsigned int k = 0; k < depth->getGeometry()->getNumK() - 1; ++ k) {
                     bottomDepth = depth->interpolate(element, k + 1);
                     if(topDepth < bottomDepth and zCoord <= bottomDepth and topDepth <= zCoord) {
                        element->zeta = (2.0 * (zCoord - (double)topDepth) / ((double)bottomDepth - (double)topDepth ) - 1.0);
                        element->z =  k;
                        break;
                     }
                     topDepth = bottomDepth;
                  }
               }
            }
            return element;
         }
         return std::shared_ptr<CauldronIO::Element> ();
      }
   }
   return std::shared_ptr<CauldronIO::Element> ();
}

float CauldronIO::SnapShot::getValueAtLocation(std::shared_ptr<const Property>& property, std::shared_ptr<CauldronIO::Element> &element) const
{

   float value = DefaultUndefinedValue;

   if(property->getAttribute() == CauldronIO::Continuous3DProperty) {
      PropertyVolumeDataList& propVolList = m_volume->getPropertyVolumeDataList();

      for(PropertyVolumeData& propVolume: propVolList) {
         std::string pname = propVolume.first->getName();
         if( propVolume.first == property ) {

            std::shared_ptr< CauldronIO::VolumeData> valueMap = propVolume.second;
            if (not valueMap->isRetrieved()) {
                valueMap->retrieve();
            }
            value = valueMap->interpolate(element);
            break;
         }
      }
   } else if(property->getAttribute() == CauldronIO::Discontinuous3DProperty) {

      std::string fname = element->formationName;
      bool elementFound = false;
      FormationVolumeList formVolumes = getFormationVolumeList();
      for(FormationVolume& formVolume: formVolumes) {
         PropertyVolumeDataList& propVolList = formVolume.second->getPropertyVolumeDataList();

         for(PropertyVolumeData& propVolume: propVolList) {
            std::string pname = propVolume.first->getName();
            if(fname == "" or (fname == formVolume.first->getName())) {
               if( pname == property->getName()) {
                  std::shared_ptr< CauldronIO::VolumeData> valueMap = propVolume.second;
                  if (not valueMap->isRetrieved()) {
                      valueMap->retrieve();
                  }
                  std::shared_ptr<const Geometry3D> geometry = valueMap->getGeometry();

                  if( element->z >= geometry->getFirstK() and element->z < (geometry->getFirstK() + geometry->getNumK())) {
                     value = valueMap->interpolate(element);
                     elementFound = true;
                  }
                  if( elementFound ) {
                     return value;
                  }
               }
            }
         }
      }
   }
   return value;
}

float CauldronIO::SnapShot::getValueAtLocation(std::shared_ptr<const Property>& property,
                                               const std::string& surfaceName, const std::string& formationName, std::shared_ptr<CauldronIO::Element> &element) const
{
   // Get surface or formation map value
   float value = DefaultUndefinedValue;

   const SurfaceList surfaces = getSurfaceList();
   for(const std::shared_ptr<Surface>& surfaceIO: surfaces)
   {
      if((surfaceName != "" and surfaceIO->getName() == surfaceName) or
         (formationName != "" and
          ((surfaceIO->getTopFormation() and surfaceIO->getTopFormation()->getName() == formationName) or
           (surfaceIO->getBottomFormation() and surfaceIO->getBottomFormation()->getName() == formationName)))) {

         const PropertySurfaceDataList valueMaps = surfaceIO->getPropertySurfaceDataList();

         if (valueMaps.size() > 0)
         {
            for(const PropertySurfaceData& propertySurfaceData: valueMaps)
            {
               if(propertySurfaceData.first->getName() == property->getName()) {
                  std::shared_ptr< CauldronIO::SurfaceData> valueMap = propertySurfaceData.second;
                  if (not valueMap->isRetrieved()) {
                      valueMap->retrieve();
                  }
                  if(element->xi == DefaultUndefinedValue) {
                     value = valueMap->getValue(element->x, element->y);
                  } else {
                     value = valueMap->interpolate(element);
                  }
                  return value;
               }
            }
         }
      }
   }
   return value;
}

bool CauldronIO::SnapShot::findFormationForSurface(const FormationList& formations, std::shared_ptr<const Property>& property,
                                                   const std::string& surfaceName, const std::string& formationName,
                                                   std::shared_ptr<CauldronIO::Element> &element) const
{
   if(surfaceName == "")  return false;

   const bool formationDefined =  formationName != "";

   if(property->getAttribute() == CauldronIO::Discontinuous3DProperty and not formationDefined) {
      // If formationName is not defined, find the value at the surface - bottom formation (as in datadriller)
      // return false;
   }
   std::shared_ptr<Formation> formationFound;
   bool surfaceElement = false;
   int k_range_start, k_range_end;
   int koffset = 0;

   for(const std::shared_ptr<Formation>& formation: formations)
   {
      if(not formationDefined or (formationDefined and formation->getName() == formationName)) {
         const string topSurface = (formation->getTopSurface() ? formation->getTopSurface()->getName() : "");
         const string bottomSurface = (formation->getBottomSurface() ? formation->getBottomSurface()->getName() : "");

         if(property->getAttribute() == CauldronIO::Discontinuous3DProperty) {
            if(topSurface == surfaceName) {
               formationFound = formation;
               surfaceElement = true;
               break;
            }
            if( formationDefined and bottomSurface == surfaceName) {
               formationFound = formation;
               break;
            }
         } else if(property->getAttribute() == CauldronIO::Continuous3DProperty) {
            // Ignore the formationName

            if(topSurface == surfaceName) {
               formationFound = formation;
               surfaceElement = true;
               formation->getK_Range(k_range_start, k_range_end);
               element->z = k_range_start - koffset;
               element->zeta = -1;
               element->formationName = formation->getName();
               break;
            }
         }
      }
      if( formation->getTopSurface() and formation->getTopSurface()->getAge() <= m_age ) {
         formation->getK_Range(k_range_start, k_range_end);
         koffset = k_range_start;
      }

   }
   if(formationFound == 0) {
      return false;
   }
   if(property->getAttribute() == CauldronIO::Continuous3DProperty) {
      return true;
   }
   const string formationFoundName = formationFound->getName();

   // What if krange is not defined??
   bool elementFound = false;
   if(property->getAttribute() == CauldronIO::Discontinuous3DProperty) {
      FormationVolumeList formVolumes = getFormationVolumeList();
      for(FormationVolume& formVolume: formVolumes) {
         std::shared_ptr<const Formation>& formation = formVolume.first;
         if(formation->getName() == formationFoundName) {
            PropertyVolumeDataList& propVolList = formVolume.second->getPropertyVolumeDataList();
            for(PropertyVolumeData& propVolume: propVolList) {
               if(propVolume.first == property){
                  std::shared_ptr< CauldronIO::VolumeData> valueMap = propVolume.second;
                  element->formationName = formationFoundName;
                 if(surfaceElement) {
                     element->z =  valueMap->getGeometry()->getFirstK();
                     element->zeta = -1;
                  } else {
                     element->z = valueMap->getGeometry()->getLastK() - 1;
                     element->zeta = -1;
                  }

                  elementFound = true;
                  break;
               }
            }
            return elementFound;
         }
      }
   }
   return false;

}

void CauldronIO::SnapShot::retrieve()
{
    if (m_volume)
        m_volume->retrieve();
    for(FormationVolume& formVolume: m_formationVolumeList)
        formVolume.second->retrieve();
    for(std::shared_ptr<Surface>& surface: m_surfaceList)
        surface->retrieve();
}

void CauldronIO::SnapShot::release()
{
    if (m_volume)
        m_volume->release();
    for(FormationVolume& formVolume: m_formationVolumeList)
        formVolume.second->release();
    for(std::shared_ptr<Surface>& surface: m_surfaceList)
        surface->release();
}

/// Property implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Property::Property(const string& name, const string& username, const string& cauldronName, const string& unit, PropertyType type, PropertyAttribute attrib)
{
    if (name.empty()) throw CauldronIOException("Property name cannot be empty");
    if (username.empty()) throw CauldronIOException("User name cannot be empty");
    if (cauldronName.empty()) throw CauldronIOException("Cauldron name cannot be empty");

    m_name = name;
    m_username = username;
    m_cauldronName = cauldronName;
    m_unit = unit;
    m_type = type;
    m_attrib = attrib;
}

const string& CauldronIO::Property::getName() const
{
    return m_name;
}

const string& CauldronIO::Property::getUserName() const
{
    return m_username;
}

const string& CauldronIO::Property::getCauldronName() const
{
    return m_cauldronName;
}

const string& CauldronIO::Property::getUnit() const
{
    return m_unit;
}

CauldronIO::PropertyType CauldronIO::Property::getType() const
{
    return m_type;
}

CauldronIO::PropertyAttribute CauldronIO::Property::getAttribute() const
{
    return m_attrib;
}

bool CauldronIO::Property::isHighRes() const
{
    if (m_name.find("HighRes") != std::string::npos) return true;
    if (m_name.find("ResRock") != std::string::npos) return true;
    return false;
}

bool CauldronIO::Property::operator==(const Property& other) const
{
    return this->m_name == other.m_name;
}

/// Formation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Formation::Formation(int kStart, int kEnd, const string& name)
   : m_topSurface(nullptr)
   , m_bottomSurface(nullptr)

{
    if (name.empty()) throw CauldronIOException("Formation name cannot be empty");

    m_kstart = kStart;
    m_kend = kEnd;
    m_name = name;

    m_isSourceRock = false;
    m_isMobileLayer = false;
    m_hasAllochthonousLithology = false;
    m_chemicalcompaction = false;
    m_constrainedOverpressure = false;
    m_igniousintrusion = false;
    m_hasAllochthonousLithology = false;
    m_enableSourceRockMixing = false;

    m_depoSequence = -1;
    m_elementRefinement = -1;
    m_igniousintrusionAge = -1;

    m_lithoType1name.clear();
    m_lithoType2name.clear();
    m_lithoType3name.clear();
    m_fluidTypeName.clear();
    m_mixingmodelname.clear();
    m_sourceRock1name.clear();
    m_sourceRock2name.clear();
    m_allochthonousLithologyName.clear();

    m_thicknessMap_index = -1;
    m_mixingHI_index = -1;
    m_lithPerc1_index = -1;
    m_lithPerc2_index = -1;
    m_lithPerc3_index = -1;
}


CauldronIO::Formation::~Formation()
{
    m_propSurfaceList.clear();
}

bool CauldronIO::Formation::isDepthRangeDefined() const
{
    return m_kstart >= 0 && m_kend >= 0;
}

void CauldronIO::Formation::updateK_range(int kStart, int kEnd)
{
    m_kstart = kStart;
    m_kend = kEnd;
}

void CauldronIO::Formation::getK_Range(int &start, int &end) const
{
    start = m_kstart;
    end = m_kend;
}

const string& CauldronIO::Formation::getName() const
{
    return m_name;
}


void CauldronIO::Formation::setIsSourceRock(bool isSourceRock)
{
    m_isSourceRock = isSourceRock;
}

bool CauldronIO::Formation::isSourceRock() const
{
    return m_isSourceRock;
}

void CauldronIO::Formation::setIsMobileLayer(bool isMobileLayer)
{
    m_isMobileLayer = isMobileLayer;
}

bool CauldronIO::Formation::isMobileLayer() const
{
    return m_isMobileLayer;
}

const PropertySurfaceDataList& CauldronIO::Formation::getPropertySurfaceDataList() const
{
   return m_propSurfaceList;
}

bool CauldronIO::Formation::hasThicknessMap() const
{
    return m_thicknessMap_index != -1;
}

bool CauldronIO::Formation::hasMap(CauldronIO::FormationMapType mapType) const
{
   switch (mapType) {
   case THICKNESS:  return m_thicknessMap_index != -1;
   case SRMIXINGHI: return m_mixingHI_index  != -1;
   case LITHOTYPE1: return m_lithPerc1_index != -1;
   case LITHOTYPE2: return m_lithPerc2_index != -1;
   case LITHOTYPE3: return m_lithPerc3_index != -1;
   default: return false;
   }
}

const CauldronIO::PropertySurfaceData& CauldronIO::Formation::getMap(CauldronIO::FormationMapType mapType) const
{
   switch (mapType) {
   case THICKNESS:  return getThicknessMap();
   case SRMIXINGHI: return getSourceRockMixingHIMap();
   case LITHOTYPE1: return getLithoType1PercentageMap();
   case LITHOTYPE2: return getLithoType2PercentageMap();
   case LITHOTYPE3: return getLithoType3PercentageMap();
   default: throw CauldronIOException("No formation map found");
   }
}

void CauldronIO::Formation::setMap(CauldronIO::FormationMapType mapType, PropertySurfaceData& thicknessMap)
{
   switch (mapType) {
   case THICKNESS:  return setThicknessMap(thicknessMap);
   case SRMIXINGHI: return setSourceRockMixingHIMap(thicknessMap);
   case LITHOTYPE1: return setLithoType1PercentageMap(thicknessMap);
   case LITHOTYPE2: return setLithoType2PercentageMap(thicknessMap);
   case LITHOTYPE3: return setLithoType3PercentageMap(thicknessMap);
   default: break;
   }
}

void CauldronIO::Formation::setThicknessMap(PropertySurfaceData& thicknessMap)
{
    if (!hasThicknessMap())
    {
        m_propSurfaceList.push_back(thicknessMap);
        m_thicknessMap_index = (int)(m_propSurfaceList.size() - 1);
    }
    else
    {
        m_propSurfaceList.at(m_thicknessMap_index).second->release();
        m_propSurfaceList.at(m_thicknessMap_index) = thicknessMap;
    }
}

const CauldronIO::PropertySurfaceData& CauldronIO::Formation::getThicknessMap() const
{
    if (!hasThicknessMap()) throw CauldronIOException("No thickness map present in this formation");
    return m_propSurfaceList.at(m_thicknessMap_index);
}


bool CauldronIO::Formation::hasSourceRockMixingHIMap() const
{
    return m_mixingHI_index != -1;
}

void CauldronIO::Formation::setSourceRockMixingHIMap(PropertySurfaceData& map)
{
    if (!hasSourceRockMixingHIMap())
    {
        m_propSurfaceList.push_back(map);
        m_mixingHI_index = (int)(m_propSurfaceList.size() - 1);
    }
    else
    {
        m_propSurfaceList.at(m_mixingHI_index).second->release();
        m_propSurfaceList.at(m_mixingHI_index) = map;
    }
}

const CauldronIO::PropertySurfaceData& CauldronIO::Formation::getSourceRockMixingHIMap() const
{
    if (!hasSourceRockMixingHIMap()) throw CauldronIOException("No sourcerock mixingHI map present in this formation");
    return m_propSurfaceList.at(m_mixingHI_index);
}


void CauldronIO::Formation::setSourceRock1Name(const std::string& name)
{
    m_sourceRock1name = name;
}


const std::string& CauldronIO::Formation::getSourceRock1Name() const
{
    return m_sourceRock1name;
}

void CauldronIO::Formation::setSourceRock2Name(const std::string& name)
{
    m_sourceRock2name = name;
}


const std::string& CauldronIO::Formation::getSourceRock2Name() const
{
    return m_sourceRock2name;
}

void CauldronIO::Formation::setEnableSourceRockMixing(bool enable)
{
    m_enableSourceRockMixing = enable;
}


bool CauldronIO::Formation::getEnableSourceRockMixing() const
{
    return m_enableSourceRockMixing;
}


bool CauldronIO::Formation::hasAllochthonousLithology() const
{
    return m_hasAllochthonousLithology;
}


void CauldronIO::Formation::setAllochthonousLithology(bool value)
{
    m_hasAllochthonousLithology = value;
}


const std::string& CauldronIO::Formation::getAllochthonousLithologyName() const
{
    return m_allochthonousLithologyName;
}


void CauldronIO::Formation::setAllochthonousLithologyName(const std::string& value)
{
    m_allochthonousLithologyName = value;
}


bool CauldronIO::Formation::isIgneousIntrusion() const
{
    return m_igniousintrusion;
}


void CauldronIO::Formation::setIgneousIntrusion(bool value)
{
    m_igniousintrusion = value;
}


double CauldronIO::Formation::getIgneousIntrusionAge() const
{
    return m_igniousintrusionAge;
}


void CauldronIO::Formation::setIgneousIntrusionAge(double age)
{
    m_igniousintrusionAge = age;
}


int CauldronIO::Formation::getDepoSequence() const
{
    return m_depoSequence;
}


void CauldronIO::Formation::setDepoSequence(int number)
{
    m_depoSequence = number;
}


const std::string& CauldronIO::Formation::getFluidType() const
{
    return m_fluidTypeName;
}


void CauldronIO::Formation::setFluidType(const std::string& type)
{
    m_fluidTypeName = type;
}


bool CauldronIO::Formation::hasConstrainedOverpressure() const
{
    return m_constrainedOverpressure;
}


void CauldronIO::Formation::setConstrainedOverpressure(bool value)
{
    m_constrainedOverpressure = value;
}


bool CauldronIO::Formation::hasChemicalCompaction() const
{
    return m_chemicalcompaction;
}


void CauldronIO::Formation::setChemicalCompaction(bool value)
{
    m_chemicalcompaction = value;
}


int CauldronIO::Formation::getElementRefinement() const
{
    return m_elementRefinement;
}


void CauldronIO::Formation::setElementRefinement(int value)
{
    m_elementRefinement = value;
}


const std::string& CauldronIO::Formation::getMixingModel() const
{
    return m_mixingmodelname;
}


void CauldronIO::Formation::setMixingModel(const std::string& model)
{
    m_mixingmodelname = model;
}


void CauldronIO::Formation::setLithoType1Name(const std::string& name)
{
    m_lithoType1name = name;
}


const std::string& CauldronIO::Formation::getLithoType1Name() const
{
    return m_lithoType1name;
}


void CauldronIO::Formation::setLithoType2Name(const std::string& name)
{
    m_lithoType2name = name;
}

const std::string& CauldronIO::Formation::getLithoType2Name() const
{
    return m_lithoType2name;
}


void CauldronIO::Formation::setLithoType3Name(const std::string& name)
{
    m_lithoType3name = name;
}


const std::string& CauldronIO::Formation::getLithoType3Name() const
{
    return m_lithoType3name;
}

void CauldronIO::Formation::setLithoType1PercentageMap(PropertySurfaceData& map)
{
    if (!hasLithoType1PercentageMap())
    {
        m_propSurfaceList.push_back(map);
        m_lithPerc1_index = (int)(m_propSurfaceList.size() - 1);
    }
    else
    {
        m_propSurfaceList.at(m_lithPerc1_index).second->release();
        m_propSurfaceList.at(m_lithPerc1_index) = map;
    }
}

void CauldronIO::Formation::setLithoType2PercentageMap(PropertySurfaceData& map)
{
    if (!hasLithoType2PercentageMap())
    {
        m_propSurfaceList.push_back(map);
        m_lithPerc2_index = (int)(m_propSurfaceList.size() - 1);
    }
    else
    {
        m_propSurfaceList.at(m_lithPerc2_index).second->release();
        m_propSurfaceList.at(m_lithPerc2_index) = map;
    }
}

void CauldronIO::Formation::setLithoType3PercentageMap(PropertySurfaceData& map)
{
    if (!hasLithoType3PercentageMap())
    {
        m_propSurfaceList.push_back(map);
        m_lithPerc3_index = (int)(m_propSurfaceList.size() - 1);
    }
    else
    {
        m_propSurfaceList.at(m_lithPerc3_index).second->release();
        m_propSurfaceList.at(m_lithPerc3_index) = map;
    }
}

const CauldronIO::PropertySurfaceData& CauldronIO::Formation::getLithoType1PercentageMap() const
{
    if (!hasLithoType1PercentageMap()) throw CauldronIOException("No lithotype percentage1 map present in this formation");
    return m_propSurfaceList.at(m_lithPerc1_index);
}

const CauldronIO::PropertySurfaceData& CauldronIO::Formation::getLithoType2PercentageMap() const
{
    if (!hasLithoType2PercentageMap()) throw CauldronIOException("No lithotype percentage2 map present in this formation");
    return m_propSurfaceList.at(m_lithPerc2_index);
}

const CauldronIO::PropertySurfaceData& CauldronIO::Formation::getLithoType3PercentageMap() const
{
    if (!hasLithoType3PercentageMap()) throw CauldronIOException("No lithotype percentage3 map present in this formation");
    return m_propSurfaceList.at(m_lithPerc3_index);
}

bool CauldronIO::Formation::hasLithoType1PercentageMap() const
{
    return m_lithPerc1_index != -1;
}

bool CauldronIO::Formation::hasLithoType2PercentageMap() const
{
    return m_lithPerc2_index != -1;
}

bool CauldronIO::Formation::hasLithoType3PercentageMap() const
{
    return m_lithPerc3_index != -1;
}

void CauldronIO::Formation::setTopSurface(Surface *surface)
{
    m_topSurface = surface;
}

const Surface* CauldronIO::Formation::getTopSurface()
{
    return m_topSurface;
}

void CauldronIO::Formation::setBottomSurface(Surface *surface)
{
    m_bottomSurface = surface;
}

const Surface* CauldronIO::Formation::getBottomSurface()
{
    return m_bottomSurface;
}

void CauldronIO::Formation::retrieve()
{
    for(PropertySurfaceData& data: m_propSurfaceList)
    {
        data.second->retrieve();
    }
}

void CauldronIO::Formation::release()
{
    for(PropertySurfaceData& data: m_propSurfaceList)
    {
        data.second->release();
    }
}

bool CauldronIO::Formation::isRetrieved() const
{
    for(const PropertySurfaceData& data: m_propSurfaceList)
    {
        if (!data.second->isRetrieved()) return false;
    }

    return true;
}

bool CauldronIO::Formation::operator==(const Formation& other) const
{
    return m_name == other.m_name;
}

/// Surface implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Surface::Surface(const std::string& name, SubsurfaceKind kind)
  : m_topFormation(nullptr)
  , m_bottomFormation(nullptr)
{
    m_name = name;
    m_subSurfaceKind = kind;
    m_propSurfaceList.clear();
    m_age = -1;
}

CauldronIO::Surface::~Surface()
{
    m_propSurfaceList.clear();
}

const PropertySurfaceDataList& CauldronIO::Surface::getPropertySurfaceDataList() const
{
    return m_propSurfaceList;
}


void CauldronIO::Surface::replaceAt(size_t index, PropertySurfaceData& data)
{
    if (index > m_propSurfaceList.size()) throw CauldronIOException("Index outside bounds in replaceAt");

    // Dereference existing surface
    m_propSurfaceList.at(index).second->release();
    m_propSurfaceList.at(index).second.reset();

    m_propSurfaceList.at(index) = data;
}

void CauldronIO::Surface::addPropertySurfaceData(PropertySurfaceData& newData)
{
   m_propSurfaceList.push_back(newData);
}

bool CauldronIO::Surface::hasDepthSurface() const
{
    for(const PropertySurfaceData& data: m_propSurfaceList)
        if (data.first->getName() == "Depth") return true;

    return false;
}

std::shared_ptr<SurfaceData> CauldronIO::Surface::getDepthSurface() const
{
    for(const PropertySurfaceData& data: m_propSurfaceList)
        if (data.first->getName() == "Depth") return data.second;

    return std::shared_ptr<SurfaceData>();
}

const string& CauldronIO::Surface::getName() const
{
    return m_name;
}

CauldronIO::SubsurfaceKind CauldronIO::Surface::getSubSurfaceKind() const
{
    return m_subSurfaceKind;
}

void CauldronIO::Surface::setFormation(Formation* formation, bool isTopFormation)
{
    if (isTopFormation)
        m_topFormation = formation;
    else
        m_bottomFormation = formation;
}

const Formation* CauldronIO::Surface::getTopFormation() const
{
    return m_topFormation;
}

const Formation* CauldronIO::Surface::getBottomFormation() const
{
    return m_bottomFormation;
}

void CauldronIO::Surface::retrieve()
{
    for(PropertySurfaceData& data: m_propSurfaceList)
    {
        data.second->retrieve();
    }
}

void CauldronIO::Surface::release()
{
    for(PropertySurfaceData& data: m_propSurfaceList)
    {
        data.second->release();
    }
}

bool CauldronIO::Surface::isRetrieved() const
{
    for(const PropertySurfaceData& data: m_propSurfaceList)
    {
        if (!data.second->isRetrieved()) return false;
    }

    return true;
}


bool CauldronIO::Surface::isAgeDefined() const
{
    return m_age >= 0;
}

void CauldronIO::Surface::setAge(float age)
{
    m_age = age;
}

float CauldronIO::Surface::getAge() const
{
    return m_age;
}

bool CauldronIO::Surface::operator==(const Surface& other) const
{
    if (this->getName() != other.getName()) return false;
    if (this->getSubSurfaceKind() != other.getSubSurfaceKind()) return false;

    auto bottomFormation1 = this->getBottomFormation();
    auto bottomFormation2 = other.getBottomFormation();
    auto topFormation1 = this->getTopFormation();
    auto topFormation2 = other.getTopFormation();

    if (bottomFormation1 && !bottomFormation2) return false;
    if (topFormation1 && !topFormation2) return false;

    if (bottomFormation1 && bottomFormation2)
        if (!(*bottomFormation1 == *bottomFormation2)) return false;
    if (topFormation1 && topFormation2)
        if (!(*topFormation1 == *topFormation2)) return false;

    return true;
}

/// Geometry2D Implementation
///////////////////////////////////////

CauldronIO::Geometry2D::Geometry2D(size_t numI, size_t numJ, double deltaI, double deltaJ, double minI, double minJ, bool cellCentered)
{
    m_numI = numI;
    m_numJ = numJ;
    m_deltaI = deltaI;
    m_deltaJ = deltaJ;
    m_minI = minI;
    m_minJ = minJ;
    m_maxI = minI + deltaI * numI;
    m_maxJ = minJ + deltaJ * numJ;
    m_isCellCentered = cellCentered;
}

size_t CauldronIO::Geometry2D::getNumI() const
{
    return m_numI;
}

size_t CauldronIO::Geometry2D::getNumJ() const
{
    return m_numJ;
}

double CauldronIO::Geometry2D::getDeltaI() const
{
    return m_deltaI;
}

double CauldronIO::Geometry2D::getDeltaJ() const
{
    return m_deltaJ;
}

double CauldronIO::Geometry2D::getMinI() const
{
    return m_minI;
}

double CauldronIO::Geometry2D::getMinJ() const
{
    return m_minJ;
}

double CauldronIO::Geometry2D::getMaxI() const
{
    return m_maxI;
}

double CauldronIO::Geometry2D::getMaxJ() const
{
    return m_maxJ;
}

size_t CauldronIO::Geometry2D::getSize() const
{
    return m_numI * m_numJ;
}


bool CauldronIO::Geometry2D::isCellCentered() const
{
    return m_isCellCentered;
}


void CauldronIO::Geometry2D::setCellCentered(bool cellCentered)
{
    m_isCellCentered = cellCentered;
}

bool CauldronIO::Geometry2D::operator==(const Geometry2D& other) const
{
    return
        m_numI == other.m_numI && m_numJ == other.m_numJ &&
        m_minI == other.m_minI && m_minJ == other.m_minJ &&
        m_isCellCentered == other.m_isCellCentered;
}

// Map implementation
//////////////////////////////////////////////////////////////////////////


CauldronIO::SurfaceData::SurfaceData(const std::shared_ptr<const Geometry2D>& geometry, float minValue, float maxValue)
{
    // For performance reasons, we cache the data locally
    m_numI   = geometry->getNumI();
    m_numJ   = geometry->getNumJ();
    m_deltaI = geometry->getDeltaI();
    m_deltaJ = geometry->getDeltaJ();
    m_minI   = geometry->getMinI();
    m_maxI   = geometry->getMaxI();
    m_minJ   = geometry->getMinJ();
    m_maxJ   = geometry->getMaxJ();
    m_isConstant = false;
    m_retrieved = false;
    m_geometry = geometry;
    m_minValue = minValue;
    m_maxValue = maxValue;
    m_updateMinMax = minValue == DefaultUndefinedValue; // if the min/max values are not set they need to be updated
    m_depoSequence = (int)DefaultUndefinedScalarValue;
    // Indexing into the map is unknown
    m_internalData = nullptr;

    m_reservoir.reset();
    m_formation.reset();
}

float CauldronIO::SurfaceData::getMinValue()
{
    if (m_updateMinMax)
        updateMinMax();
    return m_minValue;
}

float CauldronIO::SurfaceData::getMaxValue()
{
    if (m_updateMinMax)
        updateMinMax();
    return m_maxValue;
}

float CauldronIO::SurfaceData::getUndefinedValue() const
{
    return DefaultUndefinedValue;
}

int CauldronIO::SurfaceData::getDepoSequence() const
{
    return m_depoSequence;
}

void CauldronIO::SurfaceData::setDepoSequence(int number)
{
    m_depoSequence = number;
}

void CauldronIO::SurfaceData::setFormation(const std::shared_ptr<const Formation>& formation)
{
    m_formation = formation;
}

const std::shared_ptr<const Formation>& CauldronIO::SurfaceData::getFormation() const
{
    return m_formation;
}

void CauldronIO::SurfaceData::setReservoir(std::shared_ptr<const Reservoir> reservoir)
{
    m_reservoir = reservoir;
}

const std::shared_ptr<const Reservoir>& CauldronIO::SurfaceData::getReservoir() const
{
    return m_reservoir;
}

bool CauldronIO::SurfaceData::isConstant() const
{
    return m_isConstant;
}

void CauldronIO::SurfaceData::setConstantValue(float value)
{
    m_isConstant = true;
    m_constantValue = value;
    m_retrieved = true;
}

CauldronIO::SurfaceData::~SurfaceData()
{
    if (m_internalData) delete[] m_internalData;
    m_internalData = nullptr;
}

const std::shared_ptr<const Geometry2D>& CauldronIO::SurfaceData::getGeometry() const
{
    return m_geometry;
}

void CauldronIO::SurfaceData::setData_IJ(float* data)
{
   if (!data) setData(nullptr, true, 0);
   else       setData(data);
}

void CauldronIO::SurfaceData::setData(float* data, bool setValue, float value)
{
    // If our data buffer exists, we will just reuse it. Otherwise, allocate
    if (!m_internalData)
    {
        try
        {
            m_internalData = new float[m_numI * m_numJ];
        }
        catch (std::bad_alloc& ba)
        {
            throw CauldronIOException(ba.what());
        }
    }

    if (!setValue)
    {
        if (!data) throw CauldronIOException("Cannot set data from empty buffer");

        // copy the data: this will not throw an exception if it fails
        std::memcpy(m_internalData, data, sizeof(float) * m_numI * m_numJ);
    }
    // assign a value if necessary
    else
    {
        std::fill(m_internalData, m_internalData + m_numI * m_numJ, value);
    }

    m_retrieved = true;
}


void CauldronIO::SurfaceData::updateMinMax()
{
   if (!isRetrieved()) {
      throw CauldronIOException("SurfaceData::updateMinMax - data is not retrieved, need to assign data first");
   }
   if (m_isConstant)
   {
        m_minValue = m_maxValue = m_constantValue;
        m_updateMinMax = false;
        return;
    }

    size_t allElements = m_numI * m_numJ;
    float minValue = DefaultUndefinedValue;
    float maxValue = DefaultUndefinedValue;
    bool foundUndefined = false;
    bool hasNAN = false;
    int nodeCountWithNAN = 0;

    for (size_t i = 0; i < allElements; i++)
    {
        float val = m_internalData[i];
        if (isnan(val))
        {
            hasNAN = true;
            nodeCountWithNAN++;
            m_internalData[i] = DefaultUndefinedValue;
            val = DefaultUndefinedValue;
            foundUndefined = true;
            continue;
        }
        if (val != DefaultUndefinedValue)
        {
            minValue = minValue == DefaultUndefinedValue ? val : min(minValue, val);
            maxValue = maxValue == DefaultUndefinedValue ? val : max(maxValue, val);
        }
        else
        {
            foundUndefined = true;
        }
    }
    if (hasNAN)
    {
        //Check if the prop name can be specified
        LogHandler(LogHandler::DEBUG_SEVERITY) << "A total of "<< nodeCountWithNAN<< " nodes are having NAN which is resetting to "<< CauldronIO::DefaultUndefinedValue;
    }

    m_minValue = minValue;
    m_maxValue = maxValue;

    if ((minValue == maxValue ) || (minValue == DefaultUndefinedValue && maxValue == DefaultUndefinedValue))
    {
        release();
        setConstantValue(minValue);
    }

    m_updateMinMax = false;
}

bool CauldronIO::SurfaceData::canGetRow() const
{
    return true;
}

bool CauldronIO::SurfaceData::canGetColumn() const
{
    // Not implemented at this point
    return false;
}

bool CauldronIO::SurfaceData::isUndefined(size_t i, size_t j) const
{
   if (!isRetrieved()) {
      throw CauldronIOException("SurfaceData::isUndefined - data is not retrieved, need to assign data first");
   }
   if (m_isConstant) return m_constantValue == DefaultUndefinedValue;

   return m_internalData[getMapIndex(i, j)] == DefaultUndefinedValue;
}

float CauldronIO::SurfaceData::getValue(size_t i, size_t j) const

{
   if (!isRetrieved()) {
      throw CauldronIOException("SurfaceData::getValue - data is not retrieved, need to assign data first");
   }
   if (m_isConstant) return m_constantValue;

   return m_internalData[getMapIndex(i, j)];
}

const float* CauldronIO::SurfaceData::getRowValues(size_t j)
{
    if (!canGetRow()) throw CauldronIOException("Cannot return row values");
    if (!isRetrieved()) {
       throw CauldronIOException("SurfaceData::getRowValues - data is not retrieved, need to assign data first");
    }
    // Create our internal buffer if not existing
    if (!m_internalData && m_isConstant) setData(nullptr, true, m_constantValue);

    return m_internalData + getMapIndex(0, j);
}

const float* CauldronIO::SurfaceData::getColumnValues(size_t i)
{
   if (!isRetrieved()) throw CauldronIOException("SurfaceData::getColumnValues - data is not retrieved, need to assign data first");
    if (!canGetColumn()) throw CauldronIOException("Cannot return column values");

    // Create our internal buffer if not existing
    if (!m_internalData && m_isConstant) setData(nullptr, true, m_constantValue);

    return m_internalData + getMapIndex(i, 0);
}

const float* CauldronIO::SurfaceData::getSurfaceValues()
{
    if (!isRetrieved()) throw CauldronIOException("SurfaceData::getSurfaceValues - data is not retrieved, need to assign data first");

    // Create our internal buffer if not existing
    if (!m_internalData && m_isConstant) setData(nullptr, true, m_constantValue);

    return m_internalData;
}

size_t CauldronIO::SurfaceData::getMapIndex(size_t i, size_t j) const
{
    assert(i < m_numI && j < m_numJ);
    return m_numI * j + i;
}

void CauldronIO::SurfaceData::release()
{
    if (m_internalData) delete[] m_internalData;
    m_internalData = nullptr;
    m_retrieved = false;
}

bool CauldronIO::SurfaceData::isRetrieved() const
{
    return m_retrieved;
}

float CauldronIO::SurfaceData::getConstantValue() const
{
    if (!isConstant()) throw CauldronIOException("Map does not have a constant value");
    return m_constantValue;
}

bool  CauldronIO::SurfaceData::findPlaneLocation(double xCoord, double yCoord, std::shared_ptr<CauldronIO::Element> &element)
{
   if(m_minI <= xCoord and xCoord <= m_maxI and  m_minJ <= yCoord and yCoord <= m_maxJ) {
      unsigned int i = (unsigned int)(( xCoord - m_minI ) / m_deltaI );
      unsigned int j = (unsigned int)(( yCoord - m_minJ ) / m_deltaJ );

      if(i < m_numI and j < m_numJ) {
         element->x = i;
         element->y = j;

         return true;
      }
   }

   return false;
}

float CauldronIO::SurfaceData::interpolate( std::shared_ptr<CauldronIO::Element> & element) const
{
   float weights [numberOf2DPoints];
   double phi [numberOf2DPoints];
   float result;

   size_t i = element->x;
   size_t j = element->y;

   size_t i1 = (i + 1 < m_numI ? i + 1 : i);
   size_t j1 = (j + 1 < m_numJ ? j + 1 : j);

   const double xi   = element->xi;
   const double eta  = element->eta;

   if (xi == DefaultUndefinedValue or eta  == DefaultUndefinedValue) {
      return DefaultUndefinedValue;
   }

   weights [0] = getValue(i,  j);
   weights [1] = getValue(i1, j);
   weights [2] = getValue(i1, j1);
   weights [3] = getValue(i,  j1);

   for (unsigned int l = 0; l < numberOf2DPoints; ++l) {
      if (weights [ l ] == DefaultUndefinedValue) {
         return DefaultUndefinedValue;
      }
   }

   // Set the reference element basis function.
   phi [0] = 0.25 * (1.0 - xi) * (1.0 - eta);
   phi [1] = 0.25 * (1.0 + xi) * (1.0 - eta);
   phi [2] = 0.25 * (1.0 + xi) * (1.0 + eta);
   phi [3] = 0.25 * (1.0 - xi) * (1.0 + eta);

   result = 0.0;

   // Calculate inner-product of basis-functions with property-values.
   for (unsigned int l = 0; l < numberOf2DPoints; ++ l) {
      result += weights [l] * static_cast<float>(phi [l]);
   }

   return result;

}

/// Volume implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Volume::Volume(SubsurfaceKind kind)
{
    m_subSurfaceKind = kind;
}

CauldronIO::Volume::~Volume()
{
    m_propVolumeList.clear();
}

CauldronIO::SubsurfaceKind CauldronIO::Volume::getSubSurfaceKind() const
{
    return m_subSurfaceKind;
}

PropertyVolumeDataList& CauldronIO::Volume::getPropertyVolumeDataList()
{
    return m_propVolumeList;
}

void CauldronIO::Volume::removeVolumeData(PropertyVolumeData& data)
{
    for (size_t index = 0; index < m_propVolumeList.size(); index++)
    {
        if (m_propVolumeList.at(index).first == data.first)
        {
            data.second->release();
            data.second.reset();

            if (index < m_propVolumeList.size() - 1)
                m_propVolumeList.at(index) = m_propVolumeList.back();

            m_propVolumeList.pop_back();

            return;
        }
    }

    throw CauldronIOException("Cannot find data to remove");
}

void CauldronIO::Volume::addPropertyVolumeData(PropertyVolumeData& newData)
{
   m_propVolumeList.push_back(newData);
}


void CauldronIO::Volume::replaceAt(size_t index, PropertyVolumeData& data)
{
    if (index > m_propVolumeList.size()) throw CauldronIOException("Index outside bounds in replaceAt");

    // Dereference existing surface
    m_propVolumeList.at(index).second->release();
    m_propVolumeList.at(index).second.reset();

    m_propVolumeList.at(index) = data;
}

bool CauldronIO::Volume::hasDepthVolume() const
{
    for(const PropertyVolumeData& data: m_propVolumeList)
        if (data.first->getName() == "Depth") return true;

    return false;
}

std::shared_ptr<VolumeData> CauldronIO::Volume::getDepthVolume() const
{
    for(const PropertyVolumeData& data: m_propVolumeList)
        if (data.first->getName() == "Depth") return data.second;

    return std::shared_ptr<VolumeData>();
}

void CauldronIO::Volume::retrieve()
{
    for(PropertyVolumeData& data: m_propVolumeList)
        data.second->retrieve();
}

bool CauldronIO::Volume::isRetrieved() const
{
    for(const PropertyVolumeData& data: m_propVolumeList)
        if (!data.second->isRetrieved()) return false;
    return true;
}

void CauldronIO::Volume::release()
{
    for(PropertyVolumeData& data: m_propVolumeList)
        data.second->release();
}

/// Geometry3D implementation
/////////////////////////////////////////////////////////////////////////////

Geometry3D::Geometry3D(size_t numI, size_t numJ, size_t numK, size_t offsetK, double deltaI,
    double deltaJ, double minI, double minJ, bool cellCentered) : Geometry2D(numI, numJ, deltaI, deltaJ, minI, minJ, cellCentered)
{
    m_numK = numK;
    m_firstK = offsetK;
}

size_t CauldronIO::Geometry3D::getNumK() const
{
    return m_numK;
}

size_t CauldronIO::Geometry3D::getFirstK() const
{
    return m_firstK;
}

size_t CauldronIO::Geometry3D::getLastK() const
{
    return m_firstK + m_numK - 1;
}

size_t CauldronIO::Geometry3D::getSize() const
{
    return m_numI * m_numJ * m_numK;
}

void CauldronIO::Geometry3D::updateK_range(size_t firstK, size_t numK)
{
    m_firstK = firstK;
    m_numK = numK;
}


void CauldronIO::Geometry3D::updateIJ_range(size_t numI, size_t numJ)
{
    m_numI = numI;
    m_numJ = numJ;
}

bool CauldronIO::Geometry3D::operator==(const Geometry3D& other) const
{
    return
        m_numI == other.m_numI && m_numJ == other.m_numJ &&
        m_minI == other.m_minI && m_minJ == other.m_minJ &&
        m_numK == other.m_numK && m_firstK == other.m_firstK;
}

//////////////////////////////////////////////////////////////////////////
/// Reservoir implementation
//////////////////////////////////////////////////////////////////////////

bool CauldronIO::Reservoir::operator==(const Reservoir& other) const
{
    if (m_reservoirName != other.m_reservoirName) return false;
    if (m_formation != other.m_formation) return false;

    return true;
}

CauldronIO::Reservoir::Reservoir(const std::string& reservoirName, const std::shared_ptr<const Formation>& formation)
{
    m_reservoirName = reservoirName;
    m_formation = formation;
}

const std::string& CauldronIO::Reservoir::getName() const
{
    return m_reservoirName;
}

const std::shared_ptr<const Formation>& CauldronIO::Reservoir::getFormation() const
{
    return m_formation;
}

/// VolumeData implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::VolumeData::VolumeData(const std::shared_ptr<Geometry3D>& geometry, float minValue, float maxValue)
{
    m_internalDataIJK = nullptr;
    m_internalDataKIJ = nullptr;
    m_isConstant = false;
    m_retrieved = false;
    m_geometry = geometry;
    m_minValue = minValue;
    m_maxValue = maxValue;
    m_updateMinMax = minValue == DefaultUndefinedValue; // if the min/max values are not set they need to be updated
    m_sedimentMinValue = DefaultUndefinedValue;
    m_sedimentMaxValue = DefaultUndefinedValue;

    updateGeometry();
}

CauldronIO::VolumeData::~VolumeData()
{
    if (m_internalDataIJK) delete[] m_internalDataIJK;
    if (m_internalDataKIJ) delete[] m_internalDataKIJ;
    m_internalDataIJK = nullptr;
    m_internalDataKIJ = nullptr;
}

const std::shared_ptr<Geometry3D>& CauldronIO::VolumeData::getGeometry() const
{
    return m_geometry;
}

bool CauldronIO::VolumeData::isUndefined(size_t i, size_t j, size_t k) const
{
   if (!isRetrieved()) {
      throw CauldronIOException("VolumeData::isUndefined - data is not retrieved, need to assign data first");
   }
   if (m_isConstant) return m_constantValue == DefaultUndefinedValue;
   if (m_internalDataIJK) return m_internalDataIJK[computeIndex_IJK(i, j, k)] == DefaultUndefinedValue;

   assert(m_internalDataKIJ);
   return m_internalDataKIJ[computeIndex_KIJ(i, j, k)] == DefaultUndefinedValue;
}


float CauldronIO::VolumeData::getValue(size_t i, size_t j, size_t k) const
{
   if (!isRetrieved()) {
      throw CauldronIOException("VolumeData::getValue - data is not retrieved, need to assign data first");
   }
   if (m_isConstant) return m_constantValue;
   if (m_internalDataIJK) return m_internalDataIJK[computeIndex_IJK(i, j, k)];

   assert(m_internalDataKIJ);
   return m_internalDataKIJ[computeIndex_KIJ(i, j, k)];
}

const float* CauldronIO::VolumeData::getRowValues(size_t j, size_t k)
{
    if (!isRetrieved()) {
       throw CauldronIOException("VolumeData::getRowValues - data is not retrieved, need to assign data first");
    }
    if (!hasDataIJK()) throw CauldronIOException("Cannot return row values");

    // Create our internal buffer if not existing
    if (!m_internalDataIJK && m_isConstant) setData_IJK(nullptr, true, m_constantValue);

    // Assume IJK ordering
    assert(m_internalDataIJK);
    return m_internalDataIJK + computeIndex_IJK(0, j, k);
}

const float* CauldronIO::VolumeData::getColumnValues(size_t , size_t )
{
    throw CauldronIOException("Not implemented");
}

const float* CauldronIO::VolumeData::getNeedleValues(size_t i, size_t j)
{
    if (!isRetrieved()) throw CauldronIOException("VolumeData::getNeedleValues - data is not retrieved, need to assign data first");
    if (!hasDataKIJ() || isConstant()) throw CauldronIOException("Cannot return needle values");

    // Create our internal buffer if not existing
    if (!m_internalDataKIJ && m_isConstant) setData_KIJ(nullptr, true, m_constantValue);

    return m_internalDataKIJ + computeIndex_KIJ(i, j, m_firstK);
}

float CauldronIO::VolumeData::interpolate( std::shared_ptr<CauldronIO::Element> & element)
{
   float weights [numberOf3DPoints];
   double phi [numberOf3DPoints];
   float result;

   size_t i = element->x;
   size_t j = element->y;
   size_t k = element->z;

   size_t i1 = (i + 1 < m_numI ? i + 1 : i);
   size_t j1 = (j + 1 < m_numJ ? j + 1 : j);
   size_t k1 = (k + 1 < m_numK + m_firstK ? k + 1 : k);

   const double xi   = element->xi;
   const double eta  = element->eta;
   const double zeta = element->zeta;

   if (xi == DefaultUndefinedValue or eta  == DefaultUndefinedValue or zeta == DefaultUndefinedValue) {
      return DefaultUndefinedValue;
   }

   weights [0] = getValue(i , j,  k);
   weights [1] = getValue(i1, j,  k);
   weights [2] = getValue(i1, j1, k);
   weights [3] = getValue(i,  j1, k);
   weights [4] = getValue(i,  j,  k1);
   weights [5] = getValue(i1, j,  k1);
   weights [6] = getValue(i1, j1, k1);
   weights [7] = getValue(i,  j1, k1);

   for (unsigned int l = 0; l < numberOf3DPoints; ++l) {
      if (weights [ l ] == DefaultUndefinedValue) {
         return DefaultUndefinedValue;
      }
   }

   // Set the reference element basis function.
   phi[0] = 0.125 * (1.0 - xi) * (1.0 - eta) * (1.0 - zeta);
   phi[1] = 0.125 * (1.0 + xi) * (1.0 - eta) * (1.0 - zeta);
   phi[2] = 0.125 * (1.0 + xi) * (1.0 + eta) * (1.0 - zeta);
   phi[3] = 0.125 * (1.0 - xi) * (1.0 + eta) * (1.0 - zeta);
   phi[4] = 0.125 * (1.0 - xi) * (1.0 - eta) * (1.0 + zeta);
   phi[5] = 0.125 * (1.0 + xi) * (1.0 - eta) * (1.0 + zeta);
   phi[6] = 0.125 * (1.0 + xi) * (1.0 + eta) * (1.0 + zeta);
   phi[7] = 0.125 * (1.0 - xi) * (1.0 + eta) * (1.0 + zeta);

   result = 0.0;

   // Calculate inner-product of basis-functions with property-values.
   for (unsigned int l = 0; l < numberOf3DPoints; ++ l) {
      result += weights[l] * static_cast<float>(phi[l]);
   }

   return result;

}

float CauldronIO::VolumeData::interpolate( std::shared_ptr<CauldronIO::Element> & element, size_t k)
{
   float weights [numberOf2DPoints];
   double phi [numberOf2DPoints];
   float result;

   size_t i = element->x;
   size_t j = element->y;

   size_t i1 = (i + 1 < m_numI ? i + 1 : i);
   size_t j1 = (j + 1 < m_numJ ? j + 1 : j);

   const double xi   = element->xi;
   const double eta  = element->eta;

   if (xi == DefaultUndefinedValue or eta  == DefaultUndefinedValue) {
      return DefaultUndefinedValue;
   }

   weights [0] = getValue(i,  j,  k);
   weights [1] = getValue(i1, j,  k);
   weights [2] = getValue(i1, j1, k);
   weights [3] = getValue(i,  j1, k);

   for (unsigned int l = 0; l < numberOf2DPoints; ++l) {
      if (weights [ l ] == DefaultUndefinedValue) {
         return DefaultUndefinedValue;
      }
   }

   // Set the reference element basis function.
   phi [0] = 0.25 * (1.0 - xi) * (1.0 - eta);
   phi [1] = 0.25 * (1.0 + xi) * (1.0 - eta);
   phi [2] = 0.25 * (1.0 + xi) * (1.0 + eta);
   phi [3] = 0.25 * (1.0 - xi) * (1.0 + eta);

   result = 0.0;

   // Calculate inner-product of basis-functions with property-values.
   for (unsigned int l = 0; l < numberOf2DPoints; ++ l) {
      result += weights [l] * static_cast<float>(phi [l]);
   }

   return result;

}
bool  CauldronIO::VolumeData::findPlaneLocation(double xCoord, double yCoord, std::shared_ptr<CauldronIO::Element> &element)
{
   if(m_minI <= xCoord and xCoord <= m_maxI and   m_minJ <= yCoord and yCoord <= m_maxJ) {
      unsigned int i = (unsigned int)(std::floor( ( xCoord - m_minI ) / m_deltaI ));
      unsigned int j = (unsigned int)(std::floor( ( yCoord - m_minJ ) / m_deltaJ ));

      double elementOriginX;
      double elementOriginY;

      if(i < m_numI and j < m_numJ) {
         elementOriginX = i * m_deltaI + m_minI;
         elementOriginY = j * m_deltaJ + m_minJ;

         element->xi = 2.0 * (xCoord - elementOriginX) / m_deltaI - 1.0;
         element->eta = 2.0 * (yCoord - elementOriginY) / m_deltaJ - 1.0;
         element->x = i;
         element->y = j;

         return true;
      }
   }

   return false;
}

const float* CauldronIO::VolumeData::getSurface_IJ(size_t k)
{
    if (!isRetrieved()) throw CauldronIOException("VolumeData::getSurface_IJ - data is not retrieved, need to assign data first");
    if (!hasDataIJK() || isConstant()) throw CauldronIOException("Cannot return surface values");

    // Create our internal buffer if not existing
    if (!m_internalDataIJK && m_isConstant) setData_IJK(nullptr, true, m_constantValue);

    return m_internalDataIJK + computeIndex_IJK(0, 0, k);
}

const float* CauldronIO::VolumeData::getVolumeValues_KIJ()
{
    if (!isRetrieved()) throw CauldronIOException("VolumeData::getVolumeValues_KIJ - data is not retrieved, need to assign data first");
    if (!hasDataKIJ() || isConstant()) throw CauldronIOException("Cannot return volume values");

    // Create our internal buffer if not existing
    if (!m_internalDataKIJ && m_isConstant) setData_KIJ(nullptr, true, m_constantValue);

    return m_internalDataKIJ;
}

const float* CauldronIO::VolumeData::getVolumeValues_IJK()
{
    if (!isRetrieved()) throw CauldronIOException("VolumeData::getVolumeValues_IJK - data is not retrieved, need to assign data first");
    if (!hasDataIJK() || isConstant()) throw CauldronIOException("Cannot return volume values");

    // Create our internal buffer if not existing
    if (!m_internalDataIJK && m_isConstant) setData_IJK(nullptr, true, m_constantValue);

    return m_internalDataIJK;
}

void CauldronIO::VolumeData::setData_KIJ(float* data, bool setValue /*= false*/, float value /*= 0*/)
{
    setData(data, &m_internalDataKIJ, setValue, value);
}

void CauldronIO::VolumeData::setData_IJK(float* data, bool setValue /*= false*/, float value /*= 0*/)
{
    setData(data, &m_internalDataIJK, setValue, value);
}


void CauldronIO::VolumeData::updateGeometry()
{
    // Cache all geometry
    m_numI = m_geometry->getNumI();
    m_numJ = m_geometry->getNumJ();
    m_firstK = m_geometry->getFirstK();
    m_numK = m_geometry->getNumK();
    m_deltaI = m_geometry->getDeltaI();
    m_deltaJ = m_geometry->getDeltaJ();
    m_minI = m_geometry->getMinI();
    m_maxI = m_geometry->getMaxI();
    m_minJ = m_geometry->getMinJ();
    m_maxJ = m_geometry->getMaxJ();
    m_lastK = m_geometry->getLastK();
}


void CauldronIO::VolumeData::updateMinMax()
{

    if (!isRetrieved()) throw CauldronIOException("VolumeData::updateMinMax - data is not retrieved, need to assign data first");

    if (m_isConstant)
    {
        m_minValue = m_maxValue = m_constantValue;
        m_updateMinMax = false;
        return;
    }

    size_t allElements = m_numI * m_numJ * m_numK;
    float minValue = DefaultUndefinedValue;
    float maxValue = DefaultUndefinedValue;
    float* internaldata = m_internalDataIJK != NULL ? m_internalDataIJK : m_internalDataKIJ;

    for (size_t i = 0; i < allElements; i++)
    {
        float val = internaldata[i];
        if (isnan(val))
        {
            internaldata[i] = DefaultUndefinedValue;
            val = DefaultUndefinedValue;
            continue;
        }
        if (val != DefaultUndefinedValue)
        {
            minValue = minValue == DefaultUndefinedValue ? val : min(minValue, val);
            maxValue = maxValue == DefaultUndefinedValue ? val : max(maxValue, val);
        }
    }

    m_minValue = minValue;
    m_maxValue = maxValue;
    m_updateMinMax = false;
}

void CauldronIO::VolumeData::setData(float* data, float** internalData, bool setValue /*= false*/, float value /*= 0*/)
{
    // If our data buffer exists, we will just reuse it. Otherwise, allocate
    if (!*internalData)
    {
        try
        {
            *internalData = new float[m_numI * m_numJ * m_numK];
        }
        catch (std::bad_alloc& ba)
        {
            throw CauldronIOException(ba.what());
        }
    }

    if (!setValue)
    {
        if (!data) throw CauldronIOException("Cannot set data from empty buffer");

        // copy the data: this will not throw an exception if it fails
        memcpy(*internalData, data, sizeof(float) * m_numI * m_numJ * m_numK);
    }
    // assign a value if necessary
    else
    {
        std::fill(*internalData, *internalData + m_numI * m_numJ * m_numK, value);
    }

    m_retrieved = true;
}

size_t CauldronIO::VolumeData::computeIndex_IJK(size_t i, size_t j, size_t k) const
{
    assert(i < m_numI && j < m_numJ && k >= m_firstK && k < m_numK + m_firstK);

    return (i + j * m_numI + (k - m_firstK) * m_numI * m_numJ);
}

size_t CauldronIO::VolumeData::computeIndex_KIJ(size_t i, size_t j, size_t k) const
{
    assert(i < m_numI && j < m_numJ && k >= m_firstK && k < m_numK + m_firstK);

    return ((k - m_firstK) + i * m_numK + j * m_numI * m_numK);
}

float CauldronIO::VolumeData::getMinValue()
{
    if (m_updateMinMax)
        updateMinMax();
    return m_minValue;
}

float CauldronIO::VolumeData::getMaxValue()
{
    if (m_updateMinMax)
        updateMinMax();
    return m_maxValue;
}

void CauldronIO::VolumeData::setMinMax(float minValue, float maxValue)
{
  m_minValue = minValue;
  m_maxValue = maxValue;

  m_updateMinMax = false;
}

float CauldronIO::VolumeData::getSedimentMinValue() const
{
  return m_sedimentMinValue;
}

float CauldronIO::VolumeData::getSedimentMaxValue() const
{
  return m_sedimentMaxValue;
}

void CauldronIO::VolumeData::setSedimentMinMax(float minValue, float maxValue)
{
  m_sedimentMinValue = minValue;
  m_sedimentMaxValue = maxValue;
}

float CauldronIO::VolumeData::getConstantValue() const
{
    if (!isConstant())  throw CauldronIOException("Map does not have a constant value");
    return m_constantValue;
}


bool CauldronIO::VolumeData::isRetrieved() const
{
    return m_retrieved;
}

void CauldronIO::VolumeData::release()
{
    if (m_internalDataIJK) delete[] m_internalDataIJK;
    if (m_internalDataKIJ) delete[] m_internalDataKIJ;
    m_internalDataIJK = nullptr;
    m_internalDataKIJ = nullptr;
    m_retrieved = false;
}

bool CauldronIO::VolumeData::hasDataIJK() const
{
    return m_internalDataIJK != nullptr;
}

bool CauldronIO::VolumeData::hasDataKIJ() const
{
    return m_internalDataKIJ != nullptr;
}

float CauldronIO::VolumeData::getUndefinedValue() const
{
    return DefaultUndefinedValue;
}

bool CauldronIO::VolumeData::isConstant() const
{
    return m_isConstant;
}

void CauldronIO::VolumeData::setConstantValue(float value)
{
    m_isConstant = true;
    m_constantValue = value;
    m_retrieved = true;
}


const std::shared_ptr<Surface>& CauldronIO::StratigraphyTableEntry::getSurface() const
{
    return m_surface;
}

const std::shared_ptr<Formation>& CauldronIO::StratigraphyTableEntry::getFormation() const
{
    return m_formation;
}

void CauldronIO::StratigraphyTableEntry::setSurface(std::shared_ptr<Surface>& surface)
{
    m_formation.reset();
    m_surface = surface;
}

void CauldronIO::StratigraphyTableEntry::setFormation(std::shared_ptr<Formation>& formation)
{
    m_surface.reset();
    m_formation = formation;
}

CauldronIO::MigrationEvent::MigrationEvent()
{
    // Clear our buffers
    std::memset((void*)this, 0, sizeof(MigrationEvent));
}


void CauldronIO::MigrationEvent::setMigrationProcess(const std::string& name)
{
    memset((void*)m_migrationProcess, 0, maxStringLength);
    for (size_t index = 0; index < name.length(); ++index)
        m_migrationProcess[index] = name.c_str()[index];
}


std::string CauldronIO::MigrationEvent::getMigrationProcess() const
{
    return std::string(m_migrationProcess);
}


float CauldronIO::MigrationEvent::getSourceAge() const
{
    return m_srcAge;
}


void CauldronIO::MigrationEvent::setSourceAge(float val)
{
    m_srcAge = val;
}

void CauldronIO::MigrationEvent::setSourceRockName(const std::string& name)
{
    memset((void*)m_SourceRockName, 0, maxStringLength);

    for (size_t index = 0; index < name.length(); ++index)
        m_SourceRockName[index] = name.c_str()[index];
}

std::string CauldronIO::MigrationEvent::getSourceRockName() const
{
    return std::string(m_SourceRockName);
}

void CauldronIO::MigrationEvent::setSourceReservoirName(const std::string& name)
{
    memset((void*)m_SourceReservoirName, 0, maxStringLength);

    for (size_t index = 0; index < name.length(); ++index)
        m_SourceReservoirName[index] = name.c_str()[index];
}

std::string CauldronIO::MigrationEvent::getSourceReservoirName() const
{
    return std::string(m_SourceReservoirName);
}


int CauldronIO::MigrationEvent::getSourceTrapID() const
{
    return m_srcTrapID;
}


void CauldronIO::MigrationEvent::setSourceTrapID(int val)
{
    m_srcTrapID = val;
}


float CauldronIO::MigrationEvent::getSourcePointX() const
{
    return m_srcX;
}


void CauldronIO::MigrationEvent::setSourcePointX(float val)
{
    m_srcX = val;
}


float CauldronIO::MigrationEvent::getSourcePointY() const
{
    return m_srcY;
}


void CauldronIO::MigrationEvent::setSourcePointY(float val)
{
    m_srcY = val;
}


float CauldronIO::MigrationEvent::getDestinationAge() const
{
    return m_destAge;
}


void CauldronIO::MigrationEvent::setDestinationAge(float val)
{
    m_destAge = val;
}

void CauldronIO::MigrationEvent::setDestinationReservoirName(const std::string& name)
{
    memset((void*)m_DestinationReservoirName, 0, maxStringLength);

    for (size_t index = 0; index < name.length(); ++index)
        m_DestinationReservoirName[index] = name.c_str()[index];
}

std::string CauldronIO::MigrationEvent::getDestinationReservoirName() const
{
    return std::string(m_DestinationReservoirName);
}

void CauldronIO::MigrationEvent::setDestinationTrapID(int val)
{
    m_destTrapID = val;
}

int CauldronIO::MigrationEvent::getDestinationTrapID()
{
    return m_destTrapID;
}

void CauldronIO::MigrationEvent::setDestinationPointX(float val)
{
    m_destX = val;
}

float CauldronIO::MigrationEvent::getDestinationPointX() const
{
    return m_destX;
}

void CauldronIO::MigrationEvent::setDestinationPointY(float val)
{
    m_destY = val;
}

float CauldronIO::MigrationEvent::getDestinationPointY() const
{
    return m_destY;
}

double CauldronIO::MigrationEvent::getMassC1() const
{
    return m_MassC1;
}

double CauldronIO::MigrationEvent::getMassC2() const
{
    return m_MassC2;
}

double CauldronIO::MigrationEvent::getMassC3() const
{
    return m_MassC3;
}

double CauldronIO::MigrationEvent::getMassC4() const
{
    return m_MassC4;
}

double CauldronIO::MigrationEvent::getMassC5() const
{
    return m_MassC5;
}

double CauldronIO::MigrationEvent::getMassN2() const
{
    return m_MassN2;
}

double CauldronIO::MigrationEvent::getMassCOx() const
{
    return m_MassCOx;
}

double CauldronIO::MigrationEvent::getMassH2S() const
{
    return m_MassH2S;
}

double CauldronIO::MigrationEvent::getMassC6_14Aro() const
{
    return m_MassC6_14Aro;
}

double CauldronIO::MigrationEvent::getMassC6_14Sat() const
{
    return m_MassC6_14Sat;
}

double CauldronIO::MigrationEvent::getMassC15Aro() const
{
    return m_MassC15Aro;
}

double CauldronIO::MigrationEvent::getMassC15Sat() const
{
    return m_MassC15Sat;
}

double CauldronIO::MigrationEvent::getMassLSC() const
{
    return m_MassLSC;
}

double CauldronIO::MigrationEvent::getMassC15AT() const
{
    return m_MassC15AT;
}

double CauldronIO::MigrationEvent::getMassC15AroS() const
{
    return m_MassC15AroS;
}

double CauldronIO::MigrationEvent::getMassC15SatS() const
{
    return m_MassC15SatS;
}

double CauldronIO::MigrationEvent::getMassC6_14BT() const
{
    return m_MassC6_14BT;
}

double CauldronIO::MigrationEvent::getMassC6_14DBT() const
{
    return m_MassC6_14DBT;
}

double CauldronIO::MigrationEvent::getMassC6_14BP() const
{
    return m_MassC6_14BP;
}

double CauldronIO::MigrationEvent::getMassC6_14SatS() const
{
    return m_MassC6_14SatS;
}

double CauldronIO::MigrationEvent::getMassC6_14AroS() const
{
    return m_MassC6_14AroS;
}

double CauldronIO::MigrationEvent::getMassresins() const
{
    return m_Massresins;
}

double CauldronIO::MigrationEvent::getMassasphaltenes() const
{
    return m_Massasphaltenes;
}

void CauldronIO::MigrationEvent::setMassC1(double val)
{
    m_MassC1 = val;
}

void CauldronIO::MigrationEvent::setMassC2(double val)
{
    m_MassC2 = val;
}

void CauldronIO::MigrationEvent::setMassC3(double val)
{
    m_MassC3 = val;
}

void CauldronIO::MigrationEvent::setMassC4(double val)
{
    m_MassC4 = val;
}

void CauldronIO::MigrationEvent::setMassC5(double val)
{
    m_MassC5 = val;
}

void CauldronIO::MigrationEvent::setMassN2(double val)
{
    m_MassN2 = val;
}

void CauldronIO::MigrationEvent::setMassCOx(double val)
{
    m_MassCOx = val;
}

void CauldronIO::MigrationEvent::setMassH2S(double val)
{
    m_MassH2S = val;
}

void CauldronIO::MigrationEvent::setMassC6_14Aro(double val)
{
    m_MassC6_14Aro = val;
}

void CauldronIO::MigrationEvent::setMassC6_14Sat(double val)
{
    m_MassC6_14Sat = val;
}

void CauldronIO::MigrationEvent::setMassC15Aro(double val)
{
    m_MassC15Aro = val;
}

void CauldronIO::MigrationEvent::setMassC15Sat(double val)
{
    m_MassC15Sat = val;
}

void CauldronIO::MigrationEvent::setMassLSC(double val)
{
    m_MassLSC = val;
}

void CauldronIO::MigrationEvent::setMassC15AT(double val)
{
    m_MassC15AT = val;
}

void CauldronIO::MigrationEvent::setMassC15AroS(double val)
{
    m_MassC15AroS = val;
}

void CauldronIO::MigrationEvent::setMassC15SatS(double val)
{
    m_MassC15SatS = val;
}

void CauldronIO::MigrationEvent::setMassC6_14BT(double val)
{
    m_MassC6_14BT = val;
}

void CauldronIO::MigrationEvent::setMassC6_14DBT(double val)
{
    m_MassC6_14DBT = val;
}

void CauldronIO::MigrationEvent::setMassC6_14BP(double val)
{
    m_MassC6_14BP = val;
}

void CauldronIO::MigrationEvent::setMassC6_14SatS(double val)
{
    m_MassC6_14SatS = val;
}

void CauldronIO::MigrationEvent::setMassC6_14AroS(double val)
{
    m_MassC6_14AroS = val;
}

void CauldronIO::MigrationEvent::setMassresins(double val)
{
    m_Massresins = val;
}

void CauldronIO::MigrationEvent::setMassasphaltenes(double val)
{
    m_Massasphaltenes = val;
}


#ifdef _MSC_VER
#pragma warning (pop)
#endif

