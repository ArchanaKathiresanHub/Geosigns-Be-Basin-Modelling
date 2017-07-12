//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef __ExportToHDF_h__
#define __ExportToHDF_h__

#include "VisualizationAPI.h"
#include "DataStore.h"
#include "FilePath.h"
#pragma warning (disable:488)
#include <boost/lockfree/queue.hpp>
#include <boost/atomic.hpp>
#include "pugixml.hpp"
#include <memory>
#include "GeoPhysicsProjectHandle.h"

namespace CauldronIO
{
    class ExportToHDF
    {
    public:

       static bool exportToHDF(std::shared_ptr<Project>& project, const std::string& absPath, size_t numThreads, const bool includeBasement, GeoPhysics::ProjectHandle* projectHandle );
       
    private:
       ExportToHDF(const ibs::FilePath& absPath, const ibs::FilePath& relPath, size_t numThreads, const bool includeBasement, GeoPhysics::ProjectHandle* projectHandle);
 
       void addProject(std::shared_ptr<Project>& project);
       void addSurface( const std::shared_ptr<SnapShot>& snapShot, const std::shared_ptr<Surface>& surfaceIO);
       void writeMapsToHDF(const std::shared_ptr<SnapShot>& snapShot, const std::shared_ptr<Surface>& surfaceIO, const PropertySurfaceData &propertySurfaceData );
       void writeDiscVolToHDF( const std::shared_ptr<SnapShot>& snapShot,  const std::shared_ptr<const Formation> formation, const std::shared_ptr<Volume> volume );
       void addSnapShot(const std::shared_ptr<SnapShot>& snapShot);
       void writeContVolToHDF( const std::shared_ptr<SnapShot>& snapShot, const std::shared_ptr<Volume> volume );

       // member variables
       ibs::FilePath m_fullPath;
       ibs::FilePath m_relPath;
       std::shared_ptr<Project> m_project;
       size_t m_numThreads;
       bool m_basement;
       GeoPhysics::ProjectHandle* m_projectHandle;
    };
}
#endif
