//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef __ImportProjectHandle_h__
#define __ImportProjectHandle_h__

#include <string>
#include <vector>
#include <memory>

// Include forward declarations
#include "VisualizationAPIFwDecl.h"

namespace DataAccess
{
    namespace Interface
    {
        class Reservoir;
    }
}

/// \class Little class to import a projecthandle into the VisualizationIO library datamodel
class ImportProjectHandle
{
public:
    /// \brief Create a new CauldronIO::Project from given path to a projectHandle
    /// 
    /// This import will create a CauldronIO::Project but will not import (read) the data yet
    /// For this, use the Retrieve method of the Project class (to import all data), or a lower level Retrieve
    /// on one of the contained classes. Note that all depth volumes need to be read, to understand the formation geometry ordering for 3D volumes.
    /// This can take some time
    /// \param [in] projectHandle The projecthandle to import
    /// \param [in] verbose If true, output will be generated to cout to reflect the import process
    /// \returns A boost sharedpointer to a newly created CauldronIO::Project
    static std::shared_ptr<CauldronIO::Project> createFromProjectHandle(std::shared_ptr<DataAccess::Interface::ProjectHandle> projectHandle, bool verbose);
	static CauldronIO::SnapShotKind getSnapShotKind(const DataAccess::Interface::Snapshot* snapShot);
	static CauldronIO::PropertyType getPropertyType(const DataAccess::Interface::Property* prop) ;
	static CauldronIO::PropertyAttribute getPropertyAttribute(const DataAccess::Interface::Property* prop);


    /// Constructor
    ImportProjectHandle(bool verbose, std::shared_ptr<CauldronIO::Project>& project, std::shared_ptr<DataAccess::Interface::ProjectHandle>& projectHandle);

    void checkInputValues();
    void addSnapShots();
    void addMigrationIO();
    void addTrapperIO();
    void addTrapIO();
    void addGenexHistory();
    void addBurialHistory();
    void addMassBalance();
    void add1Ddata();

 private:


    /// Private members
    bool m_verbose;
    std::shared_ptr<CauldronIO::Project> m_project;
    std::shared_ptr<DataAccess::Interface::PropertyValueList> m_propValues;
    std::shared_ptr<DataAccess::Interface::PropertyList> m_props;
    std::shared_ptr<DataAccess::Interface::ProjectHandle> m_projectHandle;

    /// Private helper methods
    std::shared_ptr<CauldronIO::SnapShot> createSnapShotIO(const DataAccess::Interface::Snapshot* snapShot);

    std::vector<std::shared_ptr<CauldronIO::Surface> > createSurfaces(
        std::shared_ptr<CauldronIO::FormationInfoList> depthFormations, const DataAccess::Interface::Snapshot* snapShot, const std::shared_ptr<DataAccess::Interface::PropertyValueList>& snapShotPropVals);
    std::shared_ptr<CauldronIO::Formation> createFormation(const DataAccess::Interface::Formation* formation, std::shared_ptr<CauldronIO::FormationInfoList> depthFormations) const;

    std::shared_ptr<CauldronIO::FormationInfoList> getDepthFormations(const DataAccess::Interface::Snapshot* snapShot) const;

    std::shared_ptr<CauldronIO::SurfaceData> createMapIO(const DataAccess::Interface::PropertyValue* propVal, std::shared_ptr<const CauldronIO::Geometry2D>& geometry);

    CauldronIO::SubsurfaceKind getSubSurfaceKind(const DataAccess::Interface::Surface* surface) const;

    CauldronIO::SubsurfaceKind getSubSurfaceKind(const DataAccess::Interface::Formation* formation) const;

    std::shared_ptr<CauldronIO::Geometry3D> createGeometry3D(std::shared_ptr<CauldronIO::FormationInfoList> depthFormations);
    std::shared_ptr<CauldronIO::Geometry3D> createGeometry3D(const std::shared_ptr<CauldronIO::FormationInfo>& info);
    CauldronIO::PropertyVolumeData createPropertyVolumeData(std::shared_ptr<DataAccess::Interface::PropertyValueList> propValues, std::shared_ptr<CauldronIO::Geometry3D>& geometry3D,
        std::shared_ptr<CauldronIO::FormationInfoList> depthFormations);
    CauldronIO::PropertyVolumeData createPropertyVolumeData(const DataAccess::Interface::PropertyValue* propVal,
        std::shared_ptr<CauldronIO::Geometry3D>& geometry3D, std::shared_ptr<CauldronIO::FormationInfo> formationInfo);
    std::shared_ptr<const CauldronIO::Surface> findDepthSurface(std::shared_ptr<CauldronIO::Surface> surface, const std::vector<std::shared_ptr<CauldronIO::Surface> >& surfaces) const;
    std::shared_ptr<CauldronIO::Surface> findSurface(std::vector< std::shared_ptr<CauldronIO::Surface> > surfaces, const std::string& surfaceName) const;
    std::shared_ptr<CauldronIO::Surface> findSurface(std::vector< std::shared_ptr<CauldronIO::Surface> > surfaces, std::shared_ptr<CauldronIO::Formation>& formation) const;
    std::shared_ptr<const CauldronIO::Property> findOrCreateProperty(const DataAccess::Interface::Property* prop);
	std::shared_ptr<const CauldronIO::Property> findOrCreateProperty(const std::string& propString);
	std::shared_ptr<CauldronIO::Formation> findOrCreateFormation(const DataAccess::Interface::Formation* form, std::shared_ptr<CauldronIO::FormationInfoList> depthFormations);
	std::shared_ptr<CauldronIO::Formation> findOrCreateFormation(const DataAccess::Interface::Formation* form);
	void setFormationProperties(std::shared_ptr<CauldronIO::Formation> formationIO, const DataAccess::Interface::Formation* form) const;
	std::shared_ptr<const CauldronIO::Reservoir> findOrCreateReservoir(const DataAccess::Interface::Reservoir* reservoir, std::shared_ptr<const CauldronIO::Formation> formationIO);
    std::shared_ptr<CauldronIO::FormationInfo> findDepthFormationInfo(const DataAccess::Interface::Formation* formation, std::shared_ptr<CauldronIO::FormationInfoList> depthFormations);
    std::shared_ptr<DataAccess::Interface::PropertyValueList> getPropertyValues(const DataAccess::Interface::Snapshot* snapshot) const;
    std::shared_ptr<DataAccess::Interface::PropertyList> getProperties(const std::shared_ptr<DataAccess::Interface::PropertyValueList>& propValues) const;
    std::shared_ptr<DataAccess::Interface::PropertyValueList> getMapPropertyValues(const std::shared_ptr<DataAccess::Interface::PropertyValueList>& snapShotPropVals) const;
    std::shared_ptr<DataAccess::Interface::PropertyValueList> getFormationVolumePropertyValues(const DataAccess::Interface::Property* prop, std::shared_ptr<DataAccess::Interface::PropertyValueList> snapShotPropVals,
        const DataAccess::Interface::Formation* formation) const;
	
	void addStratTableSurface(const DataAccess::Interface::Surface* surface);
	std::shared_ptr<CauldronIO::SurfaceData> getInputMap(float value, const std::string &valueGridMapId, std::shared_ptr<const CauldronIO::Geometry2D> geometry) const;
	void addStratTableFormation(const DataAccess::Interface::Formation* formation);
	std::shared_ptr<const CauldronIO::Property> getDefaultProperty(const std::string& name);
};

#endif
