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
#include <boost/system/error_code.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <iostream>
#include <fstream>
#include <cstring>

using namespace CauldronIO;

bool ImportExport::exportToXML(boost::shared_ptr<Project>& project, const std::string& path, bool release)
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
    newExport.addProject(pt, project, release);

    // Write property tree to XML file
    std::string xmlFileName = getXMLIndexingFileName();
    write_xml(xmlFileName, pt);

    return true;
}

void ImportExport::addProject(boost::property_tree::ptree& pt, boost::shared_ptr<Project>& project, bool release)
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

    bool append = detectAppend(project);

    BOOST_FOREACH(boost::shared_ptr<const SnapShot> snapShot, snapShotList)
    {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(6);
        ss << snapShot->getAge();
        std::string snapshotString = ss.str();
        std::cout << "Writing snapshot Age=" << snapshotString << std::endl;
        
        boost::filesystem::path volumeStorePath(m_outputPath);
        volumeStorePath /= "Snapshot_" + snapshotString + "_volumes.cldrn";
        DataStoreSave volumeStore(volumeStorePath.string(), append, release);

        boost::filesystem::path surfaceStorePath(m_outputPath);
        surfaceStorePath /= "Snapshot_" + snapshotString + "_surfaces.cldrn";
        DataStoreSave surfaceDataStore(surfaceStorePath.string(), append, release);

        ptree & node = pt.add("project.snapshots.snapshot", "");
        node.put("<xmlattr>.age", snapShot->getAge());
        node.put("<xmlattr>.kind", snapShot->getKind());
        node.put("<xmlattr>.isminor", snapShot->isMinorShapshot());

        const SurfaceList surfaces = snapShot->getSurfaceList();
        BOOST_FOREACH(const boost::shared_ptr<Surface>& surfaceIO, surfaces)
        {
            // General properties
            ptree& surfaceNode = node.add("surfaces.surface", "");

            // Data storage
            surfaceDataStore.addSurface(surfaceIO, surfaceNode);

            // Set property
            addProperty(surfaceNode, surfaceIO->getProperty());

            // Set formation
            boost::shared_ptr<const Formation> formation = surfaceIO->getFormation();
            if (formation)
                addFormation(surfaceNode, formation);
        }

        const VolumeList volumes = snapShot->getVolumeList();
        BOOST_FOREACH(const boost::shared_ptr<Volume>& volume, volumes)
        {
            // Add volume
            ptree& volNode = node.add("volumes.volume", "");
            volumeStore.addVolume(volume, volNode);
            addProperty(volNode, volume->getProperty());
        }

        const DiscontinuousVolumeList discVolumes = snapShot->getDiscontinuousVolumeList();
        BOOST_FOREACH(const boost::shared_ptr<const DiscontinuousVolume>& volume, discVolumes)
        {
            // General properties
            ptree& volNode = node.add("aggregate-volumes.aggregate-volume", "");

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
                volumeStore.addVolume(subVolume, subvolNode);
                addProperty(subvolNode, subVolume->getProperty());
            }
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
    unsigned int start, end;
    formation->getK_Range(start, end);
    subNode.put("<xmlattr>.kstart", start);
    subNode.put("<xmlattr>.kend", end);
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

    ImportExport importExport;
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

boost::shared_ptr<Project> CauldronIO::ImportExport::getProject(const boost::property_tree::ptree& pt) const
{
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

            boost::optional<boost::property_tree::ptree const&> hasSurfaces = snapShotNode.get_child_optional("surfaces");
            if (hasSurfaces)
            {
                // Find all surfaces
                /////////////////////////////////////////
                BOOST_FOREACH(boost::property_tree::ptree::value_type const& surfaceNodes, snapShotNode.get_child("surfaces"))
                {
                    if (surfaceNodes.first == "surface")
                    {
                        const boost::property_tree::ptree& surfaceNode = surfaceNodes.second;
                        
                        boost::shared_ptr<Property> property = getProperty(surfaceNode);
                        boost::shared_ptr<Surface> surface = DataStoreLoad::getSurface(surfaceNode, property);

                        // Set formation
                        boost::shared_ptr<const Formation> formation = getFormation(surfaceNode);
                        if (formation)
                            surface->setFormation(formation);

                        // Add to snapshot
                        snapShot->addSurface(surface);
                    }
                }

                // Cross-reference depth surfaces
                BOOST_FOREACH(const boost::shared_ptr<Surface>& surface, snapShot->getSurfaceList())
                {
                    MapNative* mapNative = dynamic_cast<MapNative*>(surface->getValueMap().get());
                    if (mapNative->hasDepthMap())
                    {
                        const boost::uuids::uuid& depthMapUUID = mapNative->getDepthSurfaceUUID();

                        BOOST_FOREACH(const boost::shared_ptr<const Surface>& depthSurface, snapShot->getSurfaceList())
                        {
                            if (depthSurface->getValueMap()->getUUID() == depthMapUUID)
                            {
                                surface->setDepthSurface(depthSurface);
                                break;
                            }
                        }
                    }
                }
            }

            // Find all (continuous) volumes
            /////////////////////////////////////////

            boost::optional<boost::property_tree::ptree const&> hasVolumes = snapShotNode.get_child_optional("volumes");
            if (hasVolumes)
            {
                BOOST_FOREACH(boost::property_tree::ptree::value_type const& volumeNodes, snapShotNode.get_child("volumes"))
                {
                    if (volumeNodes.first == "volume")
                    {
                        const boost::property_tree::ptree& volumeNode = volumeNodes.second;
                        boost::shared_ptr<Property> property = getProperty(volumeNode);
                        boost::shared_ptr<Volume> volume = DataStoreLoad::getVolume(volumeNode, property);
                        snapShot->addVolume(volume);
                    }
                }

                // Cross-reference depth volumes
                BOOST_FOREACH(const boost::shared_ptr<Volume>& volume, snapShot->getVolumeList())
                {
                    VolumeNative* volumeNative = dynamic_cast<VolumeNative*>(volume.get());
                    if (volumeNative->hasDepthMap())
                    {
                        const boost::uuids::uuid& depthMapUUID = volumeNative->getDepthSurfaceUUID();

                        BOOST_FOREACH(const boost::shared_ptr<Volume>& depthVolume, snapShot->getVolumeList())
                        {
                            if (depthVolume->getUUID() == depthMapUUID)
                            {
                                volume->setDepthVolume(depthVolume);
                                break;
                            }
                        }
                    }
                }
            }

            // Find all discontinuous) volumes
            /////////////////////////////////////////

            boost::optional<boost::property_tree::ptree const&> hasDCVolumes = snapShotNode.get_child_optional("aggregate-volumes");
            if (hasDCVolumes)
            {
                BOOST_FOREACH(boost::property_tree::ptree::value_type const& volumeNodes, snapShotNode.get_child("aggregate-volumes"))
                {
                    if (volumeNodes.first == "aggregate-volume")
                    {
                        const boost::property_tree::ptree& volumeNode = volumeNodes.second;
                        boost::shared_ptr<DiscontinuousVolume> discVolume(new DiscontinuousVolumeNative());

                        boost::optional<boost::uuids::uuid> depthSurfaceUUID = volumeNode.get_optional<boost::uuids::uuid>("depthvolume-uuid");
                        if (depthSurfaceUUID)
                        {
                            DiscontinuousVolumeNative* volumeNative = static_cast<DiscontinuousVolumeNative*>(discVolume.get());
                            volumeNative->setDepthSurfaceUUID(*depthSurfaceUUID);
                        }

                        // Get all formation-volumes
                        BOOST_FOREACH(boost::property_tree::ptree::value_type const& formationVolumeNodes, volumeNode.get_child("formation-volumes"))
                        {
                            if (formationVolumeNodes.first == "formation-volume")
                            {
                                const boost::property_tree::ptree& formationVolumeNode = formationVolumeNodes.second;
                                boost::shared_ptr<Formation> formation = getFormation(formationVolumeNode);
                                boost::shared_ptr<Volume> volume;

                                // Find the volume
                                BOOST_FOREACH(boost::property_tree::ptree::value_type const& nodes, formationVolumeNode.get_child(""))
                                {
                                    if (nodes.first == "volume") 
                                    {
                                        boost::shared_ptr<Property> property = getProperty(nodes.second);
                                        volume = DataStoreLoad::getVolume(nodes.second, property);
                                        break;
                                    }
                                }
                                assert(volume);
                                
                                discVolume->addVolume(formation, volume);
                            }
                        }
                        
                        snapShot->addDiscontinuousVolume(discVolume);
                    }
                }

                // Cross-reference depth volumes
                BOOST_FOREACH(const boost::shared_ptr<DiscontinuousVolume>& discVolume, snapShot->getDiscontinuousVolumeList())
                {
                    DiscontinuousVolumeNative* volumeNative = static_cast<DiscontinuousVolumeNative*>(discVolume.get());
                    if (volumeNative->hasDepthMap())
                    {
                        const boost::uuids::uuid& depthMapUUID = volumeNative->getDepthSurfaceUUID();

                        BOOST_FOREACH(const boost::shared_ptr<Volume>& depthVolume, snapShot->getVolumeList())
                        {
                            if (depthVolume->getUUID() == depthMapUUID)
                            {
                                discVolume->setDepthVolume(depthVolume);
                                break;
                            }
                        }
                    }
                }
            }

            boost::optional<boost::property_tree::ptree const&> hasTrappers = snapShotNode.get_child_optional("trappers");
            if (hasTrappers)
            {
                int maxPersistentTrapperID = snapShotNode.get<int>("trappers.maxPersistentTrapperID");
                assert(maxPersistentTrapperID > -1);
                
                std::vector<boost::shared_ptr<Trapper>* > persistentIDs(maxPersistentTrapperID + 1);

                BOOST_FOREACH(boost::property_tree::ptree::value_type const& trapperNodes, snapShotNode.get_child("trappers"))
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

            project->addSnapShot(snapShot);
        }
    }

    return project;
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
            if (dynamic_cast<MapNative*>(surfaceIO->getValueMap().get()) != NULL) return true;
            return false;
        }

        const VolumeList volumes = snapShot->getVolumeList();
        BOOST_FOREACH(const boost::shared_ptr<Volume>& volume, volumes)
        {
            if (dynamic_cast<VolumeNative*>(volume.get()) != NULL) return true;
            return false;
        }
    }

    // This should not happen
    return false;
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

boost::shared_ptr<Formation> CauldronIO::ImportExport::getFormation(const boost::property_tree::ptree& surfaceNode) const
{
    boost::shared_ptr<Formation> formation;
    BOOST_FOREACH(boost::property_tree::ptree::value_type const& nodes, surfaceNode.get_child(""))
    {
        if (nodes.first == "formation")
        {
            const boost::property_tree::ptree& formationNode = nodes.second;
            unsigned int start, end;
            std::string name = formationNode.get<std::string>("<xmlattr>.name");
            start = formationNode.get<unsigned int>("<xmlattr>.kstart");
            end = formationNode.get<unsigned int>("<xmlattr>.kend");

            formation.reset(new Formation(start, end, name));
            return formation;
        }
    }

    return formation;
}