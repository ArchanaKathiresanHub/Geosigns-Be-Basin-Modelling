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

CauldronIO::MapProjectHandle::MapProjectHandle(bool cellCentered) : Map(cellCentered)
{
    _propVal = NULL;
}

void CauldronIO::MapProjectHandle::retrieve()
{
    if (isRetrieved()) return;

    assert(_propVal != NULL);
    const DataAccess::Interface::GridMap* gridmap = _propVal->getGridMap();

    // Set the geometry
    setGeometry(gridmap->numI(), gridmap->numJ(), gridmap->deltaI(), gridmap->deltaJ(), gridmap->minI(), gridmap->minJ());
    setUndefinedValue((float)gridmap->getUndefinedValue());

    if (gridmap->isConstant())
    {
        setConstantValue((float)gridmap->getConstantValue());
    }
    else
    {
        // TODO: add check on constantness?
        float* mapData = new float[getNumI() * getNumJ()];
        size_t index = 0;
        for (unsigned int j = 0; j < getNumJ(); ++j)
            for (unsigned int i = 0; i < getNumI(); ++i)
                // Store row first
                mapData[index++] = (float)gridmap->getValue(i, j);
        setData_IJ(mapData);
        delete[] mapData;
    }

    _retrieved = true;
}

void CauldronIO::MapProjectHandle::setDataStore(const DataAccess::Interface::PropertyValue* propVal)
{
    _propVal = propVal;
}

CauldronIO::VolumeProjectHandle::VolumeProjectHandle(bool cellCentered, SubsurfaceKind kind, boost::shared_ptr<const Property> property)
    : Volume(cellCentered, kind, property)
{
    _propVal = NULL;
    _depthInfo = NULL;
    _propValues.reset();
    _depthFormations.reset();
}

void CauldronIO::VolumeProjectHandle::retrieve()
{
    if (isRetrieved()) return;
    
    if (_depthFormations && _propValues)
    {
        assert(_propVal == NULL && _depthInfo == NULL);
        retrieveMultipleFormations();
    }
    else if (_propVal != NULL)
    {
        assert(!_depthFormations && !_propValues);
        retrieveSingleFormation();
    }
}

void CauldronIO::VolumeProjectHandle::retrieveMultipleFormations()
{
    // Detect a constant volume consisting of all constant subvolumes (bit extreme case though)
    float constantValue;
    bool isConstant = true;
    bool firstConstant = true;

    const PropertyValue* propVal = _propValues->at(0);
    const GridMap* propGridMap = propVal->getGridMap();

    // Find the total depth size & offset
    assert(_depthFormations->at(0)->kStart == 0);
    size_t maxK = 0;
    size_t minK = std::numeric_limits<size_t>::max();
    for (size_t i = 0; i < _propValues->size(); ++i)
    {
        CauldronIO::FormationInfo* depthInfo = CauldronIO::VolumeProjectHandle::findDepthInfo(_depthFormations, _propValues->at(i)->getFormation());
        // TODO: check if formations are continuous.. (it is assumed now)
        maxK = max(maxK, depthInfo->kEnd);
        minK = min(minK, depthInfo->kStart);
    }
    size_t depthK = 1 + maxK - minK;

    setGeometry(propGridMap->numI(), propGridMap->numJ(), depthK, minK, propGridMap->deltaI(), propGridMap->deltaJ(), propGridMap->minI(), propGridMap->minJ());
    setUndefinedValue((float)propGridMap->getUndefinedValue());

    float* inputData = new float[getNumI() * getNumJ() * depthK];

    // Get data
    for (size_t i = 0; i < _propValues->size(); ++i)
    {
        const GridMap* gridMap = _propValues->at(i)->getGridMap();
        FormationInfo* depthInfo = findDepthInfo(_depthFormations, _propValues->at(i)->getFormation());

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
    }

    // Assign the data
    if (!isConstant)
        setData_IJK(inputData);
    else
        setConstantValue(constantValue);

    _retrieved = true;

    delete[] inputData;
}

void CauldronIO::VolumeProjectHandle::retrieveSingleFormation()
{
    // Find the depth information
    size_t depthK = 1 + _depthInfo->kEnd - _depthInfo->kStart;

    // Set the values
    const GridMap* gridMap = _propVal->getGridMap();
    setGeometry(gridMap->numI(), gridMap->numJ(), depthK, _depthInfo->kStart, gridMap->deltaI(), gridMap->deltaJ(), gridMap->minI(), gridMap->minJ());
    setConstantValue((float)gridMap->getUndefinedValue());

    if (gridMap->isConstant())
    {
        setConstantValue((float)gridMap->getConstantValue());
    }
    else
    {
        float* inputData = new float[getNumI() * getNumJ() * (1 + getLastK() - getFirstK())];

        // Get the volume data for this formation
        assert(gridMap->firstI() == 0 && gridMap->firstJ() == 0 && gridMap->firstK() == 0);

        for (unsigned int k = 0; k <= gridMap->lastK(); ++k)
        {
            size_t kIndex = _depthInfo->reverseDepth ? _depthInfo->kEnd - (size_t)k : _depthInfo->kStart + (size_t)k;
            size_t index = computeIndex_IJK(0, 0, kIndex);

            for (unsigned int j = 0; j <= gridMap->lastJ(); ++j)
            {
                for (unsigned int i = 0; i <= gridMap->lastI(); ++i)
                {
                    float val = (float)gridMap->getValue(i, j, k);
                    inputData[index++] = val;
                }
            }
        }

        setData_IJK(inputData);
        delete[] inputData;
    }
    _retrieved = true;
}

void CauldronIO::VolumeProjectHandle::setDataStore(boost::shared_ptr<DataAccess::Interface::PropertyValueList> propValues,
        boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations)
{
    _propValues = propValues;
    _depthFormations = depthFormations;
}

void CauldronIO::VolumeProjectHandle::setDataStore(const DataAccess::Interface::PropertyValue* propVal, const CauldronIO::FormationInfo* depthFormation)
{
    _propVal = propVal;
    _depthInfo = depthFormation;
}

CauldronIO::FormationInfo* CauldronIO::VolumeProjectHandle::findDepthInfo(boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations, const DataAccess::Interface::Formation* formation)
{
    for (size_t i = 0; i < depthFormations->size(); ++i)
    {
        if (depthFormations->at(i)->formation == formation) return depthFormations->at(i);
    }

    throw CauldronIO::CauldronIOException("Cannot find depth formation for requested formation");
}

bool CauldronIO::FormationInfo::compareFormations(const CauldronIO::FormationInfo* info1, const CauldronIO::FormationInfo* info2)
{
    return info1->depthStart < info2->depthStart;
}
