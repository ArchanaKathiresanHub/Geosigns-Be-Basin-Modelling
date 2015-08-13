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
    _snapShotList.reset(new SnapShotList());
}

CauldronIO::Project::~Project()
{
    // Delete all snapshots
    _snapShotList->clear();
}

void CauldronIO::Project::AddSnapShot(const boost::shared_ptr<const SnapShot>& snapShot)
{
    if (!snapShot) throw CauldronIOException("Cannot add empty snapshot");
    
    // Check if snapshot exists
    for (int i = 0; i < _snapShotList->size(); ++i)
    {   
        if (_snapShotList->at(i) == snapShot) throw CauldronIOException("Cannot add snapshot twice");
    }
    _snapShotList->push_back(snapShot);
}

const string& CauldronIO::Project::GetName() const
{
    return _name;
}

const string& CauldronIO::Project::GetDescription() const
{
    return _description;
}

const string& CauldronIO::Project::GetTeam() const
{
    return _team;
}

const string& CauldronIO::Project::GetProgramVersion() const
{
    return _version;
}

CauldronIO::ModellingMode CauldronIO::Project::GetModelingMode() const
{
    return _mode;
}

const boost::shared_ptr<const SnapShotList> CauldronIO::Project::GetSnapShots() const
{
    return _snapShotList;
}

/// SnapShot implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::SnapShot::SnapShot(double age, SnapShotKind kind, bool isMinorShapshot)
{
    _age = age;
    _kind = kind;
    _isMinor = isMinorShapshot;
    _volumeList.reset(new VolumeList());
    _discVolumeList.reset(new DiscontinuousVolumeList());
    _surfaceList.reset(new SurfaceList());
}

CauldronIO::SnapShot::~SnapShot()
{
    // Delete all snapshots
    _volumeList->clear();
    _discVolumeList->clear();
    _surfaceList->clear();
}

void CauldronIO::SnapShot::AddSurface(const boost::shared_ptr<const Surface>& surface)
{
    if (!surface) throw CauldronIOException("Cannot add empty surface");

    // Check if snapshot exists
    for (int i = 0; i < _surfaceList->size(); ++i)
    {
        if (_surfaceList->at(i) == surface) throw CauldronIOException("Cannot add surface twice");
    }
    _surfaceList->push_back(surface);
}

void CauldronIO::SnapShot::AddVolume(const boost::shared_ptr<const Volume>& volume)
{
    if (!volume) throw CauldronIOException("Cannot add empty volume");

    // Check if snapshot exists
    for (int i = 0; i < _volumeList->size(); ++i)
    {
        if (_volumeList->at(i) == volume) throw CauldronIOException("Cannot add volume twice");
    }
    _volumeList->push_back(volume);
}

void CauldronIO::SnapShot::AddDiscontinuousVolume(const boost::shared_ptr<const DiscontinuousVolume>& discVolume)
{
    if (!discVolume) throw CauldronIOException("Cannot add empty volume");

    // Check if snapshot exists
    for (int i = 0; i < _discVolumeList->size(); ++i)
    {
        if (_discVolumeList->at(i) == discVolume) throw CauldronIOException("Cannot add volume twice");
    }
    _discVolumeList->push_back(discVolume);
}

double CauldronIO::SnapShot::GetAge() const
{
    return _age;
}

CauldronIO::SnapShotKind CauldronIO::SnapShot::GetKind() const
{
    return _kind;
}

bool CauldronIO::SnapShot::IsMinorShapshot() const
{
    return _isMinor;
}

const boost::shared_ptr<const SurfaceList> CauldronIO::SnapShot::GetSurfaceList() const
{
    return _surfaceList;
}

const boost::shared_ptr<const VolumeList> CauldronIO::SnapShot::GetVolumeList() const
{
    return _volumeList;
}

const boost::shared_ptr<const DiscontinuousVolumeList> CauldronIO::SnapShot::GetDiscontinuousVolumeList() const
{
    return _discVolumeList;
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

const string& CauldronIO::Property::GetName() const
{
    return _name;
}

const string& CauldronIO::Property::GetUserName() const
{
    return _username;
}

const string& CauldronIO::Property::GetCauldronName() const
{
    return _cauldronName;
}

const string& CauldronIO::Property::GetUnit() const
{
    return _unit;
}

CauldronIO::PropertyType CauldronIO::Property::GetType() const
{
    return _type;
}

CauldronIO::PropertyAttribute CauldronIO::Property::GetAttribute() const
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

void CauldronIO::Formation::GetDepthRange(size_t &start, size_t &end) const
{
    start = _kstart;
    end = _kend;
}

const string& CauldronIO::Formation::GetName() const
{
    return _name;
}

/// Surface implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Surface::Surface(const string& name, SubsurfaceKind kind, 
                             boost::shared_ptr<const Property> property, boost::shared_ptr<const Map> valueMap)
{
    _name = name;
    _subSurfaceKind = kind;
    _property = property;
    _valueMap = valueMap;
}

const string& CauldronIO::Surface::GetName() const
{
    return _name;
}

const boost::shared_ptr<const Map> CauldronIO::Surface::GetValueMap() const
{
    return _valueMap;
}

CauldronIO::SubsurfaceKind CauldronIO::Surface::GetSubSurfaceKind() const
{
    return _subSurfaceKind;
}

const boost::shared_ptr<const Property> CauldronIO::Surface::GetProperty() const
{
    return _property;
}

void CauldronIO::Surface::SetFormation(boost::shared_ptr<const Formation> formation)
{
    _formation = formation;
}

const boost::shared_ptr<const Formation> CauldronIO::Surface::GetFormation() const
{
    return _formation;
}

void CauldronIO::Surface::SetDepthSurface(boost::shared_ptr<const Surface> surface)
{
    _depthSurface = surface;
}

const boost::shared_ptr<const Surface> CauldronIO::Surface::GetDepthSurface() const
{
    return _depthSurface;
}

/// Map implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Map::Map(bool cellCentered, float undefined)
{
    // Indexing into the map is unknown
    _internalData = NULL;
    _isCellCentered = cellCentered;
    _undefinedValue = undefined;
    _isConstant = false;
}

size_t CauldronIO::Map::GetNumI() const
{
    return _numI;
}

size_t CauldronIO::Map::GetNumJ() const
{
    return _numJ;
}

double CauldronIO::Map::GetDeltaI() const
{
    return _deltaI;
}

double CauldronIO::Map::GetDeltaJ() const
{
    return _deltaJ;
}

double CauldronIO::Map::GetMinI() const
{
    return _minI;
}

double CauldronIO::Map::GetMinJ() const
{
    return _minJ;
}

double CauldronIO::Map::GetMaxI() const
{
    return _maxI;
}

double CauldronIO::Map::GetMaxJ() const
{
    return _maxJ;
}

float CauldronIO::Map::GetUndefinedValue() const
{
    return _undefinedValue;
}

bool CauldronIO::Map::IsConstant() const
{
    return _isConstant;
}

void CauldronIO::Map::SetConstantValue(float value)
{
    _isConstant = true;
    _constantValue = value;
}

CauldronIO::Map::~Map()
{
    if (_internalData) delete[] _internalData;
    _internalData = NULL;
}

void CauldronIO::Map::SetData_IJ(float* data)
{
    SetData(data);
}

void CauldronIO::Map::SetData(float* data, bool setValue, float value)
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
}

void CauldronIO::Map::SetGeometry(size_t numI, size_t numJ, double deltaI, double deltaJ, double minI, double minJ)
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

bool CauldronIO::Map::CanGetRow() const
{
    return true;
}

bool CauldronIO::Map::CanGetColumn() const
{
    // Not implemented at this point
    return false;
}

bool CauldronIO::Map::IsUndefined(size_t i, size_t j) const
{
    assert(_internalData || _isConstant);
    if (_isConstant) return _constantValue == _undefinedValue;

    return _internalData[GetMapIndex(i, j)] == _undefinedValue;
}

float CauldronIO::Map::GetValue(size_t i, size_t j) const
{
    assert(_internalData || _isConstant);
    if (_isConstant) return _constantValue;
    
    return _internalData[GetMapIndex(i, j)];
}

float const * CauldronIO::Map::GetRowValues(size_t j)
{
    assert(_internalData || _isConstant);
    if (!CanGetRow()) throw CauldronIOException("Cannot return row values");

    // Create our internal buffer if not existing
    if (!_internalData && _isConstant) SetData(NULL, true, _constantValue);

    return _internalData + GetMapIndex(0, j);
}

float const * CauldronIO::Map::GetColumnValues(size_t i)
{
    assert(_internalData || _isConstant);
    if (!CanGetColumn()) throw CauldronIOException("Cannot return column values");

    // Create our internal buffer if not existing
    if (!_internalData && _isConstant) SetData(NULL, true, _constantValue);

    return _internalData + GetMapIndex(i, 0);
}

float const * CauldronIO::Map::GetSurfaceValues()
{
    assert(_internalData || _isConstant);

    // Create our internal buffer if not existing
    if (!_internalData && _isConstant) SetData(NULL, true, _constantValue);

    return _internalData;
}

size_t CauldronIO::Map::GetMapIndex(size_t i, size_t j) const
{
    assert(_geometryAssigned);
    assert(i >= 0 && i < _numI && j >= 0 && j < _numJ);
    return _numI * j + i;
}

/// Volume implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Volume::Volume(bool cellCentered, float undefined, SubsurfaceKind kind, boost::shared_ptr<const Property> property)
{
    // Indexing into the volume is unknown
    _internalDataIJK = NULL;
    _internalDataKIJ = NULL;
    _isCellCentered = cellCentered;
    _undefinedValue = undefined;
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

const boost::shared_ptr<const Property> CauldronIO::Volume::GetProperty() const
{
    return _property;
}

void CauldronIO::Volume::SetData_KIJ(float* data, bool setValue /*= false*/, float value /*= 0*/)
{
    SetData(data, _internalDataKIJ, setValue, value);
}

void CauldronIO::Volume::SetData_IJK(float* data, bool setValue /*= false*/, float value /*= 0*/)
{
    SetData(data, _internalDataIJK, setValue, value);
}

void CauldronIO::Volume::SetData(float* data, float* internalData, bool setValue /*= false*/, float value /*= 0*/)
{
    if (!_geometryAssigned) throw CauldronIOException("Cannot assign data without geometry known");

    // If our data buffer exists, we will just reuse it. Otherwise, allocate
    if (!internalData)
    {
        try
        {
            internalData = new float[_numI * _numJ * _numK];
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
        memcpy(internalData, data, sizeof(float) * _numI * _numJ * _numK);
    }
    // assign a value if necessary
    else
    {
        std::fill(internalData, internalData + _numI * _numJ * _numK, value);
    }
}

bool CauldronIO::Volume::IsDataAvailable() const
{
    return _isConstant || (_internalDataIJK != NULL) || (_internalDataKIJ != NULL);
}

void CauldronIO::Volume::SetGeometry(size_t numI, size_t numJ, size_t numK, size_t offsetK, double deltaI,
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
}

size_t CauldronIO::Volume::GetNumI() const
{
    return _numI;
}

size_t CauldronIO::Volume::GetNumJ() const
{
    return _numJ;
}

double CauldronIO::Volume::GetDeltaI() const
{
    return _deltaI;
}

double CauldronIO::Volume::GetDeltaJ() const
{
    return _deltaJ;
}

double CauldronIO::Volume::GetMinI() const
{
    return _minI;
}

double CauldronIO::Volume::GetMinJ() const
{
    return _minJ;
}

double CauldronIO::Volume::GetMaxI() const
{
    return _maxI;
}

double CauldronIO::Volume::GetMaxJ() const
{
    return _maxJ;
}

float CauldronIO::Volume::GetUndefinedValue() const
{
    return _undefinedValue;
}

bool CauldronIO::Volume::IsConstant() const
{
    return _isConstant;
}

void CauldronIO::Volume::SetConstantValue(float value)
{
    _isConstant = true;
    _constantValue = value;
}

size_t CauldronIO::Volume::GetNumK() const
{
    return _numK;
}

size_t CauldronIO::Volume::GetFirstK() const
{
    return _firstK;
}

size_t CauldronIO::Volume::GetLastK() const
{
    return _firstK + _numK - 1;
}

bool CauldronIO::Volume::CanGetRow() const
{
    return _internalDataIJK != NULL;
}

bool CauldronIO::Volume::CanGetColumn() const
{
    return false;
}

bool CauldronIO::Volume::CanGetNeedle() const
{
    return _internalDataKIJ != NULL;
}

bool CauldronIO::Volume::CanGetSurface_IJ() const
{
    return _internalDataIJK != NULL;
}

bool CauldronIO::Volume::IsUndefined(size_t i, size_t j, size_t k) const
{
    assert(IsDataAvailable());
    if (_isConstant) return _constantValue == _undefinedValue;
    if (_internalDataIJK) return _internalDataIJK[ComputeIndex_IJK(i, j, k)] == _undefinedValue;
    
    assert(_internalDataKIJ);
    return _internalDataKIJ[ComputeIndex_KIJ(i, j, k)] == _undefinedValue;
}

float CauldronIO::Volume::GetValue(size_t i, size_t j, size_t k) const
{
    assert(IsDataAvailable());
    if (_isConstant) return _constantValue;
    if (_internalDataIJK) return _internalDataIJK[ComputeIndex_IJK(i, j, k)];

    assert(_internalDataKIJ);
    return _internalDataKIJ[ComputeIndex_KIJ(i, j, k)];
}

float const * CauldronIO::Volume::GetRowValues(size_t j, size_t k)
{
    assert(IsDataAvailable());
    if (!CanGetRow()) throw CauldronIOException("Cannot return row values");

    // Create our internal buffer if not existing
    if (!_internalDataIJK && _isConstant) SetData(NULL, NULL, true, _constantValue);

    // Assume IJK ordering
    assert(_internalDataIJK);
    return _internalDataIJK + ComputeIndex_IJK(0, j, k);
}

float const * CauldronIO::Volume::GetColumnValues(size_t i, size_t k)
{
    assert(IsDataAvailable());
    if (!CanGetColumn()) throw CauldronIOException("Cannot return column values");

    throw CauldronIOException("Not implemented");
}

float const * CauldronIO::Volume::GetNeedleValues(size_t i, size_t j)
{
    assert(IsDataAvailable());
    if (!CanGetNeedle()) throw CauldronIOException("Cannot return needle values");

    // Create our internal buffer if not existing
    if (!_internalDataKIJ && _isConstant) SetData(NULL, NULL, true, _constantValue);

    return _internalDataKIJ + ComputeIndex_KIJ(i, j, _firstK);
}

float const * CauldronIO::Volume::GetSurface_IJ(size_t k)
{
    assert(IsDataAvailable());
    if (!CanGetSurface_IJ()) throw CauldronIOException("Cannot return surface values");

    // Create our internal buffer if not existing
    if (!_internalDataIJK && _isConstant) SetData(NULL, NULL, true, _constantValue);

    return _internalDataIJK + ComputeIndex_IJK(0, 0, k);
}

float const * CauldronIO::Volume::GetVolumeValues_KIJ()
{
    assert(IsDataAvailable());

    // Create our internal buffer if not existing
    if (!_internalDataKIJ && _isConstant) SetData(NULL, NULL, true, _constantValue);

    return _internalDataKIJ;
}

float const * CauldronIO::Volume::GetVolumeValues_IJK()
{
    assert(IsDataAvailable());

    // Create our internal buffer if not existing
    if (!_internalDataIJK && _isConstant) SetData(NULL, NULL, true, _constantValue);

    return _internalDataIJK;
}

size_t CauldronIO::Volume::ComputeIndex_IJK(size_t i, size_t j, size_t k) const
{
    assert(_geometryAssigned);
    assert(i >= 0 && i < _numI && j >= 0 && j < _numJ && k >= _firstK && k < _numK + _firstK);

    return (i + j * _numI + (k -_firstK) * _numI * _numJ);
}

size_t CauldronIO::Volume::ComputeIndex_KIJ(size_t i, size_t j, size_t k) const
{
    assert(_geometryAssigned);
    assert(i >= 0 && i < _numI && j >= 0 && j < _numJ && k >= _firstK && k < _numK + _firstK);

    return ((k - _firstK) + i * _numK + j * _numI * _numK);
}

/// DiscontinuousVolume implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::DiscontinuousVolume::DiscontinuousVolume()
{
    _volumeList.reset(new FormationVolumeList());
}

CauldronIO::DiscontinuousVolume::~DiscontinuousVolume()
{
     _volumeList->clear();
}

void CauldronIO::DiscontinuousVolume::AddVolume(boost::shared_ptr<const Formation> formation, 
                                                boost::shared_ptr<const Volume> volume)
{
    if (!formation) throw CauldronIOException("Cannot add subvolume: formation cannot be null");
    if (!volume) throw CauldronIOException("Cannot add subvolume: volume cannot be null");

    boost::shared_ptr<const FormationVolume> pair(new FormationVolume(formation, volume));
    _volumeList->push_back(pair);
}

const boost::shared_ptr<const FormationVolumeList> CauldronIO::DiscontinuousVolume::GetVolumeList() const
{
    return _volumeList;
}