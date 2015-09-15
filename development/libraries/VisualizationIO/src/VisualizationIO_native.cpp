//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "VisualizationIO_native.h"
#include "DataStore.h"

using namespace CauldronIO;

CauldronIO::MapNative::MapNative(bool cellCentered) : Map(cellCentered)
{
}

void CauldronIO::MapNative::retrieve()
{
    if (!boost::filesystem::exists(m_filename))
        throw CauldronIOException("Cannot retrieve file for reading");
    
    DataStore datastore(m_filename, m_compressed, false);
    
    size_t size = m_size;
    float* data = datastore.getData(m_offset, size);

    if (size != sizeof(float)*getNumI()*getNumJ())
        throw CauldronIOException("Error during decompression of data");
    
    // Geometry should already have been set
    assert(m_geometryAssigned);
    setData_IJ(data);
    delete[] data;
}

// TODO: datastore should prepare a little struct with data that it can consume,
// the actual parameters needed to load data later should be hidden from this client
void CauldronIO::MapNative::setDataStore(const std::string& filename, bool compressed, size_t offset, size_t size)
{
    m_filename = filename;
    m_compressed = compressed;
    m_offset = offset;
    m_size = size;
}

void CauldronIO::MapNative::setDepthSurfaceUUID(const boost::uuids::uuid& uuid)
{
    m_uuid_depth = uuid;
}

CauldronIO::VolumeNative::VolumeNative(bool cellCentered, SubsurfaceKind kind, boost::shared_ptr<const Property> property)
    : Volume(cellCentered, kind, property)
{
}

void CauldronIO::VolumeNative::retrieve()
{
    throw CauldronIOException("Not implemented");
}

void CauldronIO::VolumeNative::setDataStore(const std::string& filename, bool compressed, size_t offset, size_t size)
{
    m_filename = filename;
    m_compressed = compressed;
    m_offset = offset;
    m_size = size;
}
