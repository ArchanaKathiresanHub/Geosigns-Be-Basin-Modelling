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
    m_hasDepthMap_uuid = false;
    m_params = NULL;
}

void CauldronIO::MapNative::retrieve()
{
    if (isConstant()) return;
    
    DataStoreLoad datastore(m_params);
    
    size_t size;
    float* data = datastore.getData(size);

    if (size != sizeof(float)*getNumI()*getNumJ())
        throw CauldronIOException("Error during decompression of data");
    
    // Geometry should already have been set
    assert(m_geometryAssigned);
    setData_IJ(data);
    delete[] data;
}

void CauldronIO::MapNative::setDataStore(DataStoreParams* params)
{
    m_params = params;
}

void CauldronIO::MapNative::setDepthSurfaceUUID(const boost::uuids::uuid& uuid)
{
    m_uuid_depth = uuid;
    m_hasDepthMap_uuid = true;
}

bool CauldronIO::MapNative::hasDepthMap() const
{
    return m_hasDepthMap_uuid;
}

const boost::uuids::uuid& CauldronIO::MapNative::getDepthSurfaceUUID() const
{
    return m_uuid_depth;
}


const DataStoreParams* CauldronIO::MapNative::getDataStoreParams() const
{
    return m_params;
}

/// VolumeNative implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::VolumeNative::VolumeNative(bool cellCentered, SubsurfaceKind kind, boost::shared_ptr<const Property> property)
    : Volume(cellCentered, kind, property)
{
    m_hasDepthMap_uuid = false;
    m_dataIJK = false;
    m_dataKIJ = false;
    m_paramsIJK = NULL;
    m_paramsKIJ = NULL;
}

void CauldronIO::VolumeNative::retrieve()
{
    if (isConstant()) return;

    if (m_dataIJK)
    {
        DataStoreLoad datastore(m_paramsIJK);

        size_t size;
        float* data = datastore.getData(size);

        if (size != sizeof(float)*getNumI()*getNumJ()*getNumK())
            throw CauldronIOException("Error during decompression of data");

        // Geometry should already have been set
        assert(m_geometryAssigned);
        setData_IJK(data);
        delete[] data;
    }

    if (m_dataKIJ)
    {
        DataStoreLoad datastore(m_paramsKIJ);

        size_t size;
        float* data = datastore.getData(size);

        if (size != sizeof(float)*getNumI()*getNumJ()*getNumK())
            throw CauldronIOException("Error during decompression of data");

        // Geometry should already have been set
        assert(m_geometryAssigned);
        setData_KIJ(data);
        delete[] data;
    }
}

void CauldronIO::VolumeNative::setDataStore(DataStoreParams* params, bool dataIJK)
{
    if (dataIJK)
    {
        m_paramsIJK = params;
        m_dataIJK = true;
    }
    else
    {
        m_paramsKIJ = params;
        m_dataKIJ = true;
    }
}

void CauldronIO::VolumeNative::setDepthSurfaceUUID(const boost::uuids::uuid& uuid)
{
    m_uuid_depth = uuid;
    m_hasDepthMap_uuid = true;
}

bool CauldronIO::VolumeNative::hasDepthMap() const
{
    return m_hasDepthMap_uuid;
}

const boost::uuids::uuid& CauldronIO::VolumeNative::getDepthSurfaceUUID() const
{
    return m_uuid_depth;
}

const DataStoreParams* CauldronIO::VolumeNative::getDataStoreParamsIJK() const
{
    return m_paramsIJK;
}

const DataStoreParams* CauldronIO::VolumeNative::getDataStoreParamsKIJ() const
{
    return m_paramsKIJ;
}

CauldronIO::DiscontinuousVolumeNative::DiscontinuousVolumeNative()
{
    m_hasDepthMap_uuid = false;
}

void CauldronIO::DiscontinuousVolumeNative::setDepthSurfaceUUID(const boost::uuids::uuid& uuid)
{
    m_hasDepthMap_uuid = true;
    m_uuid_depth = uuid;
}

bool CauldronIO::DiscontinuousVolumeNative::hasDepthMap() const
{
    return m_hasDepthMap_uuid;
}

const boost::uuids::uuid& CauldronIO::DiscontinuousVolumeNative::getDepthSurfaceUUID()
{
    return m_uuid_depth;
}
