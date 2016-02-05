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

#include "ImportProjectHandle.h"
#include "ImportExport.h"
#include <string>
#include <iostream>
#include <boost/foreach.hpp>

using namespace DataAccess;
using namespace DataAccess::Interface;
using namespace DataModel;

#define ALLSELECTION SURFACE | FORMATION | FORMATIONSURFACE | RESERVOIR

boost::shared_ptr<CauldronIO::Project> ImportProjectHandle::createFromProjectHandle(boost::shared_ptr<ProjectHandle> projectHandle, bool verbose)
{
    // Get modeling mode
    ModellingMode modeIn = projectHandle->getModellingMode();
    CauldronIO::ModellingMode mode = modeIn == MODE1D ? CauldronIO::MODE1D : CauldronIO::MODE3D;

    // Read general project data
    const Interface::ProjectData* projectData = projectHandle->getProjectData();

    // Create the project
    boost::shared_ptr<CauldronIO::Project> project(new CauldronIO::Project(
        projectData->getProjectName(), projectData->getDescription(), projectData->getProjectTeam(),
        projectData->getProgramVersion(), mode, (float)xml_version));

    // Import all snapshots
    ImportProjectHandle import(verbose, project);
    import.addSnapShots(projectHandle);

    return project;
}

ImportProjectHandle::ImportProjectHandle(bool verbose, boost::shared_ptr<CauldronIO::Project>& project)
{
    m_verbose = verbose;
    m_project = project;
}

void ImportProjectHandle::addSnapShots(boost::shared_ptr<Interface::ProjectHandle> projectHandle) 
{
    boost::shared_ptr<SnapshotList> snapShots;
    snapShots.reset(projectHandle->getSnapshots(MAJOR | MINOR));

    for (size_t i = 0; i < snapShots->size(); i++)
    {
        const Snapshot* snapShot = snapShots->at(i);

         // Create a new snapshot
        boost::shared_ptr<CauldronIO::SnapShot> snapShotIO = createSnapShotIO(projectHandle, snapShot);  

        // Add to project
        m_project->addSnapShot(snapShotIO);
    }
}

boost::shared_ptr<CauldronIO::SnapShot> ImportProjectHandle::createSnapShotIO(boost::shared_ptr<Interface::ProjectHandle> projectHandle,
    const DataAccess::Interface::Snapshot* snapShot)
{
    boost::shared_ptr<CauldronIO::SnapShot> snapShotIO(new CauldronIO::SnapShot(snapShot->getTime(), 
        getSnapShotKind(snapShot), snapShot->getType() == MINOR));

    //if (m_verbose)
        cout << "== Adding " << (snapShot->getType() == MINOR ? "Minor" : "Major") << " snapshot with Age: " << snapShot->getTime() << " === " << endl;

    // Create depth-geometry information
    boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations = getDepthFormations(projectHandle, snapShot);

    // Add all the surfaces
    ///////////////////////////////////////////////////////////
    vector<boost::shared_ptr<CauldronIO::Surface> > surfaces = createSurfaces(projectHandle, depthFormations, snapShot);
    BOOST_FOREACH(boost::shared_ptr<CauldronIO::Surface>& surface, surfaces)
        snapShotIO->addSurface(surface);

    // Bail out if we don't have depthFormations
    if (depthFormations->size() == 0) return snapShotIO;

    // Construct the geometry for the (continuous) volume
    boost::shared_ptr<const CauldronIO::Geometry3D> geometry3D = createGeometry3D(depthFormations);

    // Add the volume
    //////////////////////////////////////////////////////////////////////////////////////////////////
    boost::shared_ptr<CauldronIO::Volume> volume(new CauldronIO::Volume(CauldronIO::None, geometry3D));
    snapShotIO->setVolume(volume);

    // Add all volumeData: loop over properties, then get all property values for the property
    boost::shared_ptr<PropertyList> properties(projectHandle->getProperties(false, FORMATION, snapShot, 0, 0, 0, VOLUME));
    for (size_t i = 0; i < properties->size(); ++i)
    {
        // Get all the propertyValue objects for this property and snapshot
        const Property* prop = properties->at(i);
        PropertyAttribute attrib = prop->getPropertyAttribute();
        // Bail out if it is not continuous
        if (attrib != CONTINUOUS_3D_PROPERTY) continue;
        boost::shared_ptr<PropertyValueList> propValues(projectHandle->getPropertyValues(FORMATION, prop, snapShot, 0, 0, 0, VOLUME));
        // Bail out if there are no property value objects
        if (propValues->size() == 0) continue;

        if (m_verbose)
            cout << " - Adding continuous volume data with (" << propValues->size() << ") formations for property " << prop->getName() << endl;
        boost::shared_ptr<CauldronIO::PropertyVolumeData> propVolume = createPropertyVolumeData(propValues, geometry3D, depthFormations);
        volume->addPropertyVolumeData(propVolume);
    }

    // Add all formationVolumes
    //////////////////////////////////////////////////////////////////////////
    boost::shared_ptr<FormationList> formations(projectHandle->getFormations(snapShot, true));
    for (size_t i = 0; i < formations->size(); ++i)
    {
        const Formation* formation = formations->at(i);
        boost::shared_ptr<const CauldronIO::Formation> formationIO = findOrCreateFormation(formation, depthFormations);
        if (!formationIO)
        {
            if (m_verbose) cerr << "Warning: ignoring formation: " << formation->getName() << " not found as depth formation" << endl;
            continue;
        }

        boost::shared_ptr<CauldronIO::FormationInfo> formationInfo = findDepthFormationInfo(formation, depthFormations);

        // Create the volume for this formation
        boost::shared_ptr<const CauldronIO::Geometry3D> geometry = createGeometry3D(formationInfo);
        boost::shared_ptr<CauldronIO::Volume> volume(new CauldronIO::Volume(getSubSurfaceKind(formation), geometry));

        // Add all property-value data : loop over properties, then get all property values for the property and this formation
        boost::shared_ptr<PropertyList> properties(projectHandle->getProperties(false, FORMATION, snapShot, 0, formation, 0, VOLUME));
        for (size_t i = 0; i < properties->size(); ++i)
        {
            // Get all the propertyValue objects for this property and snapshot
            const Property* prop = properties->at(i);
            PropertyAttribute attrib = prop->getPropertyAttribute();
            // Bail out if it is not continuous
            if (attrib != DISCONTINUOUS_3D_PROPERTY) continue;
            boost::shared_ptr<PropertyValueList> propValues(projectHandle->getPropertyValues(FORMATION, prop, snapShot, 0, formation, 0, VOLUME));
            // Bail out if there are no property value objects
            if (propValues->size() == 0) continue;
            assert(propValues->size() == 1);
            const PropertyValue* propVal = propValues->at(0);

            boost::shared_ptr<CauldronIO::PropertyVolumeData> propVolume = createPropertyVolumeData(propVal, geometry, formationInfo);
            volume->addPropertyVolumeData(propVolume);
        }

        if (m_verbose)
            cout << " - Adding discontinuous volume data formation " << formationIO->getName() << " with " 
                 << volume->getPropertyVolumeDataList().size() << " properties" << endl;

        boost::shared_ptr<CauldronIO::FormationVolume> formVolume(new CauldronIO::FormationVolume(formationIO, volume));
        snapShotIO->addFormationVolume(formVolume);
    }
        
    // Find trappers
    boost::shared_ptr<TrapperList> trapperList(projectHandle->getTrappers(0, snapShot, 0, 0));
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

        const Reservoir* reservoir = trapper->getReservoir();
        assert(reservoir);

        // Create a new Trapper and assign some values
        boost::shared_ptr<CauldronIO::Trapper> trapperIO(new CauldronIO::Trapper(ID, persistentID));
        trapperIO->setReservoirName(reservoir->getName());
        trapperIO->setSpillDepth((float)spillPointDepth);
        trapperIO->setSpillPointPosition((float)spillPointX, (float)spillPointY);
        trapperIO->setDepth((float)depth);
        trapperIO->setPosition((float)pointX, (float)pointY);
        trapperIO->setDownStreamTrapperID(downstreamTrapperID);

        snapShotIO->addTrapper(trapperIO);
    }

    return snapShotIO;
}

vector<boost::shared_ptr<CauldronIO::Surface> > ImportProjectHandle::createSurfaces(
    boost::shared_ptr<DataAccess::Interface::ProjectHandle> projectHandle, boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations,
    const DataAccess::Interface::Snapshot* snapShot) 
{
    vector < boost::shared_ptr<CauldronIO::Surface> > surfaces;
    
    // Add all the surfaces
    //
    // 1. Collect all surfaces with the same name
    // 2. Nameless surfaces -> one map per surface
    // 3. For each property -> add to project (if not existing)
    // 4. For each formation -> add to project (if not existing)
    // 5. Add SurfaceData to Surface
    ////////////////////////////////////////////////////////
    
    boost::shared_ptr<PropertyValueList> propValues(projectHandle->getPropertyValues(ALLSELECTION, 0, snapShot, 0, 0, 0, MAP));

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
        boost::shared_ptr<const CauldronIO::Property> propertyIO = findOrCreateProperty(prop);

        // Find or create a formation
        boost::shared_ptr<const CauldronIO::Formation> formationIO;
        if (formation) 
            formationIO = findOrCreateFormation(formation, depthFormations);

        // Find or create a reservoir
        boost::shared_ptr<const CauldronIO::Reservoir> reservoirIO;
        if (reservoir) 
            reservoirIO = findOrCreateReservoir(reservoir, formationIO);

        // Does the surface exist?
        boost::shared_ptr<CauldronIO::Surface> surfaceIO;
        if (surfaceName.length() > 0)
            surfaceIO = findSurface(surfaces, surfaceName);
        else
            surfaceIO = findSurface(surfaces, formationIO);

        // Get the geometry data
        boost::shared_ptr<const CauldronIO::Geometry2D> geometry;
        if (surfaceIO)
        {
            geometry = surfaceIO->getGeometry();
        }
        else
        {
            // Get from the gridmap: this will load the gridmap :-(
            const DataAccess::Interface::GridMap* gridmap = propValue->getGridMap();
            // Set the geometry
            geometry.reset(new CauldronIO::Geometry2D(gridmap->numI(), gridmap->numJ(), gridmap->deltaI(), gridmap->deltaJ(), gridmap->minI(), gridmap->minJ()));
            gridmap->release();
        }

        // Create the surface data object
        boost::shared_ptr<CauldronIO::SurfaceData> propertyMap = createMapIO(propValue, geometry);

        // Instantiate the surface if not existing
        if (!surfaceIO)
        {
            surfaceIO.reset(new CauldronIO::Surface(surfaceName, getSubSurfaceKind(surface), geometry));
            surfaces.push_back(surfaceIO);

            if (m_verbose && surface)
                cout << " - Adding surface: " << surfaceIO->getName() << endl;

            // Set the formation
            if (formationIO) surfaceIO->setFormation(formationIO);
            // Set reservoir
            if (reservoir) surfaceIO->setReservoir(reservoirIO);
        }

        // Add the property/surfaceData object
        boost::shared_ptr<CauldronIO::PropertySurfaceData> propSurfaceData(new CauldronIO::PropertySurfaceData(propertyIO, propertyMap));
        if (m_verbose)
            cout << " --- adding surface data for property " << propertyIO->getName() << endl;
        surfaceIO->addPropertySurfaceData(propSurfaceData);
    }

    return surfaces;
}

boost::shared_ptr<CauldronIO::Surface> ImportProjectHandle::findSurface(vector< boost::shared_ptr<CauldronIO::Surface> > surfaces, const string& surfaceName) const
{
    BOOST_FOREACH(boost::shared_ptr<CauldronIO::Surface>& surface, surfaces)
        if (surface->getName() == surfaceName) return surface;
    
    return boost::shared_ptr< CauldronIO::Surface >();
}

boost::shared_ptr<CauldronIO::Surface> ImportProjectHandle::findSurface(vector< boost::shared_ptr<CauldronIO::Surface> > surfaces, 
    boost::shared_ptr<const CauldronIO::Formation>& formation) const
{
    BOOST_FOREACH(boost::shared_ptr<CauldronIO::Surface>& surface, surfaces)
        if (surface->getFormation() == formation) return surface;

    return boost::shared_ptr< CauldronIO::Surface >();
}

boost::shared_ptr<const CauldronIO::Property> ImportProjectHandle::findOrCreateProperty(const Property* prop)
{
    BOOST_FOREACH(const boost::shared_ptr<const CauldronIO::Property>& property, m_project->getProperties())
        if (property->getName() == prop->getName()) return property;

    boost::shared_ptr<const CauldronIO::Property> propertyIO(new CauldronIO::Property(
        prop->getName(), prop->getUserName(), prop->getCauldronName(),
        prop->getUnit(), getPropertyType(prop), getPropertyAttribute(prop)));
    m_project->addProperty(propertyIO);

    return propertyIO;
}

boost::shared_ptr<const CauldronIO::Formation> ImportProjectHandle::findOrCreateFormation(const Formation* form, boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations)
{
    BOOST_FOREACH(const boost::shared_ptr<const CauldronIO::Formation>& formation, m_project->getFormations())
        if (formation->getName() == form->getName()) return formation;

    boost::shared_ptr< const CauldronIO::Formation> formationIO = createFormation(form, depthFormations);
    // It can be that this formation is not met with a depthformation: don't add it to the list
    if (!formationIO)
        return formationIO;

    m_project->addFormation(formationIO);

    return formationIO;
}

boost::shared_ptr<const CauldronIO::Reservoir> ImportProjectHandle::findOrCreateReservoir(const Reservoir* reserv, boost::shared_ptr<const CauldronIO::Formation> formationIO)
{
    assert(formationIO);

    BOOST_FOREACH(const boost::shared_ptr<const CauldronIO::Reservoir>& reservoir, m_project->getReservoirs())
    {
        if (reservoir->getName() == reserv->getName() && reservoir->getFormation() == formationIO)
            return reservoir;
    }

    boost::shared_ptr<const CauldronIO::Reservoir> reservoirIO(new CauldronIO::Reservoir(reserv->getName(), formationIO));
    m_project->addReservoir(reservoirIO);

    return reservoirIO;
}


boost::shared_ptr<CauldronIO::FormationInfo> ImportProjectHandle::findDepthFormationInfo(const Formation* formation, boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations)
{
    for (size_t i = 0; i < depthFormations->size(); ++i)
    {
        boost::shared_ptr<CauldronIO::FormationInfo>& info = depthFormations->at(i);
        if (info->formation == formation)
            return info;
    }

    throw CauldronIO::CauldronIOException("Could not find depth geometry for formation");
}

boost::shared_ptr<const CauldronIO::Geometry3D> ImportProjectHandle::createGeometry3D(boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations)
{
    // Find the total depth size & offset
    assert(depthFormations->at(0)->kStart == 0);
    size_t maxK = 0;
    size_t minK = std::numeric_limits<size_t>::max();
    for (size_t i = 0; i < depthFormations->size(); ++i)
    {
        boost::shared_ptr<CauldronIO::FormationInfo>& depthInfo = depthFormations->at(i);
        // TODO: check if formations are continuous.. (it is assumed now)
        maxK = max(maxK, depthInfo->kEnd);
        minK = min(minK, depthInfo->kStart);
    }
    
    size_t depthK = 1 + maxK - minK;
    boost::shared_ptr<CauldronIO::FormationInfo>& info = depthFormations->at(0);
    boost::shared_ptr<const CauldronIO::Geometry3D> geometry(new CauldronIO::Geometry3D(info->numI, info->numJ, depthK, minK, info->deltaI, info->deltaJ, info->minI, info->minJ));

    return geometry;
}

boost::shared_ptr<const CauldronIO::Geometry3D> ImportProjectHandle::createGeometry3D(const boost::shared_ptr<CauldronIO::FormationInfo>& info)
{
    size_t numK = 1 + info->kEnd - info->kStart;
    boost::shared_ptr<const CauldronIO::Geometry3D> geometry(new CauldronIO::Geometry3D(info->numI, info->numJ, numK, (int)info->kStart, info->deltaI, info->deltaJ, info->minI, info->minJ));
    return geometry;
}

boost::shared_ptr<CauldronIO::PropertyVolumeData> ImportProjectHandle::createPropertyVolumeData(boost::shared_ptr<PropertyValueList> propValues,
    boost::shared_ptr<const CauldronIO::Geometry3D>& geometry3D, boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations)
{
    CauldronIO::VolumeProjectHandle* volumeDataProjHandle = new CauldronIO::VolumeProjectHandle(geometry3D);
    boost::shared_ptr<CauldronIO::VolumeData> volumeData(volumeDataProjHandle);
    
    volumeDataProjHandle->setDataStore(propValues, depthFormations);

    boost::shared_ptr<const CauldronIO::Property> prop = findOrCreateProperty(propValues->at(0)->getProperty());
    boost::shared_ptr<CauldronIO::PropertyVolumeData> propVolumeData(new CauldronIO::PropertyVolumeData(prop, volumeData));

    return propVolumeData;
}


boost::shared_ptr<CauldronIO::PropertyVolumeData> ImportProjectHandle::createPropertyVolumeData(const DataAccess::Interface::PropertyValue* propVal, 
    boost::shared_ptr<const CauldronIO::Geometry3D>& geometry3D, boost::shared_ptr<CauldronIO::FormationInfo> formationInfo)
{
    CauldronIO::VolumeProjectHandle* volumeDataProjHandle = new CauldronIO::VolumeProjectHandle(geometry3D);
    boost::shared_ptr<CauldronIO::VolumeData> volumeData(volumeDataProjHandle);

    volumeDataProjHandle->setDataStore(propVal, formationInfo);

    boost::shared_ptr<const CauldronIO::Property> prop = findOrCreateProperty(propVal->getProperty());
    boost::shared_ptr<CauldronIO::PropertyVolumeData> propVolumeData(new CauldronIO::PropertyVolumeData(prop, volumeData));

    return propVolumeData;
}

boost::shared_ptr<CauldronIO::SurfaceData> ImportProjectHandle::createMapIO(const DataAccess::Interface::PropertyValue* propVal,
    boost::shared_ptr<const CauldronIO::Geometry2D>& geometry)
{
    // Create a projectHandle specific Map object
    CauldronIO::MapProjectHandle* mapInternal = new CauldronIO::MapProjectHandle(geometry);

    boost::shared_ptr<CauldronIO::SurfaceData> map(mapInternal);
    // Set data to be retrieved later
    mapInternal->setDataStore(propVal);

    return map;
}

CauldronIO::SnapShotKind ImportProjectHandle::getSnapShotKind(const Snapshot* snapShot) const
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

CauldronIO::PropertyType ImportProjectHandle::getPropertyType(const Property* prop) const
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

CauldronIO::PropertyAttribute ImportProjectHandle::getPropertyAttribute(const Property* prop) const
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

boost::shared_ptr<CauldronIO::FormationInfoList> ImportProjectHandle::getDepthFormations(boost::shared_ptr<Interface::ProjectHandle> projectHandle,
                                                                                                  const Snapshot* snapShot) const
{
    boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations(new CauldronIO::FormationInfoList());

    // Find the depth property
    const Property* depthProp = projectHandle->findProperty("Depth");
    if (!depthProp) return depthFormations;

    // Find the depth property formations for this snapshot
    boost::shared_ptr<PropertyValueList> propValues(projectHandle->getPropertyValues(FORMATION, depthProp, snapShot, 0, 0, 0, VOLUME));
    if (propValues->size() == 0) return depthFormations;

    // Find all depth formations and add these
    for (size_t i = 0; i < propValues->size(); ++i)
    {
        double min, max;
        GridMap* map = propValues->at(i)->getGridMap();
        map->getMinMaxValue(min, max);
        boost::shared_ptr<CauldronIO::FormationInfo> info(new CauldronIO::FormationInfo());
        info->formation = propValues->at(i)->getFormation();
        info->kStart = map->firstK();
        info->kEnd = map->lastK();
        info->propValue = propValues->at(i);

        assert(info->kStart < info->kEnd);

        info->depthStart = min;
        info->depthEnd = max;
        info->numI = map->numI();
        info->numJ = map->numJ();
        info->deltaI = map->deltaI();
        info->deltaJ = map->deltaJ();
        info->minI = map->minI();
        info->minJ = map->minJ();

        // Find average depth values in first and last k slice
        double depth1 = 0, depth2 = 0; 
        size_t numElem1 = 0, numElem2 = 0;
        for (unsigned int i = map->firstI(); i <= map->lastI(); ++i)
        {
            for (unsigned int j = map->firstJ(); j <= map->lastJ(); ++j)
            {
                double val = map->getValue(i, j, map->firstK());
                if (val != map->getUndefinedValue())
                {
                    numElem1++;
                    depth1 += val;
                }
                val = map->getValue(i, j, map->lastK());
                if (val != map->getUndefinedValue())
                {
                    numElem2++;
                    depth2 += val;
                }
            }
        }

        if (numElem1 == 0 || numElem2 == 0)
            throw CauldronIO::CauldronIOException("Cannot sort depth formations: depth values undefined");

        depth1 /= numElem1;
        depth2 /= numElem2;

        info->reverseDepth = depth1 > depth2;
        depthFormations->push_back(info);
        map->release();
    }

    // Sort the list
    std::sort(depthFormations->begin(), depthFormations->end(), CauldronIO::FormationInfo::compareFormations);

    // Capture global k-range
    size_t currentK = depthFormations->at(0)->kEnd;
    for (int i = 1; i < depthFormations->size(); ++i)
    {
        boost::shared_ptr<CauldronIO::FormationInfo>& info = depthFormations->at(i);
        info->kStart += currentK;
        info->kEnd += currentK;
        currentK = info->kEnd;
    }
    return depthFormations;
}

boost::shared_ptr<const CauldronIO::Formation> ImportProjectHandle::createFormation(const DataAccess::Interface::Formation* formation, 
    boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations) const
{
    boost::shared_ptr<CauldronIO::Formation> formationPtr;
    
    if (!formation) return formationPtr;
    boost::shared_ptr<CauldronIO::FormationInfo> info;
    
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