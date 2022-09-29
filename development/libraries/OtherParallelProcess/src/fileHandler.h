#ifndef ONEFILEPERPROCESS_FILEHANDLER_H
#define ONEFILEPERPROCESS_FILEHANDLER_H

#include <string>
#include <vector>
#include "h5merge.h"

const int MAX_FILE_DIMENSION = 3;
const int MAX_ATTRIBUTE_NAME_SIZE = 64;


/// \brief This file contains a functionality for merging multiple HDF files (located in  $TMPDIRs) into one file (located in Project path).
///        Different merging modes:
/// CREATE - Every process reads its local file. Process with rank 0 creates a global file in project path directory and writes merging results to it
/// REUSE  - Every process reads its local file. Process with rank 0 re-writes its local file with merging results.
///          The merging file has to be copied to a final destination afterwards.
/// APPEND - Every process reads its local file. Process with rank 0 opens/create a global file in Project path and updates it with merging results.

   /// \brief Handles merging of HDF output files produced by simulation with OFPP (one file per process) enabled
class  FileHandler {

public:
   FileHandler( MPI_Comm comm, const std::string & fileName, const std::string & tempDirName );
   FileHandler( const FileHandler& fileHandler) = delete;
   FileHandler( FileHandler&& fileHandler) = delete;

   FileHandler& operator=(const FileHandler& fileHandler) = delete;
   FileHandler& operator=(FileHandler&& fileHandler) = delete;


   virtual ~FileHandler();

   // \brief Iterate over the local file and write data to the global file
   friend herr_t readDataset ( hid_t groupId, const char* name, void * voidReader);

   /// \brief Merge local files with a given name (m_fileName) and path (m_tempDirName) into a one global file
   ///
   bool mergeFiles( const bool appendRank = true );

   /// \brief Open a global file
   virtual void openGlobalFile( );
   /// \brief Open a local file with defined access properties
   virtual void openLocalFile( hid_t fileAccessPList );
   /// \brief Create or open a group in global file
   virtual void createGroup( const char* name );
   /// \brief Close global file
   virtual hid_t closeGlobalFile();
   /// \brief Close filespace and memspace
   virtual void closeSpaces();
   /// \brief Close DataSet in global file
   virtual void closeGlobalDset();
   /// \brief Write attributes into global file if necessary
   virtual void writeAttributes();
   /// \brief Create dataset into  global file
   virtual void createDataset( const char* name , hid_t dtype );

   /// \brief collective check of an error
   int  checkError ( hid_t value );

   /// \brief read/write attributes if neseccary
   herr_t readAttributes( hid_t localId, hid_t globalId );

   /// \brief Reallocate buffers for reading of dataset depends on datasize type
   herr_t reallocateBuffers ( ssize_t dataSize );

   /// \brief merge 1D dataset
   herr_t merge1D ( const char* name, hid_t dtype );

   /// \brief merge 2D dataset
   herr_t merge2D ( const char* name, hid_t dtype );

  /// \brief Access members
   int getRank() const;
   hid_t getLocalFileId () const;
   hid_t getGlobalFileId () const;
   hid_t getGroupId () const;
   hid_t getLocalDsetId() const;
   hid_t getGlobalDsetId() const;
   hid_t getMemspace() const;
   hid_t getFilespace() const;
   hid_t getSpatialDimension() const;
   const char * getFileName() const;

   void setLocalFileId ( hid_t aLocalId );
   void setGlobalFileId ( hid_t aGlobalId );
   void setGroupId( hid_t aGroupId );
   void setGlobalDsetId( hid_t aGlobalId );
   void setMemspace( hid_t aMemspace );
   void setFilespace( hid_t aMemspace );
   void setSpatialDimension ( int dimension );

protected:

   hsize_t m_count [MAX_FILE_DIMENSION];
   hsize_t m_offset [MAX_FILE_DIMENSION];

private:

   friend void readGroup(const char* name, FileHandler* reader, void* voidReader);

   int      m_rank;      // rank of the processor
   MPI_Comm m_comm;

   /// \brief Name of the file to merge
   std::string m_fileName;

   hid_t m_groupId = H5P_DEFAULT;
   hid_t m_localFileId = H5P_DEFAULT;   // local data file (to read from)
   hid_t m_globalFileId = H5P_DEFAULT;  // global data file (to write into)
   hid_t m_local_dset_id = H5P_DEFAULT;
   hid_t m_global_dset_id = H5P_DEFAULT;

   hid_t m_memspace = H5P_DEFAULT;
   hid_t m_filespace = H5P_DEFAULT;

   std::vector<float> m_data;     // local data buffer for 2D dataset (for reading)
   std::vector<float> m_sumData;  // global data buffer for 2D dataset (for collecting data on rank 0)

   std::vector<char> m_data1D;    // data buffer for 1D dataset (for reading and writing )
   std::vector<char> m_attrData;  // attributes buffer

   int m_spatialDimension = 0;        // data dimentions
   hsize_t m_dimensions [MAX_FILE_DIMENSION];

   /// \brief Path to Temporary directory where local files are located
   std::string m_tempDirName;

   /// \brief Operation to collect data on 0 process
   MPI_Op m_op;

   /// \brief Name of the current group
   std::string m_groupName;
};


herr_t readDataset ( hid_t groupId, const char* name, void * voidReader);


inline int FileHandler::getRank() const {
   return  m_rank;
}

inline hid_t FileHandler::getLocalFileId() const {
   return m_localFileId;
}

inline hid_t FileHandler::getGlobalFileId() const {
   return m_globalFileId;
}

inline hid_t FileHandler::getGroupId() const {
   return  m_groupId;
}

inline hid_t FileHandler::getLocalDsetId() const {
   return m_local_dset_id;
}

inline hid_t FileHandler::getGlobalDsetId() const {
   return m_global_dset_id;
}

inline hid_t FileHandler::getMemspace() const {
   return m_memspace;
}

inline hid_t FileHandler::getFilespace() const {
   return m_filespace;
}

inline hid_t FileHandler::getSpatialDimension() const {
   return m_spatialDimension;
}

#endif
