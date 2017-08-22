//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef __ImportFromXML_h__
#define __ImportFromXML_h__

#include "VisualizationAPI.h"
#include "DataStore.h"
#include "FilePath.h"

#ifdef _MSC_VER
#pragma warning (disable:488)
#endif

#include <boost/lockfree/queue.hpp>
#include <boost/atomic.hpp>
#include "pugixml.hpp"
#include <memory>

namespace CauldronIO
{
    class ImportFromXML
    {
    public:
        /// \brief Creates a new Project from the supplied XML indexing file
        /// Throws a CauldronIOException on failure
       static std::shared_ptr<Project> importFromXML(const std::string& filename, const bool loadTables = true);
        
    private:

        explicit ImportFromXML(const ibs::FilePath& absPath);
        
        // subroutines to load specific part of the XML file
        std::shared_ptr<Property> getProperty(pugi::xml_node propertyNode) const;
        std::shared_ptr<Formation> getFormation(pugi::xml_node formationNode, const ibs::FilePath& fullOutputPath) const;
        std::shared_ptr<Project> getProject(const pugi::xml_document& pt, const bool loadTables = true);
        std::shared_ptr<const Reservoir> getReservoir(pugi::xml_node reservoirNode) const;
        std::shared_ptr<const Geometry2D> getGeometry2D(pugi::xml_node surfaceNode) const;
        std::shared_ptr<Volume> getVolume(pugi::xml_node volumeNode, const ibs::FilePath& path);
        std::shared_ptr<CauldronIO::Surface> getSurface(pugi::xml_node surfaceNode, const ibs::FilePath& fullOutputPath) const;
        CauldronIO::PropertySurfaceData getPropertySurfaceData(pugi::xml_node &propertyMapNode, const ibs::FilePath& fullOutputPath) const;
        
        // member variables
        ibs::FilePath m_absPath;
        std::shared_ptr<Project> m_project;
    };
}
#endif
