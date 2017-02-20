//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef __ExportToXML_h__
#define __ExportToXML_h__

#include "VisualizationAPI.h"
#include "DataStore.h"
#include "FilePath.h"
#pragma warning (disable:488)
#include <boost/lockfree/queue.hpp>
#include <boost/atomic.hpp>
#include "pugixml.hpp"
#include <memory>

namespace CauldronIO
{
    class ExportToXML
    {
    public:
        /// \brief Exports the current Project to XML and saves all 2D and 3D data in the supplied path
        /// Returns true on success, throws a CauldronIOException on failure
        /// \param[in] project the project to export to XML
        /// \param[in] absPath the path with the xml file name 
        /// \param[in] numThreads number of threads (optional) used for compression
        /// \param[in] center if true, cell-center all properties except depth
        static bool exportToXML(std::shared_ptr<Project>& project, const std::shared_ptr<Project>& projectExisting,
			const std::string& absPath, size_t numThreads = 1, bool center = false);
       
    private:

        /// ImportExport supports the use case that there is an existing xml-project, to which data can be appended. In this case,
        /// we should not re-write the existing binary data (that would be very inefficient)
        /// This method detects that we need to append the binary files, instead of writing them from scratch
        bool detectAppend(std::shared_ptr<Project>& project);

        // Method to compress blocks of data on a thread
        static void compressDataQueue(std::vector< std::shared_ptr < DataToCompress > > allData, boost::lockfree::queue<int>* queue);

		ExportToXML(const ibs::FilePath& absPath, const ibs::FilePath& relPath, size_t numThreads, bool center);
		void addProject(pugi::xml_node pt, std::shared_ptr<Project>& project, const std::shared_ptr<Project>& projectExisting);
        void addProperty(pugi::xml_node node, const std::shared_ptr<const Property>& property) const;
        void addFormation(DataStoreSave& dataStore, pugi::xml_node node, const std::shared_ptr<Formation>& formation) const;
        void addSurface(DataStoreSave& dataStore, pugi::xml_node& ptree, const std::shared_ptr<Surface>& surfaceIO) const;
		void addPropertySurfaceData(pugi::xml_node &valueMapsNode, DataStoreSave &dataStore, const PropertySurfaceData &propertySurfaceData) const;
		void addVolume(DataStoreSave& dataStore, const std::shared_ptr<Volume>& volume, pugi::xml_node volNode);
		void addReferenceData(pugi::xml_node &node, const DataStoreParams* params, bool dataIJK, bool dataKIJ) const;
		void addGeometryInfo2D(pugi::xml_node node, const std::shared_ptr<const Geometry2D>& geometry) const;
        void addSnapShot(const std::shared_ptr<SnapShot>& snapShot, ibs::FilePath fullPath, pugi::xml_node node);
		void addStratTableNode(pugi::xml_node& stratTableNode, const StratigraphyTableEntry& entry, DataStoreSave& dataStoreSave);

        // member variables
		ibs::FilePath m_absPath;
		ibs::FilePath m_relPath;
        std::shared_ptr<Project> m_project;
		std::shared_ptr<const Project> m_projectExisting;
		bool m_append, m_center;
        size_t m_numThreads;
	};
}
#endif
