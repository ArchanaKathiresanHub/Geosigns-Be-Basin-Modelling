
#include "mpi.h"
#include "strings.h"
#include "stdlib.h"
#include <vector>
#include <sstream>

using namespace std;
#include "hdf5.h"
#include "H5FDmpio.h"
#include "HDF5VirtualFileDriver.h"

const int MAX_DIMENSION = 3;
const int MAX_ATTRIBUTE_NAME_SIZE = 64;

struct FileHandler {

   int rank; // rank of the current processor

   size_t valCount;    // size of allocated local buffer
   size_t attrCount;   // size of the current attribute buffer
   std::vector<char> data; // local data buffer (to read)
   vector<char> sumData;   // global data buffer (to write from rank 0)
   vector<char> attrData;  // attributes buffer

   int spatialDimension; // data dimentions
   hid_t local_file_id;  // local data file (to read from)
   hid_t gloabl_file_id; // global data file (to write into)
   hid_t group_id;

   hsize_t dimensions [ MAX_DIMENSION ]; 
   hsize_t count [ MAX_DIMENSION ];
   hsize_t offset [ MAX_DIMENSION ];
   
   MPI_Comm comm;
   string   fileName; 
};

herr_t readDataset ( hid_t groupId, const char* name, void * voidReader);
void   readAttributes(  FileHandler * reader, hid_t localId, hid_t globalId );
void   reallocateBuffers ( FileHandler * reader, ssize_t dataSize ); 

herr_t readDataset ( hid_t groupId, const char* name, void * voidReader)  {

   FileHandler* reader = (FileHandler*)( voidReader );

   hid_t memspace, filespace;
   hid_t local_dset_id;
   hid_t status;
   hid_t dataSpaceId;

   H5G_stat_t statbuf;

   status = H5Gget_objinfo ( groupId, name, 0, &statbuf );

   switch ( statbuf.type ) {
   case H5G_GROUP:
      {
         std::stringstream groupName;
         groupName << "/" << name;

         if( reader->rank == 0 ) {       
            // Greate a group in the global file
            reader->group_id = H5Gcreate( reader->gloabl_file_id, name,  H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT ); 
         }
         // Iterate over the members of the group
         H5Giterate ( reader->local_file_id, groupName.str().c_str(), 0, readDataset, voidReader );

         if( reader->rank == 0 ) {       
            H5Gclose( reader->group_id );
         }
         return 0;
      }
      break;
   case H5G_DATASET:
      break;
   case H5G_TYPE:
      // error?
      break;
   default:
      // error?
      break;
   } 

   local_dset_id = H5Dopen ( groupId, name, H5P_DEFAULT );
     
   dataSpaceId = H5Dget_space ( local_dset_id );

   reader->spatialDimension = H5Sget_simple_extent_dims ( dataSpaceId, reader->dimensions, 0 );

   H5Sclose( dataSpaceId );
 
   // Get the dataset datatype
   hid_t   dtype    = H5Dget_type( local_dset_id );
   ssize_t dataSize = H5Tget_size(dtype);
 
   // Allocate data buffers
   reallocateBuffers ( reader, dataSize );

   // Read the local data
   memspace = H5Screate_simple( reader->spatialDimension, reader->count, NULL ); 
   filespace = H5Dget_space( local_dset_id );

   H5Sselect_hyperslab( filespace, H5S_SELECT_SET, reader->offset, NULL, reader->count, NULL );

   status = H5Dread ( local_dset_id, dtype, memspace, filespace, H5P_DEFAULT, reader->data.data() );

   H5Sclose( filespace );
   H5Sclose( memspace );

   // Summarize all local data in a global array
   if( reader->spatialDimension == 1 ) {
      if( reader->rank == 0 ) {
         reader->sumData = reader->data;
      }
   } else {
      MPI_Reduce( reader->data.data(), reader->sumData.data(), reader->valCount, MPI_CHAR, MPI_SUM, 0, reader->comm );
   }
   if( reader->rank == 0 ) {
      hid_t global_dset_id;
      // Write the global data into the global file
      memspace = H5Screate_simple( reader->spatialDimension, reader->count, NULL ); 
      if( reader->group_id != H5P_DEFAULT ) {
         // Dataset is under the sub-group
         global_dset_id = H5Dcreate( reader->group_id, name, dtype, memspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
      } else {
         // Dataset is under the main group
         global_dset_id = H5Dcreate( reader->gloabl_file_id, name, dtype, memspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
      }
      filespace = H5Dget_space( global_dset_id );
      H5Sselect_hyperslab( filespace, H5S_SELECT_SET, reader->offset, NULL, reader->count, NULL );
 
      status = H5Dwrite( global_dset_id, dtype, memspace, filespace, H5P_DEFAULT, reader->sumData.data() );

      readAttributes( reader, local_dset_id, global_dset_id ); 
      
      H5Sclose( filespace );
      H5Sclose( memspace );
      H5Dclose( global_dset_id );
   }
 
   H5Dclose( local_dset_id );

  return 0;
}

void reallocateBuffers ( FileHandler * reader, ssize_t dataSize ) {

   reader->valCount = dataSize;
   
   for (int d = 0; d < reader->spatialDimension; ++d) {
      reader->count [d] = reader->dimensions[d];       
      reader->valCount *= reader->count[d];
   } 
   reader->data.resize( reader->valCount );
   reader->sumData.resize( reader->valCount );
}

void readAttributes( FileHandler * reader, hid_t localDataSetId, hid_t globalDataSetId )  {

   // Get the number of attributes for the dataSet.
    int attrNumber = H5Aget_num_attrs(localDataSetId);
    
    hsize_t spatialDims [MAX_DIMENSION];
    char attrName [MAX_ATTRIBUTE_NAME_SIZE];

    // Iterate over the number of Attributes 
    if( attrNumber > 0 ) {
       
       for( int indx = 0; indx < attrNumber; ++ indx ) {
          // read Attribute
          hid_t localAttrId = H5Aopen_idx(localDataSetId, indx );   
     
          H5Aget_name( localAttrId, MAX_ATTRIBUTE_NAME_SIZE, attrName );   

          hid_t dataTypeId = H5Aget_type( localAttrId );

          hid_t dataSpace  = H5Aget_space( localAttrId );

          int   dims = H5Sget_simple_extent_dims( dataSpace, spatialDims, 0 );
          
          size_t attrSize = H5Sget_simple_extent_npoints( dataSpace );
          
	  //  Get the size of the type
          ssize_t dataSize = H5Tget_size(dataTypeId);

          reader->attrCount = dataSize * attrSize;
          reader->attrData.resize( reader->attrCount );

          H5Aread( localAttrId, dataTypeId, reader->attrData.data() );

          // write Attribute
          size_t space = H5Screate_simple ( dims, spatialDims, NULL );

          hid_t  globalAttrId = H5Acreate( globalDataSetId, attrName, dataTypeId, space, H5P_DEFAULT, H5P_DEFAULT );
          
          H5Awrite ( globalAttrId, dataTypeId, reader->attrData.data() );
          
	  H5Tclose( dataTypeId );
          H5Aclose( localAttrId );
          H5Aclose( globalAttrId );
          H5Sclose( dataSpace );
        } 
    } 
}

bool mergeFiles( MPI_Comm comm, const string &fileName ) {
 
   FileHandler reader;
   hid_t local_status = 0, global_status = 0;

   MPI_Comm_rank( comm, &reader.rank );
   reader.comm = comm;
 
   hid_t fileAccessPList = H5Pcreate(H5P_FILE_ACCESS);

   H5Pset_fapl_ofpp ( fileAccessPList, comm, "{NAME}_{MPI_RANK}", 0 );  
   
   reader.local_file_id = H5Fopen( fileName.c_str(), H5F_ACC_RDONLY, fileAccessPList );

   H5Pclose( fileAccessPList );
   
   if( reader.local_file_id < 0 ) {
      local_status = 1;
   }

   MPI_Reduce ( &global_status, &local_status, 1, MPI_INT, MPI_SUM, 0, reader.comm ); 

   if( global_status > 0 ) {
      return false;
   }
   
   if( reader.rank == 0 ) {
      // What to use? H5F_ACC_EXCL or H5F_ACC_TRUNC?
      // HDF5 is not able to detect if the file with the same name is already open by H5Fopen when uses H5F_ACC_TRUNC.
      // If the VFD failed to output "one file per processor", then a file with the same name will be created at this point.
      // This should not happen.
      
      reader.gloabl_file_id = H5Fcreate( fileName.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT ); 

      if( reader.gloabl_file_id < 0 ) {
         local_status = 1; 
      }
      reader.group_id = H5P_DEFAULT;       
   }
   
   MPI_Bcast( &local_status, 1, MPI_INT, 0, reader.comm );

   if( local_status > 0 ) {
      // don't return an error if the file exist
      H5Fclose( reader.local_file_id ); 
      return true;
   }

   reader.valCount  = 0;
   reader.attrCount = 0;
   
   for( int d = 0; d < MAX_DIMENSION; ++ d ) {
      reader.offset [d] = 0;
   }
   // Iterate over the memebers of the local file
    
   H5Giterate ( reader.local_file_id, "/", 0, readDataset, &reader );
    
   local_status = H5Fclose( reader.local_file_id );    
  
   MPI_Reduce ( &global_status, &local_status, 1, MPI_INT, MPI_SUM, 0, reader.comm ); 
    
   if( reader.rank == 0 ) {
      local_status = H5Fclose( reader.gloabl_file_id );  
   }
 
   MPI_Bcast( &local_status, 1, MPI_INT, 0, reader.comm );
   
   if( local_status < 0 || global_status < 0 ) {
      return false;
   }
   
   return true;
}


