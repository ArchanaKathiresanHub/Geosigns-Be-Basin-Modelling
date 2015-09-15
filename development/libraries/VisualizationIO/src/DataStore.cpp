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

//#include "half.hpp"

#include <boost/filesystem.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/copy.hpp>

#include <iostream>
#include <fstream>
#include <cstring>
#include <strstream>

using namespace CauldronIO;

CauldronIO::DataStore::DataStore(const std::string& filename, bool compress, bool write)
{
    // TODO: split in two separate classes!
    if (write)
        m_file_out.open(filename, BOOST_IOS::binary);
    else
        m_file_in.open(filename, BOOST_IOS::binary);

    m_offset = 0;
    m_lastSize = 0;
    m_fileName = filename;
    m_compress = compress;
    m_write = write;
}

CauldronIO::DataStore::~DataStore()
{
    if (m_write)
    {
        m_file_out.flush();
        m_file_out.close();

        // Delete if empty
        if (m_offset == 0)
            boost::filesystem::remove(m_fileName);
    }
    else
    {
        m_file_in.close();
    }
}

size_t CauldronIO::DataStore::getOffset() const
{
    return m_offset;
}

float* CauldronIO::DataStore::getData(size_t offset, size_t& size)
{
    m_file_in.seekg(offset);
    char* data = new char[size];
    m_file_in.read(data, size);

    float* result = (float*)data;

    if (m_compress)
    {
        char* resultChar = decompress(data, size);
        result = (float*)resultChar;
        delete[] data;
    }

    return result;
}


char* CauldronIO::DataStore::decompress(const char* inputData, size_t& elements)
{
    std::strstreambuf data(inputData, elements);
    std::strstream result;

    boost::iostreams::filtering_streambuf<boost::iostreams::input> out;
    out.push(boost::iostreams::gzip_decompressor());
    out.push(data);
    elements = boost::iostreams::copy(out, result);

    // Copy the result into a char array
    std::strstreambuf* pbuf = result.rdbuf();
    pbuf->pubseekpos(0);
    char* charResult = new char[elements];
    pbuf->sgetn(charResult, elements);

    return charResult;
}


char* CauldronIO::DataStore::compress(const char* inputData, size_t& elements)
{
    std::strstreambuf data(inputData, elements);
    std::strstream result;

    boost::iostreams::filtering_streambuf<boost::iostreams::input> out;
    out.push(boost::iostreams::gzip_compressor());
    out.push(data);
    elements = boost::iostreams::copy(out, result);

    // Copy the result into a char array
    std::strstreambuf* pbuf = result.rdbuf();
    pbuf->pubseekpos(0);
    char* charResult = new char[elements];
    pbuf->sgetn(charResult, elements);

    return charResult;
}

void CauldronIO::DataStore::addData(const float* data, size_t size, float undef)
{
    char* dataToWrite = (char*)data;
    size_t sizeToWrite = sizeof(float)*size;

    std::vector<char> compressed;

    // TODO: we should not compress data < 50 bytes
    if (m_compress)
        dataToWrite = compress(dataToWrite, sizeToWrite);

    m_file_out.write((char*)dataToWrite, sizeToWrite);
    m_offset += sizeToWrite;
    m_lastSize = sizeToWrite;

    if (m_compress)
        delete[] dataToWrite;
}

template <typename T>
void CauldronIO::DataStore::getStatistics(const T* data, size_t size, T undef)
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

void CauldronIO::DataStore::addSurface(const boost::shared_ptr<Surface>& surfaceIO, boost::property_tree::ptree& ptree)
{
    boost::property_tree::ptree& subNode = ptree.add("datastore", "");
    subNode.put("<xmlattr>.file", getFileName());
    subNode.put("<xmlattr>.format", "float");
    subNode.put("<xmlattr>.row-ordered", "yes");
    if (m_compress)
        subNode.put("<xmlattr>.compression", "gzip");
    else
        subNode.put("<xmlattr>.compression", "none");
    subNode.put("<xmlattr>.offset", getOffset());

    boost::shared_ptr<Map> map = surfaceIO->getValueMap();
    if (map->isConstant()) throw CauldronIO::CauldronIOException("Cannot write constant value");
    addData(map->getSurfaceValues(), map->getNumI()*map->getNumJ(), map->getUndefinedValue());

    subNode.put("<xmlattr>.size", getLastSize());
}

void CauldronIO::DataStore::addVolume(const boost::shared_ptr<Volume>& volume, boost::property_tree::ptree& volNode)
{
    if (volume->isConstant())
    {
        volNode.put("constantvalue", volume->getConstantValue());
        return;
    }

    if (volume->hasDataIJK())
    {
        boost::property_tree::ptree& subNode = volNode.add("datastore", "");
        subNode.put("<xmlattr>.file", getFileName());
        subNode.put("<xmlattr>.format", "float");
        if (m_compress)
            subNode.put("<xmlattr>.compression", "gzip");
        else
            subNode.put("<xmlattr>.compression", "none");
        subNode.put("<xmlattr>.offset", getOffset());
        subNode.put("<xmlattr>.dataIJK", true);
        subNode.put("<xmlattr>.dataKIJ", false);
        // Write the volume and update the offset
        writeVolume(volume, true);
        subNode.put("<xmlattr>.size", getLastSize());
    }

    if (volume->hasDataKIJ())
    {
        boost::property_tree::ptree& subNode = volNode.add("datastore", "");
        subNode.put("<xmlattr>.file", getFileName());
        subNode.put("<xmlattr>.format", "float");
        if (m_compress)
            subNode.put("<xmlattr>.compression", "gzip");
        else
            subNode.put("<xmlattr>.compression", "none");
        subNode.put("<xmlattr>.offset", getOffset());
        subNode.put("<xmlattr>.dataIJK", false);
        subNode.put("<xmlattr>.dataKIJ", true);

        // Write the volume and update the offset
        writeVolume(volume, false);
        subNode.put("<xmlattr>.size", getLastSize());
    }
}

void CauldronIO::DataStore::writeVolume(const boost::shared_ptr<Volume>& volume, bool dataIJK)
{
    if (volume->isConstant()) return;
    const float* data = (dataIJK ? volume->getVolumeValues_IJK() : volume->getVolumeValues_KIJ());

    addData(data, volume->getNumI()*volume->getNumJ()*volume->getNumK(), volume->getUndefinedValue());
}

const std::string& CauldronIO::DataStore::getFileName() const
{
    return m_fileName;
}

size_t CauldronIO::DataStore::getLastSize() const
{
    return m_lastSize;
}