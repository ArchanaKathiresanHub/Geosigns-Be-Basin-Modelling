//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef __DataStore_h__
#define __DataStore_h__

#include <boost/shared_ptr.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>

namespace CauldronIO
{
    // Some forward decl
    class Surface;
    class Volume;
    class Property;
    class SurfaceData;
    class VolumeData;
    class Geometry2D;
    class Geometry3D;

    /// \brief Little struct to hold data 
    struct DataStoreParams
    {
    };

    /// \brief Native implementation
    struct DataStoreParamsNative : DataStoreParams
    {
        boost::filesystem::path fileName;
        size_t offset;
        size_t size;
        bool compressed;
    };

    /// \brief Little class to load data from binary storage
    class DataStoreLoad
    {
    public:
        /// \brief Prepares the dataload from the given parameters
        DataStoreLoad(DataStoreParams* params);
        /// \brief Load the data from the datastore
        float* getData(size_t& size);
        ~DataStoreLoad();

        // Returns a decompressed char* with size "size", for given input data char* and size
        static char* decompress(const char* data, size_t& size);

        /// \brief Creates a volume from the current XML node and assigns given Property
        static void getVolume(const boost::property_tree::ptree& ptree, boost::shared_ptr<VolumeData> volumeData, const boost::filesystem::path& path);
        /// \brief Gets surfacedata from the current XML node
        static void getSurface(const boost::property_tree::ptree& ptree, boost::shared_ptr<SurfaceData> surfaceData, const boost::filesystem::path& path);

    private:
        std::ifstream m_file_in;
        DataStoreParamsNative* m_params;
    };

    /// \brief Little class to load data from binary storage
    class DataStoreSave
    {
    public:
        /// \brief Creates a new instance, to store binary data to the given filename
        /// \param [in] append If true, appends to existing data structure, otherwise, write from scratch. This is only supported with native data
        /// \param [in] release If true, data will be release upon saving
        /// \param [in] filename filename where to save to
        DataStoreSave(const std::string& filename, bool append);
        ~DataStoreSave();

        /// \brief Adds a surface to the XML node, and writes the binary data
        void addSurface(const boost::shared_ptr<SurfaceData>& surfaceData, boost::property_tree::ptree& node, size_t size);
        /// \brief Adds a volume to the XML node, and writes the binary data
        void addVolume(const boost::shared_ptr<VolumeData>& data, boost::property_tree::ptree& node, size_t numBytes);
        // Returns a compressed char* with size "size", for given input data char* and size
        static char* compress(const char* data, size_t& size);

    private:
        void writeVolume(const boost::shared_ptr<VolumeData>& volume, bool dataIJK, bool compress);
        void writeVolumePart(boost::property_tree::ptree &volNode, bool compress, bool IJK, const boost::shared_ptr<VolumeData>& volume);
        void addData(const float* data, size_t size, bool compressData);

        std::ofstream m_file_out;
        size_t m_offset, m_lastSize;
        std::string m_fileName;
        bool m_compress;
    };
}

#endif
