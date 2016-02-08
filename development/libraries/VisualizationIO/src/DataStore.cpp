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

    m_file_in.open(m_params->fileName.c_str(), std::fstream::in | std::fstream::binary);
}

CauldronIO::DataStoreLoad::~DataStoreLoad()
{
    m_file_in.close();

    // No need to destroy the parameters; they are not owned by us
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

void CauldronIO::DataStoreLoad::getVolume(const boost::property_tree::ptree& ptree, boost::shared_ptr<VolumeData> volumeData, const boost::filesystem::path& path)
{
    BOOST_FOREACH(boost::property_tree::ptree::value_type const& nodes, ptree.get_child(""))
    {
        if (nodes.first == "datastore")
        {
            VolumeDataNative* volumeNative = dynamic_cast<VolumeDataNative*>(volumeData.get());
            assert(volumeNative);

            // Extract some data
            const boost::property_tree::ptree& datastoreNode = nodes.second;

            float undef = datastoreNode.get<float>("<xmlattr>.undef");
            volumeNative->setUndefinedValue(undef);

            DataStoreParamsNative* paramsNative = new DataStoreParamsNative();
            std::string filename = datastoreNode.get<std::string>("<xmlattr>.file");
            boost::filesystem::path filepath(path);
            filepath /= filename;
            paramsNative->fileName = filepath;

            paramsNative->compressed = datastoreNode.get<std::string>("<xmlattr>.compression") == "gzip";
            paramsNative->size = datastoreNode.get<size_t>("<xmlattr>.size");
            paramsNative->offset = datastoreNode.get<size_t>("<xmlattr>.offset");
            bool dataIJK = datastoreNode.get<bool>("<xmlattr>.dataIJK");

            volumeNative->setDataStore(paramsNative, dataIJK);

            // do not return, there can be more than one orientation
        }
    }
}

void CauldronIO::DataStoreLoad::getSurface(const boost::property_tree::ptree& ptree, boost::shared_ptr<SurfaceData> surfaceData, const boost::filesystem::path& path)
{
    BOOST_FOREACH(boost::property_tree::ptree::value_type const& nodes, ptree.get_child(""))
    {
        if (nodes.first == "datastore")
        {
            MapNative* mapNative = dynamic_cast<MapNative*>(surfaceData.get());
            assert(mapNative); 
            
            // Extract some data
            const boost::property_tree::ptree& datastoreNode = nodes.second;

            float undef = datastoreNode.get<float>("<xmlattr>.undef");
            mapNative->setUndefinedValue(undef);

            DataStoreParamsNative* paramsNative = new DataStoreParamsNative();

            std::string filename = datastoreNode.get<std::string>("<xmlattr>.file");
            boost::filesystem::path filepath(path);
            filepath /= filename;
            paramsNative->fileName = filepath;

            paramsNative->compressed = datastoreNode.get<std::string>("<xmlattr>.compression") == "gzip";
            paramsNative->size = datastoreNode.get<size_t>("<xmlattr>.size");
            paramsNative->offset = datastoreNode.get<size_t>("<xmlattr>.offset");
            mapNative->setDataStore(paramsNative);
            return;
        }
    }

    throw CauldronIOException("Could not find datastore in xml node");
}

 /// DataStoreSave
 //////////////////////////////////////////////////////////////////////////

CauldronIO::DataStoreSave::DataStoreSave(const std::string& filename, bool append)
{
    if (!append)
        m_file_out.open(filename.c_str(), std::fstream::binary);
    else
        m_file_out.open(filename.c_str(), std::fstream::binary | std::fstream::ate | std::fstream::app);

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

void CauldronIO::DataStoreSave::addSurface(const boost::shared_ptr<SurfaceData>& surfaceData, boost::property_tree::ptree& node, size_t size)
{
    boost::property_tree::ptree& subNode = node.add("datastore", "");
    subNode.put("<xmlattr>.file", boost::filesystem::path(m_fileName).filename().string());
    subNode.put("<xmlattr>.undef", surfaceData->getUndefinedValue());

    MapNative* mapNative = dynamic_cast<MapNative*>(surfaceData.get());

    size_t numBytes = size*sizeof(float);
    bool compress = m_compress && numBytes > MINIMALBYTESTOCOMPRESS;
    if (compress)
        subNode.put("<xmlattr>.compression", "gzip");
    else
        subNode.put("<xmlattr>.compression", "none");
    subNode.put("<xmlattr>.offset", m_offset);

    if (surfaceData->isConstant()) throw CauldronIO::CauldronIOException("Cannot write constant value");

    // We write the actual data if 1) this map has been loaded from projecthandle (so mapNative == null)
    // or 2) this map has been created in native format, but was not loaded from disk (so no datastoreparams were set)
    if (mapNative == NULL || (mapNative != NULL && mapNative->getDataStoreParams() == NULL))
    {
        size_t seekPos = m_file_out.tellp();
        addData(surfaceData->getSurfaceValues(), size, compress);
    }
    else
    {
        // This surface already has been written: skip it
        const DataStoreParamsNative* const params = static_cast<DataStoreParamsNative const*>(mapNative->getDataStoreParams());
        assert(m_fileName == params->fileName);
        m_lastSize = params->size;
        m_offset += m_lastSize;
    }

    subNode.put("<xmlattr>.size", m_lastSize);
}

void CauldronIO::DataStoreSave::addVolume(const boost::shared_ptr<VolumeData>& data, boost::property_tree::ptree& node, size_t numBytes)
{
    bool compress = m_compress && numBytes > MINIMALBYTESTOCOMPRESS;
    VolumeDataNative* nativeVolume = dynamic_cast<VolumeDataNative*>(data.get());

    // We need to check if this volume has IJK data, and account for the case it was not retrieved yet;
    // in that case we need to write the meta data again to XML, without writing the volume data itself.
    // To check for that, it should be a native volume, with datastore parameters set.
    if (data->hasDataIJK() || (nativeVolume != NULL && nativeVolume->getDataStoreParamsIJK() != NULL))
        writeVolumePart(node, compress, true, data);

    if (data->hasDataKIJ() || (nativeVolume != NULL && nativeVolume->getDataStoreParamsKIJ() != NULL))
        writeVolumePart(node, compress, false, data);
}

void CauldronIO::DataStoreSave::writeVolumePart(boost::property_tree::ptree &volNode, bool compress, bool IJK, const boost::shared_ptr<VolumeData>& volume)
{
    // See if we're writing a native volume
    VolumeDataNative* nativeVolume = dynamic_cast<VolumeDataNative*>(volume.get());

    boost::property_tree::ptree& subNode = volNode.add("datastore", "");
    subNode.put("<xmlattr>.file", boost::filesystem::path(m_fileName).filename().string());
    subNode.put("<xmlattr>.undef", volume->getUndefinedValue());

    if (compress)
        subNode.put("<xmlattr>.compression", "gzip");
    else
        subNode.put("<xmlattr>.compression", "none");
    subNode.put("<xmlattr>.offset", m_offset);
    subNode.put("<xmlattr>.dataIJK", IJK);

    // We write the actual data if 1) this volume has been loaded from projecthandle (so nativeVolume == null)
    // or 2) this volume has been created in native format, but was not loaded from disk (so no datastoreparams were set)
    bool writeData;
    if (IJK)
        writeData = nativeVolume == NULL || nativeVolume->getDataStoreParamsIJK() == NULL;
    else 
        writeData = nativeVolume == NULL || nativeVolume->getDataStoreParamsKIJ() == NULL;

    if (writeData)
    {
        // Write the volume and update the offset
        writeVolume(volume, IJK, compress);
    }
    else
    {
        // This volume already has been written: skip it
        const DataStoreParamsNative* const params = static_cast<DataStoreParamsNative const*>(nativeVolume->getDataStoreParamsIJK());
        assert(m_fileName == params->fileName);
        m_lastSize = params->size;
        m_offset += m_lastSize;
    }
    subNode.put("<xmlattr>.size", m_lastSize);
}

void CauldronIO::DataStoreSave::writeVolume(const boost::shared_ptr<VolumeData>& volume, bool dataIJK, bool compress)
{
    if (volume->isConstant()) return;
    const float* data = (dataIJK ? volume->getVolumeValues_IJK() : volume->getVolumeValues_KIJ());

    addData(data, volume->getVolumeSize(), compress);
}