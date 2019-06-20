//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <limits>

#include "VisualizationIO_projectHandle.h"
#include "FilePath.h"
#include "Property.h"
#include "InputValue.h"

using namespace DataAccess;
using namespace DataAccess::Interface;

CauldronIO::MapProjectHandle::MapProjectHandle(std::shared_ptr<const CauldronIO::Geometry2D>& geometry) : SurfaceData(geometry)
{
	m_propVal = nullptr;
	m_inputVal = nullptr;
}

CauldronIO::MapProjectHandle::~MapProjectHandle()
{
    release();
}

void CauldronIO::MapProjectHandle::prefetch()
{
}

void CauldronIO::MapProjectHandle::retrieve()
{
    if (isRetrieved()) return;

    if (!signalNewHDFdata())
        throw CauldronIOException("Cannot retrieve data, HDF data not available");
    else
        retrieveFromHDF();
}

void CauldronIO::MapProjectHandle::retrieve(const ArrayView<float> & /*buffer*/) const
{
  throw CauldronIOException("Not implemented");
}

void CauldronIO::MapProjectHandle::release()
{
    if (!isRetrieved()) return;

    if (m_info.size() > 0)
    {
		assert(m_info.size() == 1);
		if (m_info[0]->getData() != nullptr)
		{
			delete[] m_info[0]->getData();
		}
    }
    m_info.clear();

    SurfaceData::release();
}

std::shared_ptr<CauldronIO::HDFinfo > CauldronIO::MapProjectHandle::getHDFinfoForPropVal(const DataAccess::Interface::PropertyValue* propVal)
{
    std::shared_ptr<CauldronIO::HDFinfo > info(new CauldronIO::HDFinfo());
    string filename, datasetname, fulloutputdir;
    propVal->getHDFinfo(filename, datasetname, fulloutputdir);

    ibs::FilePath filePathName(fulloutputdir);
    filePathName << filename;

    info->dataSetName = datasetname;
    info->filepathName = filePathName.path();

    return info;
}

void CauldronIO::MapProjectHandle::retrieveFromHDF()
{
    assert(signalNewHDFdata());
    assert(m_info.size() == 1 && m_info[0]->getData() != nullptr);
    float* hdfData = m_info[0]->getData(); 

    float constantValue;
    bool isConstant = true;
    bool firstConstant = true;
    float* mapData = new float[m_numI * m_numJ];
    size_t index = 0;

    assert(m_numI == m_info[0]->numI);
    assert(m_numJ == m_info[0]->numJ);

    for (unsigned int j = 0; j < m_numJ; ++j)
    {
        for (unsigned int i = 0; i < m_numI; ++i)
        {
            // Store row first; indexing in HDF data is columnfirst (j + i * m_numJ)
            float val = hdfData[j + i * m_numJ];
            mapData[index++] = val;

            if (firstConstant)
            {
                constantValue = val;
                firstConstant = false;
            }

            if (isConstant) isConstant = val == constantValue;
        }
    }

    if (!isConstant)
        setData_IJ(mapData);
    else
        setConstantValue(constantValue);

    delete[] mapData;

    m_retrieved = true;
}

const std::vector < std::shared_ptr<CauldronIO::HDFinfo> >& CauldronIO::MapProjectHandle::getHDFinfo()
{
    if (m_info.size() > 0)
        return m_info;

    if (m_propVal != nullptr)
    {
        std::shared_ptr <CauldronIO::HDFinfo > info = MapProjectHandle::getHDFinfoForPropVal(m_propVal);
        info->parent = this;
        info->indexSub = -1;
        info->setData(nullptr);
        m_info.push_back(info);
    }
	else if (m_inputVal != nullptr)
	{
		std::shared_ptr <CauldronIO::HDFinfo > info(new CauldronIO::HDFinfo());
		m_inputVal->getHDFinfo(info->filepathName, info->dataSetName);
		info->parent = this;
		info->indexSub = -1;
		info->setData(nullptr);
		m_info.push_back(info);
	}
    
    return m_info;
}

void CauldronIO::MapProjectHandle::setDataStore(const DataAccess::Interface::PropertyValue* propVal)
{
    m_propVal = propVal;
}


void CauldronIO::MapProjectHandle::setDataStore(const DataAccess::Interface::InputValue* inputVal)
{
	m_inputVal = inputVal;
}

bool CauldronIO::MapProjectHandle::signalNewHDFdata()
{
    if (m_info.size() == 0) return false;
    
    for (int i = 0; i < m_info.size(); i++)
        if (m_info[i]->getData() == nullptr) return false;

    return true;
}

CauldronIO::VolumeProjectHandle::VolumeProjectHandle(const std::shared_ptr<Geometry3D>& geometry)
    : VolumeData(geometry)
{
    m_propVal = nullptr;
    m_depthInfo.reset();
    m_propValues.reset();
    m_depthFormations.reset();
}

CauldronIO::VolumeProjectHandle::~VolumeProjectHandle()
{
    release();
}

void CauldronIO::VolumeProjectHandle::prefetch()
{
    // Unused...
}

void CauldronIO::VolumeProjectHandle::retrieve()
{
    if (isRetrieved()) return;

    if (m_depthFormations && m_propValues)
    {
        if (!signalNewHDFdata())
            throw CauldronIOException("Cannot retrieve data, HDF binary not read yet");
        else
            // Read from prefetched HDF
            retrieveMultipleFromHDF();
    }
    else if (m_propVal != nullptr)
    {
        if (!signalNewHDFdata())
            throw CauldronIOException("Cannot retrieve data, HDF binary not read yet");
        else
            // Read from prefetched HDF
            retrieveSingleFromHDF();
    }
}

void CauldronIO::VolumeProjectHandle::retrieve(const ArrayView<float> & /*buffer*/) const
{
  throw CauldronIOException("Not implemented");
}

void CauldronIO::VolumeProjectHandle::release()
{
    if (m_info.size() > 0)
    {
        for (int i = 0; i < m_info.size(); i++)
        {
            if (m_info[i]->getData() != nullptr)
                delete[] m_info[i]->getData();
        }
    }
    m_info.clear();

    if (!isRetrieved()) return;
    VolumeData::release();
}

const std::vector < std::shared_ptr<CauldronIO::HDFinfo> >& CauldronIO::VolumeProjectHandle::getHDFinfo() 
{
    if (m_info.size() > 0)
        return m_info;

    if (m_depthFormations && m_propValues)
    {
        assert(m_propVal == nullptr && m_depthInfo == nullptr);
        for (int i = 0; i < m_propValues->size(); i++)
        {
            std::shared_ptr <CauldronIO::HDFinfo > info = MapProjectHandle::getHDFinfoForPropVal(m_propValues->at(i));
            info->parent = this;
            info->indexSub = i;
            info->setData(nullptr);
            m_info.push_back(info);
        }
    }
    else if (m_propVal != nullptr)
    {
        assert(!m_depthFormations && !m_propValues);
        std::shared_ptr <CauldronIO::HDFinfo > info = MapProjectHandle::getHDFinfoForPropVal(m_propVal);
        info->parent = this;
        info->indexSub = -1;
        info->setData(nullptr);
        m_info.push_back(info);
    }

    return m_info;
}

void CauldronIO::VolumeProjectHandle::retrieveMultipleFromHDF()
{
    assert(signalNewHDFdata());
    assert(m_info.size() >= 1 && m_info[0]->getData() != nullptr);

    // Detect a constant volume consisting of all constant subvolumes (bit extreme case though)
    float constantValue;
    bool isConstant = true;
    bool firstConstant = true;
    float* inputData = new float[m_numI * m_numJ * m_numK];

    assert(m_numI == m_info[0]->numI);
    assert(m_numJ == m_info[0]->numJ);

    // Make sure all k-range is accounted for
    size_t detected_minK = std::numeric_limits<size_t>::max();
    size_t detected_maxK = 0;

    // Determine range of values
    const float fltMax = std::numeric_limits<float>::max();
    float minValue =  fltMax;
    float maxValue = -fltMax;
    float sedimentMinValue =  fltMax;
    float sedimentMaxValue = -fltMax;

    // Get data
    for (size_t i = 0; i < m_info.size(); ++i)
    {
        shared_ptr<HDFinfo>& info = m_info[i];
        float* hdfData = info->getData(); // we don't need to dispose it here

        std::shared_ptr<CauldronIO::FormationInfo> depthInfo = findDepthInfo(m_depthFormations, m_propValues->at(i)->getFormation());
        size_t thisNumK = 1 + depthInfo->kEnd - depthInfo->kStart;

	// Find local min and max values for each formation, and combine them later into
	// the minValue / maxValue and sedimentMinValue / sedimentMaxValue
	float localMinValue =  fltMax;
	float localMaxValue = -fltMax;

        for (size_t k = 0; k <thisNumK; ++k)
        {
            size_t hdfKIndex = (thisNumK - 1) - k;  /// in the HDF file, depth is always inverse to k index
            /// in a SerialGridmap, depth is aligned (=increasing) with k index
            /// in a DistributedGridmap, depth is inverse to k index
            size_t kIndex = depthInfo->reverseDepth ? depthInfo->kEnd - k : depthInfo->kStart + k;
            size_t index = computeIndex_IJK(0, 0, kIndex);  // this will account for an offset (kStart), but will not inverse anything
            // in a single volume, it will not have any effect

            detected_maxK = max(detected_maxK, kIndex);
            detected_minK = min(detected_minK, kIndex);

            for (unsigned int j = 0; j < m_numJ; ++j)
            {
                for (unsigned int i = 0; i < m_numI; ++i)
                {
                    float val = hdfData[hdfKIndex + thisNumK * j + i * thisNumK * m_numJ];
                    inputData[index++] = val;

		    if(val != DefaultUndefinedValue)
		    {
		      localMinValue = std::min(localMinValue, val);
		      localMaxValue = std::max(localMaxValue, val);
		    }

                    if (firstConstant)
                    {
                        constantValue = val;
                        firstConstant = false;
                    }
                    if (isConstant) isConstant = val == constantValue;
                }
            }

	    minValue = std::min(minValue, localMinValue);
	    maxValue = std::max(maxValue, localMaxValue);

	    if(depthInfo->formation->kind() == DataAccess::Interface::SEDIMENT_FORMATION)
	    {
	      sedimentMinValue = std::min(sedimentMinValue, localMinValue);
	      sedimentMaxValue = std::max(sedimentMaxValue, localMaxValue);
	    }
        }
    }

    // Verify full coverage of data: check for missing end or beginning of this volume
    /////////////////////////////////////////////////////////////////////////////////////
    if (!isConstant)
    {
        // Correct if only first k index is wrong
        if (detected_minK > m_geometry->getFirstK())
        {
            assert(detected_maxK == m_geometry->getLastK());
            m_geometry->updateK_range(detected_minK, 1 + detected_maxK - detected_minK);
            updateGeometry();
            setData_IJK(inputData + computeIndex_IJK(0, 0, detected_minK));
        }
        // Correct if only last k index is wrong
        else if (detected_maxK < m_geometry->getLastK())
        {
            assert(detected_minK == m_geometry->getFirstK());
            m_geometry->updateK_range(detected_minK, 1 + detected_maxK - detected_minK);
            updateGeometry();
            setData_IJK(inputData);
        }
        else
        {
            setData_IJK(inputData);
        }
    }
    else
        setConstantValue(constantValue);

    if(minValue != fltMax && maxValue != -fltMax)
      setMinMax(minValue, maxValue);

    if(sedimentMinValue != fltMax && sedimentMaxValue != -fltMax)
      setSedimentMinMax(sedimentMinValue, sedimentMaxValue);

    m_retrieved = true;

    delete[] inputData;
}

void CauldronIO::VolumeProjectHandle::retrieveSingleFromHDF()
{
    assert(signalNewHDFdata());
    assert(m_info.size() == 1 && m_info[0]->getData() != nullptr);
    float* hdfData = m_info[0]->getData();

    float constantValue;
    bool isConstant = true;
    bool firstConstant = true;

    // Check our dimensions
    if (m_numI != m_info[0]->numI || m_numJ != m_info[0]->numJ)
    {
        m_geometry->updateIJ_range(m_info[0]->numI, m_info[0]->numJ);
        updateGeometry();
    }
    assert(m_numK == m_info[0]->numK);

    float* inputData = new float[m_numI * m_numJ * m_numK];

    const float fltMax = std::numeric_limits<float>::max();
    float minValue = fltMax;
    float maxValue = -fltMax;

    for (unsigned int k = 0; k < m_numK; ++k)
    {
        size_t hdfKIndex = (m_numK - 1) - k;  /// in the HDF file, depth is always inverse to k index
                                              /// in a serialdataaccess gridmap, depth is aligned (=increasing) with k index
                                              /// in a distributeddataaccess gridmap, depth is inverse to k index
        size_t kIndex = m_depthInfo->reverseDepth ? m_depthInfo->kEnd - (size_t)k : m_depthInfo->kStart + (size_t)k;
        size_t index = computeIndex_IJK(0, 0, kIndex);  // this will account for an offset (kStart), but will not inverse anything
                                                        // in a single volume, it will not have any effect

        for (unsigned int j = 0; j < m_numJ; ++j)
        {
            for (unsigned int i = 0; i < m_numI; ++i)
            {
                float val = hdfData[hdfKIndex + m_numK * j + i * m_numK * m_numJ];
                inputData[index++] = val;

		if(val != DefaultUndefinedValue)
		{
		  minValue = std::min(minValue, val);
		  maxValue = std::max(maxValue, val);
		}

                if (firstConstant)
                {
                    constantValue = val;
                    firstConstant = false;
                }
                if (isConstant) isConstant = val == constantValue;
            }
        }
    }

    if(minValue != fltMax && maxValue != -fltMax)
    {
      setMinMax(minValue, maxValue);

      if(m_depthInfo->formation->kind() == DataAccess::Interface::SEDIMENT_FORMATION)
	setSedimentMinMax(minValue, maxValue);
    }

    // Assign the data
    if (!isConstant)
        setData_IJK(inputData);
    else
        setConstantValue(constantValue);
    delete[] inputData;
}

void CauldronIO::VolumeProjectHandle::setDataStore(std::shared_ptr<DataAccess::Interface::PropertyValueList> propValues,
        std::shared_ptr<CauldronIO::FormationInfoList> depthFormations)
{
    m_propValues = propValues;
    m_depthFormations = depthFormations;
}


bool CauldronIO::VolumeProjectHandle::signalNewHDFdata()
{
    if (m_info.size() == 0) return false;
    
    for (int i = 0; i < m_info.size(); i++)
        if (m_info[i]->getData() == nullptr) return false;

    return true;
}

void CauldronIO::VolumeProjectHandle::setDataStore(const DataAccess::Interface::PropertyValue* propVal, std::shared_ptr<CauldronIO::FormationInfo> depthFormation)
{
    m_propVal = propVal;
    m_depthInfo = depthFormation;
}

std::shared_ptr<CauldronIO::FormationInfo> CauldronIO::VolumeProjectHandle::findDepthInfo(std::shared_ptr<CauldronIO::FormationInfoList> depthFormations, const DataAccess::Interface::Formation* formation)
{
    for (size_t i = 0; i < depthFormations->size(); ++i)
    {
        if (depthFormations->at(i)->formation == formation) return depthFormations->at(i);
    }

    throw CauldronIO::CauldronIOException("Cannot find depth formation for requested formation");
}
