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

CauldronIO::Project::Project(const string& name, const string& description, const string& team, const string& version, ModellingMode mode, int xmlVersionMajor, int xmlVersionMinor)
{
    if (name.empty()) throw CauldronIOException("Project name cannot be empty");
    
    m_name = name;
    m_description = description;
    m_team = team;
    m_version = version;
    m_mode = mode;
    m_xmlVersionMajor = xmlVersionMajor;
    m_xmlVersionMinor = xmlVersionMinor;
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
    
    // Invalidate the surface names; the list may have changed now
    m_surfaceNames.clear();
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


const PropertyList& CauldronIO::Project::getProperties() const
{
    return m_propertyList;
}

boost::shared_ptr<const Property> CauldronIO::Project::findProperty(std::string propertyName) const
{
    BOOST_FOREACH(const boost::shared_ptr<const Property>& property, m_propertyList)
    {
        if (property->getName() == propertyName) return property;
    }

    return boost::shared_ptr<const Property>();
}

const FormationList& CauldronIO::Project::getFormations() const
{
    return m_formationList;
}

boost::shared_ptr<const Formation> CauldronIO::Project::findFormation(std::string formationName) const
{
    BOOST_FOREACH(const boost::shared_ptr<const Formation>& formation, m_formationList)
    {
        if (formation->getName() == formationName) return formation;
    }

    return boost::shared_ptr<const Formation>();
}

const std::vector<std::string>& CauldronIO::Project::getSurfaceNames() 
{
    // Construct the list if needed
    if (m_surfaceNames.size() == 0)
    {
        BOOST_FOREACH(boost::shared_ptr<SnapShot>& snapShot, m_snapShotList)
        {
            BOOST_FOREACH(const boost::shared_ptr<Surface>& surface, snapShot->getSurfaceList())
            {
                const string& surfaceName = surface->getName();
                if (surfaceName.empty()) continue;

                bool exists = false;
                BOOST_FOREACH(const string& existingSurface, m_surfaceNames)
                {
                    if (existingSurface == surfaceName)
                    {
                        exists = true;
                        break;
                    }
                }
                if (!exists) m_surfaceNames.push_back(surfaceName);
            }
        }
    }

    return m_surfaceNames;
}


int CauldronIO::Project::getXmlVersionMajor() const
{
    return m_xmlVersionMajor;
}

int CauldronIO::Project::getXmlVersionMinor() const
{
    return m_xmlVersionMinor;
}

const ReservoirList& CauldronIO::Project::getReservoirs() const
{
    return m_reservoirList;
}

boost::shared_ptr<const Reservoir> CauldronIO::Project::findReservoir(std::string reservoirName) const
{
    BOOST_FOREACH(const boost::shared_ptr<const Reservoir>& reservoir, m_reservoirList)
    {
        if (reservoir->getName() == reservoirName) return reservoir;
    }

    return boost::shared_ptr<const Reservoir>();
}

void CauldronIO::Project::addProperty(boost::shared_ptr<const Property>& newProperty)
{
    BOOST_FOREACH(boost::shared_ptr<const Property>& property, m_propertyList)
    {
        if (*property == *newProperty) return;
    }

    m_propertyList.push_back(newProperty);
}

void CauldronIO::Project::addFormation(boost::shared_ptr<const Formation>& newFormation)
{
    if (!newFormation) throw CauldronIOException("Cannot add empty formation");

    // Check if formation exists
    BOOST_FOREACH(boost::shared_ptr<const Formation>& formation, m_formationList)
        if (*formation == *newFormation) throw CauldronIOException("Cannot add formation twice");

    m_formationList.push_back(newFormation);
}

void CauldronIO::Project::addReservoir(boost::shared_ptr<const Reservoir>& newReservoir)
{
    if (!newReservoir) throw CauldronIOException("Cannot add empty reservoir");

    // Check if reservoir exists
    BOOST_FOREACH(const boost::shared_ptr<const Reservoir>& reservoir, m_reservoirList)
        if (*reservoir == *newReservoir) throw CauldronIOException("Cannot add reservoir twice");

    m_reservoirList.push_back(newReservoir);
}

void CauldronIO::Project::retrieve()
{
    BOOST_FOREACH(boost::shared_ptr<SnapShot>& snapShot, m_snapShotList)
        snapShot->retrieve();
}

void CauldronIO::Project::release()
{
    BOOST_FOREACH(boost::shared_ptr<SnapShot>& snapShot, m_snapShotList)
        snapShot->release();
}

/// SnapShot implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::SnapShot::SnapShot(double age, SnapShotKind kind, bool isMinorShapshot)
{
    if (age < 0) throw CauldronIO::CauldronIOException("SnapShot age cannot be negative");
    m_age = age;
    m_kind = kind;
    m_isMinor = isMinorShapshot;
}

CauldronIO::SnapShot::~SnapShot()
{
    // Delete all snapshots
    m_volume.reset();
    m_formationVolumeList.clear();
    m_surfaceList.clear();
    m_trapperList.clear();
}

void CauldronIO::SnapShot::setVolume(boost::shared_ptr<Volume>& volume)
{
    m_volume = volume;
}

void CauldronIO::SnapShot::addSurface(boost::shared_ptr<Surface>& newSurface)
{
    if (!newSurface) throw CauldronIOException("Cannot add empty surface");

    // Check if surface exists
    BOOST_FOREACH(boost::shared_ptr<Surface>& surface, m_surfaceList)
        if (surface == newSurface) throw CauldronIOException("Cannot add surface twice");

    m_surfaceList.push_back(newSurface);
}

void CauldronIO::SnapShot::addFormationVolume(FormationVolume& formVolume)
{
    // Check if volume exists
    BOOST_FOREACH(FormationVolume& volume, m_formationVolumeList)
        if (volume == formVolume) throw CauldronIOException("Cannot add volume twice");

    m_formationVolumeList.push_back(formVolume);
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

const boost::shared_ptr<Volume>& CauldronIO::SnapShot::getVolume() const
{
    return m_volume;
}


const FormationVolumeList& CauldronIO::SnapShot::getFormationVolumeList() const
{
    return m_formationVolumeList;
}

const TrapperList& CauldronIO::SnapShot::getTrapperList() const
{
    return m_trapperList;
}

void CauldronIO::SnapShot::retrieve()
{
    if (m_volume)
        m_volume->retrieve();
    BOOST_FOREACH(FormationVolume& formVolume, m_formationVolumeList)
        formVolume.second->retrieve();
    BOOST_FOREACH(boost::shared_ptr<Surface>& surface, m_surfaceList)
        surface->retrieve();
}

void CauldronIO::SnapShot::release()
{
    if (m_volume)
        m_volume->release();
    BOOST_FOREACH(FormationVolume& formVolume, m_formationVolumeList)
        formVolume.second->release();
    BOOST_FOREACH(boost::shared_ptr<Surface>& surface, m_surfaceList)
        surface->release();
}

/// Property implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Property::Property(const string& name, const string& username, const string& cauldronName, const string& unit, PropertyType type, PropertyAttribute attrib)
{
    if (name.empty()) throw CauldronIOException("Property name cannot be empty");
    if (username.empty()) throw CauldronIOException("User name cannot be empty");
    if (cauldronName.empty()) throw CauldronIOException("Cauldron name cannot be empty");

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

bool CauldronIO::Property::isHighRes() const
{
    if (m_name.find("HighRes") != std::string::npos) return true;
    if (m_name.find("ResRock") != std::string::npos) return true;
    return false;
}

bool CauldronIO::Property::operator==(const Property& other) const
{
    return this->m_name == other.m_name;
}

/// Formation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Formation::Formation(size_t kStart, size_t kEnd, const string& name, bool isSourceRock, bool isMobileLayer)
{
    if (name.empty()) throw CauldronIOException("Formation name cannot be empty");
    
    m_kstart = kStart;
    m_kend = kEnd;
    m_name = name;
    m_isSourceRock = isSourceRock;
    m_isMobileLayer = isMobileLayer;
}

void CauldronIO::Formation::getK_Range(unsigned int &start, unsigned int &end) const
{
    start = (unsigned int)m_kstart;
    end = (unsigned int)m_kend;
}

const string& CauldronIO::Formation::getName() const
{
    return m_name;
}

bool CauldronIO::Formation::isSourceRock() const
{
    return m_isSourceRock;
}


bool CauldronIO::Formation::isMobileLayer() const
{
    return m_isMobileLayer;
}

bool CauldronIO::Formation::operator==(const Formation& other) const
{
    return m_name == other.m_name;
}

/// Surface implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Surface::Surface(const std::string& name, SubsurfaceKind kind)
{
    m_name = name;
    m_subSurfaceKind = kind;
    m_propSurfaceList.clear();
}

CauldronIO::Surface::~Surface()
{
    m_propSurfaceList.clear();
    m_geometry.reset();
}

void CauldronIO::Surface::setGeometry(boost::shared_ptr<const Geometry2D>& geometry)
{
    m_geometry = geometry;
}

void CauldronIO::Surface::setHighResGeometry(boost::shared_ptr<const Geometry2D>& geometry)
{
    m_highresgeometry = geometry;
}

const boost::shared_ptr<const Geometry2D>& CauldronIO::Surface::getGeometry() const
{
    return m_geometry;
}

const boost::shared_ptr<const Geometry2D>& CauldronIO::Surface::getHighResGeometry() const
{
    return m_highresgeometry;
}

const PropertySurfaceDataList& CauldronIO::Surface::getPropertySurfaceDataList() const
{
    return m_propSurfaceList;
}

void CauldronIO::Surface::addPropertySurfaceData(PropertySurfaceData& newData)
{
    BOOST_FOREACH(PropertySurfaceData& data, m_propSurfaceList)
        if (data == newData) throw CauldronIOException("Cannot add property-surfaceData twice");

    m_propSurfaceList.push_back(newData);
}

bool CauldronIO::Surface::hasDepthSurface() const
{
    BOOST_FOREACH(const PropertySurfaceData& data, m_propSurfaceList)
        if (data.first->getName() == "Depth") return true;

    return false;
}

boost::shared_ptr<SurfaceData> CauldronIO::Surface::getDepthSurface() const
{
    BOOST_FOREACH(const PropertySurfaceData& data, m_propSurfaceList)
        if (data.first->getName() == "Depth") return data.second;

    return boost::shared_ptr<SurfaceData>();
}

const string& CauldronIO::Surface::getName() const
{
    return m_name;
}

CauldronIO::SubsurfaceKind CauldronIO::Surface::getSubSurfaceKind() const
{
    return m_subSurfaceKind;
}

void CauldronIO::Surface::setFormation(boost::shared_ptr<const Formation>& formation, bool isTopFormation)
{
    if (isTopFormation)
        m_Topformation = formation;
    else
        m_Bottomformation = formation;
}

const boost::shared_ptr<const Formation>& CauldronIO::Surface::getTopFormation() const
{
    return m_Topformation;
}

const boost::shared_ptr<const Formation>& CauldronIO::Surface::getBottomFormation() const
{
    return m_Bottomformation;
}

void CauldronIO::Surface::retrieve()
{
    BOOST_FOREACH(PropertySurfaceData& data, m_propSurfaceList)
    {
        data.second->retrieve();
    }
}

void CauldronIO::Surface::release()
{
    BOOST_FOREACH(PropertySurfaceData& data, m_propSurfaceList)
    {
        data.second->release();
    }
}

bool CauldronIO::Surface::isRetrieved() const
{
    BOOST_FOREACH(const PropertySurfaceData& data, m_propSurfaceList)
    {
        if (!data.second->isRetrieved()) return false;
    }

    return true;
}

/// Geometry2D Implementation
///////////////////////////////////////

CauldronIO::Geometry2D::Geometry2D(size_t numI, size_t numJ, double deltaI, double deltaJ, double minI, double minJ)
{
    m_numI = numI;
    m_numJ = numJ;
    m_deltaI = deltaI;
    m_deltaJ = deltaJ;
    m_minI = minI;
    m_minJ = minJ;
    m_maxI = minI + deltaI * numI;
    m_maxJ = minJ + deltaJ * numJ;
}

size_t CauldronIO::Geometry2D::getNumI() const
{
    return m_numI;
}

size_t CauldronIO::Geometry2D::getNumJ() const
{
    return m_numJ;
}

double CauldronIO::Geometry2D::getDeltaI() const
{
    return m_deltaI;
}

double CauldronIO::Geometry2D::getDeltaJ() const
{
    return m_deltaJ;
}

double CauldronIO::Geometry2D::getMinI() const
{
    return m_minI;
}

double CauldronIO::Geometry2D::getMinJ() const
{
    return m_minJ;
}

double CauldronIO::Geometry2D::getMaxI() const
{
    return m_maxI;
}

double CauldronIO::Geometry2D::getMaxJ() const
{
    return m_maxJ;
}

size_t CauldronIO::Geometry2D::getSize() const
{
    return m_numI * m_numJ;
}


bool CauldronIO::Geometry2D::operator==(const Geometry2D& other) const
{
    return
        m_numI == other.m_numI && m_numJ == other.m_numJ &&
        m_deltaI == other.m_deltaI && m_deltaJ == other.m_deltaJ &&
        m_minI == other.m_minI && m_minJ == other.m_minJ;
}

// Map implementation
//////////////////////////////////////////////////////////////////////////


CauldronIO::SurfaceData::SurfaceData(const boost::shared_ptr<const Geometry2D>& geometry)
{
    // For performance reasons, we cache the data locally
    m_numI   = geometry->getNumI();
    m_numJ   = geometry->getNumJ();
    m_deltaI = geometry->getDeltaI();
    m_deltaJ = geometry->getDeltaJ();
    m_minI   = geometry->getMinI();
    m_maxI   = geometry->getMaxI();
    m_minJ   = geometry->getMinJ();
    m_maxJ   = geometry->getMaxJ(); 
    m_isConstant = false;
    m_retrieved = false;
    m_geometry = geometry;

    // Indexing into the map is unknown
    m_internalData = NULL;
    m_reservoir.reset();
    m_formation.reset();
}

float CauldronIO::SurfaceData::getUndefinedValue() const
{
    return m_undefinedValue;
}

void CauldronIO::SurfaceData::setUndefinedValue(float undefined)
{
    m_undefinedValue = undefined;
}

void CauldronIO::SurfaceData::setFormation(boost::shared_ptr<const Formation>& formation)
{
    m_formation = formation;
}

const boost::shared_ptr<const Formation>& CauldronIO::SurfaceData::getFormation() const
{
    return m_formation;
}

void CauldronIO::SurfaceData::setReservoir(boost::shared_ptr<const Reservoir> reservoir)
{
    m_reservoir = reservoir;
}

const boost::shared_ptr<const Reservoir>& CauldronIO::SurfaceData::getReservoir() const
{
    return m_reservoir;
}

bool CauldronIO::SurfaceData::isConstant() const
{
    return m_isConstant;
}

void CauldronIO::SurfaceData::setConstantValue(float value)
{
    m_isConstant = true;
    m_constantValue = value;
    m_retrieved = true;
}

CauldronIO::SurfaceData::~SurfaceData()
{
    if (m_internalData) delete[] m_internalData;
    m_internalData = NULL;
}

const boost::shared_ptr<const Geometry2D>& CauldronIO::SurfaceData::getGeometry() const
{
    return m_geometry;
}

void CauldronIO::SurfaceData::setData_IJ(float* data)
{
    setData(data);
}

void CauldronIO::SurfaceData::setData(float* data, bool setValue, float value)
{
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

bool CauldronIO::SurfaceData::canGetRow() const
{
    return true;
}

bool CauldronIO::SurfaceData::canGetColumn() const
{
    // Not implemented at this point
    return false;
}

bool CauldronIO::SurfaceData::isUndefined(size_t i, size_t j) const
{
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (m_isConstant) return m_constantValue == m_undefinedValue;

    return m_internalData[getMapIndex(i, j)] == m_undefinedValue;
}

float CauldronIO::SurfaceData::getValue(size_t i, size_t j) const
{
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (m_isConstant) return m_constantValue;
    
    return m_internalData[getMapIndex(i, j)];
}

float const * CauldronIO::SurfaceData::getRowValues(size_t j)
{
    if (!canGetRow()) throw CauldronIOException("Cannot return row values");
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");

    // Create our internal buffer if not existing
    if (!m_internalData && m_isConstant) setData(NULL, true, m_constantValue);

    return m_internalData + getMapIndex(0, j);
}

float const * CauldronIO::SurfaceData::getColumnValues(size_t i)
{
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (!canGetColumn()) throw CauldronIOException("Cannot return column values");

    // Create our internal buffer if not existing
    if (!m_internalData && m_isConstant) setData(NULL, true, m_constantValue);

    return m_internalData + getMapIndex(i, 0);
}

float const * CauldronIO::SurfaceData::getSurfaceValues()
{
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");

    // Create our internal buffer if not existing
    if (!m_internalData && m_isConstant) setData(NULL, true, m_constantValue);

    return m_internalData;
}

size_t CauldronIO::SurfaceData::getMapIndex(size_t i, size_t j) const
{
    assert(i >= 0 && i < m_numI && j >= 0 && j < m_numJ);
    return m_numI * j + i;
}

void CauldronIO::SurfaceData::retrieve()
{
    if (m_retrieved) return;
    throw CauldronIOException("Not implemented");
}

void CauldronIO::SurfaceData::release()
{
    if (m_internalData) delete[] m_internalData;
    m_internalData = NULL;
    m_retrieved = false;
}

bool CauldronIO::SurfaceData::isRetrieved() const
{
    return m_retrieved;
}

float CauldronIO::SurfaceData::getConstantValue() const
{
    if (!isConstant()) throw CauldronIOException("Map does not have a constant value");
    return m_constantValue;
}

/// Volume implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::Volume::Volume(SubsurfaceKind kind, boost::shared_ptr<const Geometry3D> geometry)
{
    m_subSurfaceKind = kind;
    m_geometry = geometry;
}

CauldronIO::Volume::~Volume()
{
}

CauldronIO::SubsurfaceKind CauldronIO::Volume::getSubSurfaceKind() const
{
    return m_subSurfaceKind;
}


const boost::shared_ptr<const Geometry3D>& CauldronIO::Volume::getGeometry() const
{
    return m_geometry;
}

const PropertyVolumeDataList& CauldronIO::Volume::getPropertyVolumeDataList() const
{
    return m_propVolumeList;
}

void CauldronIO::Volume::addPropertyVolumeData(PropertyVolumeData& newData)
{
    BOOST_FOREACH(PropertyVolumeData& data, m_propVolumeList)
        if (data == newData) throw CauldronIOException("Cannot add property-volumeData twice");

    m_propVolumeList.push_back(newData);
}

bool CauldronIO::Volume::hasDepthVolume() const
{
    BOOST_FOREACH(const PropertyVolumeData& data, m_propVolumeList)
        if (data.first->getName() == "Depth") return true;

    return false;
}

boost::shared_ptr<VolumeData> CauldronIO::Volume::getDepthVolume() const
{
    BOOST_FOREACH(const PropertyVolumeData& data, m_propVolumeList)
        if (data.first->getName() == "Depth") return data.second;

    return boost::shared_ptr<VolumeData>();
}

void CauldronIO::Volume::retrieve()
{
    BOOST_FOREACH(PropertyVolumeData& data, m_propVolumeList)
        data.second->retrieve();
}

bool CauldronIO::Volume::isRetrieved() const
{
    BOOST_FOREACH(const PropertyVolumeData& data, m_propVolumeList)
        if (!data.second->isRetrieved()) return false;
    return true;
}

void CauldronIO::Volume::release()
{
    BOOST_FOREACH(PropertyVolumeData& data, m_propVolumeList)
        data.second->release();
}

/// Geometry3D implementation
/////////////////////////////////////////////////////////////////////////////

Geometry3D::Geometry3D(size_t numI, size_t numJ, size_t numK, size_t offsetK, double deltaI,
    double deltaJ, double minI, double minJ) : Geometry2D(numI, numK, deltaI, deltaJ, minI, minJ)
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
}

size_t CauldronIO::Geometry3D::getNumK() const
{
    return m_numK;
}

size_t CauldronIO::Geometry3D::getFirstK() const
{
    return m_firstK;
}

size_t CauldronIO::Geometry3D::getLastK() const
{
    return m_firstK + m_numK - 1;
}

size_t CauldronIO::Geometry3D::getSize() const
{
    return m_numI * m_numJ * m_numK;
}


bool CauldronIO::Geometry3D::operator==(const Geometry3D& other) const
{
    return
        m_numI == other.m_numI && m_numJ == other.m_numJ &&
        m_deltaI == other.m_deltaI && m_deltaJ == other.m_deltaJ &&
        m_minI == other.m_minI && m_minJ == other.m_minJ &&
        m_numK == other.m_numK && m_firstK == other.m_firstK;
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

float CauldronIO::Trapper::getDepth() const
{
  return m_depth;
}

void CauldronIO::Trapper::setDepth(float depth)
{
  m_depth = depth;
}

void CauldronIO::Trapper::getPosition(float& posX, float& posY) const
{
  posX = m_positionX;
  posY = m_positionY;
}

void CauldronIO::Trapper::setPosition(float posX, float posY)
{
  m_positionX = posX;
  m_positionY = posY;
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

//////////////////////////////////////////////////////////////////////////
/// Reservoir implementation
//////////////////////////////////////////////////////////////////////////

bool CauldronIO::Reservoir::operator==(const Reservoir& other) const
{
    if (m_reservoirName != other.m_reservoirName) return false;
    if (m_formation != other.m_formation) return false;

    return true;
}

CauldronIO::Reservoir::Reservoir(const std::string& reservoirName, const boost::shared_ptr<const Formation>& formation)
{
    m_reservoirName = reservoirName;
    m_formation = formation;
}

const std::string& CauldronIO::Reservoir::getName() const
{
    return m_reservoirName;
}

const boost::shared_ptr<const Formation>& CauldronIO::Reservoir::getFormation() const
{
    return m_formation;
}

/// VolumeData implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::VolumeData::VolumeData(const boost::shared_ptr<const Geometry3D>& geometry)
{
    m_internalDataIJK = NULL;
    m_internalDataKIJ = NULL;
    m_isConstant = false;
    m_retrieved = false;

    // Cache all geometry
    m_numI = geometry->getNumI();
    m_numJ = geometry->getNumJ();
    m_firstK = geometry->getFirstK();
    m_numK = geometry->getNumK();
    m_deltaI = geometry->getDeltaI();
    m_deltaJ = geometry->getDeltaJ();
    m_minI = geometry->getMinI();
    m_maxI = geometry->getMaxI();
    m_minJ = geometry->getMinJ();
    m_maxJ = geometry->getMaxJ();
    m_lastK = geometry->getLastK();

    m_geometry = geometry;
}

CauldronIO::VolumeData::~VolumeData()
{
    if (m_internalDataIJK) delete[] m_internalDataIJK;
    if (m_internalDataKIJ) delete[] m_internalDataKIJ;
    m_internalDataIJK = NULL;
    m_internalDataKIJ = NULL;
}


const boost::shared_ptr<const Geometry3D>& CauldronIO::VolumeData::getGeometry() const
{
    return m_geometry;
}

bool CauldronIO::VolumeData::isUndefined(size_t i, size_t j, size_t k) const
{
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (m_isConstant) return m_constantValue == m_undefinedValue;
    if (m_internalDataIJK) return m_internalDataIJK[computeIndex_IJK(i, j, k)] == m_undefinedValue;

    assert(m_internalDataKIJ);
    return m_internalDataKIJ[computeIndex_KIJ(i, j, k)] == m_undefinedValue;
}

void CauldronIO::VolumeData::setUndefinedValue(float undefined)
{
    m_undefinedValue = undefined;
}

float CauldronIO::VolumeData::getValue(size_t i, size_t j, size_t k) const
{
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (m_isConstant) return m_constantValue;
    if (m_internalDataIJK) return m_internalDataIJK[computeIndex_IJK(i, j, k)];

    assert(m_internalDataKIJ);
    return m_internalDataKIJ[computeIndex_KIJ(i, j, k)];
}

float const * CauldronIO::VolumeData::getRowValues(size_t j, size_t k)
{
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (!hasDataIJK()) throw CauldronIOException("Cannot return row values");

    // Create our internal buffer if not existing
    if (!m_internalDataIJK && m_isConstant) setData_IJK(NULL, true, m_constantValue);

    // Assume IJK ordering
    assert(m_internalDataIJK);
    return m_internalDataIJK + computeIndex_IJK(0, j, k);
}

float const * CauldronIO::VolumeData::getColumnValues(size_t i, size_t k)
{
    throw CauldronIOException("Not implemented");
}

float const * CauldronIO::VolumeData::getNeedleValues(size_t i, size_t j)
{
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (!hasDataKIJ() || isConstant()) throw CauldronIOException("Cannot return needle values");

    // Create our internal buffer if not existing
    if (!m_internalDataKIJ && m_isConstant) setData_KIJ(NULL, true, m_constantValue);

    return m_internalDataKIJ + computeIndex_KIJ(i, j, m_firstK);
}

float const * CauldronIO::VolumeData::getSurface_IJ(size_t k)
{
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (!hasDataIJK() || isConstant()) throw CauldronIOException("Cannot return surface values");

    // Create our internal buffer if not existing
    if (!m_internalDataIJK && m_isConstant) setData_IJK(NULL, true, m_constantValue);

    return m_internalDataIJK + computeIndex_IJK(0, 0, k);
}

float const * CauldronIO::VolumeData::getVolumeValues_KIJ()
{
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (!hasDataKIJ() || isConstant()) throw CauldronIOException("Cannot return volume values");

    // Create our internal buffer if not existing
    if (!m_internalDataKIJ && m_isConstant) setData_KIJ(NULL, true, m_constantValue);

    return m_internalDataKIJ;
}

float const * CauldronIO::VolumeData::getVolumeValues_IJK()
{
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (!hasDataIJK() || isConstant()) throw CauldronIOException("Cannot return volume values");

    // Create our internal buffer if not existing
    if (!m_internalDataIJK && m_isConstant) setData_IJK(NULL, true, m_constantValue);

    return m_internalDataIJK;
}

void CauldronIO::VolumeData::setData_KIJ(float* data, bool setValue /*= false*/, float value /*= 0*/)
{
    setData(data, &m_internalDataKIJ, setValue, value);
}

void CauldronIO::VolumeData::setData_IJK(float* data, bool setValue /*= false*/, float value /*= 0*/)
{
    setData(data, &m_internalDataIJK, setValue, value);
}

void CauldronIO::VolumeData::setData(float* data, float** internalData, bool setValue /*= false*/, float value /*= 0*/)
{
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

size_t CauldronIO::VolumeData::computeIndex_IJK(size_t i, size_t j, size_t k) const
{
    assert(i >= 0 && i < m_numI && j >= 0 && j < m_numJ && k >= m_firstK && k < m_numK + m_firstK);

    return (i + j * m_numI + (k - m_firstK) * m_numI * m_numJ);
}

size_t CauldronIO::VolumeData::computeIndex_KIJ(size_t i, size_t j, size_t k) const
{
    assert(i >= 0 && i < m_numI && j >= 0 && j < m_numJ && k >= m_firstK && k < m_numK + m_firstK);

    return ((k - m_firstK) + i * m_numK + j * m_numI * m_numK);
}

float CauldronIO::VolumeData::getConstantValue() const
{
    if (!isConstant())  throw CauldronIOException("Map does not have a constant value");
    return m_constantValue;
}


void CauldronIO::VolumeData::retrieve()
{
    if (m_retrieved) return;
    throw CauldronIOException("Not implemented");
}

bool CauldronIO::VolumeData::isRetrieved() const
{
    return m_retrieved;
}

void CauldronIO::VolumeData::release()
{
    if (m_internalDataIJK) delete[] m_internalDataIJK;
    if (m_internalDataKIJ) delete[] m_internalDataKIJ;
    m_internalDataIJK = NULL;
    m_internalDataKIJ = NULL;
    m_retrieved = false;
}

bool CauldronIO::VolumeData::hasDataIJK() const
{
    return m_internalDataIJK != NULL;
}

bool CauldronIO::VolumeData::hasDataKIJ() const
{
    return m_internalDataKIJ != NULL;
}

float CauldronIO::VolumeData::getUndefinedValue() const
{
    return m_undefinedValue;
}

bool CauldronIO::VolumeData::isConstant() const
{
    return m_isConstant;
}

void CauldronIO::VolumeData::setConstantValue(float value)
{
    m_isConstant = true;
    m_constantValue = value;
    m_retrieved = true;
}

