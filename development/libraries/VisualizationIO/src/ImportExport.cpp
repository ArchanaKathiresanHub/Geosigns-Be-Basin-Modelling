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
#include <boost/foreach.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/system/error_code.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/lexical_cast.hpp>

//#include "half.hpp" // this is not compiling on Linux

#include <iostream>
#include <fstream>
#include <cstring>

#define __compress__ true
#define __fp16__ true

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
        DataStore volumeStore(volumeStorePath.string(), __compress__, __fp16__);

        boost::filesystem::path surfaceStorePath(m_outputPath);
        surfaceStorePath /= "Snapshot_" + boost::lexical_cast<std::string>(snapShot->getAge()) + "_surfaces.cldrn";
        DataStore surfaceDataStore(surfaceStorePath.string(), __compress__, __fp16__);

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
            {
                boost::property_tree::ptree& subNode = surface.add("datastore", "");
                std::string fileName = getFilename(surfaceIO->getValueMap()->getUUID());
                subNode.put("<xmlattr>.file", surfaceDataStore.getFileName());
                subNode.put("<xmlattr>.format", "fp16");
                subNode.put("<xmlattr>.conversion", "none");
                subNode.put("<xmlattr>.row-ordered", "yes");
                subNode.put("<xmlattr>.compression", "gzip");
                subNode.put("<xmlattr>.offset", surfaceDataStore.getOffset());
                
                writeSurface(surfaceIO, surfaceDataStore);
                subNode.put("<xmlattr>.size", surfaceDataStore.getLastSize());
            }

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
            addDataStorage(volNode, volume, volumeStore);

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
                addDataStorage(subvolNode, subVolume, volumeStore);
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

void CauldronIO::ImportExport::addDataStorage(boost::property_tree::ptree& volNode, const boost::shared_ptr<Volume>& volume, DataStore& dataStore) const
{
    if (volume->isConstant())
    {
        volNode.put("constantvalue", volume->getConstantValue());
        return;
    }

    if (volume->hasDataIJK())
    {
        boost::property_tree::ptree& subNode = volNode.add("datastore", "");
        subNode.put("<xmlattr>.file", dataStore.getFileName());
        subNode.put("<xmlattr>.format", "fp16");
        subNode.put("<xmlattr>.conversion", "none");
        subNode.put("<xmlattr>.compression", "gzip");
        subNode.put("<xmlattr>.offset", dataStore.getOffset());
        subNode.put("<xmlattr>.dataIJK", true);
        subNode.put("<xmlattr>.dataKIJ", false);
        // Write the volume and update the offset
        writeVolume(volume, true, dataStore);
        subNode.put("<xmlattr>.size", dataStore.getLastSize());
    }

    if (volume->hasDataKIJ())
    {
        boost::property_tree::ptree& subNode = volNode.add("datastore", "");
        subNode.put("<xmlattr>.file", dataStore.getFileName());
        subNode.put("<xmlattr>.format", "fp16");
        subNode.put("<xmlattr>.conversion", "none");
        subNode.put("<xmlattr>.compression", "gzip");
        subNode.put("<xmlattr>.offset", dataStore.getOffset());
        subNode.put("<xmlattr>.dataIJK", false);
        subNode.put("<xmlattr>.dataKIJ", true);

        // Write the volume and update the offset
        writeVolume(volume, false, dataStore);
        subNode.put("<xmlattr>.size", dataStore.getLastSize());
    }
}

void CauldronIO::ImportExport::writeSurface(const boost::shared_ptr<Surface>& surfaceIO, DataStore& store) const
{
    boost::shared_ptr<Map> map = surfaceIO->getValueMap();
    if (map->isConstant()) return;
    store.addData(map->getSurfaceValues(), map->getNumI()*map->getNumJ(), map->getUndefinedValue());
}

void CauldronIO::ImportExport::writeVolume(const boost::shared_ptr<Volume>& volume, bool dataIJK, DataStore& store) const
{
    if (volume->isConstant()) return;
    const float* data = (dataIJK ? volume->getVolumeValues_IJK() : volume->getVolumeValues_KIJ());

    store.addData(data, volume->getNumI()*volume->getNumK()*volume->getNumK(), volume->getUndefinedValue());
}

std::string CauldronIO::ImportExport::getXMLIndexingFileName(const boost::filesystem::path& path)
{
    boost::filesystem::path result = path / "cauldron_outputs.xml";

    return std::string(result.string());
}

//////////////////////////////////////////////////////////////////////////
/// DataStore implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::ImportExport::DataStore::DataStore(const std::string& filename, bool compress, bool fp16)
{
    m_file.open(filename, BOOST_IOS::binary);
    m_offset = 0;
    m_fileName = filename;
    m_compress = compress;
    m_fp16 = fp16;
}

CauldronIO::ImportExport::DataStore::~DataStore()
{
    m_file.flush();
    m_file.close();

    // Delete if empty
    if (m_offset == 0)
        boost::filesystem::remove(m_fileName);
}

size_t CauldronIO::ImportExport::DataStore::getOffset() const
{
    return m_offset;
}

void CauldronIO::ImportExport::DataStore::addData(const float* data, size_t size, float undef)
{
    char* dataToWrite = (char*)data;
    size_t sizeToWrite = sizeof(float)*size;

    std::vector<char> compressed;

#if 0
    using namespace half_float;
    half* halfArray = NULL;
    half undefHalf = std::numeric_limits<half>::infinity();

    if (_fp16)
    {
        // TEMP
        //GetStatistics<float>(data, size, undef);

        halfArray = new half[size];
        for (size_t i = 0; i < size; ++i)
        {
            // We need to treat undef values in a special way
            float inputData = *(data + i);
            if (inputData == undef)
                halfArray[i] = undefHalf;
            else
                halfArray[i] = half_cast<half>(inputData);
        }

        //GetStatistics<half>(halfArray, size, undefHalf);
        
        dataToWrite = (char*)halfArray;
     
        assert(sizeof(float) == 4 && sizeof(half) == 2);
        sizeToWrite /= 2;
    }
#endif

    if (m_compress)
    {
        compressed = compress(dataToWrite, sizeToWrite);

        dataToWrite = (char*)(&compressed[0]);
        sizeToWrite = compressed.size();
    }

    m_file.write((char*)dataToWrite, sizeToWrite);
    m_offset += sizeToWrite;
    m_lastSize = sizeToWrite;

    //if (halfArray) delete[] halfArray;
}

std::vector<char> CauldronIO::ImportExport::DataStore::decompress(const char* data, size_t size) const
{
    std::vector<char> decompressed = std::vector<char>();

    boost::iostreams::filtering_ostream os;
    os.push(boost::iostreams::gzip_decompressor());
    os.push(boost::iostreams::back_inserter(decompressed));
    os.write(data, size);
    os.flush();

    return decompressed;
}

std::vector<char> CauldronIO::ImportExport::DataStore::compress(const char* data, size_t size) const
{
    std::vector<char> compressed = std::vector<char>();

    boost::iostreams::filtering_ostream os;
    os.push(boost::iostreams::gzip_compressor());
    os.push(boost::iostreams::back_inserter(compressed));
    os.write(data, size);
    os.flush();

    return compressed;
}

template <typename T>
void CauldronIO::ImportExport::DataStore::getStatistics(const T* data, size_t size, T undef)
{
    T minValue = std::numeric_limits<T>::max();
    T maxValue = std::numeric_limits<T>::lowest();
    size_t undefs = 0;

    for (size_t i = 0; i < size; ++i)
    {
        if (data[i] == undef) 
            undefs++;
        else
        {
            minValue = std::min(data[i], minValue);
            maxValue = std::max(data[i], maxValue);
        }
    }

    //if (undefs > 0)
    //    std::cout << "Undef" << std::endl;

    std::cout << "Min, max, undefs: " << minValue << "," << maxValue << "," << undefs << std::endl;
}

const std::string& CauldronIO::ImportExport::DataStore::getFileName() const
{
    return m_fileName;
}

size_t CauldronIO::ImportExport::DataStore::getLastSize() const
{
    return m_lastSize;
}

bool CauldronIO::ImportExport::DataStore::getCompress() const
{
    return m_compress;
}

bool CauldronIO::ImportExport::DataStore::getFP16() const
{
    return m_fp16;
}
