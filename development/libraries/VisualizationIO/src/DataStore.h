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
#include "FilePath.h"

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

    /// \brief Little struct to hold parameters to retrieve data 
    struct DataStoreParams
    {
       DataStoreParams() {;}
    };

    /// \brief Native implementation
    struct DataStoreParamsNative : DataStoreParams
    {
        ibs::FilePath fileName;
        size_t offset;
        size_t size;
        bool compressed;
        bool compressed_lz4;
        
        DataStoreParamsNative() : fileName( "" ) {;}
    };

    /// \brief Little class to load data from binary storage
    class DataStoreLoad
    {
    public:
        /// \brief Prepares the dataload from the given parameters
        DataStoreLoad(DataStoreParams* params);
        /// \brief Load the data from the datastore: ownership is transferred to caller! 
        /// \param [inout] size (input): the expected uncompressed size of the data; output: the actual uncompressed size
        float* getData(size_t& size);
        /// \brief Load the data from disk, but do not decompress yet
        void prefetch();
        ~DataStoreLoad();

        // Returns a decompressed char* with size "size", for given input data char* and size
        static char* decompress(const char* data, size_t& size);
        // Returns a decompressed char* with size "size", for given input data char* and uncompressed size; the compressedSize will be output
        static char* decompress_lz4(const char* inputData, size_t& compressedSize, size_t uncompressedSize);
        /// \brief Creates a volume from the current XML node and assigns given Property
        static void getVolume(pugi::xml_node propertyVolNode, std::shared_ptr<VolumeData> volumeData, const ibs::FilePath& path);
        /// \brief Gets surfacedata from the current XML node
        static void getSurface(pugi::xml_node propertyMapNode, std::shared_ptr<SurfaceData> surfaceData, const ibs::FilePath& path);

    private:
        std::ifstream m_file_in;
        DataStoreParamsNative* m_params;
        char* m_data_uncompressed;
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

    private:
        const float* m_inputData; // not owned by us (!)
        float* m_outputData;
        size_t m_inputNrElements, m_outputNrBytes, m_offset;
        bool m_compress, m_processed, m_node_set;
        pugi::xml_node m_node;
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

        /// \brief Write all data to disk
        void flush();

        /// \brief Adds a surface to the XML node, and writes the binary data
        void addSurface(const std::shared_ptr<SurfaceData>& surfaceData, pugi::xml_node node);
        /// \brief Adds a volume to the XML node, and writes the binary data
        void addVolume(const std::shared_ptr<VolumeData>& data, pugi::xml_node node, size_t numBytes);
        /// \brief Returns a list with DataToCompress that can be compressed
        std::vector<std::shared_ptr<DataToCompress> > getDataToCompressList();

        // Returns a compressed char* with size "size", for given input data char* and size
        static char* compress(const char* data, size_t& size);
        // Returns a compressed char* with size "size", for given input data char* and size using the lz4 algorithm
        static char* compress_lz4(const char* inputData, size_t& size);

    private:
        void writeVolume(const std::shared_ptr<VolumeData>& volume, bool dataIJK, bool compress);
        void writeVolumePart(pugi::xml_node volNode, bool compress, bool IJK, const std::shared_ptr<VolumeData>& volume);
        void addData(const float* data, size_t size, bool compressData);

        std::ofstream m_file_out;
        std::string m_fileName;
        size_t m_offset;
        bool m_compress;
        std::vector<std::shared_ptr<DataToCompress> > m_dataToCompress;
        bool m_flushed;
    };
}

#endif
