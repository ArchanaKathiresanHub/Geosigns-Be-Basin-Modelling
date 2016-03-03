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

void CauldronIO::DataStoreLoad::getVolume(pugi::xml_node ptree, boost::shared_ptr<VolumeData> volumeData, const boost::filesystem::path& path)
{
    bool foundSome = false;

    for (pugi::xml_node datastoreNode = ptree.child("datastore"); datastoreNode; datastoreNode = datastoreNode.next_sibling("datastore"))
    {
        foundSome = true;

        VolumeDataNative* volumeNative = dynamic_cast<VolumeDataNative*>(volumeData.get());
        assert(volumeNative);

        // Extract some data
        float undef = datastoreNode.attribute("undef").as_float();
        volumeNative->setUndefinedValue(undef);

        DataStoreParamsNative* paramsNative = new DataStoreParamsNative();
        std::string filename = datastoreNode.attribute("file").value();
        boost::filesystem::path filepath(path);
        filepath /= filename;
        paramsNative->fileName = filepath;

        std::string compression = datastoreNode.attribute("compression").value();
        paramsNative->compressed = compression == "gzip";
        paramsNative->size = (size_t)datastoreNode.attribute("size").as_uint();
        paramsNative->offset = (size_t)datastoreNode.attribute("offset").as_uint();
        bool dataIJK = datastoreNode.attribute("dataIJK").as_bool();

        volumeNative->setDataStore(paramsNative, dataIJK);
    }
    
    if (!foundSome)
        throw CauldronIOException("Could not find datastore in xml node");
}

void CauldronIO::DataStoreLoad::getSurface(pugi::xml_node ptree, boost::shared_ptr<SurfaceData> surfaceData, const boost::filesystem::path& path)
{
    pugi::xml_node datastoreNode = ptree.child("datastore");
    if (!datastoreNode)
        throw CauldronIOException("Could not find datastore in xml node");

    MapNative* mapNative = dynamic_cast<MapNative*>(surfaceData.get());
    assert(mapNative); 
            
    float undef = datastoreNode.attribute("undef").as_float();
    mapNative->setUndefinedValue(undef);

    DataStoreParamsNative* paramsNative = new DataStoreParamsNative();

    std::string filename = datastoreNode.attribute("file").value();
    boost::filesystem::path filepath(path);
    filepath /= filename;
    paramsNative->fileName = filepath;

    std::string compression = datastoreNode.attribute("compression").value();
    paramsNative->compressed = compression == "gzip";
    paramsNative->size = (size_t)datastoreNode.attribute("size").as_uint();
    paramsNative->offset = (size_t)datastoreNode.attribute("offset").as_uint(); 
    mapNative->setDataStore(paramsNative);
}

 /// DataStoreSave
 //////////////////////////////////////////////////////////////////////////

CauldronIO::DataStoreSave::DataStoreSave(const std::string& filename, bool append)
{
    if (!append)
        m_file_out.open(filename.c_str(), std::fstream::binary);
    else
        m_file_out.open(filename.c_str(), std::fstream::binary | std::fstream::ate | std::fstream::app);

    m_compress = APPLY_COMPRESSION;
    m_fileName = filename;
    m_flushed = false;
    m_offset = 0;
}

CauldronIO::DataStoreSave::~DataStoreSave()
{
    if (!m_flushed)
        flush();
    
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
    boost::shared_ptr<DataToCompress> dataToCompress(new DataToCompress(data, size, compressData));
    m_dataToCompress.push_back(dataToCompress);
}

void CauldronIO::DataStoreSave::flush()
{
    for (int i = 0; i < m_dataToCompress.size(); i++)
    {
        boost::shared_ptr<DataToCompress> data = m_dataToCompress.at(i);

        // Compress all data if not done
        if (!data->isProcessed())
            data->compress();

        // Write to file
        m_file_out.write((char*)data->getOutputData(), data->getOutputSizeInBytes());
        data->setOffset(m_offset);
        m_offset += data->getOutputSizeInBytes();

        // Update offset and size in the local node
        data->updateXmlNode();
    }

    m_flushed = true;
}

void CauldronIO::DataStoreSave::addSurface(const boost::shared_ptr<SurfaceData>& surfaceData, pugi::xml_node node)
{
    pugi::xml_node subNode = node.append_child("datastore");
    subNode.append_attribute("file") = boost::filesystem::path(m_fileName).filename().string().c_str();
    subNode.append_attribute("undef") = surfaceData->getUndefinedValue();

    MapNative* mapNative = dynamic_cast<MapNative*>(surfaceData.get());

    size_t numBytes = surfaceData->getGeometry()->getSize() * sizeof(float);
    bool compress = m_compress && numBytes > MINIMALBYTESTOCOMPRESS;
    if (compress)
        subNode.append_attribute("compression") = "gzip";
    else
        subNode.append_attribute("compression") = "none";

    if (surfaceData->isConstant()) throw CauldronIO::CauldronIOException("Cannot write constant value");

    // We write the actual data if 1) this map has been loaded from projecthandle (so mapNative == null)
    // or 2) this map has been created in native format, but was not loaded from disk (so no datastoreparams were set)
    if (mapNative == NULL || (mapNative != NULL && mapNative->getDataStoreParams() == NULL))
    {
        size_t seekPos = m_file_out.tellp();
        addData(surfaceData->getSurfaceValues(), surfaceData->getGeometry()->getNumI() * surfaceData->getGeometry()->getNumJ(), compress);
        m_dataToCompress.back()->setXmlNode(subNode);
    }
    else
    {
        // This surface already has been written: skip it
        const DataStoreParamsNative* const params = static_cast<DataStoreParamsNative const*>(mapNative->getDataStoreParams());
        assert(m_fileName == params->fileName);

        subNode.append_attribute("offset") = (unsigned int)m_offset;
        subNode.append_attribute("size") = (unsigned int)params->size;
        m_offset += params->size;
    }

}

void CauldronIO::DataStoreSave::addVolume(const boost::shared_ptr<VolumeData>& data, pugi::xml_node node, size_t numBytes)
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


std::vector<boost::shared_ptr<DataToCompress> > CauldronIO::DataStoreSave::getDataToCompressList()
{
    return m_dataToCompress;
}

void CauldronIO::DataStoreSave::writeVolumePart(pugi::xml_node volNode, bool compress, bool IJK, const boost::shared_ptr<VolumeData>& volume)
{
    // See if we're writing a native volume
    VolumeDataNative* nativeVolume = dynamic_cast<VolumeDataNative*>(volume.get());

    pugi::xml_node subNode = volNode.append_child("datastore");
    subNode.append_attribute("file") = boost::filesystem::path(m_fileName).filename().string().c_str();
    subNode.append_attribute("undef") = volume->getUndefinedValue();

    if (compress)
        subNode.append_attribute("compression") = "gzip";
    else
        subNode.append_attribute("compression") = "none";
    subNode.append_attribute("dataIJK") = IJK;

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
        m_dataToCompress.back()->setXmlNode(subNode);
    }
    else
    {
        // This volume already has been written: skip it
        const DataStoreParamsNative* const params = static_cast<DataStoreParamsNative const*>(nativeVolume->getDataStoreParamsIJK());
        assert(m_fileName == params->fileName);

        subNode.append_attribute("offset") = (unsigned int)m_offset;
        subNode.append_attribute("size") = (unsigned int)params->size;
        m_offset += params->size;
    }
}

void CauldronIO::DataStoreSave::writeVolume(const boost::shared_ptr<VolumeData>& volume, bool dataIJK, bool compress)
{
    if (volume->isConstant()) return;
    const float* data = (dataIJK ? volume->getVolumeValues_IJK() : volume->getVolumeValues_KIJ());

    addData(data, volume->getGeometry()->getSize(), compress);
}

/// DataToCompress
//////////////////////////////////////////////////////////////////////////

CauldronIO::DataToCompress::DataToCompress(const float* inputData, size_t nrOfElements, bool compress)
{
    m_inputData = inputData;
    m_compress = compress;
    m_inputNrElements = nrOfElements;
    m_outputData = NULL;
    m_processed = false;
    m_node_set = false;
    m_available = true;
}

CauldronIO::DataToCompress::~DataToCompress()
{
    if (m_outputData)
    {
        delete m_outputData;
        m_outputData = NULL;
    }
}

void CauldronIO::DataToCompress::setOffset(size_t offset)
{
    m_offset = offset;
}

void CauldronIO::DataToCompress::compress()
{
    m_outputNrBytes = sizeof(float)*m_inputNrElements;

    if (m_compress)
        m_outputData = (float*)DataStoreSave::compress((char*)m_inputData, m_outputNrBytes);

    m_processed = true;
}

bool CauldronIO::DataToCompress::isProcessed() const
{
    return m_processed;
}

const float* CauldronIO::DataToCompress::getOutputData() const
{
    if (m_outputData)
        return m_outputData;
    else
        return m_inputData;
}

size_t CauldronIO::DataToCompress::getOutputSizeInBytes() const
{
    return m_outputNrBytes;
}

void CauldronIO::DataToCompress::setXmlNode(pugi::xml_node node)
{
    m_node = node;
    m_node_set = true;
}

void CauldronIO::DataToCompress::updateXmlNode()
{
    assert(m_node_set);
    m_node.append_attribute("size") = m_outputNrBytes;
    m_node.append_attribute("offset") = m_offset;
}

boost::thread::id CauldronIO::DataToCompress::getThreadId() const
{
    return m_id;
}

void CauldronIO::DataToCompress::setThreadId(const boost::thread::id& id)
{
    m_available = false;
    m_id = id;
}

bool CauldronIO::DataToCompress::canBeProcessed() const
{
    return m_available;
}
