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
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/foreach.hpp>

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
    m_data_uncompressed = NULL;
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

    m_data_uncompressed = NULL;
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

char* CauldronIO::DataStoreLoad::decompress_lz4(const char* inputData, size_t& compressedSize, size_t uncompressedSize)
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

    for (pugi::xml_node datastoreNode = ptree.child("datastore"); datastoreNode; datastoreNode = datastoreNode.next_sibling("datastore"))
    {
        foundSome = true;

        VolumeDataNative* volumeNative = dynamic_cast<VolumeDataNative*>(volumeData.get());
        assert(volumeNative);

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
        bool dataIJK = datastoreNode.attribute("dataIJK").as_bool();

        volumeNative->setDataStore(paramsNative, dataIJK);
    }
    
    if (!foundSome)
        throw CauldronIOException("Could not find datastore in xml node");
}

void CauldronIO::DataStoreLoad::getSurface(pugi::xml_node ptree, std::shared_ptr<SurfaceData> surfaceData, const ibs::FilePath& path)
{
    pugi::xml_node datastoreNode = ptree.child("datastore");
    if (!datastoreNode)
        throw CauldronIOException("Could not find datastore in xml node");

    MapNative* mapNative = dynamic_cast<MapNative*>(surfaceData.get());
    assert(mapNative); 
            
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
    int inputSize = (int)elements;
    int maxOutputSize = inputSize;
    
    char* dest = new char[maxOutputSize];
    elements = (size_t)LZ4_compress_default(inputData, dest, (int)elements, maxOutputSize);

    if (elements == 0) // failed to compress
    {
        delete[] dest;
        elements = (size_t)inputSize;
        return NULL; 
    }

    return dest;
}

void CauldronIO::DataStoreSave::addData(const float* data, size_t size, bool compressData)
{
    std::shared_ptr<DataToCompress> dataToCompress(new DataToCompress(data, size, compressData));
    m_dataToCompress.push_back(dataToCompress);
}

void CauldronIO::DataStoreSave::flush()
{
    for (int i = 0; i < m_dataToCompress.size(); i++)
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
    if (mapNative == NULL || (mapNative != NULL && mapNative->getDataStoreParams() == NULL))
    {
        size_t seekPos = m_file_out.tellp();
        addData(surfaceData->getSurfaceValues(), surfaceData->getGeometry()->getNumI() * surfaceData->getGeometry()->getNumJ(), compress);
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
    if (data->hasDataIJK() || (nativeVolume != NULL && nativeVolume->getDataStoreParamsIJK() != NULL))
        writeVolumePart(node, compress, true, data);

    if (data->hasDataKIJ() || (nativeVolume != NULL && nativeVolume->getDataStoreParamsKIJ() != NULL))
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

    if (compress)
        subNode.append_attribute("compression") = COMPRESSION_LZ4 ? "lz4" : "gzip";
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
        const DataStoreParams* const params = nativeVolume->getDataStoreParamsIJK();
        assert(m_fileName == params->fileName.path());

        subNode.append_attribute("offset") = (unsigned int)m_offset;
        subNode.append_attribute("size") = (unsigned int)params->size;
        m_offset += params->size;
    }
}

void CauldronIO::DataStoreSave::writeVolume(const std::shared_ptr<VolumeData>& volume, bool dataIJK, bool compress)
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
    m_node_set = false;
    m_processed = false;
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

    if (m_compress && !COMPRESSION_LZ4)
        m_outputData = (float*)DataStoreSave::compress((char*)m_inputData, m_outputNrBytes);
    else if (m_compress && COMPRESSION_LZ4)
        m_outputData = (float*)DataStoreSave::compress_lz4((char*)m_inputData, m_outputNrBytes);

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
    m_node.append_attribute("size") = (unsigned int)m_outputNrBytes;
    m_node.append_attribute("offset") = (unsigned int)m_offset;

    // If compression failed reset compression (none)
    if (m_compress && m_outputData == NULL)
        m_node.attribute("compression") = "none";
}