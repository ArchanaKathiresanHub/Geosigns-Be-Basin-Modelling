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

CauldronIO::MapNative::MapNative(const boost::shared_ptr<const Geometry2D>& geometry) : SurfaceData(geometry)
{
    m_params = NULL;
}


CauldronIO::MapNative::~MapNative()
{
    if (m_params)
    {
        delete m_params;
        m_params = NULL;
    }
}

void CauldronIO::MapNative::retrieve()
{
    if (isConstant()) return;
    
    DataStoreLoad datastore(m_params);
    
    size_t size;
    float* data = datastore.getData(size);

    if (size != sizeof(float)*m_numI*m_numJ)
        throw CauldronIOException("Error during decompression of data");
    
    setData_IJ(data);
    delete[] data;
}

void CauldronIO::MapNative::setDataStore(DataStoreParams* params)
{
    m_params = params;
}

const DataStoreParams* CauldronIO::MapNative::getDataStoreParams() const
{
    return m_params;
}

/// VolumeNative implementation
//////////////////////////////////////////////////////////////////////////


CauldronIO::VolumeDataNative::VolumeDataNative(const boost::shared_ptr<const Geometry3D>& geometry)
    : VolumeData(geometry)
{
    m_dataIJK = false;
    m_dataKIJ = false;
    m_paramsIJK = NULL;
    m_paramsKIJ = NULL;
}

CauldronIO::VolumeDataNative::~VolumeDataNative()
{
    if (m_paramsIJK)
    {
        delete m_paramsIJK;
        m_paramsIJK = NULL;
    }
    if (m_paramsKIJ)
    {
        delete m_paramsKIJ;
        m_paramsKIJ = NULL;
    }
}

void CauldronIO::VolumeDataNative::retrieve()
{
    if (isConstant()) return;

    if (m_dataIJK)
    {
        DataStoreLoad datastore(m_paramsIJK);

        size_t size;
        float* data = datastore.getData(size);

        if (size != sizeof(float)*m_numI*m_numJ*m_numK)
            throw CauldronIOException("Error during decompression of data");

        // Geometry should already have been set
        setData_IJK(data);
        delete[] data;
    }

    if (m_dataKIJ)
    {
        DataStoreLoad datastore(m_paramsKIJ);

        size_t size;
        float* data = datastore.getData(size);

        if (size != sizeof(float)*m_numI*m_numJ*m_numK)
            throw CauldronIOException("Error during decompression of data");

        // Geometry should already have been set
        setData_KIJ(data);
        delete[] data;
    }
}

void CauldronIO::VolumeDataNative::setDataStore(DataStoreParams* params, bool dataIJK)
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

const DataStoreParams* CauldronIO::VolumeDataNative::getDataStoreParamsIJK() const
{
    return m_paramsIJK;
}

const DataStoreParams* CauldronIO::VolumeDataNative::getDataStoreParamsKIJ() const
{
    return m_paramsKIJ;
}