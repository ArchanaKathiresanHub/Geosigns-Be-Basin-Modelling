//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "VisualizationAPI.h"
#include "VisualizationUtils.h"
#include "VisualizationIO_native.h"
#include "VisualizationIO_projectHandle.h"

#include "ProjectHandle.h"
#include "ObjectFactory.h"
#include "Property.h"
#include "PropertyValue.h"
#include "Reservoir.h"
#include "Surface.h"
#include "Formation.h"
#include "Trap.h"
#include "Trapper.h"
#include "FluidType.h"
#include "LithoType.h"
#include "AllochthonousLithology.h"
#include "Snapshot.h"
#include "GridMap.h"
#include "Grid.h"
#include "ProjectData.h"
#include "InputValue.h"
#include "BasementFormation.h"
#include "PointAdsorptionHistory.h"
#include "CrustFormation.h"
#include "MantleFormation.h"
#include "FolderPath.h"
#include "../../SerialDataAccess/src/SerialGridMap.h"
#include "database.h"
#include "cauldronschemafuncs.h"

#include "ImportProjectHandle.h"
#include "ExportToXML.h"
#include <string>
#include <iostream>
#include <iomanip>
#include <limits>

using namespace DataAccess;
using namespace DataAccess::Interface;
using namespace DataModel;

#define ALLSELECTION SURFACE | FORMATION | FORMATIONSURFACE | RESERVOIR

std::shared_ptr<CauldronIO::Project> ImportProjectHandle::createFromProjectHandle(ProjectHandle& projectHandle, bool verbose)
{
    // Read general project data
    const Interface::ProjectData* projectData = projectHandle.getProjectData();

    // Create the project
    std::shared_ptr<CauldronIO::Project> project(new CauldronIO::Project(
        projectData->getProjectName(), projectData->getDescription(),
        projectData->getProgramVersion(), xml_version_major, xml_version_minor));

    // Import all snapshots
    ImportProjectHandle import(verbose, project, projectHandle);

   // Add input values: this will define the formations
   std::cout << "Importing input data" << std::endl;
   import.checkInputValues();

    // Add snapshots
   import.addSnapShots();

    // Add migration_io data
    import.addMigrationIO();

    // Add trapper_io data
    import.addTrapperIO();

    // Add trap_io data
    import.addTrapIO();

    // Find genex/shale-gas history files
    import.addGenexHistory();

    // Add reference to massBalance file
    import.addMassBalance();

    return project;
}

ImportProjectHandle::ImportProjectHandle(bool verbose, std::shared_ptr<CauldronIO::Project>& project, ProjectHandle& projectHandle) :
  m_verbose(verbose),
  m_project(project),
  m_projectHandle(projectHandle)
{
}

void ImportProjectHandle::addSnapShots()
{
    // Get all property values once
    m_propValues.reset(m_projectHandle.getPropertyValues(ALLSELECTION, 0, 0, 0, 0, 0, MAP | VOLUME));
    sort(m_propValues->begin(), m_propValues->end(), PropertyValue::SortByAgeAndDepoAge);

    m_props.reset(m_projectHandle.getProperties(true, ALLSELECTION, 0, 0, 0, 0, MAP | VOLUME));

    if (m_verbose)
        std::cout << "Loaded " << m_propValues->size() << " propertyvalues and " << m_props->size() << " properties" << std::endl;

    std::shared_ptr<SnapshotList> snapShots;
    snapShots.reset(m_projectHandle.getSnapshots(MAJOR | MINOR));

    for (size_t i = 0; i < snapShots->size(); i++)
    {
        const Snapshot* snapShot = snapShots->at(i);

         // Create a new snapshot
        std::shared_ptr<CauldronIO::SnapShot> snapShotIO = createSnapShotIO(snapShot);

        // Add to project
        m_project->addSnapShot(snapShotIO);
    }
}

std::shared_ptr<CauldronIO::SnapShot> ImportProjectHandle::createSnapShotIO(const DataAccess::Interface::Snapshot* snapShot)
{
    std::shared_ptr<CauldronIO::SnapShot> snapShotIO(new CauldronIO::SnapShot(snapShot->getTime(),
        getSnapShotKind(snapShot), snapShot->getType() == MINOR));

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(6);
    ss << snapShot->getTime();
    std::string snapshotString = ss.str();
    std::cout << "Adding snapshot Age=" << snapshotString << std::endl;

    // Get all property and property values for this snapshot
    std::shared_ptr<PropertyValueList> snapShotPropVals = getPropertyValues(snapShot);
    std::shared_ptr<PropertyList> snapShotProps = getProperties(snapShotPropVals);

    // Create depth-geometry information
    std::shared_ptr<CauldronIO::FormationInfoList> depthFormations = getDepthFormations(snapShot);

    // Bail out if we don't have depthFormations
   if (depthFormations->size() == 0)
   {
      if (m_verbose) std::cout << "Basin_Warning: skipping snapshot as it does not contain depth-information" << std::endl;
      return snapShotIO;
   }

   // Add all the surfaces
   ///////////////////////////////////////////////////////////
   vector<std::shared_ptr<CauldronIO::Surface> > surfaces = createSurfaces(depthFormations, snapShot, snapShotPropVals);
   for(std::shared_ptr<CauldronIO::Surface>& surface: surfaces)
      snapShotIO->addSurface(surface);

    // Add the volume
    //////////////////////////////////////////////////////////////////////////////////////////////////
    std::shared_ptr<CauldronIO::Volume> volume(new CauldronIO::Volume(CauldronIO::None));

    // Add all volumeData: loop over properties, then get all property values for the property
    for (size_t i = 0; i < snapShotProps->size(); ++i)
    {
        // Get all the propertyValue objects for this property and snapshot
        const Property* prop = snapShotProps->at(i);
        PropertyAttribute attrib = prop->getPropertyAttribute();
        // Bail out if it is not continuous
        if (attrib != CONTINUOUS_3D_PROPERTY) continue;
        std::shared_ptr<PropertyValueList> propValues = getFormationVolumePropertyValues(prop, snapShotPropVals, nullptr);
        // Bail out if there are no property value objects
        if (propValues->size() == 0) continue;

        // Construct the geometry for the (continuous) volume
        std::shared_ptr<CauldronIO::Geometry3D> geometry3D = createGeometry3D(depthFormations);

        if (m_verbose)
            std::cout << " - Adding continuous volume data with (" << propValues->size() << ") formations for property " << prop->getName() << std::endl;
        CauldronIO::PropertyVolumeData propVolume = createPropertyVolumeData(propValues, geometry3D, depthFormations);
        volume->addPropertyVolumeData(propVolume);
    }
    if( volume->getPropertyVolumeDataList().size() > 0 ) {
       snapShotIO->setVolume(volume);
    }

    // Add all formationVolumes
    //////////////////////////////////////////////////////////////////////////
    std::shared_ptr<FormationList> formations(m_projectHandle.getFormations(snapShot, true));
    for (size_t i = 0; i < formations->size(); ++i)
    {
        const Formation* formation = formations->at(i);
        std::shared_ptr<CauldronIO::Formation> formationIO = findOrCreateFormation(formation, depthFormations);
        if (!formationIO || !formationIO->isDepthRangeDefined())
        {
            if (m_verbose) std::cerr << "Basin_Warning: ignoring formation: " << formation->getName() << " not found as depth formation" << std::endl;
            continue;
        }

        std::shared_ptr<CauldronIO::FormationInfo> formationInfo = findDepthFormationInfo(formation, depthFormations);

        // Create the volume for this formation
        std::shared_ptr<CauldronIO::Volume> volume(new CauldronIO::Volume(getSubSurfaceKind(formation)));

        // Add all property-value data : loop over properties, then get all property values for the property and this formation
        for (size_t i = 0; i < snapShotProps->size(); ++i)
        {
            // Get all the propertyValue objects for this property and snapshot
            const Property* prop = snapShotProps->at(i);
            PropertyAttribute attrib = prop->getPropertyAttribute();
            // Bail out if it is not continuous
            if (attrib != DISCONTINUOUS_3D_PROPERTY) continue;
            std::shared_ptr<PropertyValueList> propValues = getFormationVolumePropertyValues(prop, snapShotPropVals, formation);
            // Bail out if there are no property value objects
            if (propValues->size() == 0) continue;
            assert(propValues->size() == 1);
            const PropertyValue* propVal = propValues->at(0);
            // Create a geometry
            std::shared_ptr<CauldronIO::Geometry3D> geometry = createGeometry3D(formationInfo);

            CauldronIO::PropertyVolumeData propVolume = createPropertyVolumeData(propVal, geometry, formationInfo);
            volume->addPropertyVolumeData(propVolume);
        }

        if (m_verbose)
            std::cout << " - Adding discontinuous volume data formation " << formationIO->getName() << " with "
                 << volume->getPropertyVolumeDataList().size() << " properties" << std::endl;

        // Only add if there is actual data
        if (volume->getPropertyVolumeDataList().size() > 0)
        {
            CauldronIO::FormationVolume formVolume(formationIO, volume);
            snapShotIO->addFormationVolume(formVolume);
        }
      else if (m_verbose)
      {
         std::cout << "No properties found for formation: " << formation->getName() << std::endl;
      }

    }

    // Find trappers
    std::shared_ptr<TrapperList> trapperList(m_projectHandle.getTrappers(0, snapShot, 0, 0));
    for (size_t i = 0; i < trapperList->size(); ++i)
    {
        const Trapper* trapper = trapperList->at(i);

        // Get some info
        int downstreamTrapperID = -1;
        if (trapper->getDownstreamTrapper())
            downstreamTrapperID = trapper->getDownstreamTrapper()->getPersistentId();
        int persistentID = trapper->getPersistentId();
        int ID = trapper->getId();
        double spillPointDepth = trapper->getSpillDepth();
        double spillPointX, spillPointY;
        trapper->getSpillPointPosition(spillPointX, spillPointY);
        double depth = trapper->getDepth();
        double pointX, pointY;
        trapper->getPosition(pointX, pointY);
        double GOC = trapper->getGOC();
        double OWC = trapper->getOWC();

        const Reservoir* reservoir = trapper->getReservoir();
        assert(reservoir);

        // Create a new Trapper and assign some values
        std::shared_ptr<CauldronIO::Trapper> trapperIO(new CauldronIO::Trapper(ID, persistentID));
        trapperIO->setReservoirName(reservoir->getName());
        trapperIO->setSpillDepth((float)spillPointDepth);
        trapperIO->setSpillPointPosition((float)spillPointX, (float)spillPointY);
        trapperIO->setDepth((float)depth);
        trapperIO->setPosition((float)pointX, (float)pointY);
        trapperIO->setDownStreamTrapperID(downstreamTrapperID);
        trapperIO->setOWC((float)OWC);
        trapperIO->setGOC((float)GOC);

        snapShotIO->addTrapper(trapperIO);
    }

    return snapShotIO;
}

vector<std::shared_ptr<CauldronIO::Surface> > ImportProjectHandle::createSurfaces(
    std::shared_ptr<CauldronIO::FormationInfoList> depthFormations,
    const DataAccess::Interface::Snapshot* snapShot, const std::shared_ptr<PropertyValueList>& snapShotPropVals)
{
    vector < std::shared_ptr<CauldronIO::Surface> > surfaces;

    // Add all the surfaces
    //
    // 1. Collect all surfaces with the same name
    // 2. Nameless surfaces -> identify by formation
    // 3. For each property -> add to project (if not existing)
    // 4. For each formation -> add to project (if not existing)
    // 5. Add SurfaceData to Surface
    ////////////////////////////////////////////////////////

    std::shared_ptr<PropertyValueList> propValues = getMapPropertyValues(snapShotPropVals);

    for (size_t i = 0; i < propValues->size(); ++i)
    {
        const PropertyValue* propValue = propValues->at(i);
        const Property* prop = propValue->getProperty();
        const Formation* formation = propValue->getFormation();
        const Surface* surface = propValue->getSurface();
        const Reservoir* reservoir = propValue->getReservoir();

        // A map needs a surface or a formation
        if (!formation && reservoir)
            formation = reservoir->getFormation();
        if (!formation && !surface)
                throw new CauldronIO::CauldronIOException("Found map without formation, surface or reservoir");

       std::string surfaceName;
        if (surface)
            surfaceName = surface->getName();
        else if (m_verbose)
        {
            std::cout << " - Adding map for formation: " << formation->getName() << " with property " << prop->getName() << std::endl;
            if (reservoir)
                std::cout << " -- from reservoir: " << reservoir->getName() << std::endl;
        }

        // Find or create a property
        std::shared_ptr<const CauldronIO::Property> propertyIO = findOrCreateProperty(prop);

        // Find or create a formation
        std::shared_ptr<CauldronIO::Formation> formationIO;
        if (formation)
            formationIO = findOrCreateFormation(formation, depthFormations);

        // Find or create a reservoir
        std::shared_ptr<const CauldronIO::Reservoir> reservoirIO;
        if (reservoir)
            reservoirIO = findOrCreateReservoir(reservoir, formationIO);

        // Does the surface exist?
        std::shared_ptr<CauldronIO::Surface> surfaceIO;
        if (surfaceName.length() > 0)
            surfaceIO = findSurface(surfaces, surfaceName);
        else
            surfaceIO = findSurface(surfaces, formationIO);

        // Get the geometry data
      // Get from the gridmap: this will load the gridmap :-(
        // TODO: see if we can optimize this....
        const DataAccess::Interface::GridMap* gridmap = propValue->getGridMap();
        // Ignore this property value object if it has no gridmap
        if (!gridmap) continue;

        gridmap->retrieveData();
        // Set the geometry
        std::shared_ptr<const CauldronIO::Geometry2D> geometry(new CauldronIO::Geometry2D(gridmap->getGrid()->numIGlobal(), gridmap->getGrid()->numJGlobal(),
                                                                                          gridmap->getGrid()->deltaIGlobal(), gridmap->getGrid()->deltaJGlobal(),
                                                                                          gridmap->getGrid()->minIGlobal(), gridmap->getGrid()->minJGlobal()));
        gridmap->restoreData();
        gridmap->release();

        // Add the geometry to the project
        m_project->addGeometry(geometry);

        // Create the surface data object
        std::shared_ptr<CauldronIO::SurfaceData> propertyMap = createMapIO(propValue, geometry);
        propertyMap->setFormation(formationIO);

        // Instantiate the surface if not existing
        if (!surfaceIO)
        {
            surfaceIO.reset(new CauldronIO::Surface(surfaceName, getSubSurfaceKind(surface)));
            surfaces.push_back(surfaceIO);
            if (m_verbose && surface)
                std::cout << " - Adding surface: " << surfaceIO->getName() << std::endl;

            // Set the formations
            if (surface)
            {
                // Find or create top/bottom formations
                std::shared_ptr<CauldronIO::Formation> topFormation = findOrCreateFormation(surface->getTopFormation(), depthFormations);
                std::shared_ptr<CauldronIO::Formation> bottomFormation = findOrCreateFormation(surface->getBottomFormation(), depthFormations);
                assert(topFormation == formationIO || bottomFormation == formationIO || !formationIO);

                surfaceIO->setFormation(topFormation.get(), true);
                surfaceIO->setFormation(bottomFormation.get(), false);
            }
            else
            {
                // Assign same formation to bottom/surface formation
                assert(formationIO);
                surfaceIO->setFormation(formationIO.get(), true);
                surfaceIO->setFormation(formationIO.get(), false);
            }
        }

        // Set reservoir
        if (reservoir)
            propertyMap->setReservoir(reservoirIO);

        // Add the property/surfaceData object
        CauldronIO::PropertySurfaceData propSurfaceData(propertyIO, propertyMap);
        if (m_verbose)
            std::cout << " --- adding surface data for property " << propertyIO->getName() << std::endl;
        surfaceIO->addPropertySurfaceData(propSurfaceData);
    }

    return surfaces;
}

std::shared_ptr<CauldronIO::Surface> ImportProjectHandle::findSurface(vector< std::shared_ptr<CauldronIO::Surface> > surfaces, const std::string& surfaceName) const
{
    for(std::shared_ptr<CauldronIO::Surface>& surface: surfaces)
        if (surface->getName() == surfaceName) return surface;

    return std::shared_ptr< CauldronIO::Surface >();
}

std::shared_ptr<CauldronIO::Surface> ImportProjectHandle::findSurface(vector< std::shared_ptr<CauldronIO::Surface> > surfaces,
    std::shared_ptr<CauldronIO::Formation>& formation) const
{
    for(std::shared_ptr<CauldronIO::Surface>& surface: surfaces)
    {
       if (surface->getBottomFormation() == formation.get() && surface->getTopFormation() == formation.get()) return surface;
    }

    return std::shared_ptr< CauldronIO::Surface >();
}

std::shared_ptr<const CauldronIO::Property> ImportProjectHandle::findOrCreateProperty(const Property* prop)
{
    for(const std::shared_ptr<const CauldronIO::Property>& property: m_project->getProperties())
        if (property->getName() == prop->getName()) return property;

    std::shared_ptr<const CauldronIO::Property> propertyIO(new CauldronIO::Property(
        prop->getName(), prop->getUserName(), prop->getCauldronName(),
        prop->getUnit(), getPropertyType(prop), getPropertyAttribute(prop)));
    m_project->addProperty(propertyIO);

    return propertyIO;
}

std::shared_ptr<const CauldronIO::Property> ImportProjectHandle::findOrCreateProperty(const std::string& propString)
{
   for (auto& prop : m_project->getProperties())
   {
      if (prop->getName() == propString) return prop;
   }

   // Create a new one...
   const Property* prop = m_projectHandle.findProperty(propString);
   if (prop == nullptr) throw CauldronIO::CauldronIOException("Cannot find property");

   std::shared_ptr<const CauldronIO::Property> propertyIO(new CauldronIO::Property(
      prop->getName(), prop->getUserName(), prop->getCauldronName(),
      prop->getUnit(), getPropertyType(prop), getPropertyAttribute(prop)));
   m_project->addProperty(propertyIO);

   return propertyIO;
}

std::shared_ptr<CauldronIO::Formation> ImportProjectHandle::findOrCreateFormation(const Formation* form, std::shared_ptr<CauldronIO::FormationInfoList> depthFormations)
{
    std::shared_ptr<CauldronIO::Formation> formationIO;
    if (!form) return formationIO;

    for(const std::shared_ptr<CauldronIO::Formation>& formation: m_project->getFormations())
    {
       if (formation->getName() == form->getName())
       {
          // Update k-range if needed
          if (!formation->isDepthRangeDefined())
          {
             // Find the depth formation
             for (size_t i = 0; i < depthFormations->size(); ++i)
             {
                if (depthFormations->at(i)->formation == form)
                {
                   auto info = depthFormations->at(i);
                   formation->updateK_range((int)info->kStart, (int)info->kEnd);
                   break;
                }
             }
          }

          return formation;
       }
    }

    formationIO = createFormation(form, depthFormations);

    // It can be that this formation is not created from a depthformation: don't add it to the list    
    if (!formationIO)
       return formationIO;

    m_project->addFormation(formationIO);

    return formationIO;
}

std::shared_ptr<CauldronIO::Formation> ImportProjectHandle::findOrCreateFormation(const DataAccess::Interface::Formation* form)
{
   std::shared_ptr<CauldronIO::Formation> formationIO;
   if (!form) return formationIO;

   for(const std::shared_ptr<CauldronIO::Formation>& formation: m_project->getFormations())
      if (formation->getName() == form->getName()) return formation;

   formationIO.reset(new CauldronIO::Formation(-1, -1, form->getName()));

   // Some additional properties
   setFormationProperties(formationIO, form);

   m_project->addFormation(formationIO);

   return formationIO;
}


void ImportProjectHandle::setFormationProperties(std::shared_ptr<CauldronIO::Formation> formationIO, const DataAccess::Interface::Formation* form) const
{
   formationIO->setIsMobileLayer(form->isMobileLayer());
   formationIO->setIsSourceRock(form->isSourceRock());

   if (form->getSourceRock1())
   {
      formationIO->setSourceRock1Name(form->getSourceRockType1Name());
   }
   if (form->getSourceRock2())
   {
      formationIO->setSourceRock2Name(form->getSourceRockType2Name());
   }
   if (form->isSourceRock())
   {
      formationIO->setEnableSourceRockMixing(form->getEnableSourceRockMixing());
   }
   if (form->getFluidType())
   {
      formationIO->setFluidType(form->getFluidType()->getName());
   }
   formationIO->setAllochthonousLithology(form->hasAllochthonousLithology());
   if (form->hasAllochthonousLithology())
   {
      formationIO->setAllochthonousLithologyName(form->getAllochthonousLithology()->getLithologyName());
   }
   formationIO->setDepoSequence(form->getDepositionSequence());
   if (form->getLithoType1())
   {
      formationIO->setLithoType1Name(form->getLithoType1()->getName());
   }
   if (form->getLithoType2())
   {
      formationIO->setLithoType2Name(form->getLithoType2()->getName());
   }
   if (form->getLithoType3())
   {
      formationIO->setLithoType3Name(form->getLithoType3()->getName());
   }
   formationIO->setMixingModel(form->getMixModelStr());
   formationIO->setIgneousIntrusion(form->getIsIgneousIntrusion());
   if (form->getIsIgneousIntrusion())
   {
      formationIO->setIgneousIntrusionAge(form->getIgneousIntrusionAge());
   }
   formationIO->setConstrainedOverpressure(form->hasConstrainedOverpressure());
   formationIO->setChemicalCompaction(form->hasChemicalCompaction());

   // The method below crashes if this Formation is a BasementFormation
   if (dynamic_cast<const BasementFormation*>(form) == nullptr)
   {
      formationIO->setElementRefinement(form->getElementRefinement());
   }
}

std::shared_ptr<const CauldronIO::Reservoir> ImportProjectHandle::findOrCreateReservoir(const Reservoir* reserv, std::shared_ptr<const CauldronIO::Formation> formationIO)
{
    assert(formationIO);

    for(const std::shared_ptr<const CauldronIO::Reservoir>& reservoir: m_project->getReservoirs())
    {
        if (reservoir->getName() == reserv->getName() && reservoir->getFormation() == formationIO)
            return reservoir;
    }

    std::shared_ptr<const CauldronIO::Reservoir> reservoirIO(new CauldronIO::Reservoir(reserv->getName(), formationIO));
    m_project->addReservoir(reservoirIO);

    return reservoirIO;
}


std::shared_ptr<CauldronIO::FormationInfo> ImportProjectHandle::findDepthFormationInfo(const Formation* formation, std::shared_ptr<CauldronIO::FormationInfoList> depthFormations)
{
    for (size_t i = 0; i < depthFormations->size(); ++i)
    {
        std::shared_ptr<CauldronIO::FormationInfo>& info = depthFormations->at(i);
        if (info->formation == formation)
            return info;
    }

    throw CauldronIO::CauldronIOException("Could not find depth geometry for formation");
}


std::shared_ptr<CauldronIO::Geometry3D> ImportProjectHandle::createGeometry3D(std::shared_ptr<CauldronIO::FormationInfoList> depthFormations)
{
    // Find the total depth size & offset
    assert(depthFormations->at(0)->kStart == 0);
    size_t maxK = 0;
    // Avoid synthax errors caused by windows max
    // https://stackoverflow.com/questions/27442885/syntax-error-with-stdnumeric-limitsmax
    size_t minK = (std::numeric_limits<size_t>::max)();
    for (size_t i = 0; i < depthFormations->size(); ++i)
    {
        std::shared_ptr<CauldronIO::FormationInfo>& depthInfo = depthFormations->at(i);
        // TODO: check if formations are continuous.. (it is assumed now)
        maxK = std::max(maxK, depthInfo->kEnd);
        minK = std::min(minK, depthInfo->kStart);
    }

    size_t depthK = 1 + maxK - minK;
    std::shared_ptr<CauldronIO::FormationInfo>& info = depthFormations->at(0);
    std::shared_ptr<CauldronIO::Geometry3D> geometry(new CauldronIO::Geometry3D(info->numI, info->numJ, depthK, minK, info->deltaI, info->deltaJ, info->minI, info->minJ));

    return geometry;
}

std::shared_ptr<CauldronIO::Geometry3D> ImportProjectHandle::createGeometry3D(const std::shared_ptr<CauldronIO::FormationInfo>& info)
{
    size_t numK = 1 + info->kEnd - info->kStart;
    std::shared_ptr<CauldronIO::Geometry3D> geometry(new CauldronIO::Geometry3D(info->numI, info->numJ, numK, (int)info->kStart, info->deltaI, info->deltaJ, info->minI, info->minJ));
    return geometry;
}

CauldronIO::PropertyVolumeData ImportProjectHandle::createPropertyVolumeData(std::shared_ptr<PropertyValueList> propValues,
    std::shared_ptr<CauldronIO::Geometry3D>& geometry3D, std::shared_ptr<CauldronIO::FormationInfoList> depthFormations)
{
    std::shared_ptr<const CauldronIO::Geometry2D> geometry = geometry3D;
    m_project->addGeometry(geometry);

    CauldronIO::VolumeProjectHandle* volumeDataProjHandle = new CauldronIO::VolumeProjectHandle(geometry3D);
    std::shared_ptr<CauldronIO::VolumeData> volumeData(volumeDataProjHandle);

    volumeDataProjHandle->setDataStore(propValues, depthFormations);

    std::shared_ptr<const CauldronIO::Property> prop = findOrCreateProperty(propValues->at(0)->getProperty());
    CauldronIO::PropertyVolumeData propVolumeData(prop, volumeData);

    return propVolumeData;
}


CauldronIO::PropertyVolumeData ImportProjectHandle::createPropertyVolumeData(const DataAccess::Interface::PropertyValue* propVal,
    std::shared_ptr<CauldronIO::Geometry3D>& geometry3D, std::shared_ptr<CauldronIO::FormationInfo> formationInfo)
{
    m_project->addGeometry(geometry3D);

    CauldronIO::VolumeProjectHandle* volumeDataProjHandle = new CauldronIO::VolumeProjectHandle(geometry3D);
    std::shared_ptr<CauldronIO::VolumeData> volumeData(volumeDataProjHandle);

    volumeDataProjHandle->setDataStore(propVal, formationInfo);

    std::shared_ptr<const CauldronIO::Property> prop = findOrCreateProperty(propVal->getProperty());
    CauldronIO::PropertyVolumeData propVolumeData(prop, volumeData);

    return propVolumeData;
}

std::shared_ptr<CauldronIO::SurfaceData> ImportProjectHandle::createMapIO(const DataAccess::Interface::PropertyValue* propVal,
    std::shared_ptr<const CauldronIO::Geometry2D>& geometry)
{
    // Create a projectHandle specific Map object
    CauldronIO::MapProjectHandle* mapInternal = new CauldronIO::MapProjectHandle(geometry);

    std::shared_ptr<CauldronIO::SurfaceData> map(mapInternal);
    // Set data to be retrieved later
    mapInternal->setDataStore(propVal);

    return map;
}

CauldronIO::SnapShotKind ImportProjectHandle::getSnapShotKind(const Snapshot* snapShot)
{
    // Get snapshot kind
    const std::string snapShotKind = snapShot->getKind();
    CauldronIO::SnapShotKind kind = CauldronIO::NONE;
    if (snapShotKind == "System Generated")
        kind = CauldronIO::SYSTEM;
    else if (snapShotKind == "User Defined")
        kind = CauldronIO::USERDEFINED;

    return kind;
}

CauldronIO::PropertyType ImportProjectHandle::getPropertyType(const Property* prop)
{
    PropertyType type = prop->getType();
    CauldronIO::PropertyType typeIO;
    switch (type)
    {
    case FORMATIONPROPERTY:
        typeIO = CauldronIO::FormationProperty;
        break;
    case RESERVOIRPROPERTY:
        typeIO = CauldronIO::ReservoirProperty;
        break;
    case TRAPPROPERTY:
        typeIO = CauldronIO::TrapProperty;
        break;
    }

    return typeIO;
}

CauldronIO::SubsurfaceKind  ImportProjectHandle::getSubSurfaceKind(const Surface* surface) const
{
    CauldronIO::SubsurfaceKind kind = CauldronIO::None;
    if (!surface) return kind;

    switch (surface->kind())
    {
    case BASEMENT_SURFACE:
        kind = CauldronIO::Basement;
        break;
    case SEDIMENT_SURFACE:
        kind = CauldronIO::Sediment;
        break;
    }

    return kind;
}

CauldronIO::SubsurfaceKind ImportProjectHandle::getSubSurfaceKind(const DataAccess::Interface::Formation* formation) const
{
    CauldronIO::SubsurfaceKind kind = CauldronIO::None;
    if (!formation) return kind;

    switch (formation->kind())
    {
    case BASEMENT_FORMATION:
        kind = CauldronIO::Basement;
        break;
    case SEDIMENT_FORMATION:
        kind = CauldronIO::Sediment;
        break;
    }

    return kind;
}

CauldronIO::PropertyAttribute ImportProjectHandle::getPropertyAttribute(const Property* prop)
{
    PropertyAttribute attrib = prop->getPropertyAttribute();
    CauldronIO::PropertyAttribute attribIO;
    switch (attrib)
    {
    case CONTINUOUS_3D_PROPERTY:
        attribIO = CauldronIO::Continuous3DProperty;
        break;
    case DISCONTINUOUS_3D_PROPERTY:
        attribIO = CauldronIO::Discontinuous3DProperty;
        break;
    case FORMATION_2D_PROPERTY:
        attribIO = CauldronIO::Formation2DProperty;
        break;
    case SURFACE_2D_PROPERTY:
        attribIO = CauldronIO::Surface2DProperty;
        break;
    default:
        attribIO = CauldronIO::Other;
        break;
    }

    return attribIO;
}

std::shared_ptr<CauldronIO::FormationInfoList> ImportProjectHandle::getDepthFormations(const Snapshot* snapShot) const
{
    std::shared_ptr<CauldronIO::FormationInfoList> depthFormations(new CauldronIO::FormationInfoList());

    // Find the depth property
    const Property* depthProp = m_projectHandle.findProperty("Depth");
    if (!depthProp) return depthFormations;

    // Find the depth property formations for this snapshot
    std::shared_ptr<PropertyValueList> propValues(m_projectHandle.getPropertyValues(FORMATION, depthProp, snapShot, 0, 0, 0, VOLUME));

    if (propValues->size() == 0) return depthFormations;

    // Find all depth formations and add these
    for (size_t i = 0; i < propValues->size(); ++i)
    {
        GridMap* map = propValues->at(i)->getGridMap();

        if (!map) throw CauldronIO::CauldronIOException("Could not open project3D HDF file!");
        std::shared_ptr<CauldronIO::FormationInfo> info(new CauldronIO::FormationInfo());

        map->retrieveData();

        info->formation = propValues->at(i)->getFormation();
        info->kStart = map->firstK();
        info->kEnd = map->lastK();
        info->numI = map->numI();
        info->numJ = map->numJ();
        info->deltaI = map->deltaI();
        info->deltaJ = map->deltaJ();
        info->minI = map->minI();
        info->minJ = map->minJ();

        /// in a SerialGridMap gridmap, depth is aligned (=increasing) with k index
        /// in a DistributedGridmap, depth is inverse to k index
        SerialGridMap* sGridmap = static_cast<SerialGridMap*>(map);
        info->reverseDepth = sGridmap == nullptr;

        depthFormations->push_back(info);

        map->restoreData();
        map->release();
    }

    // Capture global k-range
    size_t currentK = depthFormations->at(0)->kEnd;
    for (int i = 1; i < depthFormations->size(); ++i)
    {
        std::shared_ptr<CauldronIO::FormationInfo>& info = depthFormations->at(i);
        info->kStart += currentK;
        info->kEnd += currentK;
        currentK = info->kEnd;
    }
    return depthFormations;
}

std::shared_ptr<CauldronIO::Formation> ImportProjectHandle::createFormation(const DataAccess::Interface::Formation* formation,
    std::shared_ptr<CauldronIO::FormationInfoList> depthFormations) const
{
    std::shared_ptr<CauldronIO::Formation> formationPtr;

    if (!formation) return formationPtr;
    std::shared_ptr<CauldronIO::FormationInfo> info;

    // Find the depth formation
    for (size_t i = 0; i < depthFormations->size(); ++i)
    {
        if (depthFormations->at(i)->formation == formation)
        {
            info = depthFormations->at(i);
            break;
        }
    }

    if (!info)
        return formationPtr;

    formationPtr.reset(new CauldronIO::Formation((int)info->kStart, (int)info->kEnd, info->formation->getName()));
    setFormationProperties(formationPtr, info->formation);

    return formationPtr;
}

std::shared_ptr<PropertyValueList> ImportProjectHandle::getPropertyValues(const DataAccess::Interface::Snapshot* snapshot) const
{
    std::shared_ptr<PropertyValueList> propertyValueList(new Interface::PropertyValueList());

    for (int i = 0; i < m_propValues->size(); i++)
    {
        const PropertyValue* propValue = m_propValues->at(i);
        if (propValue->getSnapshot() == snapshot)
            propertyValueList->push_back(propValue);
    }

    return propertyValueList;
}

std::shared_ptr<PropertyList> ImportProjectHandle::getProperties(const std::shared_ptr<PropertyValueList>& propValues) const
{
    std::shared_ptr<PropertyList> propertyList(new Interface::PropertyList);

    for (int i = 0; i < m_props->size(); i++)
    {
        const Property* prop = m_props->at(i);

        for (int j = 0; j < propValues->size(); j++)
        {
            const PropertyValue* propValue = propValues->at(j);

            if (propValue->getProperty() == prop)
            {
                propertyList->push_back(prop);
                break;
            }
        }
    }

    return propertyList;
}

std::shared_ptr<PropertyValueList> ImportProjectHandle::getMapPropertyValues(const std::shared_ptr<PropertyValueList>& snapShotPropVals) const
{
    std::shared_ptr<PropertyValueList> propertyValueList(new Interface::PropertyValueList);

    for (int i = 0; i < snapShotPropVals->size(); i++)
    {
        const PropertyValue* propValue = snapShotPropVals->at(i);

        if (propValue->getStorage() == TIMEIOTBL)
            propertyValueList->push_back(propValue);
    }

    return propertyValueList;
}

std::shared_ptr<PropertyValueList> ImportProjectHandle::getFormationVolumePropertyValues(const Property* prop, std::shared_ptr<PropertyValueList> snapShotPropVals,
    const Formation* formation) const
{
    std::shared_ptr<PropertyValueList> propertyValueList(new Interface::PropertyValueList);

    for (int i = 0; i < snapShotPropVals->size(); i++)
    {
        const PropertyValue* propValue = snapShotPropVals->at(i);

        if (propValue->getProperty() != prop) continue;
        if (propValue->getStorage() != SNAPSHOTIOTBL && propValue->getStorage() != THREEDTIMEIOTBL) continue;
        if (propValue->getSurface() != nullptr) continue;
        if (formation != nullptr && propValue->getFormation() != formation) continue;

        propertyValueList->push_back(propValue);
    }

    return propertyValueList;
}

void ImportProjectHandle::checkInputValues()
{
   SurfaceList* surfaces = m_projectHandle.getSurfaces(nullptr, false);
   if (surfaces->size() > 0)
   {
      if (m_verbose) std::cout << "== StratIOTable ==" << std::endl;

      for (const Interface::Surface* surface : *surfaces)
      {
         addStratTableSurface(surface);
         const Interface::Formation* formation = surface->getBottomFormation();
         if (formation) addStratTableFormation(formation);
      }
   }

   // Loop over surfaces/formations to connect them
   for (int index = 1; index <  m_project->getStratigraphyTable().size(); index += 2)
   {
      const CauldronIO::StratigraphyTableEntry& entry = m_project->getStratigraphyTable().at(index);
      assert(bool(entry.getFormation()));

      std::shared_ptr<CauldronIO::Surface> topSurface = m_project->getStratigraphyTable().at(index - 1).getSurface();
      std::shared_ptr<CauldronIO::Formation> formation = m_project->getStratigraphyTable().at(index).getFormation();

      formation->setTopSurface(topSurface.get());
      topSurface->setFormation(formation.get(), false);

      if (index + 1 < m_project->getStratigraphyTable().size())
      {
         std::shared_ptr<CauldronIO::Surface> bottomSurface = m_project->getStratigraphyTable().at(index + 1).getSurface();
         formation->setBottomSurface(bottomSurface.get());
         bottomSurface->setFormation(formation.get(), true);
      }
   }

   delete surfaces;
}

void ImportProjectHandle::addStratTableSurface(const DataAccess::Interface::Surface* surface)
{
   // Set the geometry
   const Grid* grid = (Grid *)m_projectHandle.getInputGrid();

   std::shared_ptr<const CauldronIO::Geometry2D> geometry(new CauldronIO::Geometry2D(grid->numIGlobal(), grid->numJGlobal(),
      grid->deltaIGlobal(), grid->deltaJGlobal(), grid->minIGlobal(), grid->minJGlobal()));


   // Add the geometry to the project
   m_project->addGeometry(geometry);

   // Create the surface
   std::shared_ptr<CauldronIO::Surface> surfaceIO(new CauldronIO::Surface(surface->getName(), getSubSurfaceKind(surface)));
   if (m_verbose) std::cout << "Adding stratigraphy surface: " << surface->getName() << std::endl;
   database::Record* record = surface->getRecord();
   assert(record);

   double depoAge = getDepoAge(record);
   if (depoAge != DefaultUndefinedScalarValue)
   {
      surfaceIO->setAge((float)depoAge);
   }

   // Get the data for depth - constant or a gridmap
   ///////////////////////////////////////////////////////
   std::shared_ptr<CauldronIO::SurfaceData> depthMap = getInputMap(static_cast<float>(getDepth(record)), getDepthGrid(record), geometry);
   if (depthMap)
   {
      CauldronIO::PropertySurfaceData propSurfaceData(findOrCreateProperty("Depth"), depthMap);
      surfaceIO->addPropertySurfaceData(propSurfaceData);
   }

   // Look for two-way time map for this surface
   ///////////////////////////////////////////////////////////////

   database::Table* twtIoTable = m_projectHandle.getTable("TwoWayTimeIoTbl");
   if (twtIoTable != nullptr && twtIoTable->size() > 0)
   {
      database::Table::iterator tblIter;
      for (tblIter = twtIoTable->begin(); tblIter != twtIoTable->end(); ++tblIter)
      {
         database::Record * twoWayTimeRecord = *tblIter;
         // find the good line for the Surface in the table
         if (database::getSurfaceName(twoWayTimeRecord) == surface->getName())
         {
            const std::string &twtGridMapId = getTwoWayTimeGrid(twoWayTimeRecord);
            float twt = static_cast<float>(getTwoWayTime(twoWayTimeRecord));
            std::shared_ptr<CauldronIO::SurfaceData> twtMap;

            if (twt >= 0 && twt != DefaultUndefinedScalarValue)
            {
               twtMap.reset(new CauldronIO::MapNative(geometry));
               twtMap->setConstantValue(twt);
            }
            else if (twtGridMapId.length() > 0)
            {
               const Interface::InputValue* inputMap = m_projectHandle.findInputValue("TwoWayTimeIoTbl", twtGridMapId);
               assert(inputMap != nullptr);

               CauldronIO::MapProjectHandle* mapProjectHandle = new CauldronIO::MapProjectHandle(geometry);
               twtMap.reset(mapProjectHandle);
               mapProjectHandle->setDataStore(inputMap);
            }

            if (twtMap)
            {
               std::shared_ptr<const CauldronIO::Property> twtProperty = findOrCreateProperty("TwoWayTime");

               CauldronIO::PropertySurfaceData propSurfaceData(twtProperty, twtMap);
               surfaceIO->addPropertySurfaceData(propSurfaceData);
            }

            break;
         }
      }
   }

   CauldronIO::StratigraphyTableEntry entry;
   entry.setSurface(surfaceIO);

   m_project->addStratigraphyTableEntry(entry);
}


std::shared_ptr<CauldronIO::SurfaceData> ImportProjectHandle::getInputMap(float value, const std::string &valueGridMapId,
   std::shared_ptr<const CauldronIO::Geometry2D> geometry) const
{
   std::shared_ptr<CauldronIO::SurfaceData> valueMap;

   if (valueGridMapId.length() > 0)
   {
      // Get some info about this input Map
      const Interface::InputValue* inputMap = m_projectHandle.findInputValue("StratIoTbl", valueGridMapId);
      assert(inputMap != nullptr);

      // Construct a SurfaceData object caching this inputMap, to derive HDFinfo later from it
      CauldronIO::MapProjectHandle* mapProjectHandle = new CauldronIO::MapProjectHandle(geometry);
      valueMap.reset(mapProjectHandle);
      mapProjectHandle->setDataStore(inputMap);
   }
   else if (value != DefaultUndefinedScalarValue)
   {
      valueMap.reset(new CauldronIO::MapNative(geometry));
      valueMap->setConstantValue(value);
   }

   return valueMap;
}

void ImportProjectHandle::addStratTableFormation(const Interface::Formation* formation)
{
   auto formationIO = findOrCreateFormation(formation);
   if (m_verbose) std::cout << "Adding formation: " << formation->getName() << std::endl;

   // Create a geometry
   const Grid* grid = (Grid *)m_projectHandle.getInputGrid();
   std::shared_ptr<const CauldronIO::Geometry2D> geometry(new CauldronIO::Geometry2D(grid->numIGlobal(), grid->numJGlobal(),
      grid->deltaIGlobal(), grid->deltaJGlobal(), grid->minIGlobal(), grid->minJGlobal()));

   // Get the database record
   database::Record* record = formation->getTopSurface()->getRecord();
   assert(record);

   // Check thickness grid
   ///////////////////////////
   std::shared_ptr<CauldronIO::SurfaceData> thicknessMap = getInputMap(static_cast<float>(getThickness(record)), getThicknessGrid(record), geometry);
   if (thicknessMap)
   {
      CauldronIO::PropertySurfaceData propSurfaceData(findOrCreateProperty("Thickness"), thicknessMap);
      formationIO->setThicknessMap(propSurfaceData);
   }

   // Check SourceRockMixingHI grid
   ////////////////////////////////////////
   std::shared_ptr<CauldronIO::SurfaceData> mixingHIMap = getInputMap(static_cast<float>(getSourceRockMixingHI(record)), getSourceRockMixingHIGrid(record), geometry);
   if (mixingHIMap)
   {
      std::shared_ptr<const CauldronIO::Property> mixingProperty = getDefaultProperty("MixingHI");
      CauldronIO::PropertySurfaceData propSurfaceData(mixingProperty, mixingHIMap);
      formationIO->setSourceRockMixingHIMap(propSurfaceData);
   }

   // Check Lithotype percentage grids
   //////////////////////////////////////////////
   std::shared_ptr<CauldronIO::SurfaceData> lithPerc1Map, lithPerc2Map, lithPerc3Map;

   if (formation->getLithoType1() != nullptr)
   {
      lithPerc1Map = getInputMap(static_cast<float>(getPercent1(record)), getPercent1Grid(record), geometry);
      if (lithPerc1Map)
      {
         std::shared_ptr<const CauldronIO::Property> lithPerc1Property = getDefaultProperty("LithoType1Percentage");
         CauldronIO::PropertySurfaceData propSurfaceData(lithPerc1Property, lithPerc1Map);
         formationIO->setLithoType1PercentageMap(propSurfaceData);
      }
   }
   // If lithotype 3 is defined read lithotype 2 from map
   if (formation->getLithoType3() != nullptr)
   {
      lithPerc2Map = getInputMap(static_cast<float>(getPercent2(record)), getPercent2Grid(record), geometry);
   }
   else if (lithPerc1Map)
   {
      // Compute from lithotype1 map: type2 = 100 - type1
      CauldronIO::VisualizationUtils::retrieveSingleData(lithPerc1Map);

      // Create a new native map
      lithPerc2Map.reset(new CauldronIO::MapNative(geometry));
      float* data = new float[geometry->getNumI() * geometry->getNumJ()];
      const float* inputData = lithPerc1Map->getSurfaceValues();

      for (int i = 0; i < geometry->getNumI() * geometry->getNumJ(); ++i) {
          if (inputData[i] != DataAccess::Interface::DefaultUndefinedMapValue)
              data[i] = 100 - inputData[i];
          else
              data[i] = DataAccess::Interface::DefaultUndefinedMapValue;
      }

      lithPerc2Map->setData_IJ(data);
      lithPerc2Map->getMaxValue(); // trigger updating min/max

      delete[] data;
   }
   if (lithPerc2Map)
   {
      std::shared_ptr<const CauldronIO::Property> lithPerc2Property = getDefaultProperty("LithoType2Percentage");
      CauldronIO::PropertySurfaceData propSurfaceData(lithPerc2Property, lithPerc2Map);
      formationIO->setLithoType2PercentageMap(propSurfaceData);
   }

   // If lithotype 3 is defined, lithotype 3 = 100 - lithotype1 - lithotype2
   if (formation->getLithoType3() != nullptr && lithPerc1Map && lithPerc2Map)
   {
      // Get all the data
      if (!lithPerc1Map->isRetrieved())
         CauldronIO::VisualizationUtils::retrieveSingleData(lithPerc1Map);

      CauldronIO::VisualizationUtils::retrieveSingleData(lithPerc2Map);

      // Create a new native map
      lithPerc3Map.reset(new CauldronIO::MapNative(geometry));
      float* data = new float[geometry->getNumI() * geometry->getNumJ()];
      const float* inputData1 = lithPerc1Map->getSurfaceValues();
      const float* inputData2 = lithPerc2Map->getSurfaceValues();

      for (int i = 0; i < geometry->getNumI() * geometry->getNumJ(); ++i) {
          if (inputData1[i] != DataAccess::Interface::DefaultUndefinedMapValue && inputData2[i] != DataAccess::Interface::DefaultUndefinedMapValue)
              data[i] = 100.0f - inputData1[i] - inputData2[i];      
          else
              data[i] = DataAccess::Interface::DefaultUndefinedMapValue;
      }

      lithPerc3Map->setData_IJ(data);
      lithPerc3Map->getMaxValue(); // trigger updating min/max

      delete[] data;
   }
   if (lithPerc3Map)
   {
      std::shared_ptr<const CauldronIO::Property> lithPerc3Property = getDefaultProperty("LithoType3Percentage");
      CauldronIO::PropertySurfaceData propSurfaceData(lithPerc3Property, lithPerc3Map);
      formationIO->setLithoType3PercentageMap(propSurfaceData);
   }

   CauldronIO::StratigraphyTableEntry entry;
   entry.setFormation(formationIO);

   m_project->addStratigraphyTableEntry(entry);
}

std::shared_ptr<const CauldronIO::Property> ImportProjectHandle::getDefaultProperty(const std::string& propString)
{
   for (auto& prop : m_project->getProperties())
   {
      if (prop->getName() == propString) return prop;
   }

   std::shared_ptr<const CauldronIO::Property> propertyIO(new CauldronIO::Property(propString, propString, propString, "", CauldronIO::FormationProperty, CauldronIO::Other));
   m_project->addProperty(propertyIO);

   return propertyIO;
}

void ImportProjectHandle::addMigrationIO()
{
    database::Table* table = m_projectHandle.getTable("MigrationIoTbl");
    if (!table || table->size() == 0) return;

    for (size_t index = 0; index < table->size(); index++)
    {
        std::shared_ptr<CauldronIO::MigrationEvent> event(new CauldronIO::MigrationEvent());
        database::Record* record = table->getRecord((int)index);

        event->setMigrationProcess(getMigrationProcess(record));

        event->setSourceAge((float)getSourceAge(record));
        event->setSourceRockName(getSourceRockName(record));
        event->setSourceReservoirName(getSourceReservoirName(record));
        event->setSourceTrapID(getSourceTrapID(record));
        event->setSourcePointX((float)getSourcePointX(record));
        event->setSourcePointY((float)getSourcePointY(record));

        event->setDestinationAge((float)getDestinationAge(record));
        event->setDestinationReservoirName(getDestinationReservoirName(record));
        event->setDestinationTrapID(getDestinationTrapID(record));
        event->setDestinationPointX((float)getDestinationPointX(record));
        event->setDestinationPointY((float)getDestinationPointY(record));

        event->setMassC1         (getMassC1(record));
        event->setMassC2         (getMassC2(record));
        event->setMassC3         (getMassC3(record));
        event->setMassC5         (getMassC5(record));
        event->setMassN2         (getMassN2(record));
        event->setMassCOx        (getMassCOx(record));
        event->setMassH2S        (getMassH2S(record));
        event->setMassC6_14Aro   (getMassC6_14Aro(record));
        event->setMassC6_14Sat   (getMassC6_14Sat(record));
        event->setMassC15Aro     (getMassC15Aro(record));
        event->setMassC15Sat     (getMassC15Sat(record));
        event->setMassLSC        (getMassLSC(record));
        event->setMassC15AT      (getMassC15AT(record));
        event->setMassC15AroS    (getMassC15AroS(record));
        event->setMassC15SatS    (getMassC15SatS(record));
        event->setMassC6_14BT    (getMassC6_14BT(record));
        event->setMassC6_14DBT   (getMassC6_14DBT(record));
        event->setMassC6_14BP    (getMassC6_14BP(record));
        event->setMassC6_14SatS  (getMassC6_14SatS(record));
        event->setMassC6_14AroS  (getMassC6_14AroS(record));
        event->setMassresins     (getMassresins(record));
        event->setMassasphaltenes(getMassasphaltenes(record));

        m_project->addMigrationEvent(event);
    }
}

void ImportProjectHandle::addTrapperIO()
{
     std::shared_ptr<TrapperList> trapperList(m_projectHandle.getTrappers(0, 0, 0, 0));

     if (trapperList->size() == 0) return;
     for (size_t index = 0; index < trapperList->size(); ++ index)
     {
        const Trapper* trapper = trapperList->at(index);

        std::shared_ptr<CauldronIO::Trapper> event(new CauldronIO::Trapper());

        event->setID(trapper->getId());
        event->setPersistentId(trapper->getPersistentId());
        if (trapper->getDownstreamTrapper()) {
           event->setDownStreamTrapperID(trapper->getDownstreamTrapper()->getPersistentId());
        }
        event->setSolutionGasVolume((float)trapper->getVolume( PhaseId::OIL, PhaseId::GAS ));
        event->setSolutionGasDensity((float)trapper->getDensity( PhaseId::OIL, PhaseId::GAS ));
        event->setSolutionGasViscosity((float)trapper->getViscosity( PhaseId::OIL, PhaseId::GAS ));
        event->setSolutionGasMass((float)trapper->getMass( PhaseId::OIL, PhaseId::GAS ));

        event->setFreeGasVolume((float)trapper->getVolume( PhaseId::GAS, PhaseId::GAS ));
        event->setFreeGasDensity((float)trapper->getDensity( PhaseId::GAS, PhaseId::GAS ));
        event->setFreeGasViscosity((float)trapper->getViscosity( PhaseId::GAS, PhaseId::GAS ));
        event->setFreeGasMass((float)trapper->getMass( PhaseId::GAS, PhaseId::GAS ));

        event->setCondensateVolume((float)trapper->getVolume( PhaseId::GAS, PhaseId::OIL ));
        event->setCondensateDensity((float)trapper->getDensity( PhaseId::GAS, PhaseId::OIL ));
        event->setCondensateViscosity((float)trapper->getViscosity( PhaseId::GAS, PhaseId::OIL ));
        event->setCondensateMass((float)trapper->getMass( PhaseId::GAS, PhaseId::OIL ));

        event->setStockTankOilVolume((float)trapper->getVolume( PhaseId::OIL, PhaseId::OIL ));
        event->setStockTankOilDensity((float)trapper->getDensity( PhaseId::OIL, PhaseId::OIL ));
        event->setStockTankOilViscosity((float)trapper->getViscosity( PhaseId::OIL, PhaseId::OIL ));
        event->setStockTankOilMass((float)trapper->getMass( PhaseId::OIL, PhaseId::OIL ));

        for( int compId = 0; compId < CauldronIO::SpeciesNamesId::NUMBER_OF_SPECIES; ++ compId ) {
           event->setSolutionGasMass ( trapper->getMass( PhaseId::OIL, PhaseId::GAS, (ComponentId)compId ), (CauldronIO::SpeciesNamesId)compId );
           event->setCondensateMass  ( trapper->getMass( PhaseId::GAS, PhaseId::OIL, (ComponentId)compId ), (CauldronIO::SpeciesNamesId)compId );
           event->setStockTankOilMass( trapper->getMass( PhaseId::OIL, PhaseId::OIL, (ComponentId)compId ), (CauldronIO::SpeciesNamesId)compId );
           event->setFreeGasMass     ( trapper->getMass( PhaseId::GAS, PhaseId::GAS, (ComponentId)compId ), (CauldronIO::SpeciesNamesId)compId );
        }

        event->setVolumeOil((float)trapper->getVolume(PhaseId::OIL));
        event->setVolumeGas((float)trapper->getVolume(PhaseId::GAS));
        event->setMassVapour((float)trapper->getMass(PhaseId::GAS));
        event->setMassLiquid((float)trapper->getMass(PhaseId::OIL));
        event->setViscosityVapour((float)trapper->getViscosity(PhaseId::GAS));
        event->setViscosityLiquid((float)trapper->getViscosity(PhaseId::OIL));
        event->setDensityVapour((float)trapper->getDensity(PhaseId::GAS));
        event->setDensityLiquid((float)trapper->getDensity(PhaseId::OIL));
        event->setCEPGas((float)trapper->getCEP(PhaseId::GAS));
        event->setCEPOil((float)trapper->getCEP(PhaseId::OIL));
        event->setCriticalTemperatureOil((float)trapper->getCriticalTemperature(PhaseId::OIL));
        event->setCriticalTemperatureGas((float)trapper->getCriticalTemperature(PhaseId::GAS));
        event->setInterfacialTensionOil((float)trapper->getInterfacialTension(PhaseId::OIL));
        event->setInterfacialTensionGas((float)trapper->getInterfacialTension(PhaseId::GAS));
        event->setOilAPI((float)trapper->getOilAPI());
        event->setCGR((float)trapper->getCGR());
        event->setGOR((float)trapper->getGOR());
        event->setFracturePressure((float)trapper->getFracturePressure());
        event->setBuoyancy((float)trapper->getBuoyancy());
        event->setWCSurface((float)trapper->getWCSurface());
        event->setReservoirName(trapper->getReservoir ()->getName());

        double x, y;
        trapper->getPosition(x, y);
        event->setPosition((float)x, (float)y);

        trapper->getSpillPointPosition(x, y);
        event->setSpillPointPosition((float)x, (float)y);

        event->setTrapCapacity((float)trapper->getCapacity());
        event->setDepth((float)trapper->getDepth());
        event->setGOC((float)trapper->getGOC());
        event->setOWC((float)trapper->getOWC());
        event->setSpillDepth((float)trapper->getSpillDepth());
        event->setPressure((float)trapper->getPressure());
        event->setTemperature((float)trapper->getTemperature());
        event->setPermeability((float)trapper->getPermeability());
        event->setSealPermeability((float)trapper->getSealPermeability());
        event->setPorosity((float)trapper->getPorosity());
        event->setNetToGross((float)trapper->getNetToGross());
        event->setAge((float)trapper->getSnapshot()->getTime());

        m_project->addTrapper(event);
     }
}

void ImportProjectHandle::addTrapIO()
{
   std::shared_ptr<TrapList> trapList(m_projectHandle.getTraps(0, 0, 0 ));

     if (trapList->size() == 0) return;
     for (size_t index = 0; index < trapList->size(); ++ index)
     {
        const Trap* trap = trapList->at(index);
        database::Record* record = trap->getRecord();

        std::shared_ptr<CauldronIO::Trap> event(new CauldronIO::Trap());

        event->setID(trap->getId());
        event->setReservoirName(trap->getReservoir ()->getName());
        for( int compId = 0; compId < CauldronIO::SpeciesNamesId::NUMBER_OF_SPECIES; ++ compId ) {
           event->setMass ( trap->getMass( (ComponentId)compId ), (CauldronIO::SpeciesNamesId)compId );
        }

        event->setVolumeOil((float)trap->getVolume(PhaseId::OIL));
        event->setVolumeGas((float)trap->getVolume(PhaseId::GAS));
        event->setCEPGas((float)getCEPGas(record));
        event->setCEPOil((float)getCEPOil(record));
        event->setCriticalTemperatureOil((float)getCriticalTemperatureOil(record));
        event->setCriticalTemperatureGas((float)getCriticalTemperatureGas(record));
        event->setInterfacialTensionOil((float)getInterfacialTensionOil(record));
        event->setInterfacialTensionGas((float)getInterfacialTensionGas(record));
        event->setFracturePressure((float)getFracturePressure(record));
        event->setFractSealStrength((float)getFractSealStrength(record));
        event->setWCSurface((float)getWCSurface(record));

        double x, y;
        trap->getPosition(x, y);
        event->setPosition((float)x, (float)y);

        trap->getSpillPointPosition(x, y);
        event->setSpillPointPosition((float)x, (float)y);

        event->setTrapCapacity((float)getTrapCapacity(record));
        event->setDepth((float)trap->getDepth());
        event->setGOC((float)trap->getGOC());
        event->setOWC((float)trap->getOWC());
        event->setSpillDepth((float)trap->getSpillDepth());
        event->setPressure((float)trap->getPressure());
        event->setTemperature((float)trap->getTemperature());
        event->setPermeability((float)getPermeability(record));
        event->setSealPermeability((float)getSealPermeability(record));
        event->setNetToGross((float)getNetToGross(record));
        event->setAge((float)trap->getSnapshot()->getTime());

        m_project->addTrap(event);
     }
}

void ImportProjectHandle::addMassBalance()  {

   const std::string massBalanceFileName = m_projectHandle.getProjectName() + "_MassBalance";

   ibs::FilePath folderPath( m_projectHandle.getProjectPath() );

   folderPath << massBalanceFileName;
   if( folderPath.exists() ) {
      m_project->setMassBalance( massBalanceFileName );
   } else {
      m_project->setMassBalance( "" );
   }
 }

void ImportProjectHandle::addGenexHistory()  {

   DataAccess::Interface::PointAdsorptionHistoryList* historyList = m_projectHandle.getPointAdsorptionHistoryList ( "" );
   DataAccess::Interface::PointAdsorptionHistoryList::const_iterator historyIter;

   std::vector<std::string> historyRecordsDefined;
   for ( historyIter = historyList->begin (); historyIter != historyList->end (); ++historyIter ) {
      if( (* historyIter)->getFileName () != "" ) {
         historyRecordsDefined.push_back( (* historyIter)->getFileName () );
      }
   }
   delete historyList;

   ibs::FilePath folderPath ( m_projectHandle.getFullOutputDir () );


   if( folderPath.exists() ) {
      boost::filesystem::directory_iterator it (folderPath.path());
      boost::filesystem::directory_iterator endit;

      while( it != endit ) {
         if( std::find( historyRecordsDefined.begin(), historyRecordsDefined.end(), it->path().filename() ) != historyRecordsDefined.end() or
             it->path().extension() == ".dat" ) {

            ibs::FilePath oneFilePath( it->path().string() );
            m_project->addGenexHistoryRecord( oneFilePath.cpath() );

         }
         it ++;
      }
   }
}
