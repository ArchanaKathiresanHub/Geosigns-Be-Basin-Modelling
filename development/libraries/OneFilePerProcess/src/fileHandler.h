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
/// RESUE  - Every process reads its local file. Process with rank 0 re-writes its local file with merging results. 
///          The merging file has to be copied to a final destination afterwards.
/// APPEND - Every process reads its local file. Process with rank 0 opens/create a global file in Project path and updates it with merging results.

   /// \brief Handles merging of HDF output files produced by simulation with OFPP (one file per process) enabled
class  FileHandler {
 
public:  
   FileHandler( MPI_Comm comm, const std::string & fileName, const std::string & tempDirName );  

   virtual ~FileHandler() {};

   // \brief Iterate over the local file and write data to the global file
   friend herr_t readDataset ( hid_t groupId, const char* name, void * voidReader);

   /// \brief Merge local files with a given name (m_fileName) and path (m_tempDirName) into a one global file
   ///
   bool mergeFiles( );

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

   /// \brief methods for unit tests
   void setGlobalId( hid_t id );
   void setLocalId( hid_t id );
   void setSpatialDimension ( int dimension );

protected:
   int   m_rank;           // rank of the processor
   hid_t m_localFileId;    // local data file (to read from)
   hid_t m_globalFileId;   // global data file (to write into)
   hid_t m_groupId;
   hid_t m_local_dset_id;
   hid_t m_global_dset_id;

   hid_t m_memspace;
   hid_t m_filespace;

   hsize_t m_count [MAX_FILE_DIMENSION];
   hsize_t m_offset [MAX_FILE_DIMENSION];

   int m_spatialDimension; // data dimentions

   /// \brief Name of the file to merge
   std::string m_fileName; 

private:
   MPI_Comm m_comm;

   size_t m_valCount;    // size of allocated local buffer
   size_t m_attrCount;   // size of the attribute buffer
   std::vector<char> m_data;     // local data buffer (to read)
   std::vector<char> m_sumData;  // global data buffer (to write from rank 0)
   std::vector<char> m_attrData; // attributes buffer

   hsize_t m_dimensions [MAX_FILE_DIMENSION]; 
   /// \brief Path to Temporary directory where local files are located
   std::string m_tempDirName; 
};

/// \brief Interface to allocate file handler
FileHandler * allocateFileHandler ( MPI_Comm comm, const std::string & fileName, const std::string & tempDirName, mergeOption anOption );
  
herr_t readDataset ( hid_t groupId, const char* name, void * voidReader);


#endif
