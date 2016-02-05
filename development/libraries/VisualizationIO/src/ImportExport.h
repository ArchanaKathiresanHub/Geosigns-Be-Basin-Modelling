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

#define xml_version 0.1

namespace CauldronIO
{
    class ImportExport
    {
    public:
        /// \brief Exports the current Project to XML and saves all 2D and 3D data in the supplied path
        /// Returns true on success, throws a CauldronIOException on failure
        /// if \param[in] release is true, data will be unloaded after storing
        /// \param[in] project the project to export to XML
        /// \param[in] absPath the output directory path where to store the xml indexing file 
        /// \param[in] relPath the output directory path where to store the binary outputs; will be created if not existing
        static bool exportToXML(boost::shared_ptr<Project>& project, const std::string& absPath, const std::string& relPath, bool release);
        /// \brief Creates a new Project from the supplied XML indexing file
        /// Throws a CauldronIOException on failure
        static boost::shared_ptr<Project> importFromXML(const std::string& filename);

    private:
        ImportExport(const boost::filesystem::path& absPath, const boost::filesystem::path& relPath);
        void addProject(boost::property_tree::ptree& pt, boost::shared_ptr<Project>& project, bool release);
        void addProperty(boost::property_tree::ptree &node, const boost::shared_ptr<const Property>& property) const;
        void addFormation(boost::property_tree::ptree& node, const boost::shared_ptr<const Formation>& formation) const;
        boost::shared_ptr<Property> getProperty(const boost::property_tree::ptree& surfaceNode) const;
        boost::shared_ptr<Formation> getFormation(const boost::property_tree::ptree& surfaceNode) const;
        boost::shared_ptr<Project> getProject(const boost::property_tree::ptree& pt);
        bool detectAppend(boost::shared_ptr<Project>& project);
        boost::shared_ptr<const Reservoir> getReservoir(const boost::property_tree::ptree& reservoirNode) const;
        boost::shared_ptr<const Geometry2D> getGeometry2D(const boost::property_tree::ptree& surfaceNode) const;
        boost::shared_ptr<const Geometry3D> getGeometry3D(const boost::property_tree::ptree& volumeNode);
        boost::shared_ptr<Volume> getVolume(const boost::property_tree::ptree& volumeNode, const boost::filesystem::path& path);
        /// \brief Returns the default XML indexing filename 
        static std::string getXMLIndexingFileName();

        // member variables
        boost::filesystem::path m_absPath, m_relPath;
        boost::shared_ptr<Project> m_project;
    };
}
#endif
