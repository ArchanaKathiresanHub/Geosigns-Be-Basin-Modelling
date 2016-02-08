//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ImportExport.h"
#include "VisualizationIO_native.h"
#include "DataStore.h"

#include <boost/foreach.hpp>
#include <boost/system/error_code.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <iostream>
#include <fstream>
#include <cstring>

using namespace CauldronIO;

bool ImportExport::exportToXML(boost::shared_ptr<Project>& project, const std::string& absPath, const std::string& relPath, bool release)
{
    // Create empty property tree object
    using boost::property_tree::ptree;
    namespace fs = boost::filesystem;
    fs::path outputPath(absPath);
    outputPath.append(relPath);

    // Create output directory if not existing
    if (!boost::filesystem::exists(outputPath))
    {
        boost::system::error_code err;
        boost::filesystem::create_directory(outputPath, err);
        if (err != boost::system::errc::success) throw CauldronIOException("cannot create path");
    }

    ptree pt;

    ImportExport newExport(absPath, relPath);

    // Create xml property tree and write datastores
    newExport.addProject(pt, project, release);

    // Write property tree to XML file
    fs::path xmlFileName(absPath);
    xmlFileName.append(getXMLIndexingFileName());
    write_xml(xmlFileName.string(), pt);

    return true;
}

void ImportExport::addProject(boost::property_tree::ptree& pt, boost::shared_ptr<Project>& project, bool release)
{
    m_release = release;
    
    // Create empty property tree object
    using boost::property_tree::ptree;

    boost::filesystem::path fullPath(m_absPath);
    fullPath.append(m_relPath.string());

    // Add general project description
    pt.put("project.name", project->getName());
    pt.put("project.description", project->getDescription());
    pt.put("project.modelingmode", project->getModelingMode());
    pt.put("project.team", project->getTeam());
    pt.put("project.programversion", project->getProgramVersion());
    pt.put("project.outputpath", m_relPath.string());
    pt.put("project.xml-version", (float)xml_version);

    // Write all formations
    ptree& formationNode = pt.add("project.formations","");
    BOOST_FOREACH(const boost::shared_ptr<const Formation>& formation, project->getFormations())
    {
        addFormation(formationNode, formation);
    }

    // Write all properties
    ptree& propertyNode = pt.add("project.properties", "");
    BOOST_FOREACH(const boost::shared_ptr<const Property>& property, project->getProperties())
    {
        addProperty(propertyNode, property);
    }

    // Write all reservoirs
    BOOST_FOREACH(const boost::shared_ptr<const Reservoir>& reservoir, project->getReservoirs())
    {
        ptree& reservoirNode = pt.add("project.reservoirs.reservoir", "");
        reservoirNode.put("<xmlattr>.name", reservoir->getName());
        reservoirNode.put("<xmlattr>.formation", reservoir->getFormation()->getName());
    }

    // Write all snapshots
    const SnapShotList snapShotList = project->getSnapShots();
    m_append = detectAppend(project);

    BOOST_FOREACH(const boost::shared_ptr<const SnapShot>& snapShot, snapShotList)
    {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(6);
        ss << snapShot->getAge();
        std::string snapshotString = ss.str();
        std::cout << "Writing snapshot Age=" << snapshotString << std::endl;
        
        boost::filesystem::path volumeStorePath(fullPath);
        volumeStorePath /= "Snapshot_" + snapshotString + "_volumes.cldrn";
        DataStoreSave volumeStore(volumeStorePath.string(), m_append);

        boost::filesystem::path surfaceStorePath(fullPath);
        surfaceStorePath /= "Snapshot_" + snapshotString + "_surfaces.cldrn";
        DataStoreSave surfaceDataStore(surfaceStorePath.string(), m_append);

        ptree& node = pt.add("project.snapshots.snapshot", "");
        node.put("<xmlattr>.age", snapShot->getAge());
        node.put("<xmlattr>.kind", snapShot->getKind());
        node.put("<xmlattr>.isminor", snapShot->isMinorShapshot());

        const SurfaceList surfaces = snapShot->getSurfaceList();
        BOOST_FOREACH(const boost::shared_ptr<Surface>& surfaceIO, surfaces)
        {
            // General properties
            ptree& surfaceNode = node.add("surfaces.surface", "");

            // Data storage
            addSurface(surfaceDataStore, surfaceIO, surfaceNode);
        }

        // Add the continuous volume
        const boost::shared_ptr<Volume> volume = snapShot->getVolume();
        if (volume)
        {
            ptree& volNode = node.add("volume", "");
            addVolume(volumeStore, volume, volNode);
        }

        // Add a volume per formation, with discontinuous properties
        const FormationVolumeList formVolumes = snapShot->getFormationVolumeList();
        BOOST_FOREACH(const boost::shared_ptr<FormationVolume>& formVolume, formVolumes)
        {
            // General properties
            ptree& volNode = node.add("formvols.formvol", "");

            const boost::shared_ptr<Volume> subVolume = formVolume->second;
            const boost::shared_ptr<const Formation> subFormation = formVolume->first;
                
            // Add formation name
            volNode.put("<xmlattr>.formation", subFormation->getName());

            // Add volume 
            ptree& subvolNode = volNode.add("volume", "");
            addVolume(volumeStore, subVolume, subvolNode);
        }

        const TrapperList trappers = snapShot->getTrapperList();
        int maxPersistentTrapperID = -1;
        BOOST_FOREACH(const boost::shared_ptr<const Trapper>& trapper, trappers)
        {
            // General properties
            ptree& trapperNode = node.add("trappers.trapper", "");

            trapperNode.put("<xmlattr>.id", trapper->getID());
            trapperNode.put("<xmlattr>.persistentID", trapper->getPersistentID());
            trapperNode.put("<xmlattr>.reservoirname", trapper->getReservoirName());
            trapperNode.put("<xmlattr>.depth", trapper->getDepth());
            trapperNode.put("<xmlattr>.spillDepth", trapper->getSpillDepth());

            float x, y;
            trapper->getPosition(x, y);
            trapperNode.put("<xmlattr>.posX", x);
            trapperNode.put("<xmlattr>.posY", y);

            trapper->getSpillPointPosition(x, y);
            trapperNode.put("<xmlattr>.spillPosX", x);
            trapperNode.put("<xmlattr>.spillPosY", y);

            int downstreamTrapperID = trapper->getDownStreamTrapperID();
            trapperNode.put("<xmlattr>.downstreamtrapper", downstreamTrapperID);
            maxPersistentTrapperID = std::max(maxPersistentTrapperID, trapper->getPersistentID());
        }
        
        if (maxPersistentTrapperID != -1)
            node.add("trappers.maxPersistentTrapperID", maxPersistentTrapperID);
    }
}

CauldronIO::ImportExport::ImportExport(const boost::filesystem::path& absPath, const boost::filesystem::path& relPath)
{
    m_absPath = absPath;
    m_relPath = relPath;
}

void CauldronIO::ImportExport::addProperty(boost::property_tree::ptree &node, const boost::shared_ptr<const Property>& property) const
{
    boost::property_tree::ptree& propNode = node.add("property","");
    propNode.put("<xmlattr>.name", property->getName());
    propNode.put("<xmlattr>.cauldronname", property->getCauldronName());
    propNode.put("<xmlattr>.username", property->getUserName());
    propNode.put("<xmlattr>.unit", property->getUnit());
    propNode.put("<xmlattr>.attribute", property->getAttribute());
    propNode.put("<xmlattr>.type", property->getType());
}

void CauldronIO::ImportExport::addFormation(boost::property_tree::ptree& node, const boost::shared_ptr<const Formation>& formation) const
{
    boost::property_tree::ptree& subNode = node.add("formation", "");
    subNode.put("<xmlattr>.name", formation->getName());
    unsigned int start, end;
    formation->getK_Range(start, end);
    subNode.put("<xmlattr>.kstart", start);
    subNode.put("<xmlattr>.kend", end);
    subNode.put("<xmlattr>.isSR", formation->isSourceRock());
    subNode.put("<xmlattr>.isML", formation->isMobileLayer());
}

void CauldronIO::ImportExport::addSurface(DataStoreSave& dataStore, const boost::shared_ptr<Surface>& surfaceIO, boost::property_tree::ptree& ptree)
{
    // Retrieve data if necessary: if the getDataStoreParams is unequal to zero this means data is saved and does not need to be saved again
    if (!surfaceIO->isRetrieved() && !m_append)
        surfaceIO->retrieve();

    // Write general info
    ptree.put("<xmlattr>.name", surfaceIO->getName());
    if (surfaceIO->getReservoir())
        ptree.put("<xmlattr>.reservoir", surfaceIO->getReservoir()->getName());
    ptree.put("<xmlattr>.subsurfacekind", surfaceIO->getSubSurfaceKind());
    if (surfaceIO->getFormation())
        ptree.put("<xmlattr>.formation", surfaceIO->getFormation()->getName());

    // Write geometry
    addGeometryInfo2D(ptree, surfaceIO->getGeometry());
    size_t size = surfaceIO->getGeometry()->getNumI()*surfaceIO->getGeometry()->getNumJ();

    // Iterate over all contained valuemaps
    const PropertySurfaceDataList valueMaps = surfaceIO->getPropertySurfaceDataList();

    BOOST_FOREACH(boost::shared_ptr<PropertySurfaceData> propertySurfaceData, valueMaps)
    {
        boost::property_tree::ptree& node = ptree.add("propertymaps.propertymap", "");
        node.put("<xmlattr>.property", propertySurfaceData->first->getName());

        boost::shared_ptr<SurfaceData>& surfaceData = propertySurfaceData->second;

        if (surfaceData->isConstant())
            node.put("<xmlattr>.constantvalue", surfaceData->getConstantValue());
        else
            dataStore.addSurface(surfaceData, node, size);
    }

    if (m_release)
        surfaceIO->release();

}

void CauldronIO::ImportExport::addVolume(DataStoreSave& dataStore, const boost::shared_ptr<Volume>& volume, boost::property_tree::ptree& volNode)
{
    if (!volume->isRetrieved() && !m_append)
        volume->retrieve();

    volNode.put("<xmlattr>.subsurfacekind", volume->getSubSurfaceKind());

    // Set geometry
    const boost::shared_ptr<const Geometry3D>& geometry = volume->getGeometry();
    addGeometryInfo3D(volNode, geometry);
    size_t numBytes = geometry->getNumI()*geometry->getNumJ()*geometry->getNumK()*sizeof(float);

    BOOST_FOREACH(const boost::shared_ptr<PropertyVolumeData>& propVolume, volume->getPropertyVolumeDataList())
    {
        const boost::shared_ptr<const Property>& prop = propVolume->first;
        const boost::shared_ptr<VolumeData>& data = propVolume->second;

        boost::property_tree::ptree& node = volNode.add("propertyvols.propertyvol", "");
        node.put("<xmlattr>.property", prop->getName());

        if (data->isConstant())
        {
            node.put("<xmlattr>.constantvalue", data->getConstantValue());
            continue;
        }

        dataStore.addVolume(data, node, numBytes);
    }

    if (m_release)
        volume->release();
}

void CauldronIO::ImportExport::addGeometryInfo2D(boost::property_tree::ptree& node, const boost::shared_ptr<const Geometry2D>& geometry) const
{
    boost::property_tree::ptree& subNode = node.add("geometry", "");
    subNode.put("<xmlattr>.numI", geometry->getNumI());
    subNode.put("<xmlattr>.numJ", geometry->getNumJ());
    subNode.put("<xmlattr>.minI", geometry->getMinI());
    subNode.put("<xmlattr>.minJ", geometry->getMinJ());
    subNode.put("<xmlattr>.deltaI", geometry->getDeltaI());
    subNode.put("<xmlattr>.deltaJ", geometry->getDeltaJ());
}

void CauldronIO::ImportExport::addGeometryInfo3D(boost::property_tree::ptree& tree, const boost::shared_ptr<const Geometry3D>& geometry) const
{
    boost::property_tree::ptree& subNode = tree.add("geometry", "");

    subNode.put("<xmlattr>.numI", geometry->getNumI());
    subNode.put("<xmlattr>.numJ", geometry->getNumJ());
    subNode.put("<xmlattr>.minI", geometry->getMinI());
    subNode.put("<xmlattr>.minJ", geometry->getMinJ());
    subNode.put("<xmlattr>.deltaI", geometry->getDeltaI());
    subNode.put("<xmlattr>.deltaJ", geometry->getDeltaJ());
    subNode.put("<xmlattr>.numK", geometry->getNumK());
    subNode.put("<xmlattr>.firstK", geometry->getFirstK());
}

// If the objects are 'native' implementation, we should append the output files, otherwise
// we should start from scratch
bool CauldronIO::ImportExport::detectAppend(boost::shared_ptr<Project>& project)
{
    const SnapShotList snapShotList = project->getSnapShots();
    BOOST_FOREACH(boost::shared_ptr<const SnapShot> snapShot, snapShotList)
    {
        const SurfaceList surfaces = snapShot->getSurfaceList();
        BOOST_FOREACH(const boost::shared_ptr<Surface>& surfaceIO, surfaces)
        {
            if (dynamic_cast<MapNative*>(surfaceIO->getPropertySurfaceDataList().at(0)->second.get()) != NULL) return true;
            return false;
        }

        const boost::shared_ptr<Volume>& volume = snapShot->getVolume();
        BOOST_FOREACH(const boost::shared_ptr<PropertyVolumeData>& volumeData, volume->getPropertyVolumeDataList())
        {
            if (dynamic_cast<VolumeDataNative*>(volumeData->second.get()) != NULL) return true;
            return false;
        }
    }

    // This should not happen
    return false;
}

//////////////////////////////////////////////////////////////////////////
/// Importing from native format
//////////////////////////////////////////////////////////////////////////

boost::shared_ptr<Project> CauldronIO::ImportExport::importFromXML(const std::string& filename)
{
    if (!boost::filesystem::exists(filename))
        throw CauldronIOException("Cannot open file");
    
    using boost::property_tree::ptree;
    ptree pt;

    std::cout << "Reading XML into boost property tree" << std::endl;

    try
    {
        read_xml(filename, pt);
    }
    catch (boost::property_tree::xml_parser::xml_parser_error e)
    {
        throw CauldronIOException("Error during parsing xml file");
    }

    boost::filesystem::path path(filename);
    path.remove_filename();
    
    ImportExport importExport(path, boost::filesystem::path(""));
    boost::shared_ptr<Project> project;

    try
    {
        project = importExport.getProject(pt);
    }
    catch (boost::property_tree::ptree_bad_path e)
    {
        throw CauldronIOException("error during xml parse");
    }
    
    return project;
}

std::string CauldronIO::ImportExport::getXMLIndexingFileName()
{
    boost::filesystem::path result = "cauldron_outputs.xml";
    return std::string(result.string());
}


boost::shared_ptr<Project> CauldronIO::ImportExport::getProject(const boost::property_tree::ptree& pt)
{
    std::string projectName = pt.get<std::string>("project.name");
    std::string projectDescript = pt.get<std::string>("project.description");
    std::string projectTeam = pt.get<std::string>("project.team");
    ModellingMode mode = (ModellingMode)pt.get<int>("project.modelingmode");
    std::string projectVersion = pt.get<std::string>("project.programversion");
    std::string outputPath = pt.get<std::string>("project.outputpath");

    // Check XML versions
    // dataXmlVersion > xml_version : actual data generated with code newer than this code, requires forward compatibility
    // dataXmlVersion < xml_version : actual data generated with code older than this code, requires backward compatibility
    //////////////////////////////////////////////////////////////////////////

    float dataXmlVersion = 0;
    boost::optional<float> xmlOptional = pt.get<float>("project.xml-version");  // xml version at moment of data creation
    if (xmlOptional) dataXmlVersion = *xmlOptional;

    float eps = 0.0001f;
    bool equalVersions = abs(dataXmlVersion - xml_version) < eps;
    bool forwardCompatible = (dataXmlVersion - xml_version) > eps;
    bool backwardCompatible = (xml_version - dataXmlVersion) > eps;

    if (forwardCompatible)
        throw CauldronIOException("Xml format not forward compatible");
    if (backwardCompatible)
        throw CauldronIOException("Xml format not backward compatible"); // we should try to fix that when the time comes

    boost::filesystem::path fullOutputPath(m_absPath);
    fullOutputPath /= outputPath;

    // Create the project
    m_project.reset(new Project(projectName, projectDescript, projectTeam, projectVersion, mode, dataXmlVersion));

    // Read all formations
    BOOST_FOREACH(boost::property_tree::ptree::value_type const& formationNodes, pt.get_child("project.formations"))
    {
        assert(formationNodes.first == "formation");
        const boost::property_tree::ptree& formationNode = formationNodes.second;
        boost::shared_ptr<const Formation> formation = getFormation(formationNode);
        m_project->addFormation(formation);
    }

    // Read all properties
    BOOST_FOREACH(boost::property_tree::ptree::value_type const& propertyNodes, pt.get_child("project.properties"))
    {
        assert(propertyNodes.first == "property");
        const boost::property_tree::ptree& propertyNode = propertyNodes.second;
        boost::shared_ptr<const Property> property = getProperty(propertyNode);
        m_project->addProperty(property);
    }

    // Read all reservoirs
    boost::optional<boost::property_tree::ptree const&> hasReservoirs = pt.get_child_optional("project.reservoirs");
    if (hasReservoirs)
    {
        BOOST_FOREACH(boost::property_tree::ptree::value_type const& reservoirNodes, *hasReservoirs)
        {
            assert(reservoirNodes.first == "reservoir");
            const boost::property_tree::ptree& reservoirNode = reservoirNodes.second;
            boost::shared_ptr<const Reservoir> reservoir = getReservoir(reservoirNode);
            m_project->addReservoir(reservoir);
        }
    }
    
    // Read all snapshots
    BOOST_FOREACH(boost::property_tree::ptree::value_type const& snapShotNodes, pt.get_child("project.snapshots"))
    {
        assert(snapShotNodes.first == "snapshot");
        const boost::property_tree::ptree& snapShotNode = snapShotNodes.second;

        double age = snapShotNode.get<double>("<xmlattr>.age");
        SnapShotKind kind = (SnapShotKind)snapShotNode.get<int>("<xmlattr>.kind");
        bool isminor = snapShotNode.get<bool>("<xmlattr>.isminor");

        // Create the snapshot
        boost::shared_ptr<SnapShot> snapShot(new SnapShot(age, kind, isminor));

        boost::optional<boost::property_tree::ptree const&> hasSurfaces = snapShotNode.get_child_optional("surfaces");
        if (hasSurfaces)
        {
            // Find all surfaces
            /////////////////////////////////////////
            BOOST_FOREACH(boost::property_tree::ptree::value_type const& surfaceNodes, snapShotNode.get_child("surfaces"))
            {
                assert(surfaceNodes.first == "surface");
                const boost::property_tree::ptree& surfaceNode = surfaceNodes.second;

                // Read some xml attributes
                std::string surfaceName = surfaceNode.get<std::string>("<xmlattr>.name");
                SubsurfaceKind surfaceKind = (SubsurfaceKind)surfaceNode.get<int>("<xmlattr>.subsurfacekind");
                
                boost::optional<std::string> formationName = surfaceNode.get_optional<std::string>("<xmlattr>.formation");
                boost::shared_ptr<const Formation> formationIO;
                if (formationName)
                    formationIO = m_project->findFormation(*formationName);

                // Find the reservoir object, if name is present
                boost::optional<std::string> reservoirName = surfaceNode.get_optional<std::string>("<xmlattr>.reservoirName");
                boost::shared_ptr<const Reservoir> reservoirIO;
                if (reservoirName)
                {
                    reservoirIO = m_project->findReservoir(*reservoirName);
                    assert(reservoirIO);
                    assert(formationIO);
                    assert(reservoirIO->getFormation() == formationIO);
                }

                // Get geometry
                boost::shared_ptr<const Geometry2D> geometry = getGeometry2D(surfaceNode);

                // Construct the surface
                boost::shared_ptr<Surface> surface(new Surface(surfaceName, surfaceKind, geometry));
                if (formationIO) surface->setFormation(formationIO);
                if (reservoirIO) surface->setReservoir(reservoirIO);

                // Get all property surface data
                BOOST_FOREACH(boost::property_tree::ptree::value_type const& propertyMapNodes, surfaceNode.get_child("propertymaps"))
                {
                    boost::shared_ptr<SurfaceData> surfaceData(new MapNative(geometry));

                    assert(propertyMapNodes.first == "propertymap");
                    const boost::property_tree::ptree& propertyMapNode = propertyMapNodes.second;
                    std::string propertyName = propertyMapNode.get<std::string>("<xmlattr>.property");
                    boost::shared_ptr<const Property> property = m_project->findProperty(propertyName);
                    assert(property);

                    // Get the datastore xml node or constantvalue
                    boost::optional<float> constantVal = propertyMapNode.get_optional<float>("<xmlattr>.constantvalue");
                    if (constantVal)
                        surfaceData->setConstantValue(*constantVal);
                    else
                        DataStoreLoad::getSurface(propertyMapNode, surfaceData, fullOutputPath);

                    boost::shared_ptr<PropertySurfaceData> propSurfaceData(new PropertySurfaceData(property, surfaceData));

                    surface->addPropertySurfaceData(propSurfaceData);
                }

                // Add to snapshot
                snapShot->addSurface(surface);
            }
        }

        // Find all (continuous) volumes
        /////////////////////////////////////////
        boost::optional<boost::property_tree::ptree const&> hasVolumes = snapShotNode.get_child_optional("volume");
        if (hasVolumes)
        {
            const boost::property_tree::ptree& volumeNode = *hasVolumes;
            boost::shared_ptr<Volume> volume = getVolume(volumeNode, fullOutputPath);
            snapShot->setVolume(volume);
        }

        // Get formation volumes
        //////////////////////////////////////////////////////////////////////////
        boost::optional<boost::property_tree::ptree const&> hasFormationVolumes = snapShotNode.get_child_optional("formvols");

        // Get all property volume data
        if (hasFormationVolumes)
        {
            BOOST_FOREACH(boost::property_tree::ptree::value_type const& formVolNodes, *hasFormationVolumes)
            {
                assert(formVolNodes.first == "formvol");
                const boost::property_tree::ptree& formVolNode = formVolNodes.second;

                std::string formationName = formVolNode.get<std::string>("<xmlattr>.formation");
                boost::shared_ptr<const Formation> formationIO = m_project->findFormation(formationName);
                assert(formationIO);

                // There should be a volume
                const boost::property_tree::ptree& volumeNode = formVolNode.get_child("volume");
                boost::shared_ptr<Volume> volume = getVolume(volumeNode, fullOutputPath);

                // Add it to the list
                boost::shared_ptr<FormationVolume> formVolume(new FormationVolume(formationIO, volume));
                snapShot->addFormationVolume(formVolume);
            }
        }

        // Get trappers
        //////////////////////////////////////////////////////////////////////////

        boost::optional<boost::property_tree::ptree const&> hasTrappers = snapShotNode.get_child_optional("trappers");
        if (hasTrappers)
        {
            int maxPersistentTrapperID = snapShotNode.get<int>("trappers.maxPersistentTrapperID");
            assert(maxPersistentTrapperID > -1);
                
            std::vector<boost::shared_ptr<Trapper>* > persistentIDs(maxPersistentTrapperID + 1);

            BOOST_FOREACH(boost::property_tree::ptree::value_type const& trapperNodes, *hasTrappers)
            {
                if (trapperNodes.first == "trapper")
                {
                    const boost::property_tree::ptree& trapperNode = trapperNodes.second;
                    int ID = trapperNode.get<int>("<xmlattr>.id");
                    int persistentID = trapperNode.get<int>("<xmlattr>.persistentID");
                    int downstreamTrapperID = trapperNode.get<int>("<xmlattr>.downstreamtrapper");
                    float depth = trapperNode.get<float>("<xmlattr>.depth");
                    float spillDepth = trapperNode.get<float>("<xmlattr>.spillDepth");
                    std::string reservoirname = trapperNode.get<std::string>("<xmlattr>.reservoirname");
                        
                    float x = trapperNode.get<float>("<xmlattr>.posX");
                    float y = trapperNode.get<float>("<xmlattr>.posY");
                    float spillX = trapperNode.get<float>("<xmlattr>.spillPosX");
                    float spillY = trapperNode.get<float>("<xmlattr>.spillPosY");

                    boost::shared_ptr<Trapper> trapperIO(new Trapper(ID, persistentID));
                    trapperIO->setDownStreamTrapperID(downstreamTrapperID);
                    trapperIO->setReservoirName(reservoirname);
                    trapperIO->setSpillDepth(spillDepth);
                    trapperIO->setSpillPointPosition(spillX, spillY);
                    trapperIO->setDepth(depth);
                    trapperIO->setPosition(x, y);

                    snapShot->addTrapper(trapperIO);

                    assert(persistentID <= maxPersistentTrapperID);
                    persistentIDs[persistentID] = &trapperIO;
                }
            }

            // Assign downstream trappers
            const TrapperList& trappers = snapShot->getTrapperList();
            BOOST_FOREACH(const boost::shared_ptr<const Trapper>& trapper, trappers)
            {
                int downstreamTrapperID = trapper->getDownStreamTrapperID();
                if (downstreamTrapperID > -1)
                {
                    boost::shared_ptr<Trapper> downstreamTrapper = *persistentIDs[downstreamTrapperID];
                    boost::shared_ptr<Trapper> thisTrapper = *persistentIDs[trapper->getPersistentID()];
                    thisTrapper->setDownStreamTrapper(downstreamTrapper);
                }
            }
        }

        m_project->addSnapShot(snapShot);
    }

    return m_project;
}

boost::shared_ptr<const Reservoir> CauldronIO::ImportExport::getReservoir(const boost::property_tree::ptree& reservoirNode) const
{
    boost::shared_ptr<Reservoir> reservoir;

    std::string name = reservoirNode.get<std::string>("<xmlattr>.name");
    std::string formation = reservoirNode.get<std::string>("<xmlattr>.formation");

    boost::shared_ptr<const Formation> formationIO = m_project->findFormation(formation);
    assert(formationIO);

    reservoir.reset(new Reservoir(name, formationIO));
    return reservoir;
}


boost::shared_ptr<const Geometry2D> CauldronIO::ImportExport::getGeometry2D(const boost::property_tree::ptree& surfaceNode) const
{
    BOOST_FOREACH(boost::property_tree::ptree::value_type const& nodes, surfaceNode.get_child(""))
    {
        if (nodes.first == "geometry")
        {
            const boost::property_tree::ptree& geometryNode = nodes.second;
            double minI, minJ, deltaI, deltaJ;
            size_t numI, numJ;

            numI = geometryNode.get<size_t>("<xmlattr>.numI");
            numJ = geometryNode.get<size_t>("<xmlattr>.numJ");
            minI = geometryNode.get<double>("<xmlattr>.minI");
            minJ = geometryNode.get<double>("<xmlattr>.minJ");
            deltaI = geometryNode.get<double>("<xmlattr>.deltaI");
            deltaJ = geometryNode.get<double>("<xmlattr>.deltaJ");

            return boost::shared_ptr<const Geometry2D>(new Geometry2D(numI, numJ, deltaI, deltaJ, minI, minJ));
        }
    }

    throw CauldronIOException("Could not parse geometry");
}

boost::shared_ptr<const Geometry3D> CauldronIO::ImportExport::getGeometry3D(const boost::property_tree::ptree& volumeNode)
{
    BOOST_FOREACH(boost::property_tree::ptree::value_type const& nodes, volumeNode.get_child(""))
    {
        if (nodes.first == "geometry")
        {
            const boost::property_tree::ptree& geometryNode = nodes.second;
            double minI, minJ, deltaI, deltaJ;
            size_t numI, numJ, numK, firstK;

            numI = geometryNode.get<size_t>("<xmlattr>.numI");
            numJ = geometryNode.get<size_t>("<xmlattr>.numJ");
            numK = geometryNode.get<size_t>("<xmlattr>.numK");
            minI = geometryNode.get<double>("<xmlattr>.minI");
            minJ = geometryNode.get<double>("<xmlattr>.minJ");
            deltaI = geometryNode.get<double>("<xmlattr>.deltaI");
            deltaJ = geometryNode.get<double>("<xmlattr>.deltaJ");
            firstK = geometryNode.get<size_t>("<xmlattr>.firstK");

            return boost::shared_ptr<const Geometry3D>(new Geometry3D(numI, numJ, numK, firstK, deltaI, deltaJ, minI, minJ));
        }
    }

    throw CauldronIOException("Could not parse geometry");
}

boost::shared_ptr<Volume> CauldronIO::ImportExport::getVolume(const boost::property_tree::ptree& volumeNode, const boost::filesystem::path& path)
{
    SubsurfaceKind surfaceKind = (SubsurfaceKind)volumeNode.get<int>("<xmlattr>.subsurfacekind");

    // Get geometry
    boost::shared_ptr<const Geometry3D> geometry = getGeometry3D(volumeNode);
    // Create the volume
    boost::shared_ptr<Volume> volume(new Volume(surfaceKind, geometry));

    // Get all property volume data
    BOOST_FOREACH(boost::property_tree::ptree::value_type const& propertyVolNodes, volumeNode.get_child("propertyvols"))
    {
        boost::shared_ptr<VolumeData> volData(new VolumeDataNative(geometry));

        assert(propertyVolNodes.first == "propertyvol");
        const boost::property_tree::ptree& propertyVolNode = propertyVolNodes.second;
        std::string propertyName = propertyVolNode.get<std::string>("<xmlattr>.property");
        boost::shared_ptr<const Property> property = m_project->findProperty(propertyName);
        assert(property);

        // Get the datastore xml node or constantvalue
        boost::optional<float> constantVal = propertyVolNode.get_optional<float>("<xmlattr>.constantvalue");
        if (constantVal)
            volData->setConstantValue(*constantVal);
        else
            DataStoreLoad::getVolume(propertyVolNode, volData, path);

        boost::shared_ptr<PropertyVolumeData> propVolData(new PropertyVolumeData(property, volData));

        volume->addPropertyVolumeData(propVolData);
    }

    return volume;
}

boost::shared_ptr<Property> CauldronIO::ImportExport::getProperty(const boost::property_tree::ptree& propertyNode) const
{
    boost::shared_ptr<Property> property;

    std::string name = propertyNode.get<std::string>("<xmlattr>.name");
    std::string cauldronname = propertyNode.get<std::string>("<xmlattr>.cauldronname");
    std::string username = propertyNode.get<std::string>("<xmlattr>.username");
    std::string unit = propertyNode.get<std::string>("<xmlattr>.unit");
    PropertyType type = (PropertyType)propertyNode.get<int>("<xmlattr>.type");
    PropertyAttribute attrib = (PropertyAttribute)propertyNode.get<int>("<xmlattr>.attribute");

    property.reset(new Property(name, username, cauldronname, unit, type, attrib));
    return property;
}

boost::shared_ptr<Formation> CauldronIO::ImportExport::getFormation(const boost::property_tree::ptree& formationNode) const
{
    boost::shared_ptr<Formation> formation;

    unsigned int start, end;
    std::string name = formationNode.get<std::string>("<xmlattr>.name");
    start = formationNode.get<unsigned int>("<xmlattr>.kstart");
    end = formationNode.get<unsigned int>("<xmlattr>.kend");
    bool isSR = formationNode.get<bool>("<xmlattr>.isSR");
    bool isML = formationNode.get<bool>("<xmlattr>.isML");

    formation.reset(new Formation(start, end, name, isSR, isML));
    return formation;
}