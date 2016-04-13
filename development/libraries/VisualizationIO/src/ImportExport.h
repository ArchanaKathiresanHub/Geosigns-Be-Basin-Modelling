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
#include "pugixml-1.7/pugixml.hpp"

#define xml_version_major 0
#define xml_version_minor 1

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
        /// \param[in] xmlIndexingName the output xml file name
        /// \param[in] numThreads number of threads (optional) used for compression
        static bool exportToXML(boost::shared_ptr<Project>& project, const std::string& absPath, const std::string& relPath, 
             const std::string&, size_t numThreads = 1);
        /// \brief Creates a new Project from the supplied XML indexing file
        /// Throws a CauldronIOException on failure
        static boost::shared_ptr<Project> importFromXML(const std::string& filename);
        /// \brief Returns a default XML indexing filename 
        static std::string getXMLIndexingFileName(const std::string& project3Dfilename);
        
    private:

        /// ImportExport supports the use case that there is an existing xml-project, to which data can be appended. In this case,
        /// we should not re-write the existing binary data (that would be very inefficient)
        /// This method detects that we need to append the binary files, instead of writing them from scratch
        bool detectAppend(boost::shared_ptr<Project>& project);

        // Method to compress blocks of data on a thread
        static void compressData(std::vector< boost::shared_ptr < DataToCompress > > allData);

        // Method to retrieve data on a thread
        typedef std::pair< const boost::shared_ptr<VisualizationIOData>, bool > RetrieveableData;
        static void retrieveData(std::vector < RetrieveableData >* allData);

        ImportExport(const boost::filesystem::path& absPath, const boost::filesystem::path& relPath, size_t numThreads);
        void addProject(pugi::xml_node pt, boost::shared_ptr<Project>& project);
        void addProperty(pugi::xml_node node, const boost::shared_ptr<const Property>& property) const;
        void addFormation(pugi::xml_node node, const boost::shared_ptr<const Formation>& formation) const;
        boost::shared_ptr<Property> getProperty(pugi::xml_node propertyNode) const;
        boost::shared_ptr<Formation> getFormation(pugi::xml_node formationNode) const;
        boost::shared_ptr<Project> getProject(const pugi::xml_document& pt);
        boost::shared_ptr<const Reservoir> getReservoir(pugi::xml_node reservoirNode) const;
        boost::shared_ptr<const Geometry2D> getGeometry2D(pugi::xml_node surfaceNode, const char* name) const;
        boost::shared_ptr<Geometry3D> getGeometry3D(pugi::xml_node volumeNode) const;
        boost::shared_ptr<Volume> getVolume(pugi::xml_node volumeNode, const boost::filesystem::path& path);
        void addSurface(DataStoreSave& dataStore, const boost::shared_ptr<Surface>& surfaceIO, pugi::xml_node ptree);
        void addVolume(DataStoreSave& dataStore, const boost::shared_ptr<Volume>& volume, pugi::xml_node volNode);
        void addGeometryInfo2D(pugi::xml_node node, const boost::shared_ptr<const Geometry2D>& geometry, const std::string& name) const;
        void addGeometryInfo3D(pugi::xml_node  tree, const boost::shared_ptr<const Geometry3D>& geometry) const;
        void addSnapShot(const boost::shared_ptr<SnapShot>& snapShot, boost::shared_ptr<Project>& project, boost::filesystem::path fullPath, pugi::xml_node node);
       

        // member variables
        boost::filesystem::path m_absPath, m_relPath;
        boost::shared_ptr<Project> m_project;
        bool m_append;
        size_t m_numThreads;
    };
}
#endif
