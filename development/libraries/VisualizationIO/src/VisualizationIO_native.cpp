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

CauldronIO::MapNative::MapNative(const std::shared_ptr<const Geometry2D>& geometry) : SurfaceData(geometry)
{
    m_params = NULL;
    m_dataStore = NULL;
}

CauldronIO::MapNative::~MapNative()
{
    if (m_params)
    {
        delete m_params;
        m_params = NULL;
    }
}

void CauldronIO::MapNative::prefetch()
{
    // Load from disk, do not decompress
    if (!m_dataStore)
    {
        m_dataStore = new DataStoreLoad(m_params);
        m_dataStore->prefetch();
    }
}

bool CauldronIO::MapNative::retrieve()
{
    if (isConstant()) return true;

    prefetch();

    size_t size = sizeof(float)*m_numI*m_numJ;
    float* data = m_dataStore->getData(size);

    delete m_dataStore;
    m_dataStore = NULL;

    setData_IJ(data);
    delete[] data;

    return true;
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


CauldronIO::VolumeDataNative::VolumeDataNative(const std::shared_ptr<Geometry3D>& geometry)
    : VolumeData(geometry)
{
    m_dataIJK = false;
    m_dataKIJ = false;
    m_paramsIJK = NULL;
    m_paramsKIJ = NULL;
    m_dataStoreIJK = NULL;
    m_dataStoreKIJ = NULL;
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

void CauldronIO::VolumeDataNative::prefetch()
{
    if (m_dataIJK && !m_dataStoreIJK)
    {
        // Load from disk, do not decompress
        m_dataStoreIJK = new DataStoreLoad(m_paramsIJK);
        m_dataStoreIJK->prefetch();
    }
    if (m_dataKIJ && !m_dataStoreKIJ)
    {
        // Load from disk, do not decompress
        m_dataStoreKIJ = new DataStoreLoad(m_paramsKIJ);
        m_dataStoreKIJ->prefetch();
    }
}

bool CauldronIO::VolumeDataNative::retrieve()
{
    if (isConstant()) return true;

    if (m_dataIJK)
    {
        prefetch();

        size_t size = sizeof(float)*m_numI*m_numJ*m_numK;
        float* data = m_dataStoreIJK->getData(size);
        
        // Close filehandles etc.
        delete m_dataStoreIJK;
        m_dataStoreIJK = NULL;

        // Geometry should already have been set
        setData_IJK(data);
        delete[] data;
    }

    if (m_dataKIJ)
    {
        prefetch();

        size_t size = sizeof(float)*m_numI*m_numJ*m_numK;
        float* data = m_dataStoreKIJ->getData(size);

        // Close filehandles etc.
        delete m_dataStoreKIJ;
        m_dataStoreKIJ = NULL;

        // Geometry should already have been set
        setData_KIJ(data);
        delete[] data;
    }

    return true;
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