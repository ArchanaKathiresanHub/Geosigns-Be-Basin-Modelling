//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4244 4996)
// boost/iostreams/copy.hpp(128):
//   warning C4244: 'argument': conversion from 'std::streamsize' to 'int', possible loss of data
//
// C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\include\xutility(2372)
//   warning C4996: 'std::copy::_Unchecked_iterators::_Deprecate': Call to 'std::copy' with parameters that may be unsafe
//   this call relies on the caller to check that the passed values are correct. To disable this warning,
//   use -D_SCL_SECURE_NO_WARNINGS. See documentation on how to use Visual C++ 'Checked Iterators'

// Visual Studio warning C4996 comes from a security check on iterators and std::copy from boost, which uses
// raw pointers, so to disable it algorithm header has to be not included yet in this scope
#endif
#ifdef _ALGORITHM_
#error algorithm header has already been included before boost (see comments)
#endif
#include <algorithm>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/foreach.hpp>
#ifdef _MSC_VER
#pragma warning (pop)
#endif

#include "DataStore.h"
#include "VisualizationAPI.h"
#include "VisualizationIO_native.h"

#include "lz4.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <streambuf>

#define MINIMALBYTESTOCOMPRESS 50
#define APPLY_COMPRESSION true
#define COMPRESSION_LZ4 true

using namespace CauldronIO;

/// DataStoreLoad
//////////////////////////////////////////////////////////////////////////

CauldronIO::DataStoreLoad::DataStoreLoad(DataStoreParams* params)
{
    m_params = static_cast<DataStoreParams*>(params);
    
    if (!m_params->fileName.exists())
        throw CauldronIOException("Cannot retrieve file for reading");

    m_file_in.open(m_params->fileName.cpath(), std::fstream::in | std::fstream::binary);
    m_data_uncompressed = nullptr;
}

CauldronIO::DataStoreLoad::~DataStoreLoad()
{
    m_file_in.close();

    // No need to destroy the parameters; they are not owned by us
}

float* CauldronIO::DataStoreLoad::getData(size_t& uncompressedSize)
{
    if (!m_data_uncompressed)
        prefetch();

    size_t compressedSize = m_params->size;
    float* result = (float*)m_data_uncompressed;

    if (m_params->compressed && !m_params->compressed_lz4)
    {
        char* resultChar = decompress(m_data_uncompressed, compressedSize);
        result = (float*)resultChar;
        delete[] m_data_uncompressed;

        if (compressedSize != uncompressedSize)
            throw CauldronIOException("Error during gzip decompression");
    }
    else if (m_params->compressed_lz4)
    {
        char* resultChar = decompress_lz4(m_data_uncompressed, compressedSize, uncompressedSize);
        result = (float*)resultChar;
        delete[] m_data_uncompressed;
    }
    else
        uncompressedSize = compressedSize;

    m_data_uncompressed = nullptr;
    return result;
}

void CauldronIO::DataStoreLoad::prefetch()
{
    if (!m_data_uncompressed)
    {
        size_t compressedSize = m_params->size;
        m_file_in.seekg(m_params->offset);
        m_data_uncompressed = new char[compressedSize];
        m_file_in.read(m_data_uncompressed, compressedSize);
    }
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

char* CauldronIO::DataStoreLoad::decompress_lz4(const char* inputData, size_t compressedSize, size_t uncompressedSize)
{
    char* dest = new char[uncompressedSize];
    size_t actualCompressedSize = (size_t)LZ4_decompress_fast(inputData, dest, (int)uncompressedSize);

    if (compressedSize != actualCompressedSize)
        throw CauldronIOException("Error during decompression");

    return dest;
}

void CauldronIO::DataStoreLoad::getVolume(pugi::xml_node ptree, std::shared_ptr<VolumeData> volumeData, const ibs::FilePath& path)
{
    bool foundSome = false;

    // There can be two datastores for a volume (one per orientation)
    for (pugi::xml_node datastoreNode = ptree.child("datastore"); datastoreNode; datastoreNode = datastoreNode.next_sibling("datastore"))
    {
        foundSome = true;

        VolumeDataNative* volumeNative = dynamic_cast<VolumeDataNative*>(volumeData.get());
        assert(volumeNative);

        DataStoreParams* paramsNative = getDatastoreParams(datastoreNode, path);
        bool dataIJK = datastoreNode.attribute("dataIJK").as_bool();

        volumeNative->setDataStore(paramsNative, dataIJK);
    }
    
    if (!foundSome)
        throw CauldronIOException("Could not find datastore in xml node");
}


CauldronIO::DataStoreParams* CauldronIO::DataStoreLoad::getDatastoreParams(pugi::xml_node &datastoreNode, const ibs::FilePath& path)
{
    DataStoreParams* paramsNative = new DataStoreParams();

    // Check for partial path
    pugi::xml_attribute partialPath = datastoreNode.attribute("partialpath");
    if (!partialPath || partialPath.as_bool())
    {
        paramsNative->fileName = path;
        paramsNative->fileName << datastoreNode.attribute("file").value();
    }
    else
    {
        paramsNative->fileName = ibs::FilePath(datastoreNode.attribute("file").value());
    }

    std::string compression = datastoreNode.attribute("compression").value();
    paramsNative->compressed = compression == "gzip" || compression == "lz4";
    paramsNative->compressed_lz4 = compression == "lz4";
    paramsNative->size = (size_t)datastoreNode.attribute("size").as_uint();
    paramsNative->offset = (size_t)datastoreNode.attribute("offset").as_uint();

    return paramsNative;
}

void CauldronIO::DataStoreLoad::getSurface(pugi::xml_node ptree, std::shared_ptr<SurfaceData> surfaceData, const ibs::FilePath& path)
{
    pugi::xml_node datastoreNode = ptree.child("datastore");
    if (!datastoreNode)
        throw CauldronIOException("Could not find datastore in xml node");

    MapNative* mapNative = dynamic_cast<MapNative*>(surfaceData.get());
    assert(mapNative); 
            
    DataStoreParams* paramsNative = getDatastoreParams(datastoreNode, path);
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
        ibs::FilePath(m_fileName).remove();
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

char* CauldronIO::DataStoreSave::compress_lz4(const char* inputData, size_t& elements)
{
    size_t inputSize = elements;
    size_t maxOutputSize = inputSize;
    
    char* dest = new char[maxOutputSize];
    elements = (size_t)LZ4_compress_default(inputData, dest, (int)elements, (int)maxOutputSize);

    if (elements == 0 || (elements == inputSize)) // failed to compress
    {
        delete[] dest;
        elements = inputSize;
        return nullptr;
    }

    return dest;
}

void CauldronIO::DataStoreSave::addData(const float* data, size_t size, bool compressData)
{
    std::shared_ptr<DataToCompress> dataToCompress(new DataToCompress(data, size * sizeof(float), compressData));
    m_dataToCompress.push_back(dataToCompress);
}

void CauldronIO::DataStoreSave::flush()
{
    for (size_t i = 0; i < m_dataToCompress.size(); i++)
    {
        std::shared_ptr<DataToCompress> data = m_dataToCompress.at(i);

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

void CauldronIO::DataStoreSave::addSurface(const std::shared_ptr<SurfaceData>& surfaceData, pugi::xml_node node)
{
    pugi::xml_node subNode = node.append_child("datastore");
    subNode.append_attribute("file") = ibs::FilePath(m_fileName).fileName().c_str();

    MapNative* mapNative = dynamic_cast<MapNative*>(surfaceData.get());

    size_t numBytes = surfaceData->getGeometry()->getSize() * sizeof(float);
    bool compress = m_compress && numBytes > MINIMALBYTESTOCOMPRESS;
    if (compress)
        subNode.append_attribute("compression") = COMPRESSION_LZ4 ? "lz4" : "gzip";
    else
        subNode.append_attribute("compression") = "none";

    if (surfaceData->isConstant()) throw CauldronIO::CauldronIOException("Cannot write constant value");

    // We write the actual data if 1) this map has been loaded from projecthandle (so mapNative == null)
    // or 2) this map has been created in native format, but was not loaded from disk (so no datastoreparams were set)
    if (mapNative == nullptr || (mapNative != nullptr && mapNative->getDataStoreParams() == nullptr))
    {
        addData(surfaceData->getSurfaceValues(), surfaceData->getGeometry()->getSize(), compress);
        m_dataToCompress.back()->setXmlNode(subNode);
    }
    else
    {
        // This surface already has been written: skip it
        const DataStoreParams* const params = mapNative->getDataStoreParams();
        assert(m_fileName == params->fileName.path());

        subNode.append_attribute("offset") = (unsigned int)m_offset;
        subNode.append_attribute("size") = (unsigned int)params->size;
        m_offset += params->size;
    }
}

void CauldronIO::DataStoreSave::addVolume(const std::shared_ptr<VolumeData>& data, pugi::xml_node node, size_t numBytes)
{
    bool compress = m_compress && numBytes > MINIMALBYTESTOCOMPRESS;
    VolumeDataNative* nativeVolume = dynamic_cast<VolumeDataNative*>(data.get());

    // We need to check if this volume has IJK data, and account for the case it was not retrieved yet;
    // in that case we need to write the meta data again to XML, without writing the volume data itself.
    // To check for that, it should be a native volume, with datastore parameters set.
    if (data->hasDataIJK() || (nativeVolume != nullptr && nativeVolume->getDataStoreParamsIJK() != nullptr))
        writeVolumePart(node, compress, true, data);

    if (data->hasDataKIJ() || (nativeVolume != nullptr && nativeVolume->getDataStoreParamsKIJ() != nullptr))
        writeVolumePart(node, compress, false, data);
}

std::vector<std::shared_ptr<DataToCompress> > CauldronIO::DataStoreSave::getDataToCompressList()
{
    return m_dataToCompress;
}

void CauldronIO::DataStoreSave::writeVolumePart(pugi::xml_node volNode, bool compress, bool IJK, const std::shared_ptr<VolumeData>& volume)
{
    // See if we're writing a native volume
    VolumeDataNative* nativeVolume = dynamic_cast<VolumeDataNative*>(volume.get());

    pugi::xml_node subNode = volNode.append_child("datastore");
    subNode.append_attribute("file") = ibs::FilePath(m_fileName).fileName().c_str();

    subNode.append_attribute("dataIJK") = IJK;

    // We write the actual data if 1) this volume has been loaded from projecthandle (so nativeVolume == nullptr)
    // or 2) this volume has been created in native format, but was not loaded from disk (so no datastoreparams were set)
    bool writeData;
    if (IJK)
        writeData = nativeVolume == nullptr || nativeVolume->getDataStoreParamsIJK() == nullptr;
    else 
        writeData = nativeVolume == nullptr || nativeVolume->getDataStoreParamsKIJ() == nullptr;

    if (writeData)
    {
       if (compress)
          subNode.append_attribute("compression") = COMPRESSION_LZ4 ? "lz4" : "gzip";
       else
          subNode.append_attribute("compression") = "none";
        // Write the volume and update the offset
        writeVolume(volume, IJK, compress);
        m_dataToCompress.back()->setXmlNode(subNode);
    }
    else
    {
        // This volume already has been written: skip it
        const DataStoreParams* const params = nativeVolume->getDataStoreParamsIJK();
        assert(m_fileName == params->fileName.path());

        if ( params->compressed )
           subNode.append_attribute("compression") =  params->compressed_lz4 ? "lz4" : "gzip";
        else
           subNode.append_attribute("compression") = "none";
        
        subNode.append_attribute("offset") = (unsigned int)m_offset;
        subNode.append_attribute("size") = (unsigned int)params->size;
        m_offset += params->size;
    }
}

void CauldronIO::DataStoreSave::addData(void* data, pugi::xml_node node, size_t numBytes)
{
    pugi::xml_node subNode = node.append_child("datastore");
    subNode.append_attribute("file") = ibs::FilePath(m_fileName).fileName().c_str();

    bool compress = m_compress && numBytes > MINIMALBYTESTOCOMPRESS;
    if (compress)
        subNode.append_attribute("compression") = COMPRESSION_LZ4 ? "lz4" : "gzip";
    else
        subNode.append_attribute("compression") = "none";

    std::shared_ptr<DataToCompress> dataToCompress(new DataToCompress(data, numBytes, compress));
    m_dataToCompress.push_back(dataToCompress);
    m_dataToCompress.back()->setXmlNode(subNode);
}

void CauldronIO::DataStoreSave::writeVolume(const std::shared_ptr<VolumeData>& volume, bool dataIJK, bool compress)
{
    if (volume->isConstant()) return;
    const float* data = (dataIJK ? volume->getVolumeValues_IJK() : volume->getVolumeValues_KIJ());

    addData(data, volume->getGeometry()->getSize(), compress);
}

/// DataToCompress
//////////////////////////////////////////////////////////////////////////

CauldronIO::DataToCompress::DataToCompress(const void* inputData, size_t numBytes, bool compress)
{
    m_inputData = inputData;
    m_compress = compress;
    m_inputSize = numBytes;
    m_outputData = nullptr;
    m_node_set = false;
    m_processed = false;
}

CauldronIO::DataToCompress::~DataToCompress()
{
    if (m_outputData)
    {
        delete[] (char*)m_outputData;
        m_outputData = nullptr;
    }
}

void CauldronIO::DataToCompress::setOffset(size_t offset)
{
    m_offset = offset;
}

void CauldronIO::DataToCompress::compress()
{
    // Nothing to do if this has been processed
    if (m_processed) return;

    m_outputNrBytes = m_inputSize;

    if (m_compress && !COMPRESSION_LZ4)
        m_outputData = (void*)DataStoreSave::compress((char*)m_inputData, m_outputNrBytes);
    else if (m_compress && COMPRESSION_LZ4)
        m_outputData = (void*)DataStoreSave::compress_lz4((char*)m_inputData, m_outputNrBytes);

    m_processed = true;
}


bool CauldronIO::DataToCompress::isProcessed() const
{
    return m_processed;
}

const void* CauldronIO::DataToCompress::getOutputData() const
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
    m_node.append_attribute("size") = (unsigned int)m_outputNrBytes;
    m_node.append_attribute("offset") = (unsigned int)m_offset;

    // If compression failed reset compression (none)
    if (m_compress && m_outputData == nullptr)
        m_node.attribute("compression") = "none";
}
