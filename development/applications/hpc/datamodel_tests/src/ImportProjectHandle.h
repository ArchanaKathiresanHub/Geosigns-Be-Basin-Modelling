#ifndef __ImportProjectHandle_h__
#define __ImportProjectHandle_h__

#include "VisualizationAPI.h"
#include <string>
#include <boost/shared_ptr.hpp>
#include "Interface/Formation.h"
#include "Interface/Snapshot.h"

/// \brief Little class to import a projecthandle into the VisualizationIO library datamodel
class ImportProjectHandle
{
public:
    /// \brief Create a new CauldronIO::Project from given path to a projectHandle
    static boost::shared_ptr<CauldronIO::Project> CreateFromProjectHandle(const string& projectHandle, bool verbose);

private:

    ImportProjectHandle(bool verbose);

    /// Little structure to hold depth information about a formation
    struct FormationInfo
    {
        double depthStart, depthEnd;
        size_t kStart, kEnd;
        const DataAccess::Interface::Formation* formation;
        bool reverseDepth;
        const DataAccess::Interface::PropertyValue* propValue;
    
        static bool CompareFormations(const FormationInfo* info1, const FormationInfo* info2);
    };
    typedef vector<FormationInfo*> FormationInfoList;

    bool _verbose;

    CauldronIO::SnapShotKind GetSnapShotKind(const DataAccess::Interface::Snapshot* snapShot) const;
    CauldronIO::PropertyType GetPropertyType(const DataAccess::Interface::Property* prop) const;
    CauldronIO::PropertyAttribute GetPropertyAttribute(const DataAccess::Interface::Property* prop) const;
    boost::shared_ptr<const CauldronIO::SnapShot> CreateSnapShotIO(boost::shared_ptr<DataAccess::Interface::ProjectHandle> projectHandle, 
        const DataAccess::Interface::Snapshot* snapShot) const;
    boost::shared_ptr<vector<boost::shared_ptr<CauldronIO::Surface> > > ImportProjectHandle::CreateSurfaces(boost::shared_ptr<DataAccess::Interface::ProjectHandle> projectHandle,
        boost::shared_ptr<FormationInfoList> depthFormations, const DataAccess::Interface::Snapshot* snapShot) const;
    boost::shared_ptr<const CauldronIO::Formation> CreateFormation(const DataAccess::Interface::Formation* formation, boost::shared_ptr<FormationInfoList> depthFormations) const;
    boost::shared_ptr<FormationInfoList> GetDepthFormations(boost::shared_ptr<DataAccess::Interface::ProjectHandle> projectHandle, 
        const DataAccess::Interface::Snapshot* snapShot) const;
    boost::shared_ptr<const CauldronIO::Map> CreateMapIO(const DataAccess::Interface::GridMap* depthValues) const;
    void AddSnapShots(boost::shared_ptr<DataAccess::Interface::ProjectHandle> projectHandle, boost::shared_ptr<CauldronIO::Project> project) const;
    CauldronIO::SubsurfaceKind GetSubSurfaceKind(const DataAccess::Interface::Surface* surface) const;
    CauldronIO::SubsurfaceKind ImportProjectHandle::GetSubSurfaceKind(const DataAccess::Interface::Formation* formation) const;
    boost::shared_ptr<CauldronIO::DiscontinuousVolume> ImportProjectHandle::CreateDiscontinuousVolume(boost::shared_ptr<DataAccess::Interface::PropertyValueList> propValueList,
        boost::shared_ptr<FormationInfoList> depthFormations) const;
    boost::shared_ptr<CauldronIO::Volume> ImportProjectHandle::CreateVolume(const DataAccess::Interface::PropertyValue* propValue,
        boost::shared_ptr<FormationInfoList> depthFormations) const;
    boost::shared_ptr<CauldronIO::Volume> ImportProjectHandle::CreateContinuousVolume(boost::shared_ptr<DataAccess::Interface::PropertyValueList> depthPropValue,
        boost::shared_ptr<FormationInfoList> depthFormations) const;
    boost::shared_ptr<CauldronIO::Volume> ImportProjectHandle::CreateEmptyVolume(const DataAccess::Interface::PropertyValue* propVal) const;
    boost::shared_ptr<const CauldronIO::Surface> FindDepthSurface(boost::shared_ptr<CauldronIO::Surface> surface, boost::shared_ptr< vector<boost::shared_ptr<CauldronIO::Surface> > > surfaces) const;
    FormationInfo* ImportProjectHandle::FindDepthInfo(boost::shared_ptr<FormationInfoList> depthFormations, const DataAccess::Interface::Formation* formation) const;
};

#endif
