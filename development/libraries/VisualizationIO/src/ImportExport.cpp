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
#include <fstream>
#include <cstring>

#define __compress__ true
#define __fp16__ true

using namespace CauldronIO;

bool ImportExport::ExportToXML(boost::shared_ptr<Project>& project, const std::string& path)
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
    newExport.AddProject(pt, project);

    // Write property tree to XML file
    std::string xmlFileName = GetXMLIndexingFileName(outputPath);
    write_xml(xmlFileName, pt);

    return true;
}

void ImportExport::AddProject(boost::property_tree::ptree& pt, boost::shared_ptr<Project>& project)
{
    // Create empty property tree object
    using boost::property_tree::ptree;

    // Add general project description
    pt.put("project.name", project->GetName());
    pt.put("project.description", project->GetDescription());
    pt.put("project.modelingmode", project->GetModelingMode());
    pt.put("project.team", project->GetTeam());
    pt.put("project.programversion", project->GetProgramVersion());

    const SnapShotList snapShotList = project->GetSnapShots();

    BOOST_FOREACH(boost::shared_ptr<const SnapShot> snapShot, snapShotList)
    {
        boost::filesystem::path volumeStorePath(_outputPath);
        volumeStorePath /= "Snapshot_" + boost::lexical_cast<std::string>(snapShot->GetAge()) + "_volumes.cldrn";
        DataStore volumeStore(volumeStorePath.string(), __compress__, __fp16__);

        boost::filesystem::path surfaceStorePath(_outputPath);
        surfaceStorePath /= "Snapshot_" + boost::lexical_cast<std::string>(snapShot->GetAge()) + "_surfaces.cldrn";
        DataStore surfaceDataStore(surfaceStorePath.string(), __compress__, __fp16__);

        ptree & node = pt.add("project.snapshots.snapshot", "");
        node.put("<xmlattr>.age", snapShot->GetAge());
        node.put("<xmlattr>.kind", snapShot->GetKind());
        node.put("<xmlattr>.isminor", snapShot->IsMinorShapshot());

        const SurfaceList surfaces = snapShot->GetSurfaceList();
        BOOST_FOREACH(const boost::shared_ptr<Surface>& surfaceIO, surfaces)
        {
            // General properties
            ptree& surface = node.add("surfaces.surface", "");
            surface.put("<xmlattr>.name", surfaceIO->GetName());
            surface.put("<xmlattr>.subsurfacekind", surfaceIO->GetSubSurfaceKind());
            surface.put("<xmlattr>.uuid", surfaceIO->GetValueMap()->GetUUID());
            
            // Set geometry
            AddGeometryInfo(surface, surfaceIO->GetValueMap());

            // Data storage
            if (surfaceIO->GetValueMap()->IsConstant())
                surface.put("constantvalue", surfaceIO->GetValueMap()->GetConstantValue());
            else
            {
                boost::property_tree::ptree& subNode = surface.add("datastore", "");
                std::string fileName = GetFilename(surfaceIO->GetValueMap()->GetUUID());
                subNode.put("<xmlattr>.file", surfaceDataStore.GetFileName());
                subNode.put("<xmlattr>.format", "fp16");
                subNode.put("<xmlattr>.conversion", "none");
                subNode.put("<xmlattr>.row-ordered", "yes");
                subNode.put("<xmlattr>.compression", "gzip");
                subNode.put("<xmlattr>.offset", surfaceDataStore.GetOffset());
                
                WriteSurface(surfaceIO, surfaceDataStore);
                subNode.put("<xmlattr>.size", surfaceDataStore.GetLastSize());
            }

            // Set depth surface
            boost::shared_ptr<const Surface> depthSurface = surfaceIO->GetDepthSurface();
            if (depthSurface)
                surface.put("depthsurface-uuid", depthSurface->GetValueMap()->GetUUID());

            // Set property
            AddProperty(surface, surfaceIO->GetProperty());

            // Set formation
            boost::shared_ptr<const Formation> formation = surfaceIO->GetFormation();
            if (formation)
                AddFormation(surface, formation);
        }

        const VolumeList volumes = snapShot->GetVolumeList();
        BOOST_FOREACH(const boost::shared_ptr<Volume>& volume, volumes)
        {
            // General properties
            ptree& volNode = node.add("volumes.volume", "");
            volNode.put("<xmlattr>.subsurfacekind", volume->GetSubSurfaceKind());
            volNode.put("<xmlattr>.uuid", volume->GetUUID());
            volNode.put("<xmlattr>.cell-centered", volume->IsCellCentered());

            // Set geometry
            AddGeometryInfo(volNode, volume);

            // Data storage
            AddDataStorage(volNode, volume, volumeStore);

            // Set depth volume
            boost::shared_ptr<const Volume> depthVolume = volume->GetDepthVolume();
            if (depthVolume)
                volNode.put("depthvolume-uuid", depthVolume->GetUUID());

            // Set property
            AddProperty(volNode, volume->GetProperty());
        }

        const DiscontinuousVolumeList discVolumes = snapShot->GetDiscontinuousVolumeList();
        BOOST_FOREACH(const boost::shared_ptr<const DiscontinuousVolume>& volume, discVolumes)
        {
            // General properties
            ptree& volNode = node.add("aggregate-volumes.aggregate-volume", "");
            volNode.put("<xmlattr>.cell-centered", volume->IsCellCentered());

            // Set depth volume
            boost::shared_ptr<const Volume> depthVolume = volume->GetDepthVolume();
            if (depthVolume)
                volNode.put("depthvolume-uuid", depthVolume->GetUUID());

            const FormationVolumeList formationVolumes = volume->GetVolumeList();
            BOOST_FOREACH(const boost::shared_ptr<const FormationVolume>& formationVolume, formationVolumes)
            {
                const boost::shared_ptr<Volume> subVolume = formationVolume->second;
                const boost::shared_ptr<const Formation> subFormation = formationVolume->first;
                
                // Add formation 
                ptree& subNode = volNode.add("formation-volumes.formation-volume", "");
                AddFormation(subNode, subFormation);

                // Add volume 
                ptree& subvolNode = subNode.add("volume", "");
                subvolNode.put("<xmlattr>.subsurfacekind", subVolume->GetSubSurfaceKind());
                subvolNode.put("<xmlattr>.uuid", subVolume->GetUUID());
                subvolNode.put("<xmlattr>.cell-centered", subVolume->IsCellCentered());
                AddGeometryInfo(subvolNode, subVolume);
                AddDataStorage(subvolNode, subVolume, volumeStore);
                AddProperty(subvolNode, subVolume->GetProperty());
            }
        }
    }
}

std::string CauldronIO::ImportExport::GetFilename(const boost::uuids::uuid& uuid) const
{
    return to_string(uuid) + ".cldrn";
}

CauldronIO::ImportExport::ImportExport(const boost::filesystem::path& path)
{
    _outputPath = path;
}

void CauldronIO::ImportExport::AddProperty(boost::property_tree::ptree &node, const boost::shared_ptr<const Property>& property)
{
    boost::property_tree::ptree& propNode = node.add("property","");
    propNode.put("<xmlattr>.name", property->GetName());
    propNode.put("<xmlattr>.cauldronname", property->GetCauldronName());
    propNode.put("<xmlattr>.username", property->GetUserName());
    propNode.put("<xmlattr>.unit", property->GetUnit());
    propNode.put("<xmlattr>.attribute", property->GetAttribute());
    propNode.put("<xmlattr>.type", property->GetType());
}

void CauldronIO::ImportExport::AddFormation(boost::property_tree::ptree& node, const boost::shared_ptr<const Formation>& formation)
{
    boost::property_tree::ptree& subNode = node.add("formation", "");
    subNode.put("<xmlattr>.name", formation->GetName());
    size_t start, end;
    formation->GetDepthRange(start, end);
    subNode.put("<xmlattr>.kstart", start);
    subNode.put("<xmlattr>.kend", end);
}

void CauldronIO::ImportExport::AddGeometryInfo(boost::property_tree::ptree& node, const boost::shared_ptr<const Map>& map)
{
    boost::property_tree::ptree& subNode = node.add("geometry", "");
    subNode.put("<xmlattr>.numI", map->GetNumI());
    subNode.put("<xmlattr>.numJ", map->GetNumJ());
    subNode.put("<xmlattr>.minI", map->GetMinI());
    subNode.put("<xmlattr>.minJ", map->GetMinJ());
    subNode.put("<xmlattr>.maxI", map->GetMaxI());
    subNode.put("<xmlattr>.maxJ", map->GetMaxJ());
    subNode.put("<xmlattr>.deltaI", map->GetDeltaI());
    subNode.put("<xmlattr>.deltaJ", map->GetDeltaJ());
    subNode.put("<xmlattr>.undefinedvalue", map->GetUndefinedValue());
}

void CauldronIO::ImportExport::AddGeometryInfo(boost::property_tree::ptree& tree, const boost::shared_ptr<const Volume>& volume)
{
    boost::property_tree::ptree& subNode = tree.add("geometry", "");
    subNode.put("<xmlattr>.numI", volume->GetNumI());
    subNode.put("<xmlattr>.numJ", volume->GetNumJ());
    subNode.put("<xmlattr>.numK", volume->GetNumK());
    subNode.put("<xmlattr>.minI", volume->GetMinI());
    subNode.put("<xmlattr>.minJ", volume->GetMinJ());
    subNode.put("<xmlattr>.maxI", volume->GetMaxI());
    subNode.put("<xmlattr>.maxJ", volume->GetMaxJ());
    subNode.put("<xmlattr>.firstK", volume->GetFirstK());
    subNode.put("<xmlattr>.lastK", volume->GetLastK());
    subNode.put("<xmlattr>.deltaI", volume->GetDeltaI());
    subNode.put("<xmlattr>.deltaJ", volume->GetDeltaJ());
    subNode.put("<xmlattr>.undefinedvalue", volume->GetUndefinedValue());
}

void CauldronIO::ImportExport::AddDataStorage(boost::property_tree::ptree& volNode, const boost::shared_ptr<Volume>& volume, DataStore& dataStore)
{
    if (volume->IsConstant())
    {
        volNode.put("constantvalue", volume->GetConstantValue());
        return;
    }

    if (volume->HasDataIJK())
    {
        boost::property_tree::ptree& subNode = volNode.add("datastore", "");
        subNode.put("<xmlattr>.file", dataStore.GetFileName());
        subNode.put("<xmlattr>.format", "fp16");
        subNode.put("<xmlattr>.conversion", "none");
        subNode.put("<xmlattr>.compression", "gzip");
        subNode.put("<xmlattr>.offset", dataStore.GetOffset());
        subNode.put("<xmlattr>.dataIJK", true);
        subNode.put("<xmlattr>.dataKIJ", false);
        // Write the volume and update the offset
        WriteVolume(volume, true, dataStore);
        subNode.put("<xmlattr>.size", dataStore.GetLastSize());
    }

    if (volume->HasDataKIJ())
    {
        boost::property_tree::ptree& subNode = volNode.add("datastore", "");
        subNode.put("<xmlattr>.file", dataStore.GetFileName());
        subNode.put("<xmlattr>.format", "fp16");
        subNode.put("<xmlattr>.conversion", "none");
        subNode.put("<xmlattr>.compression", "gzip");
        subNode.put("<xmlattr>.offset", dataStore.GetOffset());
        subNode.put("<xmlattr>.dataIJK", false);
        subNode.put("<xmlattr>.dataKIJ", true);

        // Write the volume and update the offset
        WriteVolume(volume, false, dataStore);
        subNode.put("<xmlattr>.size", dataStore.GetLastSize());
    }
}

void CauldronIO::ImportExport::WriteSurface(const boost::shared_ptr<Surface>& surfaceIO, DataStore& store)
{
    boost::shared_ptr<Map> map = surfaceIO->GetValueMap();
    if (map->IsConstant()) return;
    store.AddData(map->GetSurfaceValues(), map->GetNumI()*map->GetNumJ(), map->GetUndefinedValue());
}

void CauldronIO::ImportExport::WriteVolume(const boost::shared_ptr<Volume>& volume, bool dataIJK, DataStore& store)
{
    if (volume->IsConstant()) return;
    const float* data = (dataIJK ? volume->GetVolumeValues_IJK() : volume->GetVolumeValues_KIJ());

    store.AddData(data, volume->GetNumI()*volume->GetNumK()*volume->GetNumK(), volume->GetUndefinedValue());
}

std::string CauldronIO::ImportExport::GetXMLIndexingFileName(const boost::filesystem::path& path)
{
    boost::filesystem::path result = path / "cauldron_outputs.xml";

    return std::string(result.string());
}

//////////////////////////////////////////////////////////////////////////
/// DataStore implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::ImportExport::DataStore::DataStore(const std::string& filename, bool compress, bool fp16)
{
    _file.open(filename, BOOST_IOS::binary);
    _offset = 0;
    _fileName = filename;
    _compress = compress;
    _fp16 = fp16;
}

CauldronIO::ImportExport::DataStore::~DataStore()
{
    _file.flush();
    _file.close();

    // Delete if empty
    if (_offset == 0)
        boost::filesystem::remove(_fileName);
}

size_t CauldronIO::ImportExport::DataStore::GetOffset() const
{
    return _offset;
}

void CauldronIO::ImportExport::DataStore::AddData(const float* data, size_t size, float undef)
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

    if (_compress)
    {
        compressed = Compress(dataToWrite, sizeToWrite);

        dataToWrite = (char*)(&compressed[0]);
        sizeToWrite = compressed.size();
    }

    _file.write((char*)dataToWrite, sizeToWrite);
    _offset += sizeToWrite;
    _lastSize = sizeToWrite;

    //if (halfArray) delete[] halfArray;
}

std::vector<char> CauldronIO::ImportExport::DataStore::Decompress(const char* data, size_t size) const
{
    std::vector<char> decompressed = std::vector<char>();

    boost::iostreams::filtering_ostream os;
    os.push(boost::iostreams::gzip_decompressor());
    os.push(boost::iostreams::back_inserter(decompressed));
    os.write(data, size);
    os.flush();

    return decompressed;
}

std::vector<char> CauldronIO::ImportExport::DataStore::Compress(const char* data, size_t size) const
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
void CauldronIO::ImportExport::DataStore::GetStatistics(const T* data, size_t size, T undef)
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

const std::string& CauldronIO::ImportExport::DataStore::GetFileName() const
{
    return _fileName;
}

size_t CauldronIO::ImportExport::DataStore::GetLastSize() const
{
    return _lastSize;
}

bool CauldronIO::ImportExport::DataStore::GetCompress() const
{
    return _compress;
}

bool CauldronIO::ImportExport::DataStore::GetFP16() const
{
    return _fp16;
}
