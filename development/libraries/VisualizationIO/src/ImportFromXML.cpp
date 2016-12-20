//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ImportFromXML.h"
#include "VisualizationIO_native.h"
#include "DataStore.h"
#include "FilePath.h"
#include "FolderPath.h"

#include <boost/foreach.hpp>
#include <boost/thread.hpp>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <cstring>

using namespace CauldronIO;

//////////////////////////////////////////////////////////////////////////
/// Importing from native format
//////////////////////////////////////////////////////////////////////////

std::shared_ptr<Project> CauldronIO::ImportFromXML::importFromXML(const std::string& filename)
{
    if (!ibs::FilePath(filename).exists())
        throw CauldronIOException("Cannot open file");
    
    pugi::xml_document doc;
    std::cout << "Reading XML into pugiXML DOM" << std::endl;
    pugi::xml_parse_result result = doc.load_file(filename.c_str());

    if (!result)
        throw CauldronIOException("Error during parsing xml file");

        
	ImportFromXML importExport(ibs::FilePath(filename).filePath());
    std::shared_ptr<Project> project;
    
    try
    {
        project = importExport.getProject(doc);
    }
    catch (CauldronIOException& excp)
    {
        // Just rethrow it
        throw excp;
    }
    catch (...)
    {
        throw CauldronIOException("unspecified error during xml parse");
    }
    
    return project;
}

CauldronIO::ImportFromXML::ImportFromXML(const ibs::FilePath& absPath)
	: m_absPath(absPath)
{
}

std::shared_ptr<Project> CauldronIO::ImportFromXML::getProject(const pugi::xml_document& ptDoc)
{
    pugi::xml_node pt = ptDoc.child("project");
    if (!pt) 
        throw CauldronIOException("Invalid xml file or format");

    std::string projectName = pt.child_value("name");
    std::string projectDescript = pt.child_value("description");
    std::string projectTeam = pt.child_value("team");
    std::string projectVersion = pt.child_value("programversion");
	std::string outputPath = pt.child_value("outputpath");
    
	ModellingMode mode = (ModellingMode)pt.child("modelingmode").text().as_int();

    // Check XML versions
    // dataXmlVersion > xml_version : actual data generated with code newer than this code, requires forward compatibility
    // dataXmlVersion < xml_version : actual data generated with code older than this code, requires backward compatibility
    //////////////////////////////////////////////////////////////////////////

    int dataXmlVersionMinor = 0;
    int dataXmlVersionMajor = 0;
    pugi::xml_node xmlOptional = pt.child("xml-version");
    if (xmlOptional)
    {
        dataXmlVersionMinor = xmlOptional.attribute("minor").as_int();
        dataXmlVersionMajor = xmlOptional.attribute("major").as_int();
    }

    bool forwardCompatible = dataXmlVersionMajor > xml_version_major || (dataXmlVersionMajor == xml_version_major && dataXmlVersionMinor > xml_version_minor);
    bool backwardCompatible = dataXmlVersionMajor < xml_version_major || (dataXmlVersionMajor == xml_version_major && dataXmlVersionMinor < xml_version_minor);

    if (forwardCompatible)
        throw CauldronIOException("Xml format not forward compatible");
    if (backwardCompatible)
        throw CauldronIOException("Xml format not backward compatible"); // we should try to fix that when the time comes

    ibs::FilePath fullOutputPath(m_absPath);
	if (m_absPath.path() != ".")
	{
		fullOutputPath << outputPath;
	}
	else
	{
		fullOutputPath = ibs::FilePath(outputPath);
	}

    // Create the project
    m_project.reset(new Project(projectName, projectDescript, projectTeam, projectVersion, mode, dataXmlVersionMajor, dataXmlVersionMinor));

    // Read all formations
    pugi::xml_node formationsNode = pt.child("formations");
    for (pugi::xml_node formationNode = formationsNode.child("formation"); formationNode; formationNode = formationNode.next_sibling("formation"))
    {
        std::shared_ptr<const Formation> formation = getFormation(formationNode);
        m_project->addFormation(formation);
    }

    // Read all properties
    pugi::xml_node propertiesNode = pt.child("properties");
    for (pugi::xml_node propertyNode = propertiesNode.child("property"); propertyNode; propertyNode = propertyNode.next_sibling("property"))
    {
        std::shared_ptr<const Property> property = getProperty(propertyNode);
        m_project->addProperty(property);
    }

    // Read all reservoirs
    pugi::xml_node hasReservoirs = pt.child("reservoirs");
    if (hasReservoirs)
    {
        for (pugi::xml_node reservoirNode = hasReservoirs.child("reservoir"); reservoirNode; reservoirNode = reservoirNode.next_sibling("reservoir"))
        {
            std::shared_ptr<const Reservoir> reservoir = getReservoir(reservoirNode);
            m_project->addReservoir(reservoir);
        }
    }

    // Read all geometries
    pugi::xml_node geometriesNode = pt.child("geometries");
    if (!geometriesNode)
    {
        throw CauldronIOException("Cannot find any geometries, please reconvert your project to reflect latest xml format");
    }
    for (pugi::xml_node geometryNode = geometriesNode.child("geometry"); geometryNode; geometryNode = geometryNode.next_sibling("geometry"))
    {
        std::shared_ptr<const Geometry2D> geometry = getGeometry2D(geometryNode);
        m_project->addGeometry(geometry);
    }
    
    // Read all snapshots
    pugi::xml_node snapshotsNode = pt.child("snapshots");
    for (pugi::xml_node snapShotNode = snapshotsNode.child("snapshot"); snapShotNode; snapShotNode = snapShotNode.next_sibling("snapshot"))
    {
        double age = snapShotNode.attribute("age").as_double();
        SnapShotKind kind = (SnapShotKind)snapShotNode.attribute("kind").as_int();
        bool isminor = snapShotNode.attribute("isminor").as_bool();

        // Create the snapshot
        std::shared_ptr<SnapShot> snapShot(new SnapShot(age, kind, isminor));

        // Find all surfaces
        /////////////////////////////////////////
        pugi::xml_node hasSurfaces = snapShotNode.child("surfaces");
        if (hasSurfaces)
        {
            for (pugi::xml_node surfaceNode = hasSurfaces.child("surface"); surfaceNode; surfaceNode = surfaceNode.next_sibling("surface"))
            {
                // Read some xml attributes
                std::string surfaceName = surfaceNode.attribute("name").value();
                SubsurfaceKind surfaceKind = (SubsurfaceKind)surfaceNode.attribute("subsurfacekind").as_int();

                // Find formations, if present
                std::shared_ptr<const Formation> topFormationIO;
                pugi::xml_attribute topFormationName = surfaceNode.attribute("top-formation");
                if (topFormationName)
                    topFormationIO = m_project->findFormation(topFormationName.value());
                std::shared_ptr<const Formation> bottomFormationIO;
                pugi::xml_attribute bottomFormationName = surfaceNode.attribute("bottom-formation");
                if (bottomFormationName)
                    bottomFormationIO = m_project->findFormation(bottomFormationName.value());

                // Construct the surface
                std::shared_ptr<Surface> surface(new Surface(surfaceName, surfaceKind)); 
                if (topFormationIO) surface->setFormation(topFormationIO, true);
                if (bottomFormationIO) surface->setFormation(bottomFormationIO, false);

                // Get all property surface data
                pugi::xml_node propertyMapNodes = surfaceNode.child("propertymaps");
                for (pugi::xml_node propertyMapNode = propertyMapNodes.child("propertymap"); propertyMapNode; propertyMapNode = propertyMapNode.next_sibling("propertymap"))
                {
                    std::string propertyName = propertyMapNode.attribute("property").value();
                    std::shared_ptr<const Property> property = m_project->findProperty(propertyName);
                    assert(property);

                    // Get the geometry
                    size_t geometryIndex = (size_t)propertyMapNode.attribute("geom-index").as_int();
                    std::shared_ptr<const Geometry2D> geometry = m_project->getGeometries().at(geometryIndex);

                    // Get min/max value
                    float minValue = DefaultUndefinedValue;
                    float maxValue = DefaultUndefinedValue;

                    pugi::xml_attribute minValueAttr = propertyMapNode.attribute("min");
                    if (minValueAttr)
                        minValue = minValueAttr.as_float();

                    pugi::xml_attribute maxValueAttr = propertyMapNode.attribute("max");
                    if (maxValueAttr)
                        maxValue = maxValueAttr.as_float();

                    // Create the surface data
                    std::shared_ptr<SurfaceData> surfaceData(new MapNative(geometry, minValue, maxValue));

                    // Contains formation?
                    pugi::xml_attribute formationName = propertyMapNode.attribute("formation");
                    if (formationName)
                    {
                        std::shared_ptr<const Formation> formationIO = m_project->findFormation(formationName.value());
                        surfaceData->setFormation(formationIO);
                    }

                    // Find the reservoir object, if name is present
                    std::shared_ptr<const Reservoir> reservoirIO;
                    pugi::xml_attribute reservoirName = propertyMapNode.attribute("reservoir");
                    if (reservoirName)
                    {
                        reservoirIO = m_project->findReservoir(reservoirName.value());
                        assert(reservoirIO);
                        surfaceData->setReservoir(reservoirIO);
                    }

                    // Get the datastore xml node or constantvalue
                    pugi::xml_attribute constantVal = propertyMapNode.attribute("constantvalue");
                    if (constantVal)
                        surfaceData->setConstantValue(constantVal.as_float());
                    else
                        DataStoreLoad::getSurface(propertyMapNode, surfaceData, fullOutputPath);

                    PropertySurfaceData propSurfaceData(property, surfaceData);
                    surface->addPropertySurfaceData(propSurfaceData);
                }

                // Add to snapshot
                snapShot->addSurface(surface);
            }
        }
        
        // Find all (continuous) volumes
        /////////////////////////////////////////
        pugi::xml_node hasVolumes = snapShotNode.child("volume");
        if (hasVolumes)
        {
            std::shared_ptr<Volume> volume = getVolume(hasVolumes, fullOutputPath);
            snapShot->setVolume(volume);
        }

        // Get formation volumes
        //////////////////////////////////////////////////////////////////////////
        pugi::xml_node hasFormationVolumes = snapShotNode.child("formvols");

        // Get all property volume data
        if (hasFormationVolumes)
        {
            for (pugi::xml_node formVolNode = hasFormationVolumes.child("formvol"); formVolNode; formVolNode = formVolNode.next_sibling("formvol"))
            {
                std::string formationName = formVolNode.attribute("formation").value();
                std::shared_ptr<const Formation> formationIO = m_project->findFormation(formationName);
                assert(formationIO);

                // There should be a volume
                pugi::xml_node volumeNode = formVolNode.child("volume");
                std::shared_ptr<Volume> volume = getVolume(volumeNode, fullOutputPath);

                // Add it to the list
                FormationVolume formVolume(formationIO, volume);
                snapShot->addFormationVolume(formVolume);
            }
        }

        // Get trappers
        //////////////////////////////////////////////////////////////////////////

        pugi::xml_node hasTrappers = snapShotNode.child("trappers");
        if (hasTrappers)
        {
            int maxPersistentTrapperID = hasTrappers.child("maxPersistentTrapperID").text().as_int();
            assert(maxPersistentTrapperID > -1);
                
            std::vector<std::shared_ptr<Trapper>* > persistentIDs(maxPersistentTrapperID + 1);

            for (pugi::xml_node trapperNode = hasTrappers.child("trapper"); trapperNode; trapperNode = trapperNode.next_sibling("trapper"))
            {
                int ID = trapperNode.attribute("id").as_int();
                int persistentID = trapperNode.attribute("persistentID").as_int(); 
                int downstreamTrapperID = trapperNode.attribute("downstreamtrapper").as_int(); 
                float depth = trapperNode.attribute("depth").as_float();
                float spillDepth = trapperNode.attribute("spillDepth").as_float(); 
                std::string reservoirname = trapperNode.attribute("reservoirname").value();
                        
                float x      = trapperNode.attribute("posX").as_float();
                float y      = trapperNode.attribute("posY").as_float();
                float spillX = trapperNode.attribute("spillPosX").as_float();
                float spillY = trapperNode.attribute("spillPosY").as_float();
                float goc    = trapperNode.attribute("goc").as_float();
                float owc    = trapperNode.attribute("owc").as_float();

                std::shared_ptr<Trapper> trapperIO(new Trapper(ID, persistentID));
                trapperIO->setDownStreamTrapperID(downstreamTrapperID);
                trapperIO->setReservoirName(reservoirname);
                trapperIO->setSpillDepth(spillDepth);
                trapperIO->setSpillPointPosition(spillX, spillY);
                trapperIO->setDepth(depth);
                trapperIO->setPosition(x, y);
                trapperIO->setOWC(owc);
                trapperIO->setGOC(goc);

                snapShot->addTrapper(trapperIO);

                assert(persistentID <= maxPersistentTrapperID);
                persistentIDs[persistentID] = &trapperIO;
            }

            // Assign downstream trappers
            const TrapperList& trappers = snapShot->getTrapperList();
            BOOST_FOREACH(const std::shared_ptr<const Trapper>& trapper, trappers)
            {
                int downstreamTrapperID = trapper->getDownStreamTrapperID();
                if (downstreamTrapperID > -1)
                {
                    std::shared_ptr<Trapper> downstreamTrapper = *persistentIDs[downstreamTrapperID];
                    std::shared_ptr<Trapper> thisTrapper = *persistentIDs[trapper->getPersistentID()];
                    thisTrapper->setDownStreamTrapper(downstreamTrapper);
                }
            }
        }

        m_project->addSnapShot(snapShot);
    }
    
    return m_project;
}

std::shared_ptr<const Reservoir> CauldronIO::ImportFromXML::getReservoir(pugi::xml_node reservoirNode) const
{
    std::shared_ptr<Reservoir> reservoir;

    std::string name = reservoirNode.attribute("name").value();
    std::string formation = reservoirNode.attribute("formation").value(); 

    std::shared_ptr<const Formation> formationIO = m_project->findFormation(formation);
    assert(formationIO);

    reservoir.reset(new Reservoir(name, formationIO));
    return reservoir;
}

std::shared_ptr<const Geometry2D> CauldronIO::ImportFromXML::getGeometry2D(pugi::xml_node geometryNode) const
{
    std::shared_ptr<const Geometry2D> geometry;

    double minI, minJ, deltaI, deltaJ;
    size_t numI, numJ;

    numI = (size_t)geometryNode.attribute("numI").as_uint();
    numJ = (size_t)geometryNode.attribute("numJ").as_uint();
    minI =   geometryNode.attribute("minI").as_double();
    minJ =   geometryNode.attribute("minJ").as_double();
    deltaI = geometryNode.attribute("deltaI").as_double();
    deltaJ = geometryNode.attribute("deltaJ").as_double();

    // Cell-centered?
    bool cellCentered = false;
    pugi::xml_attribute isCellCenteredNode = geometryNode.attribute("cell-centered");
    if (isCellCenteredNode)
    {
        cellCentered = isCellCenteredNode.as_bool();
    }

    return std::shared_ptr<const Geometry2D>(new Geometry2D(numI, numJ, deltaI, deltaJ, minI, minJ, cellCentered));
}

std::shared_ptr<Volume> CauldronIO::ImportFromXML::getVolume(pugi::xml_node volumeNode, const ibs::FilePath& path)
{
    SubsurfaceKind surfaceKind = (SubsurfaceKind)volumeNode.attribute("subsurfacekind").as_int();

    // Create the volume
    std::shared_ptr<Volume> volume(new Volume(surfaceKind));

    // Get all property volume data
    pugi::xml_node propertyVolNodes = volumeNode.child("propertyvols");
    assert(propertyVolNodes);

    for (pugi::xml_node propertyVolNode = propertyVolNodes.child("propertyvol"); propertyVolNode; propertyVolNode = propertyVolNode.next_sibling("propertyvol"))
    {
        std::string propertyName = propertyVolNode.attribute("property").value();
        std::shared_ptr<const Property> property = m_project->findProperty(propertyName);
        assert(property);

        // Get the geometry
        size_t geometryIndex = (size_t)propertyVolNode.attribute("geom-index").as_int();
        std::shared_ptr<const Geometry2D> geometry = m_project->getGeometries().at(geometryIndex);

        // Get K range
        pugi::xml_attribute firstK_attrib = propertyVolNode.attribute("firstK");
        if (!firstK_attrib) throw CauldronIOException("Cannot find firstK attribute in property-volume");
        size_t firstK = firstK_attrib.as_uint();
        pugi::xml_attribute numK_attrib = propertyVolNode.attribute("numK");
        if (!numK_attrib) throw CauldronIOException("Cannot find numK attribute in property-volume");
        size_t numK = numK_attrib.as_uint();

        // Get min/max value
        float minValue = DefaultUndefinedValue;
        float maxValue = DefaultUndefinedValue;

        pugi::xml_attribute minValueAttr = propertyVolNode.attribute("min");
        if (minValueAttr)
            minValue = minValueAttr.as_float();

        pugi::xml_attribute maxValueAttr = propertyVolNode.attribute("max");
        if (maxValueAttr)
            maxValue = maxValueAttr.as_float();

        // Create the VolumeData
        std::shared_ptr<Geometry3D> geometry3D(new Geometry3D(geometry->getNumI(), geometry->getNumJ(), numK, firstK, geometry->getDeltaI(),
            geometry->getDeltaJ(), geometry->getMinI(), geometry->getMinJ()));
        std::shared_ptr<VolumeData> volData(new VolumeDataNative(geometry3D, minValue, maxValue));

        // Get the datastore xml node or constantvalue
        pugi::xml_attribute constantVal = propertyVolNode.attribute("constantvalue");
        if (constantVal)
            volData->setConstantValue(constantVal.as_float());
        else
            DataStoreLoad::getVolume(propertyVolNode, volData, path);

        PropertyVolumeData propVolData(property, volData);
        volume->addPropertyVolumeData(propVolData);
    }
    
    return volume;
}

std::shared_ptr<Property> CauldronIO::ImportFromXML::getProperty(pugi::xml_node propertyNode) const
{
    std::shared_ptr<Property> property;

    std::string name = propertyNode.attribute("name").value();
    std::string cauldronname = propertyNode.attribute("cauldronname").value(); 
    std::string username = propertyNode.attribute("username").value(); 
    std::string unit = propertyNode.attribute("unit").value();  
    PropertyType type = (PropertyType)propertyNode.attribute("type").as_int();
    PropertyAttribute attrib = (PropertyAttribute)propertyNode.attribute("attribute").as_int();

    property.reset(new Property(name, username, cauldronname, unit, type, attrib));
    return property;
}

std::shared_ptr<Formation> CauldronIO::ImportFromXML::getFormation(pugi::xml_node formationNode) const
{
    std::shared_ptr<Formation> formation;

    unsigned int start, end;
    std::string name = formationNode.attribute("name").value();
    start = formationNode.attribute("kstart").as_uint();
    end = formationNode.attribute("kend").as_uint();
    bool isSR = formationNode.attribute("isSR").as_bool();
    bool isML = formationNode.attribute("isML").as_bool(); 

    formation.reset(new Formation(start, end, name, isSR, isML));
    return formation;
}