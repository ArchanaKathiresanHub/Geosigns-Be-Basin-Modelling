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
#include <boost/shared_ptr.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/filesystem.hpp>

namespace CauldronIO
{
    class ImportExport
    {
    public:
        static bool exportToXML(boost::shared_ptr<Project>& project, const std::string& filename);
        static std::string getXMLIndexingFileName(const boost::filesystem::path& path);

    private:
        class DataStore;

        ImportExport(const boost::filesystem::path& path);
        void addProject(boost::property_tree::ptree& pt, boost::shared_ptr<Project>& project);
        void addProperty(boost::property_tree::ptree &node, const boost::shared_ptr<const Property>& property) const;
        void addFormation(boost::property_tree::ptree& node, const boost::shared_ptr<const Formation>& formation) const;
        void addGeometryInfo(boost::property_tree::ptree& tree, const boost::shared_ptr<const Map>& map) const;
        void addGeometryInfo(boost::property_tree::ptree& tree, const boost::shared_ptr<const Volume>& volume) const;
        void addDataStorage(boost::property_tree::ptree& volNode, const boost::shared_ptr<Volume>& volume, DataStore& dataStore) const;
        std::string getFilename(const boost::uuids::uuid& uuid) const;

        void writeSurface(const boost::shared_ptr<Surface>& surfaceIO, DataStore& store) const;
        void writeVolume(const boost::shared_ptr<Volume>& volume, bool dataIJK, DataStore& store) const;

        boost::filesystem::path m_outputPath;

        // Internal helper class
        class DataStore
        {
        public:
            DataStore(const std::string& filename, bool compress, bool fp16);
            ~DataStore();
            size_t getOffset() const;
            void addData(const float* data, size_t size, float undef);
            const std::string& getFileName() const;
            size_t getLastSize() const;
            bool getCompress() const;
            bool getFP16() const;

        private:
            std::vector<char> decompress(const char* data, size_t size) const;
            std::vector<char> compress(const char* data, size_t size) const;
            template <typename T> void getStatistics(const T* data, size_t size, T undef);
            boost::iostreams::stream<boost::iostreams::file_sink> m_file;
            size_t m_offset, m_lastSize;
            std::string m_fileName;
            bool m_compress;
            bool m_fp16;
        };
    };
}
#endif
