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
        static bool ExportToXML(boost::shared_ptr<Project>& project, const std::string& filename);
        static std::string ImportExport::GetXMLIndexingFileName(const boost::filesystem::path& path);

    private:
        class DataStore;

        ImportExport(const boost::filesystem::path& path);
        void AddProject(boost::property_tree::ptree& pt, boost::shared_ptr<Project>& project);
        void AddProperty(boost::property_tree::ptree &node, const boost::shared_ptr<const Property>& property);
        void AddFormation(boost::property_tree::ptree& node, const boost::shared_ptr<const Formation>& formation);
        void AddGeometryInfo(boost::property_tree::ptree& tree, const boost::shared_ptr<const Map>& map);
        void AddGeometryInfo(boost::property_tree::ptree& tree, const boost::shared_ptr<const Volume>& volume);
        void AddDataStorage(boost::property_tree::ptree& volNode, const boost::shared_ptr<Volume>& volume, DataStore& dataStore);
        std::string GetFilename(const boost::uuids::uuid& uuid) const;

        void WriteSurface(const boost::shared_ptr<Surface>& surfaceIO, DataStore& store);
        void WriteVolume(const boost::shared_ptr<Volume>& volume, bool dataIJK, DataStore& store);

        boost::filesystem::path _outputPath;

        // Internal helper class        
        class DataStore
        {
        public:
            DataStore(const std::string& filename, bool compress, bool fp16);
            ~DataStore();
            size_t GetOffset() const;
            void AddData(const float* data, size_t size, float undef);
            const std::string& GetFileName() const;
            size_t GetLastSize() const;
            bool GetCompress() const;
            bool GetFP16() const;
        
        private:
            std::vector<char> Decompress(const char* data, size_t size) const;
            std::vector<char> Compress(const char* data, size_t size) const;
            template <typename T> void GetStatistics(const T* data, size_t size, T undef);
            boost::iostreams::stream<boost::iostreams::file_sink> _file;
            size_t _offset, _lastSize;
            std::string _fileName;
            bool _compress;
            bool _fp16;
        };
    };
}
#endif
