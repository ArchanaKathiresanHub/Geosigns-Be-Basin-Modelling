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

void CauldronIO::Project::addSnapShot(std::shared_ptr<SnapShot>& newSnapShot)
{
    if (!newSnapShot) throw CauldronIOException("Cannot add empty snapshot");
    
    // Check if snapshot exists
    BOOST_FOREACH(std::shared_ptr<SnapShot>& snapShot, m_snapShotList)
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

bool CauldronIO::Project::operator==(const Project& other) const
{
	return
		m_mode == other.m_mode &&
		m_version == other.m_version &&
		m_team == other.m_team &&
		m_description == other.m_description &&
		m_name == other.m_name;
}

const SnapShotList& CauldronIO::Project::getSnapShots() const
{
    return m_snapShotList;
}


const PropertyList& CauldronIO::Project::getProperties() const
{
    return m_propertyList;
}


const GeometryList& CauldronIO::Project::getGeometries() const
{
    return m_geometries;
}

std::shared_ptr<const Property> CauldronIO::Project::findProperty(std::string propertyName) const
{
    BOOST_FOREACH(const std::shared_ptr<const Property>& property, m_propertyList)
    {
        if (property->getName() == propertyName) return property;
    }

    return std::shared_ptr<const Property>();
}

const FormationList& CauldronIO::Project::getFormations() const
{
    return m_formationList;
}

std::shared_ptr<Formation> CauldronIO::Project::findFormation(std::string formationName) const
{
    BOOST_FOREACH(const std::shared_ptr<Formation>& formation, m_formationList)
    {
        if (formation->getName() == formationName) return formation;
    }

    return std::shared_ptr<Formation>();
}

int CauldronIO::Project::getXmlVersionMajor() const
{
    return m_xmlVersionMajor;
}

int CauldronIO::Project::getXmlVersionMinor() const
{
    return m_xmlVersionMinor;
}

const std::vector<CauldronIO::StratigraphyTableEntry>& CauldronIO::Project::getStratigraphyTable() const
{
	return m_stratTable;
}

void CauldronIO::Project::addStratigraphyTableEntry(StratigraphyTableEntry entry)
{
	m_stratTable.push_back(entry);
}


void CauldronIO::Project::retrieveStratigraphyTable()
{
	for (auto& entry : m_stratTable)
	{
		if (entry.getFormation())
		{
			entry.getFormation()->retrieve();
		}
		if (entry.getSurface())
		{
			entry.getSurface()->retrieve();
		}
	}
}

const ReservoirList& CauldronIO::Project::getReservoirs() const
{
    return m_reservoirList;
}

std::shared_ptr<const Reservoir> CauldronIO::Project::findReservoir(std::string reservoirName) const
{
    BOOST_FOREACH(const std::shared_ptr<const Reservoir>& reservoir, m_reservoirList)
    {
        if (reservoir->getName() == reservoirName) return reservoir;
    }

    return std::shared_ptr<const Reservoir>();
}

void CauldronIO::Project::addProperty(std::shared_ptr<const Property>& newProperty)
{
    BOOST_FOREACH(std::shared_ptr<const Property>& property, m_propertyList)
    {
        if (*property == *newProperty) return;
    }

    m_propertyList.push_back(newProperty);
}

void CauldronIO::Project::addFormation(std::shared_ptr<Formation>& newFormation)
{
    if (!newFormation) throw CauldronIOException("Cannot add empty formation");

    // Check if formation exists
    BOOST_FOREACH(std::shared_ptr<Formation>& formation, m_formationList)
        if (*formation == *newFormation) throw CauldronIOException("Cannot add formation twice");

    m_formationList.push_back(newFormation);
}

void CauldronIO::Project::addReservoir(std::shared_ptr<const Reservoir>& newReservoir)
{
    if (!newReservoir) throw CauldronIOException("Cannot add empty reservoir");

    // Check if reservoir exists
    BOOST_FOREACH(const std::shared_ptr<const Reservoir>& reservoir, m_reservoirList)
        if (*reservoir == *newReservoir) throw CauldronIOException("Cannot add reservoir twice");

    m_reservoirList.push_back(newReservoir);
}

void CauldronIO::Project::addGeometry(const std::shared_ptr<const Geometry2D>& newGeometry)
{
    if (!newGeometry) throw CauldronIOException("Cannot add empty geometry");

    // Check if exists
    BOOST_FOREACH(const std::shared_ptr<const Geometry2D>& geometry, m_geometries)
        if (*geometry == *newGeometry) return;

    m_geometries.push_back(newGeometry);
}

size_t CauldronIO::Project::getGeometryIndex(const std::shared_ptr<const Geometry2D>& newGeometry) const
{
    if (!newGeometry) throw CauldronIOException("Cannot find empty geometry");

    // Check if exists
    for (size_t i = 0; i < m_geometries.size(); i++)
    {
        if (*m_geometries.at(i) == *newGeometry) return i;
    }

    throw CauldronIOException("Geometry not found");
}

void CauldronIO::Project::release()
{
    BOOST_FOREACH(std::shared_ptr<SnapShot>& snapShot, m_snapShotList)
        snapShot->release();

	// Release strat-table
	for (auto& entry : m_stratTable)
	{
		if (entry.getSurface())
		{
			entry.getSurface()->release();
		}
		if (entry.getFormation())
		{
			entry.getFormation()->release();
		}
	}

	m_stratTable.clear();
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

void CauldronIO::SnapShot::setVolume(std::shared_ptr<Volume>& volume)
{
    m_volume = volume;
}

void CauldronIO::SnapShot::addSurface(std::shared_ptr<Surface>& newSurface)
{
    if (!newSurface) throw CauldronIOException("Cannot add empty surface");

    // Check if surface exists
    BOOST_FOREACH(std::shared_ptr<Surface>& surface, m_surfaceList)
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


void CauldronIO::SnapShot::addTrapper(std::shared_ptr<Trapper>& newTrapper)
{
    if (!newTrapper) throw CauldronIOException("Cannot add empty trapper");

    BOOST_FOREACH(std::shared_ptr<Trapper>& trapper, m_trapperList)
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

const std::shared_ptr<Volume>& CauldronIO::SnapShot::getVolume() const
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

std::vector < VisualizationIOData* > CauldronIO::SnapShot::getAllRetrievableData() const
{
    // Collect all data to retrieve
    //////////////////////////////////////////////////////////////////////////
    std::vector < VisualizationIOData* > allReadData;
    for (const std::shared_ptr<Surface>& surfaceIO: getSurfaceList())
    {
        for (const PropertySurfaceData& propertySurfaceData : surfaceIO->getPropertySurfaceDataList())
        {
            VisualizationIOData* surfaceData = propertySurfaceData.second.get();
            if (!surfaceData->isRetrieved())
                allReadData.push_back(surfaceData);
        }
    }

    if (getVolume())
    {
        for (const PropertyVolumeData& propVolume: getVolume()->getPropertyVolumeDataList())
        {
            VisualizationIOData* data = propVolume.second.get();
            if (!data->isRetrieved())
                allReadData.push_back(data);
        }
    }

    for (const FormationVolume& formVolume: getFormationVolumeList())
    {
        const std::shared_ptr<Volume> subVolume = formVolume.second;
        for (const PropertyVolumeData& propVolume : subVolume->getPropertyVolumeDataList())
        {
            VisualizationIOData* data = propVolume.second.get();
            if (!data->isRetrieved())
                allReadData.push_back(data);
        }
    }

    return allReadData;
}

void CauldronIO::SnapShot::retrieve()
{
    if (m_volume)
        m_volume->retrieve();
    BOOST_FOREACH(FormationVolume& formVolume, m_formationVolumeList)
        formVolume.second->retrieve();
    BOOST_FOREACH(std::shared_ptr<Surface>& surface, m_surfaceList)
        surface->retrieve();
}

void CauldronIO::SnapShot::release()
{
    if (m_volume)
        m_volume->release();
    BOOST_FOREACH(FormationVolume& formVolume, m_formationVolumeList)
        formVolume.second->release();
    BOOST_FOREACH(std::shared_ptr<Surface>& surface, m_surfaceList)
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

CauldronIO::Formation::Formation(int kStart, int kEnd, const string& name)
{
    if (name.empty()) throw CauldronIOException("Formation name cannot be empty");
    
    m_kstart = kStart;
    m_kend = kEnd;
    m_name = name;
	
	m_isSourceRock = false;
    m_isMobileLayer = false;
	m_hasAllochthonousLithology = false;
	m_chemicalcompaction = false;
	m_constrainedOverpressure = false;
	m_igniousintrusion = false;
	m_hasAllochthonousLithology = false;
	m_enableSourceRockMixing = false;

	m_depoSequence = -1;
	m_elementRefinement = -1;
	m_igniousintrusionAge = -1;

	m_lithoType1name.clear();
	m_lithoType2name.clear();
	m_lithoType3name.clear(); 
	m_fluidTypeName.clear();
	m_mixingmodelname.clear(); 
	m_sourceRock1name.clear();
	m_sourceRock2name.clear();
	m_allochthonousLithologyName.clear();
}

bool CauldronIO::Formation::isDepthRangeDefined() const
{
	return m_kstart >= 0 && m_kend >= 0;
}

void CauldronIO::Formation::updateK_range(int kStart, int kEnd)
{
	m_kstart = kStart;
	m_kend = kEnd;
}

void CauldronIO::Formation::getK_Range(int &start, int &end) const
{
    start = m_kstart;
    end = m_kend;
}

const string& CauldronIO::Formation::getName() const
{
    return m_name;
}


void CauldronIO::Formation::setIsSourceRock(bool isSourceRock)
{
	m_isSourceRock = isSourceRock;
}

bool CauldronIO::Formation::isSourceRock() const
{
    return m_isSourceRock;
}

void CauldronIO::Formation::setIsMobileLayer(bool isMobileLayer)
{
	m_isMobileLayer = isMobileLayer;
}

bool CauldronIO::Formation::isMobileLayer() const
{
    return m_isMobileLayer;
}

bool CauldronIO::Formation::hasThicknessMap() const
{
	return bool(m_thickness.second);
}

void CauldronIO::Formation::setThicknessMap(PropertySurfaceData& thicknessMap)
{
	m_thickness = thicknessMap;
}

const CauldronIO::PropertySurfaceData& CauldronIO::Formation::getThicknessMap() const
{
	return m_thickness;
}


bool CauldronIO::Formation::hasSourceRockMixingHIMap() const
{
	return bool(m_mixingHI.second);
}

void CauldronIO::Formation::setSourceRockMixingHIMap(PropertySurfaceData& map)
{
	m_mixingHI = map;
}

const CauldronIO::PropertySurfaceData& CauldronIO::Formation::getSourceRockMixingHIMap() const
{
	return m_mixingHI;
}


void CauldronIO::Formation::setSourceRock1Name(const std::string& name)
{
	m_sourceRock1name = name;
}


const std::string& CauldronIO::Formation::getSourceRock1Name() const
{
	return m_sourceRock1name;
}

void CauldronIO::Formation::setSourceRock2Name(const std::string& name)
{
	m_sourceRock2name = name;
}


const std::string& CauldronIO::Formation::getSourceRock2Name() const
{
	return m_sourceRock2name;
}

void CauldronIO::Formation::setEnableSourceRockMixing(bool enable)
{
	m_enableSourceRockMixing = enable;
}


bool CauldronIO::Formation::getEnableSourceRockMixing() const
{
	return m_enableSourceRockMixing;
}


bool CauldronIO::Formation::hasAllochthonousLithology() const
{
	return m_hasAllochthonousLithology;
}


void CauldronIO::Formation::setAllochthonousLithology(bool value)
{
	m_hasAllochthonousLithology = value;
}


const std::string& CauldronIO::Formation::getAllochthonousLithologyName() const
{
	return m_allochthonousLithologyName;
}


void CauldronIO::Formation::setAllochthonousLithologyName(const std::string& value)
{
	m_allochthonousLithologyName = value;
}


bool CauldronIO::Formation::isIgneousIntrusion() const
{
	return m_igniousintrusion;
}


void CauldronIO::Formation::setIgneousIntrusion(bool value)
{
	m_igniousintrusion = value;
}


double CauldronIO::Formation::getIgneousIntrusionAge() const
{
	return m_igniousintrusionAge;
}


void CauldronIO::Formation::setIgneousIntrusionAge(double age)
{
	m_igniousintrusionAge = age;
}


int CauldronIO::Formation::getDepoSequence() const
{
	return m_depoSequence;
}


void CauldronIO::Formation::setDepoSequence(int number)
{
	m_depoSequence = number;
}


const std::string& CauldronIO::Formation::getFluidType() const
{
	return m_fluidTypeName;
}


void CauldronIO::Formation::setFluidType(const std::string& type)
{
	m_fluidTypeName = type;
}


bool CauldronIO::Formation::hasConstrainedOverpressure() const
{
	return m_constrainedOverpressure;
}


void CauldronIO::Formation::setConstrainedOverpressure(bool value)
{
	m_constrainedOverpressure = value;
}


bool CauldronIO::Formation::hasChemicalCompaction() const
{
	return m_chemicalcompaction;
}


void CauldronIO::Formation::setChemicalCompaction(bool value)
{
	m_chemicalcompaction = value;
}


int CauldronIO::Formation::getElementRefinement() const
{
	return m_elementRefinement;
}


void CauldronIO::Formation::setElementRefinement(int value)
{
	m_elementRefinement = value;
}


const std::string& CauldronIO::Formation::getMixingModel() const
{
	return m_mixingmodelname;
}


void CauldronIO::Formation::setMixingModel(const std::string& model)
{
	m_mixingmodelname = model;
}


void CauldronIO::Formation::setLithoType1Name(const std::string& name)
{
	m_lithoType1name = name;
}


const std::string& CauldronIO::Formation::getLithoType1Name() const
{
	return m_lithoType1name;
}


void CauldronIO::Formation::setLithoType2Name(const std::string& name)
{
	m_lithoType2name = name;
}

const std::string& CauldronIO::Formation::getLithoType2Name() const
{
	return m_lithoType2name;
}


void CauldronIO::Formation::setLithoType3Name(const std::string& name)
{
	m_lithoType3name = name;
}


const std::string& CauldronIO::Formation::getLithoType3Name() const
{
	return m_lithoType3name;
}

void CauldronIO::Formation::setLithoType1PercentageMap(PropertySurfaceData& map)
{
	m_lithPerc1map = map;
}

void CauldronIO::Formation::setLithoType2PercentageMap(PropertySurfaceData& map)
{
	m_lithPerc2map = map;
}

void CauldronIO::Formation::setLithoType3PercentageMap(PropertySurfaceData& map)
{
	m_lithPerc3map = map;
}

const CauldronIO::PropertySurfaceData& CauldronIO::Formation::getLithoType1PercentageMap() const
{
	return m_lithPerc1map;
}

const CauldronIO::PropertySurfaceData& CauldronIO::Formation::getLithoType2PercentageMap() const
{
	return m_lithPerc2map;
}

const CauldronIO::PropertySurfaceData& CauldronIO::Formation::getLithoType3PercentageMap() const
{
	return m_lithPerc3map;
}

bool CauldronIO::Formation::hasLithoType1PercentageMap() const
{
	return bool(m_lithPerc1map.second);
}

bool CauldronIO::Formation::hasLithoType2PercentageMap() const
{
	return bool(m_lithPerc2map.second);
}

bool CauldronIO::Formation::hasLithoType3PercentageMap() const
{
	return bool(m_lithPerc3map.second);
}

void CauldronIO::Formation::setTopSurface(std::shared_ptr<CauldronIO::Surface>& surface)
{
	m_topSurface = surface;
}

const std::shared_ptr<CauldronIO::Surface>& CauldronIO::Formation::getTopSurface()
{
	return m_topSurface;
}

void CauldronIO::Formation::setBottomSurface(std::shared_ptr<CauldronIO::Surface>& surface)
{
	m_bottomSurface = surface;
}

const std::shared_ptr<CauldronIO::Surface>& CauldronIO::Formation::getBottomSurface()
{
	return m_bottomSurface;
}

void CauldronIO::Formation::release()
{
	if (hasThicknessMap())
	{
		m_thickness.second->release();
		m_thickness.second.reset();
	}
	if (hasSourceRockMixingHIMap())
	{
		m_mixingHI.second->release();
		m_mixingHI.second.reset();
	}
	if (hasLithoType1PercentageMap())
	{
		m_lithPerc1map.second->release();
		m_lithPerc1map.second.reset();
	}
	if (hasLithoType2PercentageMap())
	{
		m_lithPerc2map.second->release();
		m_lithPerc2map.second.reset();
	}
	if (hasLithoType3PercentageMap())
	{
		m_lithPerc3map.second->release();
		m_lithPerc3map.second.reset();
	}
}

void CauldronIO::Formation::retrieve()
{
	if (hasThicknessMap() && !m_thickness.second->isRetrieved())
	{
		m_thickness.second->retrieve();
	}
	if (hasSourceRockMixingHIMap() && !m_mixingHI.second->isRetrieved())
	{
		m_mixingHI.second->retrieve();
	}
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
	m_age = -1;
}

CauldronIO::Surface::~Surface()
{
    m_propSurfaceList.clear();
}

const PropertySurfaceDataList& CauldronIO::Surface::getPropertySurfaceDataList() const
{
    return m_propSurfaceList;
}


void CauldronIO::Surface::replaceAt(size_t index, PropertySurfaceData& data)
{
    if (index > m_propSurfaceList.size()) throw CauldronIOException("Index outside bounds in replaceAt");
    
    // Dereference existing surface
    m_propSurfaceList.at(index).second->release();
    m_propSurfaceList.at(index).second.reset();

    m_propSurfaceList.at(index) = data;
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

std::shared_ptr<SurfaceData> CauldronIO::Surface::getDepthSurface() const
{
    BOOST_FOREACH(const PropertySurfaceData& data, m_propSurfaceList)
        if (data.first->getName() == "Depth") return data.second;

    return std::shared_ptr<SurfaceData>();
}

const string& CauldronIO::Surface::getName() const
{
    return m_name;
}

CauldronIO::SubsurfaceKind CauldronIO::Surface::getSubSurfaceKind() const
{
    return m_subSurfaceKind;
}

void CauldronIO::Surface::setFormation(std::shared_ptr<Formation>& formation, bool isTopFormation)
{
    if (isTopFormation)
        m_Topformation = formation;
    else
        m_Bottomformation = formation;
}

const std::shared_ptr<Formation>& CauldronIO::Surface::getTopFormation() const
{
    return m_Topformation;
}

const std::shared_ptr<Formation>& CauldronIO::Surface::getBottomFormation() const
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


bool CauldronIO::Surface::isAgeDefined() const
{
	return m_age >= 0;
}

void CauldronIO::Surface::setAge(float age)
{
	m_age = age;
}

float CauldronIO::Surface::getAge() const
{
	return m_age;
}

bool CauldronIO::Surface::operator==(const Surface& other) const
{
	if (this->getName() != other.getName()) return false;
	if (this->getSubSurfaceKind() != other.getSubSurfaceKind()) return false;

	auto& bottomFormation1 = this->getBottomFormation();
	auto& bottomFormation2 = other.getBottomFormation();
	auto& topFormation1 = this->getTopFormation();
	auto& topFormation2 = other.getTopFormation();

	if (bottomFormation1 && !bottomFormation2) return false;
	if (topFormation1 && !topFormation2) return false;

	if (bottomFormation1 && bottomFormation2)
		if (!(*bottomFormation1 == *bottomFormation2)) return false;
	if (topFormation1 && topFormation2)
		if (!(*topFormation1 == *topFormation2)) return false;

	return true;
}

/// Geometry2D Implementation
///////////////////////////////////////

CauldronIO::Geometry2D::Geometry2D(size_t numI, size_t numJ, double deltaI, double deltaJ, double minI, double minJ, bool cellCentered)
{
    m_numI = numI;
    m_numJ = numJ;
    m_deltaI = deltaI;
    m_deltaJ = deltaJ;
    m_minI = minI;
    m_minJ = minJ;
    m_maxI = minI + deltaI * numI;
    m_maxJ = minJ + deltaJ * numJ;
    m_isCellCentered = cellCentered;
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


bool CauldronIO::Geometry2D::isCellCentered() const
{
    return m_isCellCentered;
}


void CauldronIO::Geometry2D::setCellCentered(bool cellCentered)
{
    m_isCellCentered = cellCentered;
}

bool CauldronIO::Geometry2D::operator==(const Geometry2D& other) const
{
    return
        m_numI == other.m_numI && m_numJ == other.m_numJ &&
        m_deltaI == other.m_deltaI && m_deltaJ == other.m_deltaJ &&
        m_minI == other.m_minI && m_minJ == other.m_minJ &&
        m_isCellCentered == other.m_isCellCentered;
}

// Map implementation
//////////////////////////////////////////////////////////////////////////


CauldronIO::SurfaceData::SurfaceData(const std::shared_ptr<const Geometry2D>& geometry, float minValue, float maxValue)
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
    m_minValue = minValue;
    m_maxValue = maxValue;
    m_updateMinMax = minValue == DefaultUndefinedValue; // if the min/max values are not set they need to be updated

    // Indexing into the map is unknown
    m_internalData = nullptr;
    
    m_reservoir.reset();
    m_formation.reset();
}

float CauldronIO::SurfaceData::getMinValue()
{
    if (m_updateMinMax)
        updateMinMax();
    return m_minValue;
}

float CauldronIO::SurfaceData::getMaxValue()
{
    if (m_updateMinMax)
        updateMinMax();
    return m_maxValue;
}

float CauldronIO::SurfaceData::getUndefinedValue() const
{
    return DefaultUndefinedValue;
}

void CauldronIO::SurfaceData::setFormation(const std::shared_ptr<const Formation>& formation)
{
    m_formation = formation;
}

const std::shared_ptr<const Formation>& CauldronIO::SurfaceData::getFormation() const
{
    return m_formation;
}

void CauldronIO::SurfaceData::setReservoir(std::shared_ptr<const Reservoir> reservoir)
{
    m_reservoir = reservoir;
}

const std::shared_ptr<const Reservoir>& CauldronIO::SurfaceData::getReservoir() const
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
    m_internalData = nullptr;
}

const std::shared_ptr<const Geometry2D>& CauldronIO::SurfaceData::getGeometry() const
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


void CauldronIO::SurfaceData::updateMinMax()
{
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");

    if (m_isConstant)
    {
        m_minValue = m_maxValue = m_constantValue;
        m_updateMinMax = false;
        return;
    }

    size_t allElements = m_numI * m_numJ;
    float minValue = DefaultUndefinedValue;
    float maxValue = DefaultUndefinedValue;
	bool foundUndefined = false;

    for (size_t i = 0; i < allElements; i++)
    {
        float val = m_internalData[i];
		if (val != DefaultUndefinedValue)
		{
			minValue = minValue == DefaultUndefinedValue ? val : min(minValue, val);
			maxValue = maxValue == DefaultUndefinedValue ? val : max(maxValue, val);
		}
		else
			foundUndefined = true;
    }

    m_minValue = minValue;
    m_maxValue = maxValue;

	if ((minValue == maxValue && !foundUndefined) || (minValue == DefaultUndefinedValue && maxValue == DefaultUndefinedValue))
	{
		release();
		setConstantValue(minValue);
	}

	m_updateMinMax = false;
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
    if (m_isConstant) return m_constantValue == DefaultUndefinedValue;

    return m_internalData[getMapIndex(i, j)] == DefaultUndefinedValue;
}

float CauldronIO::SurfaceData::getValue(size_t i, size_t j) const
{
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (m_isConstant) return m_constantValue;
    
    return m_internalData[getMapIndex(i, j)];
}

const float* CauldronIO::SurfaceData::getRowValues(size_t j)
{
    if (!canGetRow()) throw CauldronIOException("Cannot return row values");
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");

    // Create our internal buffer if not existing
    if (!m_internalData && m_isConstant) setData(nullptr, true, m_constantValue);

    return m_internalData + getMapIndex(0, j);
}

const float* CauldronIO::SurfaceData::getColumnValues(size_t i)
{
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (!canGetColumn()) throw CauldronIOException("Cannot return column values");

    // Create our internal buffer if not existing
    if (!m_internalData && m_isConstant) setData(nullptr, true, m_constantValue);

    return m_internalData + getMapIndex(i, 0);
}

const float* CauldronIO::SurfaceData::getSurfaceValues()
{
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");

    // Create our internal buffer if not existing
    if (!m_internalData && m_isConstant) setData(nullptr, true, m_constantValue);

    return m_internalData;
}

size_t CauldronIO::SurfaceData::getMapIndex(size_t i, size_t j) const
{
    assert(i < m_numI && j < m_numJ);
    return m_numI * j + i;
}

void CauldronIO::SurfaceData::release()
{
    if (m_internalData) delete[] m_internalData;
    m_internalData = nullptr;
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

CauldronIO::Volume::Volume(SubsurfaceKind kind)
{
    m_subSurfaceKind = kind;
}

CauldronIO::Volume::~Volume()
{
    m_propVolumeList.clear();
}

CauldronIO::SubsurfaceKind CauldronIO::Volume::getSubSurfaceKind() const
{
    return m_subSurfaceKind;
}

PropertyVolumeDataList& CauldronIO::Volume::getPropertyVolumeDataList() 
{
    return m_propVolumeList;
}

void CauldronIO::Volume::removeVolumeData(PropertyVolumeData& data)
{
    for (size_t index = 0; index < m_propVolumeList.size(); index++)
    {
        if (m_propVolumeList.at(index).first == data.first)
        {
            data.second->release();
            data.second.reset();
            
            if (index < m_propVolumeList.size() - 1)
                m_propVolumeList.at(index) = m_propVolumeList.back();

            m_propVolumeList.pop_back();

            return;
        }
    }

    throw CauldronIOException("Cannot find data to remove");
}

void CauldronIO::Volume::addPropertyVolumeData(PropertyVolumeData& newData)
{
    BOOST_FOREACH(PropertyVolumeData& data, m_propVolumeList)
        if (data == newData) throw CauldronIOException("Cannot add property-volumeData twice");

    m_propVolumeList.push_back(newData);
}


void CauldronIO::Volume::replaceAt(size_t index, PropertyVolumeData& data)
{
    if (index > m_propVolumeList.size()) throw CauldronIOException("Index outside bounds in replaceAt");

    // Dereference existing surface
    m_propVolumeList.at(index).second->release();
    m_propVolumeList.at(index).second.reset();

    m_propVolumeList.at(index) = data;
}

bool CauldronIO::Volume::hasDepthVolume() const
{
    BOOST_FOREACH(const PropertyVolumeData& data, m_propVolumeList)
        if (data.first->getName() == "Depth") return true;

    return false;
}

std::shared_ptr<VolumeData> CauldronIO::Volume::getDepthVolume() const
{
    BOOST_FOREACH(const PropertyVolumeData& data, m_propVolumeList)
        if (data.first->getName() == "Depth") return data.second;

    return std::shared_ptr<VolumeData>();
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
    double deltaJ, double minI, double minJ, bool cellCentered) : Geometry2D(numI, numJ, deltaI, deltaJ, minI, minJ, cellCentered)
{
    m_numK = numK;
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

void CauldronIO::Geometry3D::updateK_range(size_t firstK, size_t numK)
{
    m_firstK = firstK;
    m_numK = numK;
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

std::shared_ptr<const Trapper> CauldronIO::Trapper::getDownStreamTrapper() const
{
    return m_downstreamTrapper;
}

void CauldronIO::Trapper::setDownStreamTrapper(std::shared_ptr<const Trapper> trapper)
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


void CauldronIO::Trapper::setGOC(float goc)
{
    m_goc = goc;
}

float CauldronIO::Trapper::getGOC() const
{
    return m_goc;
}

void CauldronIO::Trapper::setOWC(float woc)
{
    m_owc = woc;
}

float CauldronIO::Trapper::getOWC() const
{
    return m_owc;
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

CauldronIO::Reservoir::Reservoir(const std::string& reservoirName, const std::shared_ptr<const Formation>& formation)
{
    m_reservoirName = reservoirName;
    m_formation = formation;
}

const std::string& CauldronIO::Reservoir::getName() const
{
    return m_reservoirName;
}

const std::shared_ptr<const Formation>& CauldronIO::Reservoir::getFormation() const
{
    return m_formation;
}

/// VolumeData implementation
//////////////////////////////////////////////////////////////////////////

CauldronIO::VolumeData::VolumeData(const std::shared_ptr<Geometry3D>& geometry, float minValue, float maxValue)
{
    m_internalDataIJK = nullptr;
    m_internalDataKIJ = nullptr;
    m_isConstant = false;
    m_retrieved = false;
    m_geometry = geometry;
    m_minValue = minValue;
    m_maxValue = maxValue;
    m_updateMinMax = minValue == DefaultUndefinedValue; // if the min/max values are not set they need to be updated

    updateGeometry();
}

CauldronIO::VolumeData::~VolumeData()
{
    if (m_internalDataIJK) delete[] m_internalDataIJK;
    if (m_internalDataKIJ) delete[] m_internalDataKIJ;
    m_internalDataIJK = nullptr;
    m_internalDataKIJ = nullptr;
}

const std::shared_ptr<Geometry3D>& CauldronIO::VolumeData::getGeometry() const
{
    return m_geometry;
}

bool CauldronIO::VolumeData::isUndefined(size_t i, size_t j, size_t k) const
{
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (m_isConstant) return m_constantValue == DefaultUndefinedValue;
    if (m_internalDataIJK) return m_internalDataIJK[computeIndex_IJK(i, j, k)] == DefaultUndefinedValue;

    assert(m_internalDataKIJ);
    return m_internalDataKIJ[computeIndex_KIJ(i, j, k)] == DefaultUndefinedValue;
}


float CauldronIO::VolumeData::getValue(size_t i, size_t j, size_t k) const
{
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (m_isConstant) return m_constantValue;
    if (m_internalDataIJK) return m_internalDataIJK[computeIndex_IJK(i, j, k)];

    assert(m_internalDataKIJ);
    return m_internalDataKIJ[computeIndex_KIJ(i, j, k)];
}

const float* CauldronIO::VolumeData::getRowValues(size_t j, size_t k)
{
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (!hasDataIJK()) throw CauldronIOException("Cannot return row values");

    // Create our internal buffer if not existing
    if (!m_internalDataIJK && m_isConstant) setData_IJK(nullptr, true, m_constantValue);

    // Assume IJK ordering
    assert(m_internalDataIJK);
    return m_internalDataIJK + computeIndex_IJK(0, j, k);
}

const float* CauldronIO::VolumeData::getColumnValues(size_t i, size_t k)
{
    throw CauldronIOException("Not implemented");
}

const float* CauldronIO::VolumeData::getNeedleValues(size_t i, size_t j)
{
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (!hasDataKIJ() || isConstant()) throw CauldronIOException("Cannot return needle values");

    // Create our internal buffer if not existing
    if (!m_internalDataKIJ && m_isConstant) setData_KIJ(nullptr, true, m_constantValue);

    return m_internalDataKIJ + computeIndex_KIJ(i, j, m_firstK);
}

const float* CauldronIO::VolumeData::getSurface_IJ(size_t k)
{
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (!hasDataIJK() || isConstant()) throw CauldronIOException("Cannot return surface values");

    // Create our internal buffer if not existing
    if (!m_internalDataIJK && m_isConstant) setData_IJK(nullptr, true, m_constantValue);

    return m_internalDataIJK + computeIndex_IJK(0, 0, k);
}

const float* CauldronIO::VolumeData::getVolumeValues_KIJ()
{
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (!hasDataKIJ() || isConstant()) throw CauldronIOException("Cannot return volume values");

    // Create our internal buffer if not existing
    if (!m_internalDataKIJ && m_isConstant) setData_KIJ(nullptr, true, m_constantValue);

    return m_internalDataKIJ;
}

const float* CauldronIO::VolumeData::getVolumeValues_IJK()
{
    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");
    if (!hasDataIJK() || isConstant()) throw CauldronIOException("Cannot return volume values");

    // Create our internal buffer if not existing
    if (!m_internalDataIJK && m_isConstant) setData_IJK(nullptr, true, m_constantValue);

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


void CauldronIO::VolumeData::updateGeometry()
{
    // Cache all geometry
    m_numI = m_geometry->getNumI();
    m_numJ = m_geometry->getNumJ();
    m_firstK = m_geometry->getFirstK();
    m_numK = m_geometry->getNumK();
    m_deltaI = m_geometry->getDeltaI();
    m_deltaJ = m_geometry->getDeltaJ();
    m_minI = m_geometry->getMinI();
    m_maxI = m_geometry->getMaxI();
    m_minJ = m_geometry->getMinJ();
    m_maxJ = m_geometry->getMaxJ();
    m_lastK = m_geometry->getLastK();
}


void CauldronIO::VolumeData::updateMinMax()
{

    if (!isRetrieved()) throw CauldronIOException("Need to assign data first");

    if (m_isConstant)
    {
        m_minValue = m_maxValue = m_constantValue;
        m_updateMinMax = false;
        return;
    }

    size_t allElements = m_numI * m_numJ * m_numK;
    float minValue = DefaultUndefinedValue;
    float maxValue = DefaultUndefinedValue;
    float* internaldata = m_internalDataIJK != NULL ? m_internalDataIJK : m_internalDataKIJ;
    
    for (size_t i = 0; i < allElements; i++)
    {
        float val = internaldata[i];
        if (val != DefaultUndefinedValue)
        {
			minValue = minValue == DefaultUndefinedValue ? val : min(minValue, val);
			maxValue = maxValue == DefaultUndefinedValue ? val : max(maxValue, val);
        }
    }
    
    m_minValue = minValue;
    m_maxValue = maxValue;
    m_updateMinMax = false;
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
    assert(i < m_numI && j < m_numJ && k >= m_firstK && k < m_numK + m_firstK);

    return (i + j * m_numI + (k - m_firstK) * m_numI * m_numJ);
}

size_t CauldronIO::VolumeData::computeIndex_KIJ(size_t i, size_t j, size_t k) const
{
    assert(i < m_numI && j < m_numJ && k >= m_firstK && k < m_numK + m_firstK);

    return ((k - m_firstK) + i * m_numK + j * m_numI * m_numK);
}

float CauldronIO::VolumeData::getMinValue()
{
    if (m_updateMinMax)
        updateMinMax();
    return m_minValue;
}

float CauldronIO::VolumeData::getMaxValue()
{
    if (m_updateMinMax)
        updateMinMax();
    return m_maxValue;
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
    m_internalDataIJK = nullptr;
    m_internalDataKIJ = nullptr;
    m_retrieved = false;
}

bool CauldronIO::VolumeData::hasDataIJK() const
{
    return m_internalDataIJK != nullptr;
}

bool CauldronIO::VolumeData::hasDataKIJ() const
{
    return m_internalDataKIJ != nullptr;
}

float CauldronIO::VolumeData::getUndefinedValue() const
{
    return DefaultUndefinedValue;
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


const std::shared_ptr<Surface>& CauldronIO::StratigraphyTableEntry::getSurface() const
{
	return m_surface;
}

const std::shared_ptr<Formation>& CauldronIO::StratigraphyTableEntry::getFormation() const
{
	return m_formation;
}

void CauldronIO::StratigraphyTableEntry::setSurface(std::shared_ptr<Surface>& surface)
{
	m_formation.reset();
	m_surface = surface;
}

void CauldronIO::StratigraphyTableEntry::setFormation(std::shared_ptr<Formation>& formation)
{
	m_surface.reset();
	m_formation = formation;
}
