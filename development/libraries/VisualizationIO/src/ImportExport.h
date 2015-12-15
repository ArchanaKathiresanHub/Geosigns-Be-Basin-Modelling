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
        /// \brief Exports the current Project to XML and saves all 2D and 3D data in the supplied path
        /// Returns true on success, throws a CauldronIOException on failure
        /// if \param[in] release is true, data will be unloaded after storing
        static bool exportToXML(boost::shared_ptr<Project>& project, const std::string& path, bool release = false);
        /// \brief Creates a new Project from the supplied XML indexing file
        /// Throws a CauldronIOException on failure
        static boost::shared_ptr<Project> importFromXML(const std::string& filename);
        /// \brief Returns the default XML indexing filename 
        static std::string getXMLIndexingFileName();

    private:
        ImportExport(const boost::filesystem::path& path = "");
        void addProject(boost::property_tree::ptree& pt, boost::shared_ptr<Project>& project, bool release);
        void addProperty(boost::property_tree::ptree &node, const boost::shared_ptr<const Property>& property) const;
        void addFormation(boost::property_tree::ptree& node, const boost::shared_ptr<const Formation>& formation) const;
        std::string getFilename(const boost::uuids::uuid& uuid) const;
        boost::shared_ptr<Property> getProperty(const boost::property_tree::ptree& surfaceNode) const;
        boost::shared_ptr<Formation> getFormation(const boost::property_tree::ptree& surfaceNode) const;
        boost::shared_ptr<Project> getProject(const boost::property_tree::ptree& pt) const;
        bool detectAppend(boost::shared_ptr<Project>& project);

        boost::filesystem::path m_outputPath;
    };
}
#endif
