#include "VisualizationAPI.h"
#include <new>
#include <assert.h>
#include <algorithm>

using namespace CauldronIO;

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
    for (int i = 0; i < _snapShotList->size(); ++i)
    {
        delete _snapShotList->at(i);
    }
    _snapShotList->clear();
}

void CauldronIO::Project::AddSnapShot(const SnapShot * snapShot)
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

const boost::shared_ptr<SnapShotList> CauldronIO::Project::GetSnapShots() const
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
    _gridList.reset(new GridList());
}

CauldronIO::SnapShot::~SnapShot()
{
    // Delete all snapshots
    for (int i = 0; i < _gridList->size(); ++i)
    {
        delete _gridList->at(i);
    }
    _gridList->clear();
}

void CauldronIO::SnapShot::AddGrid(const Grid* grid)
{
    if (!grid) throw CauldronIOException("Cannot add empty snapshot");

    // Check if snapshot exists
    for (int i = 0; i < _gridList->size(); ++i)
    {
        if (_gridList->at(i) == grid) throw CauldronIOException("Cannot add snapshot twice");
    }
    _gridList->push_back(grid);
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

const boost::shared_ptr<GridList> CauldronIO::SnapShot::GetGridList() const
{
    return _gridList;
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

/// Grid implementation
//////////////////////////////////////////////////////////////////////////


CauldronIO::Grid::Grid(bool cellCentered, float undefined)
{
    _isCellCentered = cellCentered;
    _isConstant = false;
    _geometryAssigned = false;
    _undefinedValue = undefined;
}

bool CauldronIO::Grid::IsCellCentered() const
{
    return _isCellCentered;
}

size_t CauldronIO::Grid::GetNumI() const
{
    return _numI;
}

size_t CauldronIO::Grid::GetNumJ() const
{
    return _numJ;
}

double CauldronIO::Grid::GetDeltaI() const
{
    return _deltaI;
}

double CauldronIO::Grid::GetDeltaJ() const
{
    return _deltaJ;
}

double CauldronIO::Grid::GetMinI() const
{
    return _minI;
}

double CauldronIO::Grid::GetMinJ() const
{
    return _minJ;
}

double CauldronIO::Grid::GetMaxI() const
{
    return _maxI;
}

double CauldronIO::Grid::GetMaxJ() const
{
    return _maxJ;
}

float CauldronIO::Grid::GetUndefinedValue() const
{
    return _undefinedValue;
}

bool CauldronIO::Grid::IsConstant() const
{
    return _isConstant;
}

void CauldronIO::Grid::SetConstantValue(float value)
{
    _isConstant = false;
    _constantValue = value;
}

CauldronIO::SubsurfaceKind CauldronIO::Grid::GetSubSurfaceKind() const
{
    return _subSurfaceKind;
}

void CauldronIO::Grid::SetSubserfaceKind(SubsurfaceKind kind)
{
    _subSurfaceKind = kind;
}

const boost::shared_ptr<const Property> CauldronIO::Grid::GetProperty() const
{
    return _property;
}

void CauldronIO::Grid::SetProperty(const Property* property)
{
    if (!property) throw CauldronIOException("Cannot set null property");
    _property.reset(property);
}

/// Map implementation
//////////////////////////////////////////////////////////////////////////


CauldronIO::Map::Map(bool cellCentered, float undefined) : Grid(cellCentered, undefined)
{
    // Indexing into the map is unknown
    _indexCalc = NULL;
    _internalData = NULL;
}

CauldronIO::Map::~Map()
{
    if (_internalData) delete[] _internalData;
    _internalData = NULL;
}

void CauldronIO::Map::SetData_IJ(float* data)
{
    SetData(data);
    _indexCalc = &Map::ComputeIndex_IJ;
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
        memcpy(_internalData, data, sizeof(float) * _numI * _numJ);
    }
    // assign a value if necessary
    else
    {
        std::fill(_internalData, _internalData + _numI * _numJ, value);
    }
}

size_t CauldronIO::Map::ComputeIndex_IJ(size_t i, size_t j) const
{
    assert(_geometryAssigned);
    assert(i >= 0 && i < _numI && j >= 0 && j < _numJ);
    return _numI * j + i;
}

void CauldronIO::Map::SetGeometry(size_t numI, size_t numJ, double deltaI, double deltaJ, double minI, double minJ, double maxI, double maxJ)
{
    _numI = numI;
    _numJ = numJ;
    _deltaI = deltaI;
    _deltaJ = deltaJ;
    _minI = minI;
    _maxI = maxI;
    _maxI = maxI;
    _maxJ = maxJ;
    _geometryAssigned = true;
}

bool CauldronIO::Map::CanGetRow() const
{
    return _indexCalc == &Map::ComputeIndex_IJ;
}

bool CauldronIO::Map::CanGetColumn() const
{
    // Not implemented at this point
    return false;
}

bool CauldronIO::Map::IsUndefined(size_t i, size_t j) const
{
    assert((_internalData || _isConstant) && _indexCalc);
    if (_isConstant) return _constantValue == _undefinedValue;

    return _internalData[(this->*_indexCalc)(i, j)] == _undefinedValue;
}

float CauldronIO::Map::GetValue(size_t i, size_t j) const
{
    assert((_internalData || _isConstant) && _indexCalc);
    if (_isConstant) return _constantValue;
    
    return _internalData[(this->*_indexCalc)(i, j)];
}

const float* CauldronIO::Map::GetRowValues(size_t j) 
{
    assert((_internalData || _isConstant) && _indexCalc);
    if (!CanGetRow()) throw CauldronIOException("Cannot return row values");

    // Create our internal buffer if not existing
    if (!_internalData && _isConstant) SetData(NULL, true, _constantValue);

    return _internalData + GetMapIndex(0, j);
}

const float* CauldronIO::Map::GetColumnValues(size_t i) 
{
    assert((_internalData || _isConstant) && _indexCalc);
    if (!CanGetColumn()) throw CauldronIOException("Cannot return column values");

    // Create our internal buffer if not existing
    if (!_internalData && _isConstant) SetData(NULL, true, _constantValue);

    return _internalData + GetMapIndex(i, 0);
}

const float* CauldronIO::Map::GetSurfaceValues() 
{
    assert((_internalData || _isConstant) && _indexCalc);

    // Create our internal buffer if not existing
    if (!_internalData && _isConstant) SetData(NULL, true, _constantValue);

    return _internalData;
}

void CauldronIO::Map::SetFormation(const Formation* formation)
{
    if (!formation) throw CauldronIOException("Cannot set null formation");
    _formation.reset(formation);
}

const boost::shared_ptr<const Formation> CauldronIO::Map::GetFormation() const
{
    return _formation;
}

size_t CauldronIO::Map::GetMapIndex(size_t i, size_t j) const
{
    if (!_indexCalc) throw CauldronIOException("Cannot compute index without data assigned");
    return (this->*_indexCalc)(i, j);
}

/// Volume implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Volume::Volume(bool cellCentered, float undefined) : Grid(cellCentered, undefined)
{
    // Indexing into the volume is unknown
    _indexCalc = NULL;
    _internalData = NULL;
}

CauldronIO::Volume::~Volume()
{
    if (_internalData) delete[] _internalData;
    _internalData = NULL;
}

void CauldronIO::Volume::SetData_KIJ(float* data, bool setValue /*= false*/, float value /*= 0*/)
{
    SetData(data, setValue, value);
    _indexCalc = &Volume::ComputeIndex_KIJ;
}

void CauldronIO::Volume::SetData_IJK(float* data, bool setValue /*= false*/, float value /*= 0*/)
{
    SetData(data, setValue, value);
    _indexCalc = &Volume::ComputeIndex_IJK;
}

void CauldronIO::Volume::SetData(float* data, bool setValue /*= false*/, float value /*= 0*/)
{
    if (!_geometryAssigned) throw CauldronIOException("Cannot assign data without geometry known");

    // If our data buffer exists, we will just reuse it. Otherwise, allocate
    if (!_internalData)
    {
        try
        {
            _internalData = new float[_numI * _numJ * _numK];
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
        memcpy(_internalData, data, sizeof(float) * _numI * _numJ * _numK);
    }
    // assign a value if necessary
    else
    {
        std::fill(_internalData, _internalData + _numI * _numJ * _numK, value);
    }
}

void CauldronIO::Volume::SetGeometry(size_t numI, size_t numJ, size_t numK, double deltaI, 
    double deltaJ, double minI, double minJ, double minK, double maxI, double maxJ, double maxK)
{
    _numI = numI;
    _numJ = numJ;
    _numK = numK;
    _deltaI = deltaI;
    _deltaJ = deltaJ;
    _maxI = maxI;
    _maxJ = maxJ;
    _maxK = maxK;
    _minI = minI;
    _minJ = minJ;
    _minK = minK;
    _geometryAssigned = true;
}

size_t CauldronIO::Volume::GetNumK() const
{
    return _numK;
}

double CauldronIO::Volume::GetMinK() const
{
    return _minK;
}

double CauldronIO::Volume::GetMaxK() const
{
    return _maxK;
}

bool CauldronIO::Volume::CanGetRow() const
{
    return _indexCalc == &Volume::ComputeIndex_IJK;
}

bool CauldronIO::Volume::CanGetColumn() const
{
    return false;
}

bool CauldronIO::Volume::CanGetNeedle() const
{
    return _indexCalc == &Volume::ComputeIndex_KIJ;
}

bool CauldronIO::Volume::CanGetSurface() const
{
    return _indexCalc == &Volume::ComputeIndex_IJK;
}

bool CauldronIO::Volume::IsUndefined(size_t i, size_t j, size_t k) const
{
    assert((_internalData || _isConstant) && _indexCalc);
    if (_isConstant) return _constantValue == _undefinedValue;

    return _internalData[(this->*_indexCalc)(i, j, k)] == _undefinedValue;
}

float CauldronIO::Volume::GetValue(size_t i, size_t j, size_t k) const
{
    assert((_internalData || _isConstant) && _indexCalc);
    if (_isConstant) return _constantValue;

    return _internalData[(this->*_indexCalc)(i, j, k)];
}

const float* CauldronIO::Volume::GetRowValues(size_t j, size_t k)
{
    assert((_internalData || _isConstant) && _indexCalc);
    if (!CanGetRow()) throw CauldronIOException("Cannot return row values");

    // Create our internal buffer if not existing
    if (!_internalData && _isConstant) SetData(NULL, true, _constantValue);

    // Assume IJK ordering
    assert(_indexCalc == &Volume::ComputeIndex_IJK);
    return _internalData + GetVolumeIndex(0, j, k);
}

const float* CauldronIO::Volume::GetColumnValues(size_t i, size_t k)
{
    assert((_internalData || _isConstant) && _indexCalc);
    if (!CanGetColumn()) throw CauldronIOException("Cannot return column values");

    throw CauldronIOException("Not implemented");
}

const float* CauldronIO::Volume::GetNeedleValues(size_t i, size_t j)
{
    assert((_internalData || _isConstant) && _indexCalc);
    if (!CanGetNeedle()) throw CauldronIOException("Cannot return needle values");

    // Create our internal buffer if not existing
    if (!_internalData && _isConstant) SetData(NULL, true, _constantValue);

    return _internalData + GetVolumeIndex(i, j, 0);
}

const float* CauldronIO::Volume::GetSurface(size_t k)
{
    assert((_internalData || _isConstant) && _indexCalc);
    if (!CanGetSurface()) throw CauldronIOException("Cannot return surface values");

    // Create our internal buffer if not existing
    if (!_internalData && _isConstant) SetData(NULL, true, _constantValue);

    return _internalData + GetVolumeIndex(0, 0, k);
}

const float* CauldronIO::Volume::GetVolumeValues()
{
    assert((_internalData || _isConstant) && _indexCalc);

    // Create our internal buffer if not existing
    if (!_internalData && _isConstant) SetData(NULL, true, _constantValue);

    return _internalData;
}

size_t CauldronIO::Volume::GetVolumeIndex(size_t i, size_t j, size_t k) const
{
    if (!_indexCalc) throw CauldronIOException("Cannot compute index without data assigned");
    return (this->*_indexCalc)(i, j, k);
}

size_t CauldronIO::Volume::ComputeIndex_IJK(size_t i, size_t j, size_t k) const
{
    assert(_geometryAssigned);
    assert(i >= 0 && i < _numI && j >= 0 && j < _numJ && k >= 0 && k < _numK);

    return (i + j * _numI + k * _numI * _numJ);
}

size_t CauldronIO::Volume::ComputeIndex_KIJ(size_t i, size_t j, size_t k) const
{
    assert(_geometryAssigned);
    assert(i >= 0 && i < _numI && j >= 0 && j < _numJ && k >= 0 && k < _numK);

    return (k + i * _numK + j * _numI * _numK);
}

/// DiscontinuousVolume implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::DiscontinuousVolume::DiscontinuousVolume(const Formation* formation, const Volume* volume, float undefined) : Grid(false, undefined)
{
    if (!formation) throw CauldronIOException("Cannot create Discontinuous volume: formation cannot be null");
    if (!volume) throw CauldronIOException("Cannot create Discontinuous volume: volume cannot be null");

    // Set some Grid parameters from the first volume
    _deltaI = volume->GetDeltaI();
    _deltaJ = volume->GetDeltaJ();
    _minI = volume->GetMinI();
    _minJ = volume->GetMinJ();
    _maxI = volume->GetMaxI();
    _maxJ = volume->GetMaxJ();
    _numI = volume->GetNumI();
    _numJ = volume->GetNumJ();
    _geometryAssigned = true;
    _volumeList.reset(new FormationVolumeList());

    AddVolume(formation, volume);
}

CauldronIO::DiscontinuousVolume::~DiscontinuousVolume()
{
    for (size_t i = 0; i < _volumeList->size(); ++i)
    {
        // We do not need to delete a formation
        delete _volumeList->at(i)->second;
    }

    _volumeList->clear();
}

void CauldronIO::DiscontinuousVolume::AddVolume(const Formation* formation, const Volume* volume)
{
    if (!formation) throw CauldronIOException("Cannot add subvolume: formation cannot be null");
    if (!volume) throw CauldronIOException("Cannot add subvolume: volume cannot be null");

    const FormationVolume* pair = new FormationVolume(formation, volume);
    _volumeList->push_back(pair);
}

const boost::shared_ptr<FormationVolumeList> CauldronIO::DiscontinuousVolume::GetVolumeList() const
{
    return _volumeList;
}

bool CauldronIO::DiscontinuousVolume::IsConstant() const
{
    for (size_t i = 0; i < _volumeList->size(); ++i)
    {
        if (!_volumeList->at(i)->second->IsConstant()) return false;
    }

    return true;
}

void CauldronIO::DiscontinuousVolume::SetConstantValue(float value)
{
    throw CauldronIOException("Cannot set a constant value on a discontinuous volume");
}
