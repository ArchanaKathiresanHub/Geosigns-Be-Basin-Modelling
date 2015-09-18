//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef __ImportExport_h__
#define __ImportExport_h__

#include "VisualizationAPI.h"
#include "DataStore.h"
#include <boost/shared_ptr.hpp>
#include <boost/property_tree/ptree.hpp>

namespace CauldronIO
{
    class ImportExport
    {
    public:
        static bool exportToXML(boost::shared_ptr<Project>& project, const std::string& filename);
        static boost::shared_ptr<Project> importFromXML(const std::string& filename);
        static std::string getXMLIndexingFileName(const boost::filesystem::path& path);

    private:
        ImportExport(const boost::filesystem::path& path = "");
        void addProject(boost::property_tree::ptree& pt, boost::shared_ptr<Project>& project);
        void addProperty(boost::property_tree::ptree &node, const boost::shared_ptr<const Property>& property) const;
        void addFormation(boost::property_tree::ptree& node, const boost::shared_ptr<const Formation>& formation) const;
        std::string getFilename(const boost::uuids::uuid& uuid) const;
        boost::shared_ptr<Property> getProperty(const boost::property_tree::ptree& surfaceNode) const;
        boost::shared_ptr<Formation> getFormation(const boost::property_tree::ptree& surfaceNode) const;
        boost::shared_ptr<Project> getProject(const boost::property_tree::ptree& pt) const;
        
        boost::filesystem::path m_outputPath;
    };
}
#endif
