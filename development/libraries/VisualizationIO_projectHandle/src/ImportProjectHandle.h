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
#include <boost/shared_ptr.hpp>

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
    static boost::shared_ptr<CauldronIO::Project> createFromProjectHandle(boost::shared_ptr<DataAccess::Interface::ProjectHandle> projectHandle, bool verbose);

private:

    /// Private constructor
    ImportProjectHandle(bool verbose, boost::shared_ptr<CauldronIO::Project>& project);

    bool m_verbose;
    boost::shared_ptr<CauldronIO::Project> m_project;
    boost::shared_ptr<DataAccess::Interface::PropertyValueList> m_propValues;
    boost::shared_ptr<DataAccess::Interface::PropertyList> m_props;

    /// Private helper methods
    CauldronIO::SnapShotKind getSnapShotKind(const DataAccess::Interface::Snapshot* snapShot) const;

    CauldronIO::PropertyType getPropertyType(const DataAccess::Interface::Property* prop) const;

    CauldronIO::PropertyAttribute getPropertyAttribute(const DataAccess::Interface::Property* prop) const;

    boost::shared_ptr<CauldronIO::SnapShot> createSnapShotIO(boost::shared_ptr<DataAccess::Interface::ProjectHandle> projectHandle,
        const DataAccess::Interface::Snapshot* snapShot);

    std::vector<boost::shared_ptr<CauldronIO::Surface> > createSurfaces(
        boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations, const DataAccess::Interface::Snapshot* snapShot, const boost::shared_ptr<DataAccess::Interface::PropertyValueList>& snapShotPropVals);
    boost::shared_ptr<const CauldronIO::Formation> createFormation(const DataAccess::Interface::Formation* formation, boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations) const;

    boost::shared_ptr<CauldronIO::FormationInfoList> getDepthFormations(boost::shared_ptr<DataAccess::Interface::ProjectHandle> projectHandle,
        const DataAccess::Interface::Snapshot* snapShot) const;

    boost::shared_ptr<CauldronIO::SurfaceData> createMapIO(const DataAccess::Interface::PropertyValue* propVal, boost::shared_ptr<const CauldronIO::Geometry2D>& geometry);

    void addSnapShots(boost::shared_ptr<DataAccess::Interface::ProjectHandle> projectHandle);

    CauldronIO::SubsurfaceKind getSubSurfaceKind(const DataAccess::Interface::Surface* surface) const;

    CauldronIO::SubsurfaceKind getSubSurfaceKind(const DataAccess::Interface::Formation* formation) const;

    boost::shared_ptr<CauldronIO::Geometry3D> createGeometry3D(boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations);
    boost::shared_ptr<CauldronIO::Geometry3D> createGeometry3D(const boost::shared_ptr<CauldronIO::FormationInfo>& info);
    CauldronIO::PropertyVolumeData createPropertyVolumeData(boost::shared_ptr<DataAccess::Interface::PropertyValueList> propValues, boost::shared_ptr<CauldronIO::Geometry3D>& geometry3D,
        boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations);
    CauldronIO::PropertyVolumeData createPropertyVolumeData(const DataAccess::Interface::PropertyValue* propVal,
        boost::shared_ptr<CauldronIO::Geometry3D>& geometry3D, boost::shared_ptr<CauldronIO::FormationInfo> formationInfo);
    boost::shared_ptr<const CauldronIO::Surface> findDepthSurface(boost::shared_ptr<CauldronIO::Surface> surface, const std::vector<boost::shared_ptr<CauldronIO::Surface> >& surfaces) const;
    boost::shared_ptr<CauldronIO::Surface> findSurface(std::vector< boost::shared_ptr<CauldronIO::Surface> > surfaces, const std::string& surfaceName) const;
    boost::shared_ptr<CauldronIO::Surface> findSurface(std::vector< boost::shared_ptr<CauldronIO::Surface> > surfaces, boost::shared_ptr<const CauldronIO::Formation>& formation) const;
    boost::shared_ptr<const CauldronIO::Property> findOrCreateProperty(const DataAccess::Interface::Property* prop);
    boost::shared_ptr<const CauldronIO::Formation> findOrCreateFormation(const DataAccess::Interface::Formation* form, boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations);
    boost::shared_ptr<const CauldronIO::Reservoir> findOrCreateReservoir(const DataAccess::Interface::Reservoir* reservoir, boost::shared_ptr<const CauldronIO::Formation> formationIO);
    boost::shared_ptr<CauldronIO::FormationInfo> findDepthFormationInfo(const DataAccess::Interface::Formation* formation, boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations);

    boost::shared_ptr<DataAccess::Interface::PropertyValueList> getPropertyValues(const DataAccess::Interface::Snapshot* snapshot) const;
    boost::shared_ptr<DataAccess::Interface::PropertyList> getProperties(const boost::shared_ptr<DataAccess::Interface::PropertyValueList>& propValues) const;
    boost::shared_ptr<DataAccess::Interface::PropertyValueList> getMapPropertyValues(const boost::shared_ptr<DataAccess::Interface::PropertyValueList>& snapShotPropVals) const;
    boost::shared_ptr<DataAccess::Interface::PropertyValueList> getFormationVolumePropertyValues(const DataAccess::Interface::Property* prop, boost::shared_ptr<DataAccess::Interface::PropertyValueList> snapShotPropVals,
        const DataAccess::Interface::Formation* formation) const;
};

#endif
