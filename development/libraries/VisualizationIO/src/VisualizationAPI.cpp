//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "VisualizationAPI.h"
#include <assert.h>
#include <new>
#include <algorithm>
#include <cstring>
#include <boost/foreach.hpp>

using namespace CauldronIO;
using namespace std;

/// Project Implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Project::Project(const string& name, const string& description, const string& team, const string& version, ModellingMode mode)
{
    _name = name;
    _description = description;
    _team = team;
    _version = version;
    _mode = mode;
}

CauldronIO::Project::~Project()
{
    // Delete all snapshots
    _snapShotList.clear();
}

void CauldronIO::Project::addSnapShot(boost::shared_ptr<SnapShot>& newSnapShot)
{
    if (!newSnapShot) throw CauldronIOException("Cannot add empty snapshot");
    
    // Check if snapshot exists
    BOOST_FOREACH(boost::shared_ptr<SnapShot>& snapShot, _snapShotList)
        if (snapShot == newSnapShot) throw CauldronIOException("Cannot add snapshot twice");

    _snapShotList.push_back(newSnapShot);
    }

const string& CauldronIO::Project::getName() const
{
    return _name;
}

const string& CauldronIO::Project::getDescription() const
{
    return _description;
}

const string& CauldronIO::Project::getTeam() const
{
    return _team;
}

const string& CauldronIO::Project::getProgramVersion() const
{
    return _version;
}

CauldronIO::ModellingMode CauldronIO::Project::getModelingMode() const
{
    return _mode;
}

const SnapShotList& CauldronIO::Project::getSnapShots() const
{
    return _snapShotList;
}

void CauldronIO::Project::retrieve()
{
    BOOST_FOREACH(boost::shared_ptr<SnapShot>& snapShot, _snapShotList)
        snapShot->retrieve();
}

/// SnapShot implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::SnapShot::SnapShot(double age, SnapShotKind kind, bool isMinorShapshot)
{
    _age = age;
    _kind = kind;
    _isMinor = isMinorShapshot;
}

CauldronIO::SnapShot::~SnapShot()
{
    // Delete all snapshots
    _volumeList.clear();
    _discVolumeList.clear();
    _surfaceList.clear();
}

void CauldronIO::SnapShot::addSurface(boost::shared_ptr<Surface>& newSurface)
{
    if (!newSurface) throw CauldronIOException("Cannot add empty surface");

    // Check if surface exists
    BOOST_FOREACH(boost::shared_ptr<Surface>& surface, _surfaceList)
        if (surface == newSurface) throw CauldronIOException("Cannot add surface twice");

    _surfaceList.push_back(newSurface);
    }

void CauldronIO::SnapShot::addVolume(boost::shared_ptr<Volume>& newVolume)
{
    if (!newVolume) throw CauldronIOException("Cannot add empty volume");

    // Check if snapshot exists
    BOOST_FOREACH(boost::shared_ptr<Volume>& volume, _volumeList)
        if (volume == newVolume) throw CauldronIOException("Cannot add surface twice");

    _volumeList.push_back(newVolume);
    }

void CauldronIO::SnapShot::addDiscontinuousVolume(boost::shared_ptr<DiscontinuousVolume>& newDiscVolume)
{
    if (!newDiscVolume) throw CauldronIOException("Cannot add empty volume");

    // Check if snapshot exists
    BOOST_FOREACH(boost::shared_ptr<DiscontinuousVolume>& volume, _discVolumeList)
        if (volume == newDiscVolume) throw CauldronIOException("Cannot add surface twice");

    _discVolumeList.push_back(newDiscVolume);
}

double CauldronIO::SnapShot::getAge() const
{
    return _age;
}

CauldronIO::SnapShotKind CauldronIO::SnapShot::getKind() const
{
    return _kind;
}

bool CauldronIO::SnapShot::isMinorShapshot() const
{
    return _isMinor;
}

const SurfaceList& CauldronIO::SnapShot::getSurfaceList() const
{
    return _surfaceList;
}

const VolumeList& CauldronIO::SnapShot::getVolumeList() const
{
    return _volumeList;
}

const DiscontinuousVolumeList& CauldronIO::SnapShot::getDiscontinuousVolumeList() const
{
    return _discVolumeList;
}

void CauldronIO::SnapShot::retrieve()
{
    BOOST_FOREACH(boost::shared_ptr<DiscontinuousVolume>& volume, _discVolumeList)
        volume->retrieve();
    BOOST_FOREACH(boost::shared_ptr<Volume>& volume, _volumeList)
        volume->retrieve();
    BOOST_FOREACH(boost::shared_ptr<Surface>& surface, _surfaceList)
        surface->retrieve();
}

/// Property implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Property::Property(const string& name, const string& username, const string& cauldronName, const string& unit, PropertyType type, PropertyAttribute attrib)
{
    _name = name;
    _username = username;
    _cauldronName = cauldronName;
    _unit = unit;
    _type = type;
    _attrib = attrib;
}

const string& CauldronIO::Property::getName() const
{
    return _name;
}

const string& CauldronIO::Property::getUserName() const
{
    return _username;
}

const string& CauldronIO::Property::getCauldronName() const
{
    return _cauldronName;
}

const string& CauldronIO::Property::getUnit() const
{
    return _unit;
}

CauldronIO::PropertyType CauldronIO::Property::getType() const
{
    return _type;
}

CauldronIO::PropertyAttribute CauldronIO::Property::getAttribute() const
{
    return _attrib;
}

/// Formation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Formation::Formation(size_t kStart, size_t kEnd, const string& name)
{
    _kstart = kStart;
    _kend = kEnd;
    _name = name;
}

void CauldronIO::Formation::getDepthRange(size_t &start, size_t &end) const
{
    start = _kstart;
    end = _kend;
}

const string& CauldronIO::Formation::getName() const
{
    return _name;
}

/// Surface implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Surface::Surface(const string& name, SubsurfaceKind kind, 
                             boost::shared_ptr<const Property> property, boost::shared_ptr<Map> valueMap)
{
    _name = name;
    _subSurfaceKind = kind;
    _property = property;
    _valueMap = valueMap;
    _depthSurface.reset();
}

const string& CauldronIO::Surface::getName() const
{
    return _name;
}

const boost::shared_ptr<Map> CauldronIO::Surface::getValueMap() const
{
    return _valueMap;
}

CauldronIO::SubsurfaceKind CauldronIO::Surface::getSubSurfaceKind() const
{
    return _subSurfaceKind;
}

const boost::shared_ptr<const Property> CauldronIO::Surface::getProperty() const
{
    return _property;
}

void CauldronIO::Surface::setFormation(boost::shared_ptr<const Formation> formation)
{
    _formation = formation;
}

const boost::shared_ptr<const Formation> CauldronIO::Surface::getFormation() const
{
    return _formation;
}

void CauldronIO::Surface::setDepthSurface(boost::shared_ptr<const Surface> surface)
{
    _depthSurface = surface;
}

const boost::shared_ptr<const Surface> CauldronIO::Surface::getDepthSurface() const
{
    return _depthSurface;
}

void CauldronIO::Surface::retrieve()
{
    _valueMap->retrieve();
}

bool CauldronIO::Surface::isRetrieved() const
{
    return _valueMap->isRetrieved();
}

/// Map implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Map::Map(bool cellCentered)
{
    // Indexing into the map is unknown
    _internalData = NULL;
    _isCellCentered = cellCentered;
    _isConstant = false;
    _retrieved = false;
}

size_t CauldronIO::Map::getNumI() const
{
    return _numI;
}

size_t CauldronIO::Map::getNumJ() const
{
    return _numJ;
}

double CauldronIO::Map::getDeltaI() const
{
    return _deltaI;
}

double CauldronIO::Map::getDeltaJ() const
{
    return _deltaJ;
}

double CauldronIO::Map::getMinI() const
{
    return _minI;
}

double CauldronIO::Map::getMinJ() const
{
    return _minJ;
}

double CauldronIO::Map::getMaxI() const
{
    return _maxI;
}

double CauldronIO::Map::getMaxJ() const
{
    return _maxJ;
}

float CauldronIO::Map::getUndefinedValue() const
{
    return _undefinedValue;
}


void CauldronIO::Map::setUndefinedValue(float undefined)
{
    _undefinedValue = undefined;
}

bool CauldronIO::Map::isConstant() const
{
    return _isConstant;
}

void CauldronIO::Map::setConstantValue(float value)
{
    _isConstant = true;
    _constantValue = value;
    _retrieved = true;
}

CauldronIO::Map::~Map()
{
    if (_internalData) delete[] _internalData;
    _internalData = NULL;
}

void CauldronIO::Map::setData_IJ(float* data)
{
    setData(data);
}

void CauldronIO::Map::setData(float* data, bool setValue, float value)
{
    if (!_geometryAssigned) throw CauldronIOException("Cannot assign data without geometry known");

    // If our data buffer exists, we will just reuse it. Otherwise, allocate
    if (!_internalData)
    {
        try
        {
            _internalData = new float[_numI * _numJ];
        }
        catch (std::bad_alloc& ba)
        {
            throw CauldronIOException(ba.what());
        }
    }

    if (!setValue)
    {
        if (!data) throw CauldronIOException("Cannot set data from empty buffer");

        // copy the data: this will not throw an exception if it fails
        std::memcpy(_internalData, data, sizeof(float) * _numI * _numJ);
    }
    // assign a value if necessary
    else
    {
        std::fill(_internalData, _internalData + _numI * _numJ, value);
    }

    _retrieved = true;
}

void CauldronIO::Map::setGeometry(size_t numI, size_t numJ, double deltaI, double deltaJ, double minI, double minJ)
{
    _numI = numI;
    _numJ = numJ;
    _deltaI = deltaI;
    _deltaJ = deltaJ;
    _minI = minI;
    _minJ = minJ;
    _maxI = minI + deltaI * numI;
    _maxJ = minJ + deltaJ * numJ;
    _geometryAssigned = true;
}

bool CauldronIO::Map::canGetRow() const
{
    return true;
}

bool CauldronIO::Map::canGetColumn() const
{
    // Not implemented at this point
    return false;
}

bool CauldronIO::Map::isUndefined(size_t i, size_t j) const
{
    if (!_geometryAssigned) throw CauldronIOException("Need to assign a geometry first!");
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (_isConstant) return _constantValue == _undefinedValue;

    return _internalData[getMapIndex(i, j)] == _undefinedValue;
}

float CauldronIO::Map::getValue(size_t i, size_t j) const
{
    if (!_geometryAssigned) throw CauldronIOException("Need to assign a geometry first!");
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (_isConstant) return _constantValue;
    
    return _internalData[getMapIndex(i, j)];
}

float const * CauldronIO::Map::getRowValues(size_t j)
{
    if (!_geometryAssigned) throw CauldronIOException("Need to assign a geometry first!");
    if (!canGetRow()) throw CauldronIOException("Cannot return row values");
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");

    // Create our internal buffer if not existing
    if (!_internalData && _isConstant) setData(NULL, true, _constantValue);

    return _internalData + getMapIndex(0, j);
}

float const * CauldronIO::Map::getColumnValues(size_t i)
{
    if (!_geometryAssigned) throw CauldronIOException("Need to assign a geometry first!");
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (!canGetColumn()) throw CauldronIOException("Cannot return column values");

    // Create our internal buffer if not existing
    if (!_internalData && _isConstant) setData(NULL, true, _constantValue);

    return _internalData + getMapIndex(i, 0);
}

float const * CauldronIO::Map::getSurfaceValues()
{
    if (!_geometryAssigned) throw CauldronIOException("Need to assign a geometry first!");
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");

    // Create our internal buffer if not existing
    if (!_internalData && _isConstant) setData(NULL, true, _constantValue);

    return _internalData;
}

size_t CauldronIO::Map::getMapIndex(size_t i, size_t j) const
{
    assert(_geometryAssigned);
    assert(i >= 0 && i < _numI && j >= 0 && j < _numJ);
    return _numI * j + i;
}

void CauldronIO::Map::retrieve()
{
    if (_retrieved) return;
    throw CauldronIOException("Not implemented");
}

bool CauldronIO::Map::isRetrieved() const
{
    return _retrieved;
}

const boost::uuids::uuid& CauldronIO::Map::getUUID() const
{
    return _uuid;
}

void CauldronIO::Map::setUUID(const boost::uuids::uuid& uuid)
{
    _uuid = uuid;
}

float CauldronIO::Map::getConstantValue() const
{
    if (!isConstant()) throw CauldronIOException("Map does not have a constant value");
    return _constantValue;
}

/// Volume implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Volume::Volume(bool cellCentered, SubsurfaceKind kind, boost::shared_ptr<const Property> property)
{
    // Indexing into the volume is unknown
    _internalDataIJK = NULL;
    _internalDataKIJ = NULL;
    _isCellCentered = cellCentered;
    _isConstant = false;
    _subSurfaceKind = kind;
    _property = property;
}

CauldronIO::Volume::~Volume()
{
    if (_internalDataIJK) delete[] _internalDataIJK;
    if (_internalDataKIJ) delete[] _internalDataKIJ;
    _internalDataIJK = NULL;
    _internalDataKIJ = NULL;
}

const boost::shared_ptr<const Property> CauldronIO::Volume::getProperty() const
{
    return _property;
}

void CauldronIO::Volume::setData_KIJ(float* data, bool setValue /*= false*/, float value /*= 0*/)
{
    setData(data, &_internalDataKIJ, setValue, value);
}

void CauldronIO::Volume::setData_IJK(float* data, bool setValue /*= false*/, float value /*= 0*/)
{
    setData(data, &_internalDataIJK, setValue, value);
}

void CauldronIO::Volume::setData(float* data, float** internalData, bool setValue /*= false*/, float value /*= 0*/)
{
    if (!_geometryAssigned) throw CauldronIOException("Cannot assign data without geometry known");

    // If our data buffer exists, we will just reuse it. Otherwise, allocate
    if (!*internalData)
    {
        try
        {
            *internalData = new float[_numI * _numJ * _numK];
        }
        catch (std::bad_alloc& ba)
        {
            throw CauldronIOException(ba.what());
        }
    }

    if (!setValue)
    {
        if (!data) throw CauldronIOException("Cannot set data from empty buffer");

        // copy the data: this will not throw an exception if it fails
        memcpy(*internalData, data, sizeof(float) * _numI * _numJ * _numK);
    }
    // assign a value if necessary
    else
    {
        std::fill(*internalData, *internalData + _numI * _numJ * _numK, value);
    }

    _retrieved = true;
}

void CauldronIO::Volume::setGeometry(size_t numI, size_t numJ, size_t numK, size_t offsetK, double deltaI,
    double deltaJ, double minI, double minJ)
{
    _numI = numI;
    _numJ = numJ;
    _numK = numK;
    _deltaI = deltaI;
    _deltaJ = deltaJ;
    _maxI = minI + deltaI * numI;
    _maxJ = minJ + deltaJ * numJ;
    _minI = minI;
    _minJ = minJ;
    _firstK = offsetK;
    _geometryAssigned = true;
    _retrieved = false;
}

size_t CauldronIO::Volume::getNumI() const
{
    return _numI;
}

size_t CauldronIO::Volume::getNumJ() const
{
    return _numJ;
}

double CauldronIO::Volume::getDeltaI() const
{
    return _deltaI;
}

double CauldronIO::Volume::getDeltaJ() const
{
    return _deltaJ;
}

double CauldronIO::Volume::getMinI() const
{
    return _minI;
}

double CauldronIO::Volume::getMinJ() const
{
    return _minJ;
}

double CauldronIO::Volume::getMaxI() const
{
    return _maxI;
}

double CauldronIO::Volume::getMaxJ() const
{
    return _maxJ;
}


bool CauldronIO::Volume::hasDataIJK() const
{
    return _internalDataIJK != NULL;
}


bool CauldronIO::Volume::hasDataKIJ() const
{
    return _internalDataKIJ != NULL;
}

float CauldronIO::Volume::getUndefinedValue() const
{
    return _undefinedValue;
}

bool CauldronIO::Volume::isConstant() const
{
    return _isConstant;
}

void CauldronIO::Volume::setConstantValue(float value)
{
    _isConstant = true;
    _constantValue = value;
    _retrieved = true;
}

size_t CauldronIO::Volume::getNumK() const
{
    return _numK;
}

size_t CauldronIO::Volume::getFirstK() const
{
    return _firstK;
}

size_t CauldronIO::Volume::getLastK() const
{
    return _firstK + _numK - 1;
}

bool CauldronIO::Volume::isUndefined(size_t i, size_t j, size_t k) const
{
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (_isConstant) return _constantValue == _undefinedValue;
    if (_internalDataIJK) return _internalDataIJK[computeIndex_IJK(i, j, k)] == _undefinedValue;
    
    assert(_internalDataKIJ);
    return _internalDataKIJ[computeIndex_KIJ(i, j, k)] == _undefinedValue;
}


void CauldronIO::Volume::setUndefinedValue(float undefined)
{
    _undefinedValue = undefined;
}

float CauldronIO::Volume::getValue(size_t i, size_t j, size_t k) const
{
    if (!_geometryAssigned) throw CauldronIOException("Need to assign a geometry first!");
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (_isConstant) return _constantValue;
    if (_internalDataIJK) return _internalDataIJK[computeIndex_IJK(i, j, k)];

    assert(_internalDataKIJ);
    return _internalDataKIJ[computeIndex_KIJ(i, j, k)];
}

float const * CauldronIO::Volume::getRowValues(size_t j, size_t k)
{
    if (!_geometryAssigned) throw CauldronIOException("Need to assign a geometry first!");
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (!hasDataIJK()) throw CauldronIOException("Cannot return row values");

    // Create our internal buffer if not existing
    if (!_internalDataIJK && _isConstant) setData_IJK(NULL, true, _constantValue);

    // Assume IJK ordering
    assert(_internalDataIJK);
    return _internalDataIJK + computeIndex_IJK(0, j, k);
}

float const * CauldronIO::Volume::getColumnValues(size_t i, size_t k)
{
    throw CauldronIOException("Not implemented");
}

float const * CauldronIO::Volume::getNeedleValues(size_t i, size_t j)
{
    if (!_geometryAssigned) throw CauldronIOException("Need to assign a geometry first!");
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (!hasDataKIJ() || isConstant()) throw CauldronIOException("Cannot return needle values");

    // Create our internal buffer if not existing
    if (!_internalDataKIJ && _isConstant) setData_KIJ(NULL, true, _constantValue);

    return _internalDataKIJ + computeIndex_KIJ(i, j, _firstK);
}

float const * CauldronIO::Volume::getSurface_IJ(size_t k)
{
    if (!_geometryAssigned) throw CauldronIOException("Need to assign a geometry first!");
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (!hasDataIJK() || isConstant()) throw CauldronIOException("Cannot return surface values");

    // Create our internal buffer if not existing
    if (!_internalDataIJK && _isConstant) setData_IJK(NULL, true, _constantValue);

    return _internalDataIJK + computeIndex_IJK(0, 0, k);
}

float const * CauldronIO::Volume::getVolumeValues_KIJ()
{
    if (!_geometryAssigned) throw CauldronIOException("Need to assign a geometry first!");
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (!hasDataKIJ() || isConstant()) throw CauldronIOException("Cannot return volume values");

    // Create our internal buffer if not existing
    if (!_internalDataKIJ && _isConstant) setData_KIJ(NULL, true, _constantValue);

    return _internalDataKIJ;
}

float const * CauldronIO::Volume::getVolumeValues_IJK()
{
    if (!_geometryAssigned) throw CauldronIOException("Need to assign a geometry first!");
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (!hasDataIJK() || isConstant()) throw CauldronIOException("Cannot return volume values");

    // Create our internal buffer if not existing
    if (!_internalDataIJK && _isConstant) setData_IJK(NULL, true, _constantValue);

    return _internalDataIJK;
}

size_t CauldronIO::Volume::computeIndex_IJK(size_t i, size_t j, size_t k) const
{
    assert(_geometryAssigned);
    assert(i >= 0 && i < _numI && j >= 0 && j < _numJ && k >= _firstK && k < _numK + _firstK);

    return (i + j * _numI + (k -_firstK) * _numI * _numJ);
}

size_t CauldronIO::Volume::computeIndex_KIJ(size_t i, size_t j, size_t k) const
{
    assert(_geometryAssigned);
    assert(i >= 0 && i < _numI && j >= 0 && j < _numJ && k >= _firstK && k < _numK + _firstK);

    return ((k - _firstK) + i * _numK + j * _numI * _numK);
}

const boost::uuids::uuid& CauldronIO::Volume::getUUID() const
{
    return _uuid;
}

void CauldronIO::Volume::setUUID(const boost::uuids::uuid& uuid)
{
    _uuid = uuid;
}

float CauldronIO::Volume::getConstantValue() const
{
    if (!isConstant())  throw CauldronIOException("Map does not have a constant value");
    return _constantValue;
}

void CauldronIO::Volume::setDepthVolume(boost::shared_ptr<const Volume> depthVolume)
{
    _depthVolume = depthVolume;
}

boost::shared_ptr<const Volume> CauldronIO::Volume::getDepthVolume() const
{
    return _depthVolume;
}

CauldronIO::SubsurfaceKind CauldronIO::Volume::getSubSurfaceKind() const
{
    return _subSurfaceKind;
}

void CauldronIO::Volume::retrieve()
{
    if (_retrieved) return;
    throw CauldronIOException("Not implemented");
}

bool CauldronIO::Volume::isRetrieved() const
{
    return _retrieved;
}

bool CauldronIO::Volume::isCellCentered() const
{
    return _isCellCentered;
}

/// DiscontinuousVolume implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::DiscontinuousVolume::DiscontinuousVolume()
{
}

CauldronIO::DiscontinuousVolume::~DiscontinuousVolume()
{
     _volumeList.clear();
}

void CauldronIO::DiscontinuousVolume::addVolume(boost::shared_ptr<Formation> formation, 
                                                boost::shared_ptr<Volume> volume)
{
    if (!formation) throw CauldronIOException("Cannot add subvolume: formation cannot be null");
    if (!volume) throw CauldronIOException("Cannot add subvolume: volume cannot be null");

    boost::shared_ptr<FormationVolume> pair(new FormationVolume(formation, volume));
    _volumeList.push_back(pair);
}

const FormationVolumeList& CauldronIO::DiscontinuousVolume::getVolumeList() const
{
    return _volumeList;
}

void CauldronIO::DiscontinuousVolume::setDepthVolume(boost::shared_ptr<const Volume> depthVolume)
{
    _depthVolume = depthVolume;
}

boost::shared_ptr<const Volume> CauldronIO::DiscontinuousVolume::getDepthVolume() const
{
    return _depthVolume;
}

void CauldronIO::DiscontinuousVolume::retrieve()
{   
    BOOST_FOREACH(boost::shared_ptr<FormationVolume>& pair, _volumeList)
        pair->second->retrieve();
}
