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
#include <boost/thread.hpp>
#include <iostream>
#include <fstream>
#include "pugixml-1.7/pugixml.hpp"

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
        static void getVolume(pugi::xml_node propertyVolNode, boost::shared_ptr<VolumeData> volumeData, const boost::filesystem::path& path);
        /// \brief Gets surfacedata from the current XML node
        static void getSurface(pugi::xml_node propertyMapNode, boost::shared_ptr<SurfaceData> surfaceData, const boost::filesystem::path& path);

    private:
        std::ifstream m_file_in;
        DataStoreParamsNative* m_params;
    };

    /// \brief Little class to hold data to compress
    class DataToCompress
    {
    public:
        /// \brief Creates a new instance of DataToCompress
        /// \param[in] inputData the data to compress
        /// \param[in] nrOfElements the number of floats in the input data to compress
        /// \param[in] compress if true the data will be compressed, otherwise, no compression 
        DataToCompress(const float* inputData, size_t nrOfElements, bool compress);
        /// \brief Destroys the obect
        ~DataToCompress();
        /// \brief Sets the offset within the binary output file; to be written to 
        void setOffset(size_t offset);
        /// \brief Compress this block of data
        void compress();
        /// \brief Returns true if this block has been compressed
        bool isProcessed() const;
        /// \brief Returns a float* to the output data
        const float* getOutputData() const;
        /// \brief
        size_t getOutputSizeInBytes() const;
        /// \brief
        void setXmlNode(pugi::xml_node node);
        /// \brief Writes size and offset to the xml node
        void updateXmlNode();
        /// \brief Returns the threadID assigned to compress this block
        boost::thread::id getThreadId() const;
        /// \brief Sets the threadID to compress this block
        void setThreadId(const boost::thread::id& id);
        /// \return true if this block is not assigned yet to a thread
        bool canBeProcessed() const;

    private:
        const float* m_inputData; // not owned by us (!)
        float* m_outputData;
        size_t m_inputNrElements, m_outputNrBytes, m_offset;
        bool m_compress, m_processed, m_node_set, m_available;
        pugi::xml_node m_node;
        boost::thread::id m_id;
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
        void addSurface(const boost::shared_ptr<SurfaceData>& surfaceData, pugi::xml_node node);
        /// \brief Adds a volume to the XML node, and writes the binary data
        void addVolume(const boost::shared_ptr<VolumeData>& data, pugi::xml_node node, size_t numBytes);

        std::vector<boost::shared_ptr<DataToCompress> > getDataToCompressList();

        // Returns a compressed char* with size "size", for given input data char* and size
        static char* compress(const char* data, size_t& size);

    private:
        void flush();
        void writeVolume(const boost::shared_ptr<VolumeData>& volume, bool dataIJK, bool compress);
        void writeVolumePart(pugi::xml_node volNode, bool compress, bool IJK, const boost::shared_ptr<VolumeData>& volume);
        void addData(const float* data, size_t size, bool compressData);

        std::ofstream m_file_out;
        std::string m_fileName;
        size_t m_offset;
        bool m_compress;
        std::vector<boost::shared_ptr<DataToCompress> > m_dataToCompress;
        bool m_flushed;
    };
}

#endif
