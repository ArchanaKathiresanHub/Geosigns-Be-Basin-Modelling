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
#include "FilePath.h"
#pragma warning (disable:488)
#include <boost/lockfree/queue.hpp>
#include <boost/atomic.hpp>
#include "pugixml-1.7/pugixml.hpp"
#include <memory>

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
        /// \param[in] absPath the path with the xml file name 
        /// \param[in] numThreads number of threads (optional) used for compression
        static bool exportToXML(std::shared_ptr<Project>& project, const std::string& absPath, size_t numThreads = 1);
        /// \brief Creates a new Project from the supplied XML indexing file
        /// Throws a CauldronIOException on failure
        static std::shared_ptr<Project> importFromXML(const std::string& filename);
        /// \brief Returns a default XML indexing filename 
        static std::string getXMLIndexingFileName(const std::string& project3Dfilename);
        
    private:

        /// ImportExport supports the use case that there is an existing xml-project, to which data can be appended. In this case,
        /// we should not re-write the existing binary data (that would be very inefficient)
        /// This method detects that we need to append the binary files, instead of writing them from scratch
        bool detectAppend(std::shared_ptr<Project>& project);

        // Method to compress blocks of data on a thread
        static void compressDataQueue(std::vector< std::shared_ptr < DataToCompress > > allData, boost::lockfree::queue<int>* queue);
        // Method to retrieve data on a thread
        static void retrieveDataQueue(std::vector < VisualizationIOData* >* allData, boost::lockfree::queue<int>* queue, boost::atomic<bool>* done);

        ImportExport(const ibs::FilePath& absPath, const ibs::FilePath& relPath, size_t numThreads);
        void addProject(pugi::xml_node pt, std::shared_ptr<Project>& project);
        void addProperty(pugi::xml_node node, const std::shared_ptr<const Property>& property) const;
        void addFormation(pugi::xml_node node, const std::shared_ptr<const Formation>& formation) const;
        std::shared_ptr<Property> getProperty(pugi::xml_node propertyNode) const;
        std::shared_ptr<Formation> getFormation(pugi::xml_node formationNode) const;
        std::shared_ptr<Project> getProject(const pugi::xml_document& pt);
        std::shared_ptr<const Reservoir> getReservoir(pugi::xml_node reservoirNode) const;
        std::shared_ptr<const Geometry2D> getGeometry2D(pugi::xml_node surfaceNode, const char* name) const;
        std::shared_ptr<Geometry3D> getGeometry3D(pugi::xml_node volumeNode) const;
        std::shared_ptr<Volume> getVolume(pugi::xml_node volumeNode, const ibs::FilePath& path);
        void addSurface(DataStoreSave& dataStore, const std::shared_ptr<Surface>& surfaceIO, pugi::xml_node ptree);
        void addVolume(DataStoreSave& dataStore, const std::shared_ptr<Volume>& volume, pugi::xml_node volNode);
        void addGeometryInfo2D(pugi::xml_node node, const std::shared_ptr<const Geometry2D>& geometry, const std::string& name) const;
        void addGeometryInfo3D(pugi::xml_node  tree, const std::shared_ptr<const Geometry3D>& geometry) const;
        void addSnapShot(const std::shared_ptr<SnapShot>& snapShot, std::shared_ptr<Project>& project, ibs::FilePath fullPath, pugi::xml_node node);
        void retrieveAllData(const std::shared_ptr<SnapShot>& snapShot);
        void prefetchHDFdata(std::vector< VisualizationIOData* > allReadData, boost::lockfree::queue<int>* queue);
        void loadHDFdata(std::vector< std::shared_ptr<HDFinfo> > hdfInfoList, boost::lockfree::queue<int>* queue);
        // member variables
		ibs::FilePath m_absPath;
		ibs::FilePath m_relPath;
        std::shared_ptr<Project> m_project;
        bool m_append;
        size_t m_numThreads;
    };
}
#endif
