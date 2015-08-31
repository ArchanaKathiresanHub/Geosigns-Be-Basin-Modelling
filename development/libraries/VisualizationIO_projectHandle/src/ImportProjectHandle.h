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

#include "VisualizationAPI.h"
#include "VisualizationIO_projectHandle.h"
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/uuid/uuid.hpp>         
#include <boost/uuid/uuid_generators.hpp> // generators
#include "Interface/Formation.h"
#include "Interface/Snapshot.h"

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
    static boost::shared_ptr<CauldronIO::Project> CreateFromProjectHandle(boost::shared_ptr<DataAccess::Interface::ProjectHandle> projectHandle, bool verbose);

private:

    /// Private constructor
    ImportProjectHandle(bool verbose);

    bool _verbose;
    boost::uuids::random_generator _uuidGenerator;

    /// Private helper methods
    CauldronIO::SnapShotKind GetSnapShotKind(const DataAccess::Interface::Snapshot* snapShot) const;
    
    CauldronIO::PropertyType GetPropertyType(const DataAccess::Interface::Property* prop) const;
    
    CauldronIO::PropertyAttribute GetPropertyAttribute(const DataAccess::Interface::Property* prop) const;
    
    boost::shared_ptr<CauldronIO::SnapShot> CreateSnapShotIO(boost::shared_ptr<DataAccess::Interface::ProjectHandle> projectHandle, 
        const DataAccess::Interface::Snapshot* snapShot);
    
    boost::shared_ptr<vector<boost::shared_ptr<CauldronIO::Surface> > > CreateSurfaces(boost::shared_ptr<DataAccess::Interface::ProjectHandle> projectHandle,
        boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations, const DataAccess::Interface::Snapshot* snapShot);
    
    boost::shared_ptr<CauldronIO::Formation> CreateFormation(const DataAccess::Interface::Formation* formation, boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations) const;
    
    boost::shared_ptr<CauldronIO::FormationInfoList> GetDepthFormations(boost::shared_ptr<DataAccess::Interface::ProjectHandle> projectHandle,
        const DataAccess::Interface::Snapshot* snapShot) const;
    
    boost::shared_ptr<CauldronIO::Map> CreateMapIO(const DataAccess::Interface::PropertyValue* propVal);
    
    void AddSnapShots(boost::shared_ptr<DataAccess::Interface::ProjectHandle> projectHandle, boost::shared_ptr<CauldronIO::Project> project);
    
    CauldronIO::SubsurfaceKind GetSubSurfaceKind(const DataAccess::Interface::Surface* surface) const;
    
    CauldronIO::SubsurfaceKind GetSubSurfaceKind(const DataAccess::Interface::Formation* formation) const;
    
    boost::shared_ptr<CauldronIO::DiscontinuousVolume> CreateDiscontinuousVolume(boost::shared_ptr<DataAccess::Interface::PropertyValueList> propValueList,
        boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations);
    
    boost::shared_ptr<CauldronIO::Volume> CreateVolume(const DataAccess::Interface::PropertyValue* propValue,
        boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations);
    
    boost::shared_ptr<CauldronIO::Volume> CreateContinuousVolume(boost::shared_ptr<DataAccess::Interface::PropertyValueList> depthPropValue,
        boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations);
    
    boost::shared_ptr<CauldronIO::Volume> CreateEmptyVolume(const DataAccess::Interface::PropertyValue* propVal) const;
    
    boost::shared_ptr<const CauldronIO::Surface> FindDepthSurface(boost::shared_ptr<CauldronIO::Surface> surface, boost::shared_ptr< vector<boost::shared_ptr<CauldronIO::Surface> > > surfaces) const;
};

#endif
