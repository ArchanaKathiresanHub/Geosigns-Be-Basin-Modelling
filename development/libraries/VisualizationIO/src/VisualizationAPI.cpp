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
    m_name = name;
    m_description = description;
    m_team = team;
    m_version = version;
    m_mode = mode;
}

CauldronIO::Project::~Project()
{
    // Delete all snapshots
    m_snapShotList.clear();
}

void CauldronIO::Project::addSnapShot(boost::shared_ptr<SnapShot>& newSnapShot)
{
    if (!newSnapShot) throw CauldronIOException("Cannot add empty snapshot");
    
    // Check if snapshot exists
    BOOST_FOREACH(boost::shared_ptr<SnapShot>& snapShot, m_snapShotList)
        if (snapShot == newSnapShot) throw CauldronIOException("Cannot add snapshot twice");

    m_snapShotList.push_back(newSnapShot);
}

const string& CauldronIO::Project::getName() const
{
    return m_name;
}

const string& CauldronIO::Project::getDescription() const
{
    return m_description;
}

const string& CauldronIO::Project::getTeam() const
{
    return m_team;
}

const string& CauldronIO::Project::getProgramVersion() const
{
    return m_version;
}

CauldronIO::ModellingMode CauldronIO::Project::getModelingMode() const
{
    return m_mode;
}

const SnapShotList& CauldronIO::Project::getSnapShots() const
{
    return m_snapShotList;
}

void CauldronIO::Project::retrieve()
{
    BOOST_FOREACH(boost::shared_ptr<SnapShot>& snapShot, m_snapShotList)
        snapShot->retrieve();
}

/// SnapShot implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::SnapShot::SnapShot(double age, SnapShotKind kind, bool isMinorShapshot)
{
    m_age = age;
    m_kind = kind;
    m_isMinor = isMinorShapshot;
}

CauldronIO::SnapShot::~SnapShot()
{
    // Delete all snapshots
    m_volumeList.clear();
    m_discVolumeList.clear();
    m_surfaceList.clear();
    m_trapperList.clear();
}

void CauldronIO::SnapShot::addSurface(boost::shared_ptr<Surface>& newSurface)
{
    if (!newSurface) throw CauldronIOException("Cannot add empty surface");

    // Check if surface exists
    BOOST_FOREACH(boost::shared_ptr<Surface>& surface, m_surfaceList)
        if (surface == newSurface) throw CauldronIOException("Cannot add surface twice");

    m_surfaceList.push_back(newSurface);
}

void CauldronIO::SnapShot::addVolume(boost::shared_ptr<Volume>& newVolume)
{
    if (!newVolume) throw CauldronIOException("Cannot add empty volume");

    // Check if volume exists
    BOOST_FOREACH(boost::shared_ptr<Volume>& volume, m_volumeList)
        if (volume == newVolume) throw CauldronIOException("Cannot add volume twice");

    m_volumeList.push_back(newVolume);
}

void CauldronIO::SnapShot::addDiscontinuousVolume(boost::shared_ptr<DiscontinuousVolume>& newDiscVolume)
{
    if (!newDiscVolume) throw CauldronIOException("Cannot add empty volume");

    // Check if volume exists
    BOOST_FOREACH(boost::shared_ptr<DiscontinuousVolume>& volume, m_discVolumeList)
        if (volume == newDiscVolume) throw CauldronIOException("Cannot add volume twice");

    m_discVolumeList.push_back(newDiscVolume);
}


void CauldronIO::SnapShot::addTrapper(boost::shared_ptr<Trapper>& newTrapper)
{
    if (!newTrapper) throw CauldronIOException("Cannot add empty trapper");

    BOOST_FOREACH(boost::shared_ptr<Trapper>& trapper, m_trapperList)
        if (trapper == newTrapper) throw CauldronIOException("Cannot add trapper twice");

    m_trapperList.push_back(newTrapper);
}

double CauldronIO::SnapShot::getAge() const
{
    return m_age;
}

CauldronIO::SnapShotKind CauldronIO::SnapShot::getKind() const
{
    return m_kind;
}

bool CauldronIO::SnapShot::isMinorShapshot() const
{
    return m_isMinor;
}

const SurfaceList& CauldronIO::SnapShot::getSurfaceList() const
{
    return m_surfaceList;
}

const VolumeList& CauldronIO::SnapShot::getVolumeList() const
{
    return m_volumeList;
}

const DiscontinuousVolumeList& CauldronIO::SnapShot::getDiscontinuousVolumeList() const
{
    return m_discVolumeList;
}


const TrapperList& CauldronIO::SnapShot::getTrapperList() const
{
    return m_trapperList;
}

void CauldronIO::SnapShot::retrieve()
{
    BOOST_FOREACH(boost::shared_ptr<Volume>& volume, m_volumeList)
        volume->retrieve();
    BOOST_FOREACH(boost::shared_ptr<DiscontinuousVolume>& volume, m_discVolumeList)
        volume->retrieve();
    BOOST_FOREACH(boost::shared_ptr<Surface>& surface, m_surfaceList)
        surface->retrieve();
}

/// Property implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Property::Property(const string& name, const string& username, const string& cauldronName, const string& unit, PropertyType type, PropertyAttribute attrib)
{
    m_name = name;
    m_username = username;
    m_cauldronName = cauldronName;
    m_unit = unit;
    m_type = type;
    m_attrib = attrib;
}

const string& CauldronIO::Property::getName() const
{
    return m_name;
}

const string& CauldronIO::Property::getUserName() const
{
    return m_username;
}

const string& CauldronIO::Property::getCauldronName() const
{
    return m_cauldronName;
}

const string& CauldronIO::Property::getUnit() const
{
    return m_unit;
}

CauldronIO::PropertyType CauldronIO::Property::getType() const
{
    return m_type;
}

CauldronIO::PropertyAttribute CauldronIO::Property::getAttribute() const
{
    return m_attrib;
}

/// Formation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Formation::Formation(size_t kStart, size_t kEnd, const string& name)
{
    m_kstart = kStart;
    m_kend = kEnd;
    m_name = name;
}

void CauldronIO::Formation::getDepthRange(size_t &start, size_t &end) const
{
    start = m_kstart;
    end = m_kend;
}

const string& CauldronIO::Formation::getName() const
{
    return m_name;
}

/// Surface implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Surface::Surface(const string& name, SubsurfaceKind kind, 
                             boost::shared_ptr<const Property> property, boost::shared_ptr<Map> valueMap)
{
    m_name = name;
    m_subSurfaceKind = kind;
    m_property = property;
    m_valueMap = valueMap;
    m_depthSurface.reset();
}

const string& CauldronIO::Surface::getName() const
{
    return m_name;
}

const boost::shared_ptr<Map> CauldronIO::Surface::getValueMap() const
{
    return m_valueMap;
}

CauldronIO::SubsurfaceKind CauldronIO::Surface::getSubSurfaceKind() const
{
    return m_subSurfaceKind;
}

const boost::shared_ptr<const Property> CauldronIO::Surface::getProperty() const
{
    return m_property;
}

void CauldronIO::Surface::setFormation(boost::shared_ptr<const Formation> formation)
{
    m_formation = formation;
}

const boost::shared_ptr<const Formation> CauldronIO::Surface::getFormation() const
{
    return m_formation;
}

void CauldronIO::Surface::setDepthSurface(boost::shared_ptr<const Surface> surface)
{
    m_depthSurface = surface;
}

const boost::shared_ptr<const Surface> CauldronIO::Surface::getDepthSurface() const
{
    return m_depthSurface;
}

void CauldronIO::Surface::retrieve()
{
    m_valueMap->retrieve();
}

bool CauldronIO::Surface::isRetrieved() const
{
    return m_valueMap->isRetrieved();
}


const std::string& CauldronIO::Surface::getReservoirName() const
{
    return m_reservoirName;
}

void CauldronIO::Surface::setReservoirName(const std::string& reservoirName)
{
    m_reservoirName = reservoirName;
}

/// Map implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Map::Map(bool cellCentered)
{
    // Indexing into the map is unknown
    m_internalData = NULL;
    m_isCellCentered = cellCentered;
    m_isConstant = false;
    m_retrieved = false;
}

size_t CauldronIO::Map::getNumI() const
{
    return m_numI;
}

size_t CauldronIO::Map::getNumJ() const
{
    return m_numJ;
}

double CauldronIO::Map::getDeltaI() const
{
    return m_deltaI;
}

double CauldronIO::Map::getDeltaJ() const
{
    return m_deltaJ;
}

double CauldronIO::Map::getMinI() const
{
    return m_minI;
}

double CauldronIO::Map::getMinJ() const
{
    return m_minJ;
}

double CauldronIO::Map::getMaxI() const
{
    return m_maxI;
}

double CauldronIO::Map::getMaxJ() const
{
    return m_maxJ;
}

float CauldronIO::Map::getUndefinedValue() const
{
    return m_undefinedValue;
}


void CauldronIO::Map::setUndefinedValue(float undefined)
{
    m_undefinedValue = undefined;
}

bool CauldronIO::Map::isConstant() const
{
    return m_isConstant;
}

void CauldronIO::Map::setConstantValue(float value)
{
    m_isConstant = true;
    m_constantValue = value;
    m_retrieved = true;
}

CauldronIO::Map::~Map()
{
    if (m_internalData) delete[] m_internalData;
    m_internalData = NULL;
}

void CauldronIO::Map::setData_IJ(float* data)
{
    setData(data);
}

void CauldronIO::Map::setData(float* data, bool setValue, float value)
{
    if (!m_geometryAssigned) throw CauldronIOException("Cannot assign data without geometry known");

    // If our data buffer exists, we will just reuse it. Otherwise, allocate
    if (!m_internalData)
    {
        try
        {
            m_internalData = new float[m_numI * m_numJ];
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
        std::memcpy(m_internalData, data, sizeof(float) * m_numI * m_numJ);
    }
    // assign a value if necessary
    else
    {
        std::fill(m_internalData, m_internalData + m_numI * m_numJ, value);
    }

    m_retrieved = true;
}

void CauldronIO::Map::setGeometry(size_t numI, size_t numJ, double deltaI, double deltaJ, double minI, double minJ)
{
    m_numI = numI;
    m_numJ = numJ;
    m_deltaI = deltaI;
    m_deltaJ = deltaJ;
    m_minI = minI;
    m_minJ = minJ;
    m_maxI = minI + deltaI * numI;
    m_maxJ = minJ + deltaJ * numJ;
    m_geometryAssigned = true;
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
    if (!m_geometryAssigned) throw CauldronIOException("Need to assign a geometry first!");
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (m_isConstant) return m_constantValue == m_undefinedValue;

    return m_internalData[getMapIndex(i, j)] == m_undefinedValue;
}

float CauldronIO::Map::getValue(size_t i, size_t j) const
{
    if (!m_geometryAssigned) throw CauldronIOException("Need to assign a geometry first!");
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (m_isConstant) return m_constantValue;
    
    return m_internalData[getMapIndex(i, j)];
}

float const * CauldronIO::Map::getRowValues(size_t j)
{
    if (!m_geometryAssigned) throw CauldronIOException("Need to assign a geometry first!");
    if (!canGetRow()) throw CauldronIOException("Cannot return row values");
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");

    // Create our internal buffer if not existing
    if (!m_internalData && m_isConstant) setData(NULL, true, m_constantValue);

    return m_internalData + getMapIndex(0, j);
}

float const * CauldronIO::Map::getColumnValues(size_t i)
{
    if (!m_geometryAssigned) throw CauldronIOException("Need to assign a geometry first!");
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (!canGetColumn()) throw CauldronIOException("Cannot return column values");

    // Create our internal buffer if not existing
    if (!m_internalData && m_isConstant) setData(NULL, true, m_constantValue);

    return m_internalData + getMapIndex(i, 0);
}

float const * CauldronIO::Map::getSurfaceValues()
{
    if (!m_geometryAssigned) throw CauldronIOException("Need to assign a geometry first!");
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");

    // Create our internal buffer if not existing
    if (!m_internalData && m_isConstant) setData(NULL, true, m_constantValue);

    return m_internalData;
}

size_t CauldronIO::Map::getMapIndex(size_t i, size_t j) const
{
    assert(m_geometryAssigned);
    assert(i >= 0 && i < m_numI && j >= 0 && j < m_numJ);
    return m_numI * j + i;
}

void CauldronIO::Map::retrieve()
{
    if (m_retrieved) return;
    throw CauldronIOException("Not implemented");
}

bool CauldronIO::Map::isRetrieved() const
{
    return m_retrieved;
}


bool CauldronIO::Map::isCellCentered() const
{
    return m_isCellCentered;
}

const boost::uuids::uuid& CauldronIO::Map::getUUID() const
{
    return m_uuid;
}

void CauldronIO::Map::setUUID(const boost::uuids::uuid& uuid)
{
    m_uuid = uuid;
}

float CauldronIO::Map::getConstantValue() const
{
    if (!isConstant()) throw CauldronIOException("Map does not have a constant value");
    return m_constantValue;
}

/// Volume implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Volume::Volume(bool cellCentered, SubsurfaceKind kind, boost::shared_ptr<const Property> property)
{
    // Indexing into the volume is unknown
    m_internalDataIJK = NULL;
    m_internalDataKIJ = NULL;
    m_isCellCentered = cellCentered;
    m_isConstant = false;
    m_subSurfaceKind = kind;
    m_property = property;
    m_retrieved = false;
    m_geometryAssigned = false;
}

CauldronIO::Volume::~Volume()
{
    if (m_internalDataIJK) delete[] m_internalDataIJK;
    if (m_internalDataKIJ) delete[] m_internalDataKIJ;
    m_internalDataIJK = NULL;
    m_internalDataKIJ = NULL;
}

const boost::shared_ptr<const Property> CauldronIO::Volume::getProperty() const
{
    return m_property;
}

void CauldronIO::Volume::setData_KIJ(float* data, bool setValue /*= false*/, float value /*= 0*/)
{
    setData(data, &m_internalDataKIJ, setValue, value);
}

void CauldronIO::Volume::setData_IJK(float* data, bool setValue /*= false*/, float value /*= 0*/)
{
    setData(data, &m_internalDataIJK, setValue, value);
}

void CauldronIO::Volume::setData(float* data, float** internalData, bool setValue /*= false*/, float value /*= 0*/)
{
    if (!m_geometryAssigned) throw CauldronIOException("Cannot assign data without geometry known");

    // If our data buffer exists, we will just reuse it. Otherwise, allocate
    if (!*internalData)
    {
        try
        {
            *internalData = new float[m_numI * m_numJ * m_numK];
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
        memcpy(*internalData, data, sizeof(float) * m_numI * m_numJ * m_numK);
    }
    // assign a value if necessary
    else
    {
        std::fill(*internalData, *internalData + m_numI * m_numJ * m_numK, value);
    }

    m_retrieved = true;
}

void CauldronIO::Volume::setGeometry(size_t numI, size_t numJ, size_t numK, size_t offsetK, double deltaI,
    double deltaJ, double minI, double minJ)
{
    m_numI = numI;
    m_numJ = numJ;
    m_numK = numK;
    m_deltaI = deltaI;
    m_deltaJ = deltaJ;
    m_maxI = minI + deltaI * numI;
    m_maxJ = minJ + deltaJ * numJ;
    m_minI = minI;
    m_minJ = minJ;
    m_firstK = offsetK;
    m_geometryAssigned = true;
    m_retrieved = false;
}

size_t CauldronIO::Volume::getNumI() const
{
    return m_numI;
}

size_t CauldronIO::Volume::getNumJ() const
{
    return m_numJ;
}

double CauldronIO::Volume::getDeltaI() const
{
    return m_deltaI;
}

double CauldronIO::Volume::getDeltaJ() const
{
    return m_deltaJ;
}

double CauldronIO::Volume::getMinI() const
{
    return m_minI;
}

double CauldronIO::Volume::getMinJ() const
{
    return m_minJ;
}

double CauldronIO::Volume::getMaxI() const
{
    return m_maxI;
}

double CauldronIO::Volume::getMaxJ() const
{
    return m_maxJ;
}


bool CauldronIO::Volume::hasDataIJK() const
{
    return m_internalDataIJK != NULL;
}


bool CauldronIO::Volume::hasDataKIJ() const
{
    return m_internalDataKIJ != NULL;
}

float CauldronIO::Volume::getUndefinedValue() const
{
    return m_undefinedValue;
}

bool CauldronIO::Volume::isConstant() const
{
    return m_isConstant;
}

void CauldronIO::Volume::setConstantValue(float value)
{
    m_isConstant = true;
    m_constantValue = value;
    m_retrieved = true;
}

size_t CauldronIO::Volume::getNumK() const
{
    return m_numK;
}

size_t CauldronIO::Volume::getFirstK() const
{
    return m_firstK;
}

size_t CauldronIO::Volume::getLastK() const
{
    return m_firstK + m_numK - 1;
}

bool CauldronIO::Volume::isUndefined(size_t i, size_t j, size_t k) const
{
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (m_isConstant) return m_constantValue == m_undefinedValue;
    if (m_internalDataIJK) return m_internalDataIJK[computeIndex_IJK(i, j, k)] == m_undefinedValue;
    
    assert(m_internalDataKIJ);
    return m_internalDataKIJ[computeIndex_KIJ(i, j, k)] == m_undefinedValue;
}


void CauldronIO::Volume::setUndefinedValue(float undefined)
{
    m_undefinedValue = undefined;
}

float CauldronIO::Volume::getValue(size_t i, size_t j, size_t k) const
{
    if (!m_geometryAssigned) throw CauldronIOException("Need to assign a geometry first!");
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (m_isConstant) return m_constantValue;
    if (m_internalDataIJK) return m_internalDataIJK[computeIndex_IJK(i, j, k)];

    assert(m_internalDataKIJ);
    return m_internalDataKIJ[computeIndex_KIJ(i, j, k)];
}

float const * CauldronIO::Volume::getRowValues(size_t j, size_t k)
{
    if (!m_geometryAssigned) throw CauldronIOException("Need to assign a geometry first!");
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (!hasDataIJK()) throw CauldronIOException("Cannot return row values");

    // Create our internal buffer if not existing
    if (!m_internalDataIJK && m_isConstant) setData_IJK(NULL, true, m_constantValue);

    // Assume IJK ordering
    assert(m_internalDataIJK);
    return m_internalDataIJK + computeIndex_IJK(0, j, k);
}

float const * CauldronIO::Volume::getColumnValues(size_t i, size_t k)
{
    throw CauldronIOException("Not implemented");
}

float const * CauldronIO::Volume::getNeedleValues(size_t i, size_t j)
{
    if (!m_geometryAssigned) throw CauldronIOException("Need to assign a geometry first!");
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (!hasDataKIJ() || isConstant()) throw CauldronIOException("Cannot return needle values");

    // Create our internal buffer if not existing
    if (!m_internalDataKIJ && m_isConstant) setData_KIJ(NULL, true, m_constantValue);

    return m_internalDataKIJ + computeIndex_KIJ(i, j, m_firstK);
}

float const * CauldronIO::Volume::getSurface_IJ(size_t k)
{
    if (!m_geometryAssigned) throw CauldronIOException("Need to assign a geometry first!");
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (!hasDataIJK() || isConstant()) throw CauldronIOException("Cannot return surface values");

    // Create our internal buffer if not existing
    if (!m_internalDataIJK && m_isConstant) setData_IJK(NULL, true, m_constantValue);

    return m_internalDataIJK + computeIndex_IJK(0, 0, k);
}

float const * CauldronIO::Volume::getVolumeValues_KIJ()
{
    if (!m_geometryAssigned) throw CauldronIOException("Need to assign a geometry first!");
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (!hasDataKIJ() || isConstant()) throw CauldronIOException("Cannot return volume values");

    // Create our internal buffer if not existing
    if (!m_internalDataKIJ && m_isConstant) setData_KIJ(NULL, true, m_constantValue);

    return m_internalDataKIJ;
}

float const * CauldronIO::Volume::getVolumeValues_IJK()
{
    if (!m_geometryAssigned) throw CauldronIOException("Need to assign a geometry first!");
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (!hasDataIJK() || isConstant()) throw CauldronIOException("Cannot return volume values");

    // Create our internal buffer if not existing
    if (!m_internalDataIJK && m_isConstant) setData_IJK(NULL, true, m_constantValue);

    return m_internalDataIJK;
}

size_t CauldronIO::Volume::computeIndex_IJK(size_t i, size_t j, size_t k) const
{
    assert(m_geometryAssigned);
    assert(i >= 0 && i < m_numI && j >= 0 && j < m_numJ && k >= m_firstK && k < m_numK + m_firstK);

    return (i + j * m_numI + (k -m_firstK) * m_numI * m_numJ);
}

size_t CauldronIO::Volume::computeIndex_KIJ(size_t i, size_t j, size_t k) const
{
    assert(m_geometryAssigned);
    assert(i >= 0 && i < m_numI && j >= 0 && j < m_numJ && k >= m_firstK && k < m_numK + m_firstK);

    return ((k - m_firstK) + i * m_numK + j * m_numI * m_numK);
}

const boost::uuids::uuid& CauldronIO::Volume::getUUID() const
{
    return m_uuid;
}

void CauldronIO::Volume::setUUID(const boost::uuids::uuid& uuid)
{
    m_uuid = uuid;
}

float CauldronIO::Volume::getConstantValue() const
{
    if (!isConstant())  throw CauldronIOException("Map does not have a constant value");
    return m_constantValue;
}

void CauldronIO::Volume::setDepthVolume(boost::shared_ptr<const Volume> depthVolume)
{
    m_depthVolume = depthVolume;
}

boost::shared_ptr<const Volume> CauldronIO::Volume::getDepthVolume() const
{
    return m_depthVolume;
}

CauldronIO::SubsurfaceKind CauldronIO::Volume::getSubSurfaceKind() const
{
    return m_subSurfaceKind;
}

void CauldronIO::Volume::retrieve()
{
    if (m_retrieved) return;
    throw CauldronIOException("Not implemented");
}

bool CauldronIO::Volume::isRetrieved() const
{
    return m_retrieved;
}

bool CauldronIO::Volume::isCellCentered() const
{
    return m_isCellCentered;
}

/// DiscontinuousVolume implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::DiscontinuousVolume::DiscontinuousVolume()
{
}

CauldronIO::DiscontinuousVolume::~DiscontinuousVolume()
{
     m_volumeList.clear();
}

void CauldronIO::DiscontinuousVolume::addVolume(boost::shared_ptr<Formation> formation, 
                                                boost::shared_ptr<Volume> volume)
{
    if (!formation) throw CauldronIOException("Cannot add subvolume: formation cannot be null");
    if (!volume) throw CauldronIOException("Cannot add subvolume: volume cannot be null");

    boost::shared_ptr<FormationVolume> pair(new FormationVolume(formation, volume));
    m_volumeList.push_back(pair);
}

const FormationVolumeList& CauldronIO::DiscontinuousVolume::getVolumeList() const
{
    return m_volumeList;
}

void CauldronIO::DiscontinuousVolume::setDepthVolume(boost::shared_ptr<const Volume> depthVolume)
{
    m_depthVolume = depthVolume;
}

boost::shared_ptr<const Volume> CauldronIO::DiscontinuousVolume::getDepthVolume() const
{
    return m_depthVolume;
}

void CauldronIO::DiscontinuousVolume::retrieve()
{   
    BOOST_FOREACH(boost::shared_ptr<FormationVolume>& pair, m_volumeList)
        pair->second->retrieve();
}

//////////////////////////////////////////////////////////////////////////
/// Trapper implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Trapper::Trapper(int ID, int persistentID)
{
    m_ID = ID;
    m_persistentID = persistentID;
    m_downstreamTrapperID = -1;
}

const std::string& CauldronIO::Trapper::getReservoirName() const
{
    return m_reservoir;
}

void CauldronIO::Trapper::setReservoirName(const std::string& reservoirName)
{
    m_reservoir = reservoirName;
}

float CauldronIO::Trapper::getSpillDepth() const
{
    return m_spillDepth;
}


void CauldronIO::Trapper::setSpillDepth(float depth)
{
    m_spillDepth = depth;
}

void CauldronIO::Trapper::getSpillPointPosition(float& posX, float& posY) const
{
    posX = m_spillPositionX;
    posY = m_spillPositionY;
}


void CauldronIO::Trapper::setSpillPointPosition(float posX, float posY)
{
    m_spillPositionX = posX;
    m_spillPositionY = posY;
}

int CauldronIO::Trapper::getID() const
{
    return m_ID;
}

int CauldronIO::Trapper::getPersistentID() const
{
    return m_persistentID;
}

boost::shared_ptr<const Trapper> CauldronIO::Trapper::getDownStreamTrapper() const
{
    return m_downstreamTrapper;
}

void CauldronIO::Trapper::setDownStreamTrapper(boost::shared_ptr<const Trapper> trapper)
{
    m_downstreamTrapper = trapper;
}

void CauldronIO::Trapper::setDownStreamTrapperID(int persistentID)
{
    m_downstreamTrapperID = persistentID;
}

int CauldronIO::Trapper::getDownStreamTrapperID() const
{
    return m_downstreamTrapperID;
}
