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
    class Surface;
    class Volume;
    class Property;
    class Map;

    /// \brief Little struct to hold data 
    struct DataStoreParams
    {
    };

    /// \brief Native implementation
    struct DataStoreParamsNative : DataStoreParams
    {
        std::string fileName;
        size_t offset;
        size_t size;
        bool compressed;
    };

    class DataStoreLoad
    {
    public:
        DataStoreLoad(DataStoreParams* params);
        float* getData(size_t& size);
        ~DataStoreLoad();

        // Returns a decompressed char* with size "size", for given input data char* and size
        static char* decompress(const char* data, size_t& size);

        static boost::shared_ptr<Volume> getVolume(const boost::property_tree::ptree& ptree, boost::shared_ptr<Property> property);
        static boost::shared_ptr<Surface> getSurface(const boost::property_tree::ptree& ptree, boost::shared_ptr<Property> property);

    private:
        std::ifstream m_file_in;
        DataStoreParamsNative* m_params;
    };

    // Internal helper class
    class DataStoreSave
    {
    public:
        DataStoreSave(const std::string& filename);
        ~DataStoreSave();

        void addSurface(const boost::shared_ptr<Surface>& surfaceIO, boost::property_tree::ptree& ptree);
        void addVolume(const boost::shared_ptr<Volume>& volume, boost::property_tree::ptree& ptree);
        
        // Returns a compressed char* with size "size", for given input data char* and size
        static char* compress(const char* data, size_t& size);

    private:
        void addGeometryInfo(boost::property_tree::ptree& tree, const boost::shared_ptr<const Map>& map) const;
        void addGeometryInfo(boost::property_tree::ptree& tree, const boost::shared_ptr<const Volume>& volume) const;
        void addData(const float* data, size_t size, bool compressData);
        void writeVolume(const boost::shared_ptr<Volume>& volume, bool dataIJK, bool compress);

        std::ofstream m_file_out;
        size_t m_offset, m_lastSize;
        std::string m_fileName;
        bool m_compress;
    };
}

#endif
