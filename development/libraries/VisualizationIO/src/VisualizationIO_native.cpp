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

CauldronIO::MapNative::MapNative(const std::shared_ptr<const Geometry2D>& geometry, float minValue, float maxValue)
    : SurfaceData(geometry, minValue, maxValue)
{
    m_params = nullptr;
    m_dataStore = nullptr;
}

CauldronIO::MapNative::~MapNative()
{
    if (m_params)
    {
        delete m_params;
        m_params = nullptr;
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

void CauldronIO::MapNative::retrieve()
{
    if (isConstant()) return;

    prefetch();

    size_t size = sizeof(float)*m_numI*m_numJ;
    float* data = m_dataStore->getData(size);

    delete m_dataStore;
    m_dataStore = nullptr;

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


CauldronIO::VolumeDataNative::VolumeDataNative(const std::shared_ptr<Geometry3D>& geometry, float minValue, float maxValue)
    : VolumeData(geometry, minValue, maxValue)
{
    m_dataIJK = false;
    m_dataKIJ = false;
    m_paramsIJK = nullptr;
    m_paramsKIJ = nullptr;
    m_dataStoreIJK = nullptr;
    m_dataStoreKIJ = nullptr;
}

CauldronIO::VolumeDataNative::~VolumeDataNative()
{
    if (m_paramsIJK)
    {
        delete m_paramsIJK;
        m_paramsIJK = nullptr;
    }
    if (m_paramsKIJ)
    {
        delete m_paramsKIJ;
        m_paramsKIJ = nullptr;
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

void CauldronIO::VolumeDataNative::retrieve()
{
    if (isConstant()) return;

    if (m_dataIJK)
    {
        prefetch();

        size_t size = sizeof(float)*m_numI*m_numJ*m_numK;
        float* data = m_dataStoreIJK->getData(size);
        
        // Close filehandles etc.
        delete m_dataStoreIJK;
        m_dataStoreIJK = nullptr;

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
        m_dataStoreKIJ = nullptr;

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

// Reference volume implementation
//////////////////////////////////////////////////////////////////

CauldronIO::ReferenceVolume::ReferenceVolume(const std::shared_ptr<Geometry3D>& geometry, float minValue /*= DefaultUndefinedValue*/,
	float maxValue /*= DefaultUndefinedValue*/)
	: VolumeData(geometry, minValue, maxValue)
{
	m_paramsIJK = nullptr;
	m_paramsKIJ = nullptr;
	m_dataIJK = false;
	m_dataKIJ = false;
}

void CauldronIO::ReferenceVolume::setDataStore(const DataStoreParams* params, bool dataIJK)
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

const DataStoreParams* CauldronIO::ReferenceVolume::getDataStoreParamsIJK() const
{
	return m_paramsIJK;
}

const DataStoreParams* CauldronIO::ReferenceVolume::getDataStoreParamsKIJ() const
{
	return m_paramsKIJ;
}

// Reference map implementation
//////////////////////////////////////////////////////////////////

CauldronIO::ReferenceMap::ReferenceMap(const std::shared_ptr<const Geometry2D>& geometry, float minValue /*= DefaultUndefinedValue*/, float maxValue /*= DefaultUndefinedValue*/)
	: SurfaceData(geometry, minValue, maxValue)
{
	m_params = nullptr;
}

void CauldronIO::ReferenceMap::setDataStore(const DataStoreParams* params)
{
	m_params = params;
}

const CauldronIO::DataStoreParams* CauldronIO::ReferenceMap::getDataStoreParams() const
{
	return m_params;
}

