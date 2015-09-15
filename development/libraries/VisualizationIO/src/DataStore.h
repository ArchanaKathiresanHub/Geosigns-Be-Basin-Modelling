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
#include <boost/property_tree/xml_parser.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/filesystem.hpp>

namespace CauldronIO
{
    class Surface;
    class Volume;

    // Internal helper class
    class DataStore
    {
    public:
        DataStore(const std::string& filename, bool compress, bool write);
        ~DataStore();
        void addSurface(const boost::shared_ptr<Surface>& surfaceIO, boost::property_tree::ptree& ptree);
        void addVolume(const boost::shared_ptr<Volume>& volume, boost::property_tree::ptree& ptree);
        
        // TODO: This should be higher level, similar to the addSurface/addVolume
        float* getData(size_t offset, size_t& size);

        // Returns a decompressed char* with size "size", for given input data char* and size
        static char* decompress(const char* data, size_t& size);
        // Returns a compressed char* with size "size", for given input data char* and size
        static char* compress(const char* data, size_t& size);

    private:
        size_t getOffset() const;
        const std::string& getFileName() const;
        size_t getLastSize() const;
        void addData(const float* data, size_t size, float undef);
        void writeVolume(const boost::shared_ptr<Volume>& volume, bool dataIJK);
        template <typename T> void getStatistics(const T* data, size_t size, T undef);

        boost::iostreams::stream<boost::iostreams::file_sink> m_file_out;
        boost::iostreams::stream<boost::iostreams::file_source> m_file_in;
        size_t m_offset, m_lastSize;
        std::string m_fileName;
        bool m_compress;
        bool m_write;
    };
}

#endif
