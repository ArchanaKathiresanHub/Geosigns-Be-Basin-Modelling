//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "DataStore.h"
#include "VisualizationAPI.h"
#include "VisualizationIO_native.h"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file.hpp>

#include <iostream>
#include <fstream>
#include <cstring>
#include <streambuf>

#define MINIMALBYTESTOCOMPRESS 50
#define APPLY_COMPRESSION true

using namespace CauldronIO;

/// DataStoreLoad
//////////////////////////////////////////////////////////////////////////

CauldronIO::DataStoreLoad::DataStoreLoad(DataStoreParams* params)
{
    m_params = static_cast<DataStoreParamsNative*>(params);
    
    if (!boost::filesystem::exists(m_params->fileName))
        throw CauldronIOException("Cannot retrieve file for reading");

    m_file_in.open(m_params->fileName.c_str(), BOOST_IOS::binary);
}

CauldronIO::DataStoreLoad::~DataStoreLoad()
{
    m_file_in.close();
    delete m_params;
}

float* CauldronIO::DataStoreLoad::getData(size_t& size)
{
    size = m_params->size;
    m_file_in.seekg(m_params->offset);
    char* data = new char[size];
    m_file_in.read(data, size);

    float* result = (float*)data;

    if (m_params->compressed)
    {
        char* resultChar = decompress(data, size);
        result = (float*)resultChar;
        delete[] data;
    }

    return result;
}

char* CauldronIO::DataStoreLoad::decompress(const char* inputData, size_t& elements)
{
    std::stringstream data, result;
    data.write(inputData, elements);

    boost::iostreams::filtering_streambuf<boost::iostreams::input> out;
    out.push(boost::iostreams::gzip_decompressor());
    out.push(data);
    elements = boost::iostreams::copy(out, result);

    // Copy the result into a char array
    std::streambuf* pbuf = result.rdbuf();
    pbuf->pubseekpos(0);
    char* charResult = new char[elements];
    pbuf->sgetn(charResult, elements);

    return charResult;
}

boost::shared_ptr<Volume> CauldronIO::DataStoreLoad::getVolume(const boost::property_tree::ptree& ptree, boost::shared_ptr<Property> property)
{
    SubsurfaceKind surfaceKind = (SubsurfaceKind)ptree.get<int>("<xmlattr>.subsurfacekind");
    boost::uuids::uuid uuid = ptree.get<boost::uuids::uuid>("<xmlattr>.uuid");
    bool cellCentered = ptree.get<bool>("<xmlattr>.cell-centered");

    // Create the volume
    VolumeNative* volumeNative = new VolumeNative(cellCentered, surfaceKind, property);

    boost::shared_ptr<Volume> volume(volumeNative);

    BOOST_FOREACH(boost::property_tree::ptree::value_type const& nodes, ptree.get_child(""))
    {
        if (nodes.first == "geometry")
        {
            const boost::property_tree::ptree& geometryNode = nodes.second;
            double minI, minJ, maxI, maxJ, deltaI, deltaJ;
            float undef;
            size_t numI, numJ, numK, firstK, lastK;

            numI = geometryNode.get<size_t>("<xmlattr>.numI");
            numJ = geometryNode.get<size_t>("<xmlattr>.numJ");
            numK = geometryNode.get<size_t>("<xmlattr>.numK");
            minI = geometryNode.get<double>("<xmlattr>.minI");
            minJ = geometryNode.get<double>("<xmlattr>.minJ");
            maxI = geometryNode.get<double>("<xmlattr>.maxI");
            maxJ = geometryNode.get<double>("<xmlattr>.maxJ");
            deltaI = geometryNode.get<double>("<xmlattr>.deltaI");
            deltaJ = geometryNode.get<double>("<xmlattr>.deltaJ");
            firstK = geometryNode.get<size_t>("<xmlattr>.firstK");
            lastK = geometryNode.get<size_t>("<xmlattr>.lastK");
            undef = geometryNode.get<float>("<xmlattr>.undefinedvalue");

            volume->setGeometry(numI, numJ, numK, firstK, deltaI, deltaJ, minI, minJ);
            volume->setUndefinedValue(undef);
        }
        else if (nodes.first == "constantvalue")
        {
            float value = ptree.get<float>("constantvalue");
            volume->setConstantValue(value);
        }
        else if (nodes.first == "datastore")
        {
            // Extract some data
            const boost::property_tree::ptree& datastoreNode = nodes.second;

            DataStoreParamsNative* paramsNative = new DataStoreParamsNative();
            paramsNative->fileName = datastoreNode.get<std::string>("<xmlattr>.file");
            paramsNative->compressed = datastoreNode.get<std::string>("<xmlattr>.compression") == "gzip";
            paramsNative->size = datastoreNode.get<size_t>("<xmlattr>.size");
            paramsNative->offset = datastoreNode.get<size_t>("<xmlattr>.offset");
            bool dataIJK = datastoreNode.get<bool>("<xmlattr>.dataIJK");

            volumeNative->setDataStore(paramsNative, dataIJK);
        }
    }

    volume->setUUID(uuid);

    // Check for optional depthSurfaceUUID
    boost::optional<boost::uuids::uuid> depthSurfaceUUID = ptree.get_optional<boost::uuids::uuid>("depthvolume-uuid");
    if (depthSurfaceUUID)
    {
        VolumeNative* volumeNative = dynamic_cast<VolumeNative*>(volume.get());
        volumeNative->setDepthSurfaceUUID(*depthSurfaceUUID);
    }

    return volume;
}

boost::shared_ptr<Surface> CauldronIO::DataStoreLoad::getSurface(const boost::property_tree::ptree& ptree, boost::shared_ptr<Property> property)
{
    std::string surfaceName = ptree.get<std::string>("<xmlattr>.name");
    std::string reservoirName = ptree.get<std::string>("<xmlattr>.reservoirName");

    SubsurfaceKind surfaceKind = (SubsurfaceKind)ptree.get<int>("<xmlattr>.subsurfacekind");
    boost::uuids::uuid uuid = ptree.get<boost::uuids::uuid>("<xmlattr>.uuid");
    bool cellCentered = ptree.get<bool>("<xmlattr>.cell-centered");

    // Create the value map
    MapNative* mapNative = new MapNative(cellCentered);

    boost::shared_ptr<Map> valueMap(mapNative);
    BOOST_FOREACH(boost::property_tree::ptree::value_type const& nodes, ptree.get_child(""))
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
        else if (nodes.first == "constantvalue")
        {
            float value = ptree.get<float>("constantvalue");
            valueMap->setConstantValue(value);
        }
        else if (nodes.first == "datastore")
        {
            // Extract some data
            const boost::property_tree::ptree& datastoreNode = nodes.second;

            DataStoreParamsNative* paramsNative = new DataStoreParamsNative();
            paramsNative->fileName = datastoreNode.get<std::string>("<xmlattr>.file");
            paramsNative->compressed = datastoreNode.get<std::string>("<xmlattr>.compression") == "gzip";
            paramsNative->size = datastoreNode.get<size_t>("<xmlattr>.size");
            paramsNative->offset = datastoreNode.get<size_t>("<xmlattr>.offset");
            mapNative->setDataStore(paramsNative);
        }
    }

    valueMap->setUUID(uuid);

    // Check for optional depthSurfaceUUID
    boost::optional<boost::uuids::uuid> depthSurfaceUUID = ptree.get_optional<boost::uuids::uuid>("depthsurface-uuid");
    if (depthSurfaceUUID)
    {
        mapNative->setDepthSurfaceUUID(*depthSurfaceUUID);
    }

    // Create the surface
    boost::shared_ptr<Surface> surface(new Surface(surfaceName, surfaceKind, property, valueMap));
    surface->setReservoirName(reservoirName);

    return surface;
}

 /// DataStoreSave
 //////////////////////////////////////////////////////////////////////////

CauldronIO::DataStoreSave::DataStoreSave(const std::string& filename)
{
    m_file_out.open(filename.c_str(), BOOST_IOS::binary);
    m_offset = 0;
    m_lastSize = 0;
    m_compress = APPLY_COMPRESSION;
    m_fileName = filename;
}

CauldronIO::DataStoreSave::~DataStoreSave()
{
    m_file_out.flush();
    m_file_out.close();

    // Delete if empty
    if (m_offset == 0)
        boost::filesystem::remove(m_fileName);
}


char* CauldronIO::DataStoreSave::compress(const char* inputData, size_t& elements)
{
    std::stringstream data, result;
    data.write(inputData, elements);

    boost::iostreams::filtering_streambuf<boost::iostreams::input> out;
    out.push(boost::iostreams::gzip_compressor());
    out.push(data);
    elements = boost::iostreams::copy(out, result);

    // Copy the result into a char array
    std::streambuf* pbuf = result.rdbuf();
    pbuf->pubseekpos(0);
    char* charResult = new char[elements];
    pbuf->sgetn(charResult, elements);

    return charResult;
}

void CauldronIO::DataStoreSave::addData(const float* data, size_t size, bool compressData)
{
    char* dataToWrite = (char*)data;
    size_t sizeToWrite = sizeof(float)*size;

    std::vector<char> compressed;

    if (compressData)
        dataToWrite = compress(dataToWrite, sizeToWrite);

    m_file_out.write((char*)dataToWrite, sizeToWrite);
    m_offset += sizeToWrite;
    m_lastSize = sizeToWrite;

    if (compressData)
        delete[] dataToWrite;
}

void CauldronIO::DataStoreSave::addSurface(const boost::shared_ptr<Surface>& surfaceIO, boost::property_tree::ptree& ptree)
{
    if (!surfaceIO->isRetrieved())
        surfaceIO->retrieve();

    ptree.put("<xmlattr>.name", surfaceIO->getName());
    ptree.put("<xmlattr>.reservoirName", surfaceIO->getReservoirName());
    ptree.put("<xmlattr>.subsurfacekind", surfaceIO->getSubSurfaceKind());
    ptree.put("<xmlattr>.uuid", surfaceIO->getValueMap()->getUUID());
    ptree.put("<xmlattr>.cell-centered", surfaceIO->getValueMap()->isCellCentered());

    // Set geometry
    addGeometryInfo(ptree, surfaceIO->getValueMap());

    // Set depth surface
    boost::shared_ptr<const Surface> depthSurface = surfaceIO->getDepthSurface();
    if (depthSurface)
        ptree.put("depthsurface-uuid", depthSurface->getValueMap()->getUUID());

    if (surfaceIO->getValueMap()->isConstant())
        ptree.put("constantvalue", surfaceIO->getValueMap()->getConstantValue());
    else
    {
        boost::property_tree::ptree& subNode = ptree.add("datastore", "");
        subNode.put("<xmlattr>.file", m_fileName);

        boost::shared_ptr<Map> map = surfaceIO->getValueMap();

        size_t numBytes = map->getNumI()*map->getNumJ()*sizeof(float);
        bool compress = m_compress && numBytes > MINIMALBYTESTOCOMPRESS;
        if (compress)
            subNode.put("<xmlattr>.compression", "gzip");
        else
            subNode.put("<xmlattr>.compression", "none");
        subNode.put("<xmlattr>.offset", m_offset);

        if (map->isConstant()) throw CauldronIO::CauldronIOException("Cannot write constant value");
        addData(map->getSurfaceValues(), map->getNumI()*map->getNumJ(), compress);

        subNode.put("<xmlattr>.size", m_lastSize);
    }
}

void CauldronIO::DataStoreSave::addVolume(const boost::shared_ptr<Volume>& volume, boost::property_tree::ptree& volNode)
{
    if (!volume->isRetrieved())
        volume->retrieve();

    volNode.put("<xmlattr>.subsurfacekind", volume->getSubSurfaceKind());
    volNode.put("<xmlattr>.uuid", volume->getUUID());
    volNode.put("<xmlattr>.cell-centered", volume->isCellCentered());

    // Set geometry
    addGeometryInfo(volNode, volume);

    // Set depth volume
    boost::shared_ptr<const Volume> depthVolume = volume->getDepthVolume();
    if (depthVolume)
        volNode.put("depthvolume-uuid", depthVolume->getUUID());

    if (volume->isConstant())
    {
        volNode.put("constantvalue", volume->getConstantValue());
        return;
    }

    size_t numBytes = volume->getNumI()*volume->getNumJ()*volume->getNumK()*sizeof(float);
    bool compress = m_compress && numBytes > MINIMALBYTESTOCOMPRESS;

    if (volume->hasDataIJK())
    {
        boost::property_tree::ptree& subNode = volNode.add("datastore", "");
        subNode.put("<xmlattr>.file", m_fileName);
        
        if (compress)
            subNode.put("<xmlattr>.compression", "gzip");
        else
            subNode.put("<xmlattr>.compression", "none");
        subNode.put("<xmlattr>.offset", m_offset);
        subNode.put("<xmlattr>.dataIJK", true);
        // Write the volume and update the offset
        writeVolume(volume, true, compress);
        subNode.put("<xmlattr>.size", m_lastSize);
    }

    if (volume->hasDataKIJ())
    {
        boost::property_tree::ptree& subNode = volNode.add("datastore", "");
        subNode.put("<xmlattr>.file", m_fileName);
        if (compress)
            subNode.put("<xmlattr>.compression", "gzip");
        else
            subNode.put("<xmlattr>.compression", "none");
        subNode.put("<xmlattr>.offset", m_offset);
        subNode.put("<xmlattr>.dataIJK", false);

        // Write the volume and update the offset
        writeVolume(volume, false, compress);
        subNode.put("<xmlattr>.size", m_lastSize);
    }
}

void CauldronIO::DataStoreSave::addGeometryInfo(boost::property_tree::ptree& node, const boost::shared_ptr<const Map>& map) const
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

void CauldronIO::DataStoreSave::addGeometryInfo(boost::property_tree::ptree& tree, const boost::shared_ptr<const Volume>& volume) const
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

void CauldronIO::DataStoreSave::writeVolume(const boost::shared_ptr<Volume>& volume, bool dataIJK, bool compress)
{
    if (volume->isConstant()) return;
    const float* data = (dataIJK ? volume->getVolumeValues_IJK() : volume->getVolumeValues_KIJ());

    addData(data, volume->getNumI()*volume->getNumJ()*volume->getNumK(), compress);
}