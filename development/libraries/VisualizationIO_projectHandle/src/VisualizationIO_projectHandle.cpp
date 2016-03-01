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

using namespace DataAccess;
using namespace DataAccess::Interface;

CauldronIO::MapProjectHandle::MapProjectHandle(boost::shared_ptr<const CauldronIO::Geometry2D>& geometry) : SurfaceData(geometry)
{
    m_propVal = NULL;
}

void CauldronIO::MapProjectHandle::retrieve()
{
    if (isRetrieved()) return;

    assert(m_propVal != NULL);
    const DataAccess::Interface::GridMap* gridmap = m_propVal->getGridMap();

    setUndefinedValue((float)gridmap->getUndefinedValue());

    if (gridmap->isConstant())
    {
        setConstantValue((float)gridmap->getConstantValue());
    }
    else
    {
        float constantValue;
        bool isConstant = true;
        bool firstConstant = true;
        float* mapData = new float[m_numI*m_numJ];
        size_t index = 0;

        // Verify sizes
        assert(gridmap->numI() == m_numI && gridmap->numJ() == m_numJ);

        for (unsigned int j = 0; j < m_numJ; ++j)
        { 
            for (unsigned int i = 0; i < m_numI; ++i)
            {
                // Store row first
                float val = (float)gridmap->getValue(i, j);
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
    }

    gridmap->release();

    m_retrieved = true;
}


void CauldronIO::MapProjectHandle::release()
{
    if (!isRetrieved()) return;
    SurfaceData::release();
}

void CauldronIO::MapProjectHandle::setDataStore(const DataAccess::Interface::PropertyValue* propVal)
{
    m_propVal = propVal;
}

CauldronIO::VolumeProjectHandle::VolumeProjectHandle(const boost::shared_ptr<const Geometry3D>& geometry)
    : VolumeData(geometry)
{
    m_propVal = NULL;
    m_depthInfo.reset();
    m_propValues.reset();
    m_depthFormations.reset();
}

void CauldronIO::VolumeProjectHandle::retrieve()
{
    if (isRetrieved()) return;
    
    if (m_depthFormations && m_propValues)
    {
        assert(m_propVal == NULL && m_depthInfo == NULL);
        retrieveMultipleFormations();
    }
    else if (m_propVal != NULL)
    {
        assert(!m_depthFormations && !m_propValues);
        retrieveSingleFormation();
    }
}

void CauldronIO::VolumeProjectHandle::release()
{
    if (!isRetrieved()) return;
    VolumeData::release();
}

void CauldronIO::VolumeProjectHandle::retrieveMultipleFormations()
{
    // Detect a constant volume consisting of all constant subvolumes (bit extreme case though)
    float constantValue;
    bool isConstant = true;
    bool firstConstant = true;

    const PropertyValue* propVal = m_propValues->at(0);
    const GridMap* propGridMap = propVal->getGridMap();

    setUndefinedValue((float)propGridMap->getUndefinedValue());

    float* inputData = new float[m_numI * m_numJ * m_numK];

    // Verify sizes
    assert(propGridMap->numI() == m_numI && propGridMap->numJ());

    // Get data
    for (size_t i = 0; i < m_propValues->size(); ++i)
    {
        const GridMap* gridMap = m_propValues->at(i)->getGridMap();
        boost::shared_ptr<CauldronIO::FormationInfo> depthInfo = findDepthInfo(m_depthFormations, m_propValues->at(i)->getFormation());

        // Get the volume data for this formation
        assert(gridMap->firstI() == 0 && gridMap->firstJ() == 0 && gridMap->firstK() == 0);

        for (unsigned int k = 0; k <= gridMap->lastK(); ++k)
        {
            size_t kIndex = depthInfo->reverseDepth ? depthInfo->kEnd - (size_t)k : depthInfo->kStart + (size_t)k;
            size_t index = computeIndex_IJK(0, 0, kIndex);

            for (unsigned int j = 0; j <= gridMap->lastJ(); ++j)
            {
                for (unsigned int i = 0; i <= gridMap->lastI(); ++i)
                {
                    float val = (float)gridMap->getValue(i, j, k);
                    inputData[index++] = val;

                    if (firstConstant)
                    {
                        constantValue = val;
                        firstConstant = false;
                    }
                    if (isConstant) isConstant = val == constantValue;
                }
            }
        }
        gridMap->release();
    }

    // Assign the data
    if (!isConstant)
        setData_IJK(inputData);
    else
        setConstantValue(constantValue);

    m_retrieved = true;

    delete[] inputData;
}

void CauldronIO::VolumeProjectHandle::retrieveSingleFormation()
{
    // Find the depth information
    size_t depthK = 1 + m_depthInfo->kEnd - m_depthInfo->kStart;

    // Set the values
    const GridMap* gridMap = m_propVal->getGridMap();
    setUndefinedValue((float)gridMap->getUndefinedValue());

    if (gridMap->isConstant())
    {
        setConstantValue((float)gridMap->getConstantValue());
    }
    else
    {
        float constantValue;
        bool isConstant = true;
        bool firstConstant = true;

        float* inputData = new float[m_numI * m_numJ * m_numK];

        // Get the volume data for this formation
        assert(gridMap->firstI() == 0 && gridMap->firstJ() == 0 && gridMap->firstK() == 0);

        for (unsigned int k = 0; k <= gridMap->lastK(); ++k)
        {
            size_t kIndex = m_depthInfo->reverseDepth ? m_depthInfo->kEnd - (size_t)k : m_depthInfo->kStart + (size_t)k;
            size_t index = computeIndex_IJK(0, 0, kIndex);

            for (unsigned int j = 0; j <= gridMap->lastJ(); ++j)
            {
                for (unsigned int i = 0; i <= gridMap->lastI(); ++i)
                {
                    float val = (float)gridMap->getValue(i, j, k);
                    inputData[index++] = val;

                    if (firstConstant)
                    {
                        constantValue = val;
                        firstConstant = false;
                    }
                    if (isConstant) isConstant = val == constantValue;
                }
            }
        }

        // Assign the data
        if (!isConstant)
            setData_IJK(inputData);
        else
            setConstantValue(constantValue);
        delete[] inputData;
    }

    gridMap->release();
    m_retrieved = true;
}

void CauldronIO::VolumeProjectHandle::setDataStore(boost::shared_ptr<DataAccess::Interface::PropertyValueList> propValues,
        boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations)
{
    m_propValues = propValues;
    m_depthFormations = depthFormations;
}

void CauldronIO::VolumeProjectHandle::setDataStore(const DataAccess::Interface::PropertyValue* propVal, boost::shared_ptr<CauldronIO::FormationInfo> depthFormation)
{
    m_propVal = propVal;
    m_depthInfo = depthFormation;
}

boost::shared_ptr<CauldronIO::FormationInfo> CauldronIO::VolumeProjectHandle::findDepthInfo(boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations, const DataAccess::Interface::Formation* formation)
{
    for (size_t i = 0; i < depthFormations->size(); ++i)
    {
        if (depthFormations->at(i)->formation == formation) return depthFormations->at(i);
    }

    throw CauldronIO::CauldronIOException("Cannot find depth formation for requested formation");
}

bool CauldronIO::FormationInfo::compareFormations(boost::shared_ptr<CauldronIO::FormationInfo> info1, boost::shared_ptr<CauldronIO::FormationInfo> info2)
{
    return info1->depthStart < info2->depthStart;
}
