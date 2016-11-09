//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "VisualizationIO_projectHandle.h"
#include "VisualizationAPI.h"
#include "VisualizationUtils.h"

#include "Interface/ProjectHandle.h"
#include "Interface/ObjectFactory.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"
#include "Interface/Reservoir.h"
#include "Interface/Surface.h"
#include "Interface/Trapper.h"
#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"
#include "Interface/ProjectData.h"
#include "../../SerialDataAccess/src/Interface/SerialGridMap.h"

#include "ImportProjectHandle.h"
#include "ImportExport.h"
#include <string>
#include <iostream>
#include <iomanip>
#include <limits>
#include <boost/foreach.hpp>

using namespace DataAccess;
using namespace DataAccess::Interface;
using namespace DataModel;

#define ALLSELECTION SURFACE | FORMATION | FORMATIONSURFACE | RESERVOIR

std::shared_ptr<CauldronIO::Project> ImportProjectHandle::createFromProjectHandle(std::shared_ptr<ProjectHandle> projectHandle, bool verbose)
{
    // Get modeling mode
    ModellingMode modeIn = projectHandle->getModellingMode();
    CauldronIO::ModellingMode mode = modeIn == MODE1D ? CauldronIO::MODE1D : CauldronIO::MODE3D;

    // Read general project data
    const Interface::ProjectData* projectData = projectHandle->getProjectData();

    // Create the project
    std::shared_ptr<CauldronIO::Project> project(new CauldronIO::Project(
        projectData->getProjectName(), projectData->getDescription(), projectData->getProjectTeam(),
        projectData->getProgramVersion(), mode, xml_version_major, xml_version_minor));

    // Import all snapshots
    ImportProjectHandle import(verbose, project);

    import.addSnapShots(projectHandle);

    return project;
}

ImportProjectHandle::ImportProjectHandle(bool verbose, std::shared_ptr<CauldronIO::Project>& project)
{
    m_verbose = verbose;
    m_project = project;
}

void ImportProjectHandle::addSnapShots(std::shared_ptr<Interface::ProjectHandle> projectHandle) 
{
    // Get all property values once
    m_propValues.reset(projectHandle->getPropertyValues(ALLSELECTION, 0, 0, 0, 0, 0, MAP | VOLUME));
    sort(m_propValues->begin(), m_propValues->end(), PropertyValue::SortByAgeAndDepoAge);

    m_props.reset(projectHandle->getProperties(true, ALLSELECTION, 0, 0, 0, 0, MAP | VOLUME));

    if (m_verbose)
        cout << "Loaded " << m_propValues->size() << " propertyvalues and " << m_props->size() << " properties" << endl;

    std::shared_ptr<SnapshotList> snapShots;
    snapShots.reset(projectHandle->getSnapshots(MAJOR | MINOR));

    for (size_t i = 0; i < snapShots->size(); i++)
    {
        const Snapshot* snapShot = snapShots->at(i);

         // Create a new snapshot
        std::shared_ptr<CauldronIO::SnapShot> snapShotIO = createSnapShotIO(projectHandle, snapShot);  

        // Add to project
        m_project->addSnapShot(snapShotIO);
    }
}

std::shared_ptr<CauldronIO::SnapShot> ImportProjectHandle::createSnapShotIO(std::shared_ptr<Interface::ProjectHandle> projectHandle,
    const DataAccess::Interface::Snapshot* snapShot)
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
    std::shared_ptr<CauldronIO::FormationInfoList> depthFormations = getDepthFormations(projectHandle, snapShot);

    // Add all the surfaces
    ///////////////////////////////////////////////////////////
    vector<std::shared_ptr<CauldronIO::Surface> > surfaces = createSurfaces(depthFormations, snapShot, snapShotPropVals);
    BOOST_FOREACH(std::shared_ptr<CauldronIO::Surface>& surface, surfaces)
        snapShotIO->addSurface(surface);

    // Bail out if we don't have depthFormations
	if (depthFormations->size() == 0)
	{
		if (m_verbose) std::cout << "Snapshot does not contain depth-information" << std::endl;
		return snapShotIO;
	}

    // Add the volume
    //////////////////////////////////////////////////////////////////////////////////////////////////
    std::shared_ptr<CauldronIO::Volume> volume(new CauldronIO::Volume(CauldronIO::None));
    snapShotIO->setVolume(volume);

    // Add all volumeData: loop over properties, then get all property values for the property
    for (size_t i = 0; i < snapShotProps->size(); ++i)
    {
        // Get all the propertyValue objects for this property and snapshot
        const Property* prop = snapShotProps->at(i);
        PropertyAttribute attrib = prop->getPropertyAttribute();
        // Bail out if it is not continuous
        if (attrib != CONTINUOUS_3D_PROPERTY) continue;
        std::shared_ptr<PropertyValueList> propValues = getFormationVolumePropertyValues(prop, snapShotPropVals, NULL);
        // Bail out if there are no property value objects
        if (propValues->size() == 0) continue;

        // Construct the geometry for the (continuous) volume
        std::shared_ptr<CauldronIO::Geometry3D> geometry3D = createGeometry3D(depthFormations);

        if (m_verbose)
            cout << " - Adding continuous volume data with (" << propValues->size() << ") formations for property " << prop->getName() << endl;
        CauldronIO::PropertyVolumeData propVolume = createPropertyVolumeData(propValues, geometry3D, depthFormations);
        volume->addPropertyVolumeData(propVolume);
    }

    // Add all formationVolumes
    //////////////////////////////////////////////////////////////////////////
    std::shared_ptr<FormationList> formations(projectHandle->getFormations(snapShot, true));
    for (size_t i = 0; i < formations->size(); ++i)
    {
        const Formation* formation = formations->at(i);
        std::shared_ptr<const CauldronIO::Formation> formationIO = findOrCreateFormation(formation, depthFormations);
        if (!formationIO)
        {
            if (m_verbose) cerr << "Warning: ignoring formation: " << formation->getName() << " not found as depth formation" << endl;
            continue;
        }

        std::shared_ptr<CauldronIO::FormationInfo> formationInfo = findDepthFormationInfo(formation, depthFormations);

        // Create the volume for this formation
        std::shared_ptr<CauldronIO::Geometry3D> geometry = createGeometry3D(formationInfo);
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

            CauldronIO::PropertyVolumeData propVolume = createPropertyVolumeData(propVal, geometry, formationInfo);
            volume->addPropertyVolumeData(propVolume);
        }

        if (m_verbose)
            cout << " - Adding discontinuous volume data formation " << formationIO->getName() << " with " 
                 << volume->getPropertyVolumeDataList().size() << " properties" << endl;

        // Only add if there is actual data
        if (volume->getPropertyVolumeDataList().size() > 0)
        {
            CauldronIO::FormationVolume formVolume(formationIO, volume);
            snapShotIO->addFormationVolume(formVolume);
        }
		else if (m_verbose)
		{
			cout << "No properties found for formation: " << formation->getName() << endl;
		}

    }
        
    // Find trappers
    std::shared_ptr<TrapperList> trapperList(projectHandle->getTrappers(0, snapShot, 0, 0));
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
    // 2. Nameless surfaces -> one map per surface
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

        string surfaceName;
        if (surface)
            surfaceName = surface->getName();
        else if (m_verbose)
        {
            cout << " - Adding map for formation: " << formation->getName() << " with property " << prop->getName() << endl;
            if (reservoir)
                cout << " -- from reservoir: " << reservoir->getName() << endl;
        }

        // Find or create a property
        std::shared_ptr<const CauldronIO::Property> propertyIO = findOrCreateProperty(prop);

        // Find or create a formation
        std::shared_ptr<const CauldronIO::Formation> formationIO;
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
		std::shared_ptr<const CauldronIO::Geometry2D> geometry(new CauldronIO::Geometry2D(gridmap->numI(), gridmap->numJ(), 
			gridmap->deltaI(), gridmap->deltaJ(), gridmap->minI(), gridmap->minJ()));
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
                cout << " - Adding surface: " << surfaceIO->getName() << endl;

            // Set the formations
            if (surface)
            {
                // Find or create top/bottom formations
                std::shared_ptr<const CauldronIO::Formation> topFormation = findOrCreateFormation(surface->getTopFormation(), depthFormations);
                std::shared_ptr<const CauldronIO::Formation> bottomFormation = findOrCreateFormation(surface->getBottomFormation(), depthFormations);
                assert(topFormation == formationIO || bottomFormation == formationIO || !formationIO);

                surfaceIO->setFormation(topFormation, true);
                surfaceIO->setFormation(bottomFormation, false);
            }
            else
            {
                // Assign same formation to bottom/surface formation
                assert(formationIO);
                surfaceIO->setFormation(formationIO, true);
                surfaceIO->setFormation(formationIO, false);
            }
        }

        // Set reservoir
        if (reservoir)
            propertyMap->setReservoir(reservoirIO);

        // Add the property/surfaceData object
        CauldronIO::PropertySurfaceData propSurfaceData(propertyIO, propertyMap);
        if (m_verbose)
            cout << " --- adding surface data for property " << propertyIO->getName() << endl;
        surfaceIO->addPropertySurfaceData(propSurfaceData);
    }

    return surfaces;
}

std::shared_ptr<CauldronIO::Surface> ImportProjectHandle::findSurface(vector< std::shared_ptr<CauldronIO::Surface> > surfaces, const string& surfaceName) const
{
    BOOST_FOREACH(std::shared_ptr<CauldronIO::Surface>& surface, surfaces)
        if (surface->getName() == surfaceName) return surface;
    
    return std::shared_ptr< CauldronIO::Surface >();
}

std::shared_ptr<CauldronIO::Surface> ImportProjectHandle::findSurface(vector< std::shared_ptr<CauldronIO::Surface> > surfaces, 
    std::shared_ptr<const CauldronIO::Formation>& formation) const
{
    BOOST_FOREACH(std::shared_ptr<CauldronIO::Surface>& surface, surfaces)
    {
        if (surface->getBottomFormation() == formation && surface->getTopFormation() == formation) return surface;
    }

    return std::shared_ptr< CauldronIO::Surface >();
}

std::shared_ptr<const CauldronIO::Property> ImportProjectHandle::findOrCreateProperty(const Property* prop)
{
    BOOST_FOREACH(const std::shared_ptr<const CauldronIO::Property>& property, m_project->getProperties())
        if (property->getName() == prop->getName()) return property;

    std::shared_ptr<const CauldronIO::Property> propertyIO(new CauldronIO::Property(
        prop->getName(), prop->getUserName(), prop->getCauldronName(),
        prop->getUnit(), getPropertyType(prop), getPropertyAttribute(prop)));
    m_project->addProperty(propertyIO);

    return propertyIO;
}

std::shared_ptr<const CauldronIO::Formation> ImportProjectHandle::findOrCreateFormation(const Formation* form, std::shared_ptr<CauldronIO::FormationInfoList> depthFormations)
{
    std::shared_ptr< const CauldronIO::Formation> formationIO;
    if (!form) return formationIO;
    
    BOOST_FOREACH(const std::shared_ptr<const CauldronIO::Formation>& formation, m_project->getFormations())
        if (formation->getName() == form->getName()) return formation;

    formationIO = createFormation(form, depthFormations);
    // It can be that this formation is not met with a depthformation: don't add it to the list
    if (!formationIO)
        return formationIO;

    m_project->addFormation(formationIO);

    return formationIO;
}

std::shared_ptr<const CauldronIO::Reservoir> ImportProjectHandle::findOrCreateReservoir(const Reservoir* reserv, std::shared_ptr<const CauldronIO::Formation> formationIO)
{
    assert(formationIO);

    BOOST_FOREACH(const std::shared_ptr<const CauldronIO::Reservoir>& reservoir, m_project->getReservoirs())
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
    size_t minK = (std::numeric_limits<size_t>::max)();
    for (size_t i = 0; i < depthFormations->size(); ++i)
    {
        std::shared_ptr<CauldronIO::FormationInfo>& depthInfo = depthFormations->at(i);
        // TODO: check if formations are continuous.. (it is assumed now)
        maxK = max(maxK, depthInfo->kEnd);
        minK = min(minK, depthInfo->kStart);
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
    const string snapShotKind = snapShot->getKind();
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

std::shared_ptr<CauldronIO::FormationInfoList> ImportProjectHandle::getDepthFormations(std::shared_ptr<Interface::ProjectHandle> projectHandle,
                                                                                                  const Snapshot* snapShot) const
{
    std::shared_ptr<CauldronIO::FormationInfoList> depthFormations(new CauldronIO::FormationInfoList());

    // Find the depth property
    const Property* depthProp = projectHandle->findProperty("Depth");
    if (!depthProp) return depthFormations;

    // Find the depth property formations for this snapshot
    std::shared_ptr<PropertyValueList> propValues(projectHandle->getPropertyValues(FORMATION, depthProp, snapShot, 0, 0, 0, VOLUME));
    if (propValues->size() == 0) return depthFormations;

    // Find all depth formations and add these
    for (size_t i = 0; i < propValues->size(); ++i)
    {
        GridMap* map = propValues->at(i)->getGridMap();

        if (!map) throw CauldronIO::CauldronIOException("Could not open project3D HDF file!");
        std::shared_ptr<CauldronIO::FormationInfo> info(new CauldronIO::FormationInfo());

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
        info->reverseDepth = sGridmap == NULL;
        
        depthFormations->push_back(info);
        
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

std::shared_ptr<const CauldronIO::Formation> ImportProjectHandle::createFormation(const DataAccess::Interface::Formation* formation, 
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

    formationPtr.reset(new CauldronIO::Formation(info->kStart, info->kEnd, info->formation->getName(), formation->isSourceRock(), formation->isMobileLayer()));

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
        if (propValue->getSurface() != NULL) continue;
        if (formation != NULL && propValue->getFormation() != formation) continue;

        propertyValueList->push_back(propValue);
    }

    return propertyValueList;
}
