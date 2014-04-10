#ifndef ONEFILEPERPROCESS_H5MERGE_H
#define ONEFILEPERPROCESS_H5MERGE_H

#include <string>
#include <vector>
#include "stdlib.h"

#include "hdf5.h"


const int MAX_FILE_DIMENSION = 3;
const int MAX_ATTRIBUTE_NAME_SIZE = 64;

class  FileHandler {

   // iterate over the local file and write data to the global file
   friend herr_t readDataset ( hid_t groupId, const char* name, void * voidReader);

   // megre all files with the given name
   friend bool mergeFiles( MPI_Comm comm, const std::string &fileName );

public:
   FileHandler( MPI_Comm comm );

   herr_t reallocateBuffers ( ssize_t dataSize ); 

   // read/write attributes
   herr_t readAttributes( hid_t localId, hid_t globalId );
   
    // collective check of an error
   int  checkError ( hid_t value );

   void setGlobalId( hid_t id );
   void setSpatialDimension ( int dimension );

private:
   int m_rank; // rank of the processor

   size_t m_valCount;    // size of allocated local buffer
   size_t m_attrCount;   // size of the attribute buffer
   std::vector<char> m_data;     // local data buffer (to read)
   std::vector<char> m_sumData;  // global data buffer (to write from rank 0)
   std::vector<char> m_attrData; // attributes buffer

   int m_spatialDimension; // data dimentions
   hid_t m_localFileId;    // local data file (to read from)
   hid_t m_globalFileId;   // global data file (to write into)
   hid_t m_groupId;

   hsize_t m_dimensions [MAX_FILE_DIMENSION]; 
   hsize_t m_count [MAX_FILE_DIMENSION];
   hsize_t m_offset [MAX_FILE_DIMENSION];
   
   MPI_Comm m_comm;
   std::string m_fileName; 
  
};

#endif
