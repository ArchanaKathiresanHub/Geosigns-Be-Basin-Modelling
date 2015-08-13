//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ImportProjectHandle.h"
#include "Interface/ProjectHandle.h"
#include "Interface/ObjectFactory.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"
#include "Interface/Surface.h"
#include "Interface/GridMap.h"
#include "Interface/ProjectData.h"
#include <string>
#include <iostream>

using namespace DataAccess;
using namespace DataAccess::Interface;
using namespace DataModel;

#define allSelection SURFACE | FORMATION | FORMATIONSURFACE | RESERVOIR

boost::shared_ptr<CauldronIO::Project> ImportProjectHandle::CreateFromProjectHandle(const string& projectname, bool verbose,
    double& readMB, size_t& readSurfaces, size_t& nonzeroSurfaces, size_t& readFormations, size_t& nonzeroFormations)
{
    // Try to open the projectHandle
    boost::shared_ptr<ObjectFactory> factory(new ObjectFactory());
    boost::shared_ptr<Interface::ProjectHandle> projectHandle(OpenCauldronProject(projectname, "r", factory.get()));
    if (!projectHandle)
    {
        cerr << "Could not open the project handle" << endl;
        return NULL;
    }

    // For distributed data access, we need an activity output grid
    //projectHandle->startActivity("Dummy", projectHandle->getLowResolutionOutputGrid());

    // Get modeling mode
    ModellingMode modeIn = projectHandle->getModellingMode();
    CauldronIO::ModellingMode mode = modeIn == MODE1D ? CauldronIO::MODE1D : CauldronIO::MODE3D;

    // Read general project data
    const Interface::ProjectData* projectData = projectHandle->getProjectData();

    // Create the project
    boost::shared_ptr<CauldronIO::Project> project(new CauldronIO::Project(
        projectData->getProjectName(), projectData->getDescription(), projectData->getProjectTeam(),
        projectData->getProgramVersion(), mode));

    // Import all snapshots
    ImportProjectHandle import(verbose);
    import.AddSnapShots(projectHandle, project);

    readMB = import._readMB;
    readSurfaces = import._readSurfaces;
    readFormations = import._readFormations;
    nonzeroFormations = import._nonzeroFormations;
    nonzeroSurfaces = import._nonzeroSurfaces;
    
    return project;
}

void ImportProjectHandle::AddSnapShots(boost::shared_ptr<Interface::ProjectHandle> projectHandle, 
                                       boost::shared_ptr<CauldronIO::Project> project)
{
    boost::shared_ptr<SnapshotList> snapShots;
    snapShots.reset(projectHandle->getSnapshots(MAJOR | MINOR));

    for (size_t i = 0; i < snapShots->size(); i++)
    {
        const Snapshot* snapShot = snapShots->at(i);

         // Create a new snapshot
        boost::shared_ptr<const CauldronIO::SnapShot> snapShotIO = CreateSnapShotIO(projectHandle, snapShot);  

        // Add to project
        project->AddSnapShot(snapShotIO);
    }
}

boost::shared_ptr<const CauldronIO::SnapShot> ImportProjectHandle::CreateSnapShotIO(boost::shared_ptr<Interface::ProjectHandle> projectHandle,
    const DataAccess::Interface::Snapshot* snapShot)
{
    boost::shared_ptr<CauldronIO::SnapShot> snapShotIO(new CauldronIO::SnapShot(snapShot->getTime(), 
        GetSnapShotKind(snapShot), snapShot->getType() == MINOR));

    if (_verbose)
        cout << "== Adding " << (snapShot->getType() == MINOR ? "Minor" : "Major") << " snapshot with Age: " << snapShot->getTime() << " === " << endl;

    // Create depth-geometry information
    boost::shared_ptr<FormationInfoList> depthFormations = GetDepthFormations(projectHandle, snapShot);

    // Add all the surfaces
    ///////////////////////////////////////////////////////////
    
    boost::shared_ptr< vector<boost::shared_ptr<CauldronIO::Surface> > > surfaces = CreateSurfaces(projectHandle, depthFormations, snapShot);

    // Set the depth map for these surfaces and add to the snapShot
    for (size_t i = 0; i < surfaces->size(); ++i)
    {
        boost::shared_ptr<CauldronIO::Surface> surface = surfaces->at(i);
        boost::shared_ptr<const CauldronIO::Surface> depthSurface = FindDepthSurface(surface, surfaces);
        if (depthSurface) surface->SetDepthSurface(depthSurface);
        snapShotIO->AddSurface(surface);
    }

    // Bail out if we don't have depthFormations
    if (depthFormations->size() == 0) return snapShotIO;
    
    // Add all the volumes
    ////////////////////////////////////////////////////////

    // Construct the depth volume first
    boost::shared_ptr<PropertyValueList> depthPropValues(new PropertyValueList());
    for (size_t i = 0; i < depthFormations->size(); ++i)
        depthPropValues->push_back(depthFormations->at(i)->propValue);
    boost::shared_ptr<CauldronIO::Volume> depthVolume = CreateContinuousVolume(depthPropValues, depthFormations);
    snapShotIO->AddVolume(depthVolume);

    // Add all other volumes: loop over properties, then get all property values for the property
    boost::shared_ptr<PropertyList> properties(projectHandle->getProperties(false, FORMATION, snapShot, 0, 0, 0, VOLUME));
    for (size_t i = 0; i < properties->size(); ++i)
    {
        // Don't add the depth volume twice
        const Property* prop = properties->at(i);
        if (prop->getName() == "Depth") continue;
        boost::shared_ptr<PropertyValueList> propValues(projectHandle->getPropertyValues(FORMATION, prop, snapShot, 0, 0, 0, VOLUME));

        PropertyAttribute attrib = prop->getPropertyAttribute();
        if (attrib == CONTINUOUS_3D_PROPERTY)
        {
            if (_verbose)
                cout << " - Adding continuous volume size(" << propValues->size() << ") for property" << prop->getName() << endl;
            boost::shared_ptr<CauldronIO::Volume> volume = CreateContinuousVolume(propValues, depthFormations);
            snapShotIO->AddVolume(volume);
        }
        else if (attrib == DISCONTINUOUS_3D_PROPERTY)
        {
            if (_verbose)
                cout << " - Adding discontinuous volume size(" << propValues->size() << ") for property" << prop->getName() << endl;
            boost::shared_ptr<CauldronIO::DiscontinuousVolume> discVolume = CreateDiscontinuousVolume(propValues, depthFormations);
            snapShotIO->AddDiscontinuousVolume(discVolume);
        }
        else if (prop->getName() == "Reflectivity")
        {
            if (_verbose)
                cout << " - Ignoring Reflectivity volume" << endl;
        }
        else
        {
            throw CauldronIO::CauldronIOException("Unrecognized volume property");
        }
    }

    return snapShotIO;
}

boost::shared_ptr<vector<boost::shared_ptr<CauldronIO::Surface> > >  ImportProjectHandle::CreateSurfaces(
    boost::shared_ptr<DataAccess::Interface::ProjectHandle> projectHandle, boost::shared_ptr<FormationInfoList> depthFormations,
    const DataAccess::Interface::Snapshot* snapShot) 
{
    boost::shared_ptr<vector<boost::shared_ptr<CauldronIO::Surface> > > surfaces(new vector<boost::shared_ptr<CauldronIO::Surface> >());
    
    // Add all the surfaces
    ////////////////////////////////////////////////////////
    boost::shared_ptr<PropertyValueList> propValues(projectHandle->getPropertyValues(allSelection, 0, snapShot, 0, 0, 0, MAP));

    for (size_t i = 0; i < propValues->size(); ++i)
    {
        const PropertyValue* propValue = propValues->at(i);
        const Property* prop = propValue->getProperty();
        const Formation* formation = propValue->getFormation();
        CauldronIO::PropertyType propType = GetPropertyType(prop);
        CauldronIO::PropertyAttribute propAttrib = GetPropertyAttribute(prop);
        const Surface* surface = propValue->getSurface();

        // A map needs a surface or a formation
        if (!surface && !formation) throw CauldronIO::CauldronIOException("Found map without a surface");
        string surfaceName;

        if (surface)
        {
            surfaceName = surface->getName();
            if (_verbose)
                cout << " - Adding surface: " << surface->getName() << " with property " << prop->getName() << endl;
        }
        else
            if (_verbose)
                cout << " - Adding map for formation: " << formation->getName() << " with property " << prop->getName() << endl;

        // Create a property
        boost::shared_ptr<const CauldronIO::Property> propertyIO(new CauldronIO::Property(prop->getName(), prop->getUserName(), prop->getCauldronName(),
            prop->getUnit(), propType, propAttrib));
        // Create a formation
        boost::shared_ptr<const CauldronIO::Formation> formationIO = CreateFormation(formation, depthFormations);

        // Get the surface data
        boost::shared_ptr<const CauldronIO::Map> propertyMap = CreateMapIO(propValue->getGridMap());

        // Create the surface object
        boost::shared_ptr<CauldronIO::Surface> surfaceIO(new CauldronIO::Surface(surfaceName,
            GetSubSurfaceKind(surface), propertyIO, propertyMap));
        
        // Set the formation
        if (formationIO) surfaceIO->SetFormation(formationIO);

        surfaces->push_back(surfaceIO);
    }

    return surfaces;
}

boost::shared_ptr<const CauldronIO::Surface> ImportProjectHandle::FindDepthSurface(
    boost::shared_ptr<CauldronIO::Surface> targetSurface, boost::shared_ptr< vector<boost::shared_ptr<CauldronIO::Surface> > > surfaces) const
{
    for (size_t i = 0; i < surfaces->size(); ++i)
    {
        boost::shared_ptr<CauldronIO::Surface> depthSurface = surfaces->at(i);
        if ((depthSurface->GetProperty()->GetName() == "Depth") && depthSurface->GetName() == targetSurface->GetName())
            return depthSurface;
    }

    return NULL;
}

ImportProjectHandle::FormationInfo* ImportProjectHandle::FindDepthInfo(boost::shared_ptr<FormationInfoList> depthFormations, 
    const DataAccess::Interface::Formation* formation) const
{
    for (size_t i = 0; i < depthFormations->size(); ++i)
    {
        if (depthFormations->at(i)->formation == formation) return depthFormations->at(i);
    }

    throw CauldronIO::CauldronIOException("Cannot find depth formation for requested formation");
}

boost::shared_ptr<CauldronIO::DiscontinuousVolume> ImportProjectHandle::CreateDiscontinuousVolume(
    boost::shared_ptr<PropertyValueList> propValueList,
    boost::shared_ptr<FormationInfoList> depthFormations) 
{
    boost::shared_ptr<CauldronIO::DiscontinuousVolume> aggregrateVolume(new CauldronIO::DiscontinuousVolume());

    for (size_t i = 0; i < propValueList->size(); ++i)
    {
        const PropertyValue* propValue = propValueList->at(i);
        
        // Create a new volume & formation
        boost::shared_ptr<const CauldronIO::Volume> volume = CreateVolume(propValue, depthFormations);
        boost::shared_ptr<const CauldronIO::Formation> formation = CreateFormation(propValue->getFormation(), depthFormations);

        // Add pair
        aggregrateVolume->AddVolume(formation, volume);
    }

    return aggregrateVolume;
}

boost::shared_ptr<CauldronIO::Volume> ImportProjectHandle::CreateVolume(const DataAccess::Interface::PropertyValue* propValue,
    boost::shared_ptr<FormationInfoList> depthFormations) 
{
    // Create a new empty volume
    boost::shared_ptr<CauldronIO::Volume> volume = CreateEmptyVolume(propValue);

    // Find the depth information
    FormationInfo* depthInfo = FindDepthInfo(depthFormations, propValue->getFormation());
    size_t depthK = 1 + depthInfo->kEnd - depthInfo->kStart;

    // Set the values
    const GridMap* gridMap = propValue->getGridMap();
    gridMap->retrieveData();

    if (gridMap->isConstant())
    {
        volume->SetConstantValue((float)gridMap->getConstantValue());
    }
    else
    {
        float* inputData = new float[gridMap->numI() * gridMap->numJ() * depthK];
        volume->SetGeometry(gridMap->numI(), gridMap->numJ(), depthK, depthInfo->kStart, gridMap->deltaI(), gridMap->deltaJ(), gridMap->minI(), gridMap->minJ());

        // Get the volume data for this formation
        gridMap->retrieveData();
        assert(gridMap->firstI() == 0 && gridMap->firstJ() == 0 && gridMap->firstK() == 0);

        for (unsigned int k = 0; k <= gridMap->lastK(); ++k)
        {
            size_t kIndex = depthInfo->reverseDepth ? depthInfo->kEnd - (size_t)k : depthInfo->kStart + (size_t)k;
            size_t index = volume->ComputeIndex_IJK(0, 0, kIndex);

            for (unsigned int j = 0; j <= gridMap->lastJ(); ++j)
            {
                for (unsigned int i = 0; i <= gridMap->lastI(); ++i)
                {
                    float val = (float)gridMap->getValue(i, j, k);
                    inputData[index++] = val;
                }
            }
        }

        volume->SetData_IJK(inputData);
        delete[] inputData;

        _readMB += 4 * gridMap->numI()*gridMap->numJ()*(1 + gridMap->lastK()) / double(1024 * 1024);
        _nonzeroFormations++;
    }

    _readFormations++;
    return volume;
}

boost::shared_ptr<CauldronIO::Volume> ImportProjectHandle::CreateEmptyVolume(const PropertyValue* propVal) const
{
    assert(propVal->getFormation());
    const Property* prop = propVal->getProperty();
    const GridMap* propGridMap = propVal->getGridMap();
    propGridMap->retrieveData();

    boost::shared_ptr<const CauldronIO::Property> propertyIO(new CauldronIO::Property(prop->getName(), prop->getUserName(), prop->getCauldronName(),
        prop->getUnit(), GetPropertyType(prop), GetPropertyAttribute(prop)));
    boost::shared_ptr<CauldronIO::Volume> volume(new CauldronIO::Volume(false, (float)propGridMap->getUndefinedValue(), GetSubSurfaceKind(propVal->getFormation()), propertyIO));

    return volume;
}

boost::shared_ptr<CauldronIO::Volume> ImportProjectHandle::CreateContinuousVolume(boost::shared_ptr<DataAccess::Interface::PropertyValueList> propValues,
    boost::shared_ptr<FormationInfoList> depthFormations) 
{
    // Construct a volume from a set of formations; the prop-values should allow for this
    if (propValues->size() == 0) throw CauldronIO::CauldronIOException("Cannot create volume from empty property value list");

    // Create a new empty volume
    const PropertyValue* propVal = propValues->at(0);
    boost::shared_ptr<CauldronIO::Volume> volume = CreateEmptyVolume(propVal);
    if (!(propVal->getProperty()->getPropertyAttribute() == CONTINUOUS_3D_PROPERTY))
        throw CauldronIO::CauldronIOException("Cannot create a volume for a non-continuous 3D property");

    // Find the total depth size & offset
    assert(depthFormations->at(0)->kStart == 0);
    size_t maxK = 0;
    size_t minK = std::numeric_limits<size_t>::max();
    for (size_t i = 0; i < propValues->size(); ++i)
    {
        FormationInfo* depthInfo = FindDepthInfo(depthFormations, propValues->at(i)->getFormation());
        // TODO: check if formations are continuous.. (it is assumed now)
        maxK = max(maxK, depthInfo->kEnd);
        minK = min(minK, depthInfo->kStart);
    }
    size_t depthK = 1 + maxK - minK;

    // Allocate some space
    const GridMap* propGridMap = propVal->getGridMap();
    float* inputData = new float[propGridMap->numI() * propGridMap->numJ() * depthK];
    volume->SetGeometry(propGridMap->numI(), propGridMap->numJ(), depthK, minK, propGridMap->deltaI(), propGridMap->deltaJ(), propGridMap->minI(), propGridMap->minJ());
    
    // Detect a constant volume consisting of all constant subvolumes (bit extreme case though)
    float constantValue;
    bool isConstant = true;
    bool firstConstant = true;

    // Get data
    for (size_t i = 0; i < propValues->size(); ++i)
    {
        const GridMap* gridMap = propValues->at(i)->getGridMap();
        FormationInfo* depthInfo = FindDepthInfo(depthFormations, propValues->at(i)->getFormation());

        // Get the volume data for this formation
        gridMap->retrieveData();
        assert(gridMap->firstI() == 0 && gridMap->firstJ() == 0 && gridMap->firstK() == 0);

        for (unsigned int k = 0; k <= gridMap->lastK(); ++k)
        {
            size_t kIndex = depthInfo->reverseDepth ? depthInfo->kEnd - (size_t)k : depthInfo->kStart + (size_t)k;
            size_t index = volume->ComputeIndex_IJK(0, 0, kIndex);
            
            for (unsigned int j = 0; j <= gridMap->lastJ(); ++j)
            {
                for (unsigned int i = 0; i <= gridMap->lastI(); ++i)
                {
                    float val = (float)gridMap->getValue(i, j, k);
                    inputData[index++] = val;

                    if (firstConstant)
                    {
                        constantValue = val; 
                        firstConstant = false;
                    }
                    if (isConstant) isConstant = val == constantValue;
                }
            }
        }

        _readMB += 4 * gridMap->numI()*gridMap->numJ()*(1 + gridMap->lastK()) / double(1024 * 1024);
        _readFormations++;
    }

    // Assign the data
    if (!isConstant)
    {
        _nonzeroFormations += propValues->size();
        volume->SetData_IJK(inputData);
    }
    else
        volume->SetConstantValue(constantValue);

    delete[] inputData;

    return volume;
}

boost::shared_ptr<const CauldronIO::Map> ImportProjectHandle::CreateMapIO(const GridMap* gridmap) 
{
    assert(gridmap);
    gridmap->retrieveData();
    assert(gridmap->firstI() == 0);
    assert(gridmap->firstJ() == 0);

    boost::shared_ptr<CauldronIO::Map> map(new CauldronIO::Map(false, (float)gridmap->getUndefinedValue()));
    // Set the geometry
    map->SetGeometry(gridmap->numI(), gridmap->numJ(), gridmap->deltaI(), gridmap->deltaJ(), gridmap->minI(), gridmap->minJ());
    
    if (gridmap->isConstant())
    {
        map->SetConstantValue((float)gridmap->getConstantValue());
    }
    else
    {
        // Get the data
        float* mapData = new float[map->GetNumI() * map->GetNumJ()];
        size_t index = 0;
        for (unsigned int j = 0; j < map->GetNumJ(); ++j)
            for (unsigned int i = 0; i < map->GetNumI(); ++ i)
                // Store row first
                mapData[index++] = (float)gridmap->getValue(i, j);
        map->SetData_IJ(mapData);
        delete[] mapData;

        _readMB += 4 * map->GetNumI()*map->GetNumJ() / double(1024 * 1024);
        _nonzeroSurfaces++;
    }

    _readSurfaces++;

    return map;
}

ImportProjectHandle::ImportProjectHandle(bool verbose)
{
    _verbose = verbose;
    _readMB = 0;
    _readSurfaces = _readFormations = _nonzeroFormations = _nonzeroSurfaces = 0;
}

CauldronIO::SnapShotKind ImportProjectHandle::GetSnapShotKind(const Snapshot* snapShot) const
{
    // Get snapshot kind
    const string snapShotKind = snapShot->getKind();
    CauldronIO::SnapShotKind kind = CauldronIO::SnapShotKind::NONE;
    if (snapShotKind == "System Generated")
        kind = CauldronIO::SnapShotKind::SYSTEM;
    else if (snapShotKind == "User Defined")
        kind = CauldronIO::SnapShotKind::USERDEFINED;

    return kind;
}

CauldronIO::PropertyType ImportProjectHandle::GetPropertyType(const Property* prop) const
{
    PropertyType type = prop->getType();
    CauldronIO::PropertyType typeIO;
    switch (type)
    {
    case PropertyType::FORMATIONPROPERTY:
        typeIO = CauldronIO::PropertyType::FormationProperty;
        break;
    case PropertyType::RESERVOIRPROPERTY:
        typeIO = CauldronIO::PropertyType::ReservoirProperty;
        break;
    case PropertyType::TRAPPROPERTY:
        typeIO = CauldronIO::PropertyType::TrapProperty;
        break;
    }

    return typeIO;
}

CauldronIO::SubsurfaceKind  ImportProjectHandle::GetSubSurfaceKind(const Surface* surface) const
{
    CauldronIO::SubsurfaceKind kind = CauldronIO::SubsurfaceKind::None;
    if (!surface) return kind;

    switch (surface->kind())
    {
    case SurfaceKind::BASEMENT_SURFACE:
        kind = CauldronIO::SubsurfaceKind::Basement;
        break;
    case SurfaceKind::SEDIMENT_SURFACE:
        kind = CauldronIO::SubsurfaceKind::Sediment;
        break;
    }

    return kind;
}

CauldronIO::SubsurfaceKind ImportProjectHandle::GetSubSurfaceKind(const DataAccess::Interface::Formation* formation) const
{
    CauldronIO::SubsurfaceKind kind = CauldronIO::SubsurfaceKind::None;
    if (!formation) return kind;

    switch (formation->kind())
    {
    case FormationKind::BASEMENT_FORMATION:
        kind = CauldronIO::SubsurfaceKind::Basement;
        break;
    case FormationKind::SEDIMENT_FORMATION:
        kind = CauldronIO::SubsurfaceKind::Sediment;
        break;
    }

    return kind;
}

CauldronIO::PropertyAttribute ImportProjectHandle::GetPropertyAttribute(const Property* prop) const
{
    PropertyAttribute attrib = prop->getPropertyAttribute();
    CauldronIO::PropertyAttribute attribIO;
    switch (attrib)
    {
    case PropertyAttribute::CONTINUOUS_3D_PROPERTY:
        attribIO = CauldronIO::PropertyAttribute::Continuous3DProperty;
        break;
    case PropertyAttribute::DISCONTINUOUS_3D_PROPERTY:
        attribIO = CauldronIO::PropertyAttribute::Discontinuous3DProperty;
        break;
    case PropertyAttribute::FORMATION_2D_PROPERTY:
        attribIO = CauldronIO::PropertyAttribute::Formation2DProperty;
        break;
    case PropertyAttribute::SURFACE_2D_PROPERTY:
        attribIO = CauldronIO::PropertyAttribute::Surface2DProperty;
        break;
    default:
        attribIO = CauldronIO::PropertyAttribute::Other;
        break;
    }

    return attribIO;
}

boost::shared_ptr<ImportProjectHandle::FormationInfoList> ImportProjectHandle::GetDepthFormations(boost::shared_ptr<Interface::ProjectHandle> projectHandle, 
                                                                                                  const Snapshot* snapShot) const
{
    boost::shared_ptr<FormationInfoList> depthFormations(new FormationInfoList());

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
        map->retrieveData();
        map->getMinMaxValue(min, max);
        FormationInfo* info = new FormationInfo();
        info->formation = propValues->at(i)->getFormation();
        info->kStart = map->firstK();
        info->kEnd = map->lastK();
        info->propValue = propValues->at(i);

        assert(info->kStart < info->kEnd);

        info->depthStart = min;
        info->depthEnd = max;

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
    }

    // Sort the list
    std::sort(depthFormations->begin(), depthFormations->end(), FormationInfo::CompareFormations);

    // Capture global k-range
    size_t currentK = depthFormations->at(0)->kEnd;
    for (int i = 1; i < depthFormations->size(); ++i)
    {
        FormationInfo* info = depthFormations->at(i);
        info->kStart += currentK;
        info->kEnd += currentK;
        currentK = info->kEnd;
    }
    return depthFormations;
}

boost::shared_ptr<const CauldronIO::Formation> ImportProjectHandle::CreateFormation(const DataAccess::Interface::Formation* formation, 
                                                            boost::shared_ptr<FormationInfoList> depthFormations) const
{
    if (!formation) return NULL;
    FormationInfo* info = NULL;
    
    // Find the depth formation
    for (size_t i = 0; i < depthFormations->size(); ++i)
    {
        if (depthFormations->at(i)->formation == formation)
        {
            info = depthFormations->at(i);
            break;
        }
    }

    if (!info) return NULL;

    boost::shared_ptr<const CauldronIO::Formation>formationIO(new CauldronIO::Formation(info->kStart, info->kEnd, info->formation->getName()));

    return formationIO;
}

bool ImportProjectHandle::FormationInfo::CompareFormations(const FormationInfo* info1, const FormationInfo* info2)
{
    return info1->depthStart < info2->depthStart;
}
