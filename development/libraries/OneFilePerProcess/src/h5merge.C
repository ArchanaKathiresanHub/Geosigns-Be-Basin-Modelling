
#include "mpi.h"
#include "h5merge.h"
#include "strings.h"
#include "stdlib.h"
#include <vector>
#include <sstream>

#include "hdf5.h"
#include "H5FDmpio.h"
#include "HDF5VirtualFileDriver.h"

//#define SAFE_RUN 1

FileHandler:: FileHandler ( MPI_Comm comm ) {

   m_comm = comm;
   MPI_Comm_rank( comm, &m_rank );

   m_valCount  = 0;
   m_attrCount = 0;

   m_groupId   = H5P_DEFAULT;
   m_localFileId = H5P_DEFAULT;
   m_globalFileId = H5P_DEFAULT;

   for( int d = 0; d < MAX_FILE_DIMENSION; ++ d ) {
      m_offset [d] = 0;
      m_count  [d] = 0;
      m_dimensions [d] = 0;
   }
   m_spatialDimension = 0;
 
}

herr_t FileHandler::reallocateBuffers ( ssize_t dataSize ) {

   if( dataSize >= 0 ) {
      m_valCount = dataSize;
      
      for (int d = 0; d < m_spatialDimension; ++d) {
         m_count [d] = m_dimensions[d];       
         m_valCount *= m_count[d];
      } 
      m_data.resize( m_valCount );
      m_sumData.resize( m_valCount );
   }
   return 0;
}

herr_t FileHandler::readAttributes( hid_t localDataSetId, hid_t globalDataSetId )  {

   hid_t status;
   // Get the number of attributes for the dataSet.
   int attrNumber = H5Aget_num_attrs(localDataSetId);
   
   hsize_t spatialDims [MAX_FILE_DIMENSION];
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
         
         H5Sclose( dataSpace );
         //  Get the size of the type
         ssize_t dataSize = H5Tget_size(dataTypeId);
         
         m_attrCount = dataSize * attrSize;
         m_attrData.resize( m_attrCount );
         
         status = H5Aread( localAttrId, dataTypeId, m_attrData.data() );

         if( status < 0 ) {
            H5Tclose( dataTypeId );
            H5Aclose( localAttrId );

            return -1;
         }
         // write Attribute
         hid_t space = H5Screate_simple ( dims, spatialDims, NULL );
         
         hid_t globalAttrId = H5Acreate( globalDataSetId, attrName, dataTypeId, space, H5P_DEFAULT, H5P_DEFAULT );
         
         status = H5Awrite ( globalAttrId, dataTypeId, m_attrData.data() );
         
         H5Tclose( dataTypeId );
         H5Aclose( localAttrId );
         H5Aclose( globalAttrId );
         H5Sclose( space );

         if( status < 0 ) {
            return -1;
         }
      } 
   } 
   return 0;
}

herr_t readDataset ( hid_t groupId, const char* name, void * voidReader)  {

   if( groupId == NULL ) return -1;

   FileHandler* reader = (FileHandler*)( voidReader );

   hid_t memspace, filespace;
   hid_t local_dset_id;
   hid_t dataSpaceId;
   int   status = 0;
 
   H5G_stat_t statbuf;

   status = H5Gget_objinfo ( groupId, name, 0, &statbuf );

   if( reader->checkError( status ) < 0 ) {
      return -1;
   }

   switch ( statbuf.type ) {
   case H5G_GROUP:
      {
         std::stringstream groupName;
         groupName << "/" << name;

         if( reader->m_rank == 0 ) {       
            // Greate a group in the global file
            reader->m_groupId = H5Gcreate( reader->m_globalFileId, name,  H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT ); 
         }
         // Iterate over the members of the group
         H5Giterate ( reader-> m_localFileId, groupName.str().c_str(), 0, readDataset, voidReader );

         if( reader->m_rank == 0 ) {       
            H5Gclose( reader->m_groupId );
         }
         return 0;
      }
      break;
   case H5G_DATASET:
      break;
   case H5G_TYPE:
      break;
   default: ;

   } 

   local_dset_id = H5Dopen ( groupId, name, H5P_DEFAULT );

#ifdef SAFE_RUN
   if( reader->checkError ( local_dset_id ) < 0 ) {
      return -1;
   }
#endif

   dataSpaceId = H5Dget_space ( local_dset_id );

   reader->m_spatialDimension = H5Sget_simple_extent_dims ( dataSpaceId, reader->m_dimensions, 0 );

   H5Sclose( dataSpaceId );
 
   // Get the dataset datatype
   hid_t   dtype    = H5Dget_type( local_dset_id );
   ssize_t dataSize = H5Tget_size(dtype);
 
#ifdef SAFE_RUN
   if( reader->checkError ( dataSize ) < 0 || dataSize < 0 ) {
      return -1;
   }
#endif

    // Allocate data buffers
   reader->reallocateBuffers ( dataSize );

   // Read the local data
   memspace = H5Screate_simple( reader->m_spatialDimension, reader->m_count, NULL ); 
   filespace = H5Dget_space( local_dset_id );

   H5Sselect_hyperslab( filespace, H5S_SELECT_SET, reader->m_offset, NULL, reader->m_count, NULL );

   H5Dread ( local_dset_id, dtype, memspace, filespace, H5P_DEFAULT, reader->m_data.data() );

   H5Sclose( filespace );
   H5Sclose( memspace );

   // Summarize all local data in a global array
   if( reader->m_spatialDimension == 1 ) {
      if( reader->m_rank == 0 ) {
         reader->m_sumData = reader->m_data;
      }
   } else {
      MPI_Reduce( reader->m_data.data(), reader->m_sumData.data(), reader->m_valCount, MPI_CHAR, MPI_SUM, 0, reader->m_comm );
   }

   if( reader->m_rank == 0 ) {
      hid_t global_dset_id;
      // Write the global data into the global file
      memspace = H5Screate_simple( reader->m_spatialDimension, reader->m_count, NULL ); 
      if( reader->m_groupId != H5P_DEFAULT ) {
         // Dataset is under the sub-group
         global_dset_id = H5Dcreate( reader->m_groupId, name, dtype, memspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
      } else {
         // Dataset is under the main group
         global_dset_id = H5Dcreate( reader->m_globalFileId, name, dtype, memspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
      }
      filespace = H5Dget_space( global_dset_id );
      H5Sselect_hyperslab( filespace, H5S_SELECT_SET, reader->m_offset, NULL, reader->m_count, NULL );
 
      status = H5Dwrite( global_dset_id, dtype, memspace, filespace, H5P_DEFAULT, reader->m_sumData.data() );

      if( status >= 0 ) {
         status = reader->readAttributes( local_dset_id, global_dset_id ); 
      }

      H5Sclose( filespace );
      H5Sclose( memspace );
      H5Dclose( global_dset_id );
   }
 
   H5Dclose( local_dset_id );

#ifdef SAFE_RUN
   MPI_Bcast( &status, 1, MPI_INT, 0, reader->m_comm );
   
   if( status < 0 ) {
      return -1;
   }
#endif
   
   
   return 0;
}

bool mergeFiles( MPI_Comm comm, const std::string &fileName ) {
 
   FileHandler reader ( comm );
 
   hid_t status = 0, close_status = 0, iteration_status = 0;
 
   hid_t fileAccessPList = H5Pcreate(H5P_FILE_ACCESS);

   H5Pset_fapl_ofpp ( fileAccessPList, comm, "{NAME}_{MPI_RANK}", 0 );  
   
   reader.m_localFileId = H5Fopen( fileName.c_str(), H5F_ACC_RDONLY, fileAccessPList );

   H5Pclose( fileAccessPList );
   
   if( reader.checkError( reader.m_localFileId ) < 0 ) {
      return false;
   }
  
   if( reader.m_rank == 0 ) {
      // What to use? H5F_ACC_EXCL or H5F_ACC_TRUNC?
      // HDF5 is not able to detect if the file with the same name is already open by H5Fopen when uses H5F_ACC_TRUNC.
      // If the VFD failed to output "one file per processor", then a file with the same name will be created at this point.
      // This should not happen.
      
      reader.m_globalFileId = H5Fcreate( fileName.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT ); 

      if( reader.m_globalFileId < 0 ) {
         status = 1; 
      }
   }
   
   MPI_Bcast( &status, 1, MPI_INT, 0, comm );

   if( status > 0 ) {
      // global file cannot be created
      H5Fclose( reader.m_localFileId ); 
      return false;
   }
   // Iterate over the memebers of the local file
    
   iteration_status = H5Giterate ( reader.m_localFileId, "/", 0, readDataset, &reader );
    
   status = H5Fclose( reader.m_localFileId );    
  
   close_status = reader.checkError( status ); 
    
   if( reader.m_rank == 0 ) {
      status = H5Fclose( reader.m_globalFileId );  
   }
 
   MPI_Bcast( &status, 1, MPI_INT, 0, comm );
   
   if( status < 0 || close_status < 0 || iteration_status < 0 ) {
      return false;
   }
   
   return true;
}

void FileHandler::setGlobalId( hid_t id ) {
   m_globalFileId = id;

}

void FileHandler::setSpatialDimension( int dimension ) {
   m_spatialDimension = dimension;

}

int FileHandler::checkError ( hid_t value ) {
   
   int local_status  = 0;
   int global_status = 0;
   
   if( value < 0 ) {
      local_status = 1;
   }
   MPI_Allreduce ( &local_status, &global_status,  1, MPI_INT, MPI_SUM, m_comm ); 

   if( global_status > 0 ) {
      return -1;
   }
 
   return 0;

}
