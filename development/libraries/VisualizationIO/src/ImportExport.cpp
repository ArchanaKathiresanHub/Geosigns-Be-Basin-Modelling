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
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/system/error_code.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <iostream>
#include <fstream>
#include <cstring>

#define __compress__ true

using namespace CauldronIO;

bool ImportExport::exportToXML(boost::shared_ptr<Project>& project, const std::string& path)
{
    // Create empty property tree object
    using boost::property_tree::ptree;
    namespace fs = boost::filesystem;
    fs::path outputPath(path);

    // Create output directory if not existing
    if (!boost::filesystem::exists(outputPath))
    {
        boost::system::error_code err;
        boost::filesystem::create_directory(outputPath, err);
        if (err != boost::system::errc::success) throw CauldronIOException("cannot create path");
    }

    ptree pt;

    ImportExport newExport(outputPath);

    // Create xml property tree and write datastores
    newExport.addProject(pt, project);

    // Write property tree to XML file
    std::string xmlFileName = getXMLIndexingFileName(outputPath);
    write_xml(xmlFileName, pt);

    return true;
}

void ImportExport::addProject(boost::property_tree::ptree& pt, boost::shared_ptr<Project>& project)
{
    // Create empty property tree object
    using boost::property_tree::ptree;

    // Add general project description
    pt.put("project.name", project->getName());
    pt.put("project.description", project->getDescription());
    pt.put("project.modelingmode", project->getModelingMode());
    pt.put("project.team", project->getTeam());
    pt.put("project.programversion", project->getProgramVersion());

    const SnapShotList snapShotList = project->getSnapShots();

    BOOST_FOREACH(boost::shared_ptr<const SnapShot> snapShot, snapShotList)
    {
        boost::filesystem::path volumeStorePath(m_outputPath);
        volumeStorePath /= "Snapshot_" + boost::lexical_cast<std::string>(snapShot->getAge()) + "_volumes.cldrn";
        DataStore volumeStore(volumeStorePath.string(), __compress__, true);

        boost::filesystem::path surfaceStorePath(m_outputPath);
        surfaceStorePath /= "Snapshot_" + boost::lexical_cast<std::string>(snapShot->getAge()) + "_surfaces.cldrn";
        DataStore surfaceDataStore(surfaceStorePath.string(), __compress__, true);

        ptree & node = pt.add("project.snapshots.snapshot", "");
        node.put("<xmlattr>.age", snapShot->getAge());
        node.put("<xmlattr>.kind", snapShot->getKind());
        node.put("<xmlattr>.isminor", snapShot->isMinorShapshot());

        const SurfaceList surfaces = snapShot->getSurfaceList();
        BOOST_FOREACH(const boost::shared_ptr<Surface>& surfaceIO, surfaces)
        {
            // General properties
            ptree& surface = node.add("surfaces.surface", "");
            surface.put("<xmlattr>.name", surfaceIO->getName());
            surface.put("<xmlattr>.subsurfacekind", surfaceIO->getSubSurfaceKind());
            surface.put("<xmlattr>.uuid", surfaceIO->getValueMap()->getUUID());
            
            // Set geometry
            addGeometryInfo(surface, surfaceIO->getValueMap());

            // Data storage
            if (surfaceIO->getValueMap()->isConstant())
                surface.put("constantvalue", surfaceIO->getValueMap()->getConstantValue());
            else
                surfaceDataStore.addSurface(surfaceIO, surface);

            // Set depth surface
            boost::shared_ptr<const Surface> depthSurface = surfaceIO->getDepthSurface();
            if (depthSurface)
                surface.put("depthsurface-uuid", depthSurface->getValueMap()->getUUID());

            // Set property
            addProperty(surface, surfaceIO->getProperty());

            // Set formation
            boost::shared_ptr<const Formation> formation = surfaceIO->getFormation();
            if (formation)
                addFormation(surface, formation);
        }

        const VolumeList volumes = snapShot->getVolumeList();
        BOOST_FOREACH(const boost::shared_ptr<Volume>& volume, volumes)
        {
            // General properties
            ptree& volNode = node.add("volumes.volume", "");
            volNode.put("<xmlattr>.subsurfacekind", volume->getSubSurfaceKind());
            volNode.put("<xmlattr>.uuid", volume->getUUID());
            volNode.put("<xmlattr>.cell-centered", volume->isCellCentered());

            // Set geometry
            addGeometryInfo(volNode, volume);

            // Data storage
            volumeStore.addVolume(volume, volNode);

            // Set depth volume
            boost::shared_ptr<const Volume> depthVolume = volume->getDepthVolume();
            if (depthVolume)
                volNode.put("depthvolume-uuid", depthVolume->getUUID());

            // Set property
            addProperty(volNode, volume->getProperty());
        }

        const DiscontinuousVolumeList discVolumes = snapShot->getDiscontinuousVolumeList();
        BOOST_FOREACH(const boost::shared_ptr<const DiscontinuousVolume>& volume, discVolumes)
        {
            // General properties
            ptree& volNode = node.add("aggregate-volumes.aggregate-volume", "");
            volNode.put("<xmlattr>.cell-centered", volume->isCellCentered());

            // Set depth volume
            boost::shared_ptr<const Volume> depthVolume = volume->getDepthVolume();
            if (depthVolume)
                volNode.put("depthvolume-uuid", depthVolume->getUUID());

            const FormationVolumeList formationVolumes = volume->getVolumeList();
            BOOST_FOREACH(const boost::shared_ptr<const FormationVolume>& formationVolume, formationVolumes)
            {
                const boost::shared_ptr<Volume> subVolume = formationVolume->second;
                const boost::shared_ptr<const Formation> subFormation = formationVolume->first;
                
                // Add formation 
                ptree& subNode = volNode.add("formation-volumes.formation-volume", "");
                addFormation(subNode, subFormation);

                // Add volume 
                ptree& subvolNode = subNode.add("volume", "");
                subvolNode.put("<xmlattr>.subsurfacekind", subVolume->getSubSurfaceKind());
                subvolNode.put("<xmlattr>.uuid", subVolume->getUUID());
                subvolNode.put("<xmlattr>.cell-centered", subVolume->isCellCentered());
                addGeometryInfo(subvolNode, subVolume);
                volumeStore.addVolume(subVolume, subvolNode);
                addProperty(subvolNode, subVolume->getProperty());
            }
        }
    }
}

std::string CauldronIO::ImportExport::getFilename(const boost::uuids::uuid& uuid) const
{
    return to_string(uuid) + ".cldrn";
}

CauldronIO::ImportExport::ImportExport(const boost::filesystem::path& path)
{
    m_outputPath = path;
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
    size_t start, end;
    formation->getDepthRange(start, end);
    subNode.put("<xmlattr>.kstart", start);
    subNode.put("<xmlattr>.kend", end);
}

void CauldronIO::ImportExport::addGeometryInfo(boost::property_tree::ptree& node, const boost::shared_ptr<const Map>& map) const
{
    boost::property_tree::ptree& subNode = node.add("geometry", "");
    subNode.put("<xmlattr>.numI", map->getNumI());
    subNode.put("<xmlattr>.numJ", map->getNumJ());
    subNode.put("<xmlattr>.minI", map->getMinI());
    subNode.put("<xmlattr>.minJ", map->getMinJ());
    subNode.put("<xmlattr>.maxI", map->getMaxI());
    subNode.put("<xmlattr>.maxJ", map->getMaxJ());
    subNode.put("<xmlattr>.deltaI", map->getDeltaI());
    subNode.put("<xmlattr>.deltaJ", map->getDeltaJ());
    subNode.put("<xmlattr>.undefinedvalue", map->getUndefinedValue());
}

void CauldronIO::ImportExport::addGeometryInfo(boost::property_tree::ptree& tree, const boost::shared_ptr<const Volume>& volume) const
{
    boost::property_tree::ptree& subNode = tree.add("geometry", "");
    subNode.put("<xmlattr>.numI", volume->getNumI());
    subNode.put("<xmlattr>.numJ", volume->getNumJ());
    subNode.put("<xmlattr>.numK", volume->getNumK());
    subNode.put("<xmlattr>.minI", volume->getMinI());
    subNode.put("<xmlattr>.minJ", volume->getMinJ());
    subNode.put("<xmlattr>.maxI", volume->getMaxI());
    subNode.put("<xmlattr>.maxJ", volume->getMaxJ());
    subNode.put("<xmlattr>.firstK", volume->getFirstK());
    subNode.put("<xmlattr>.lastK", volume->getLastK());
    subNode.put("<xmlattr>.deltaI", volume->getDeltaI());
    subNode.put("<xmlattr>.deltaJ", volume->getDeltaJ());
    subNode.put("<xmlattr>.undefinedvalue", volume->getUndefinedValue());
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
    
    try
    {
        read_xml(filename, pt);
    }
    catch (boost::property_tree::xml_parser::xml_parser_error e)
    {
        throw CauldronIOException("Error during parsing xml file");
    }

    ImportExport importExport;
    boost::shared_ptr<Project> project;

    // todo: add a try/catch here and catch the specific exception
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

std::string CauldronIO::ImportExport::getXMLIndexingFileName(const boost::filesystem::path& path)
{
    boost::filesystem::path result = "cauldron_outputs.xml";
    return std::string(result.string());
}

boost::shared_ptr<Project> CauldronIO::ImportExport::getProject(const boost::property_tree::ptree& pt) const
{
    boost::uuids::string_generator gen;
    
    std::string projectName = pt.get<std::string>("project.name");
    std::string projectDescript = pt.get<std::string>("project.description");
    std::string projectTeam = pt.get<std::string>("project.team");
    ModellingMode mode = (ModellingMode)pt.get<int>("project.modelingmode");
    std::string projectVersion = pt.get<std::string>("project.programversion");

    // Create the project
    boost::shared_ptr<Project> project(new Project(projectName, projectDescript, projectTeam, projectVersion, mode));

    BOOST_FOREACH(boost::property_tree::ptree::value_type const& snapShotNodes, pt.get_child("project.snapshots"))
    {
        if (snapShotNodes.first == "snapshot") // there are no other values
        {
            const boost::property_tree::ptree& snapShotNode = snapShotNodes.second;
            double age = snapShotNode.get<double>("<xmlattr>.age");
            SnapShotKind kind = (SnapShotKind)snapShotNode.get<int>("<xmlattr>.kind");
            bool isminor = snapShotNode.get<bool>("<xmlattr>.isminor");

            // Create the snapshot
            boost::shared_ptr<SnapShot> snapShot(new SnapShot(age, kind, isminor));

            // Find all surfaces
            BOOST_FOREACH(boost::property_tree::ptree::value_type const& surfaceNodes, snapShotNode.get_child("surfaces"))
            {
                if (surfaceNodes.first == "surface") 
                {
                    const boost::property_tree::ptree& surfaceNode = surfaceNodes.second;
                    std::string surfaceName = surfaceNode.get<std::string>("<xmlattr>.name");
                    SubsurfaceKind surfaceKind = (SubsurfaceKind)surfaceNode.get<int>("<xmlattr>.subsurfacekind");
                    std::string uuid = surfaceNode.get<std::string>("<xmlattr>.uuid");

                    // Get the property
                    boost::shared_ptr<Property> property = getProperty(surfaceNode);
                    boost::shared_ptr<Map> valueMap = getValueMap(surfaceNode);
                    valueMap->setUUID(gen(uuid));

                    // Check for optional depthSurfaceUUID
                    boost::optional<std::string> depthSurfaceUUID = surfaceNode.get_optional<std::string>("depthsurface-uuid");
                    if (depthSurfaceUUID)
                    {
                        MapNative* mapNative = dynamic_cast<MapNative*>(valueMap.get());
                        mapNative->setDepthSurfaceUUID(gen(*depthSurfaceUUID));
                    }

                    // Create the surface
                    boost::shared_ptr<Surface> surface(new Surface(surfaceName, surfaceKind, property, valueMap));

                    // Set formation
                    boost::shared_ptr<const Formation> formation = getFormation(surfaceNode);
                    if (formation)
                        surface->setFormation(formation);

                    // Add to snapshot
                    snapShot->addSurface(surface);
                }
            }

            // TODO: cross-reference depth surfaces 

            project->addSnapShot(snapShot);
        }
    }

    return project;
}

boost::shared_ptr<Property> CauldronIO::ImportExport::getProperty(const boost::property_tree::ptree& surfaceNode) const
{
    boost::shared_ptr<Property> property;
    BOOST_FOREACH(boost::property_tree::ptree::value_type const& propertyNodes, surfaceNode.get_child(""))
    {
        if (propertyNodes.first == "property")
        {
            // We only need one (and there should only be one)
            const boost::property_tree::ptree& propertyNode = propertyNodes.second;
            std::string name = propertyNode.get<std::string>("<xmlattr>.name");
            std::string cauldronname = propertyNode.get<std::string>("<xmlattr>.cauldronname");
            std::string username = propertyNode.get<std::string>("<xmlattr>.username");
            std::string unit = propertyNode.get<std::string>("<xmlattr>.unit");
            PropertyType type = (PropertyType)propertyNode.get<int>("<xmlattr>.type");
            PropertyAttribute attrib = (PropertyAttribute)propertyNode.get<int>("<xmlattr>.attribute");

            property.reset(new Property(name, username, cauldronname, unit, type, attrib));
            return property;
        }
    }

    return property;
}

boost::shared_ptr<Map> CauldronIO::ImportExport::getValueMap(const boost::property_tree::ptree& surfaceNode) const
{
    // TODO: cell-centered should be read/written to xml, not assumed
    MapNative* mapNative = new MapNative(false);

    boost::shared_ptr<Map> valueMap(mapNative);
    BOOST_FOREACH(boost::property_tree::ptree::value_type const& nodes, surfaceNode.get_child(""))
    {
        if (nodes.first == "geometry")
        {
            const boost::property_tree::ptree& geometryNode = nodes.second;
            double minI, minJ, maxI, maxJ, deltaI, deltaJ;
            float undef;
            size_t numI, numJ;

            numI = geometryNode.get<size_t>("<xmlattr>.numI");
            numJ = geometryNode.get<size_t>("<xmlattr>.numJ");
            minI = geometryNode.get<double>("<xmlattr>.minI");
            minJ = geometryNode.get<double>("<xmlattr>.minJ");
            maxI = geometryNode.get<double>("<xmlattr>.maxI");
            maxJ = geometryNode.get<double>("<xmlattr>.maxJ");
            deltaI = geometryNode.get<double>("<xmlattr>.deltaI");
            deltaJ = geometryNode.get<double>("<xmlattr>.deltaJ");
            undef = geometryNode.get<float>("<xmlattr>.undefinedvalue");

            valueMap->setGeometry(numI, numJ, deltaI, deltaJ, minI, minJ);
            valueMap->setUndefinedValue(undef);
        }
        //////////////////////////////////////////////////////////////////////////////
        /// TODO: move this logic (as well as storing it) to VisualizationIO_native
        //////////////////////////////////////////////////////////////////////////////
        else if (nodes.first == "constantvalue")
        {
            float value = surfaceNode.get<float>("constantvalue");
            valueMap->setConstantValue(value);
        }
        else if (nodes.first == "datastore")
        {
            // Extract some data
            const boost::property_tree::ptree& datastoreNode = nodes.second;
            bool compressed, row_ordered;
            std::string format, conversion, filename;
            size_t offset, size;

            filename = datastoreNode.get<std::string>("<xmlattr>.file");
            compressed = datastoreNode.get<std::string>("<xmlattr>.compression") == "gzip";
            format = datastoreNode.get<std::string>("<xmlattr>.format");
            size = datastoreNode.get<size_t>("<xmlattr>.size");
            offset = datastoreNode.get<size_t>("<xmlattr>.offset");
            row_ordered = datastoreNode.get<std::string>("<xmlattr>.row-ordered") == "yes";

            // TODO: we ignore the format and row_ordered-ness for now
            mapNative->setDataStore(filename, compressed, offset, size);
        }
    }

    return valueMap;
}

boost::shared_ptr<const Formation> CauldronIO::ImportExport::getFormation(const boost::property_tree::ptree& surfaceNode) const
{
    boost::shared_ptr<const Formation> formation;
    BOOST_FOREACH(boost::property_tree::ptree::value_type const& nodes, surfaceNode.get_child(""))
    {
        if (nodes.first == "formation")
        {
            const boost::property_tree::ptree& formationNode = nodes.second;
            size_t start, end;
            std::string name = formationNode.get<std::string>("<xmlattr>.name");
            start = formationNode.get<size_t>("<xmlattr>.kstart");
            end = formationNode.get<size_t>("<xmlattr>.kend");

            formation.reset(new Formation(start, end, name));
            return formation;
        }
    }

    return formation;
}