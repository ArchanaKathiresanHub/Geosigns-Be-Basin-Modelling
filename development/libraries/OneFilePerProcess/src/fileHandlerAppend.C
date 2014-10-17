#include "mpi.h"
#include "strings.h"
#include "stdlib.h"
#include <sstream>
#include <iostream>

#include "hdf5.h"
#include "fileHandlerAppend.h"

FileHandlerAppend::FileHandlerAppend( MPI_Comm comm, const std::string & fileName, const std::string & tempDirName ):
      FileHandler( comm, fileName, tempDirName) { 

   m_update = true;

};  

void FileHandlerAppend::openGlobalFile () {

   if( m_rank == 0 ) {
      // Update existing file
      m_globalFileId = H5Fopen( m_fileName.c_str(), H5F_ACC_RDWR, H5P_DEFAULT ); 
      //If it doesn't exist create one
      if(  m_globalFileId  < 0 ) {
         m_globalFileId = H5Fcreate( m_fileName.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT );
      }
   }
}

//  Open or create a group in the global file
void FileHandlerAppend::createGroup( const char* name ) {

   if( m_rank == 0 ) {       
      // Open existing group for update
      m_groupId = H5Gopen( m_globalFileId, name, H5P_DEFAULT ); 
      // create a group if it doesn't exist
      if( m_groupId < 0 ) { 
         m_groupId = H5Gcreate( m_globalFileId, name,  H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT ); 
         // Unset update flag because the file is created
         m_update = false;
      } 
      if( m_groupId < 0 ) {
         std::cout << " ERROR Cannot open or create the group " << name << std::endl;
      }
   }
}
// Writes attributes into created global file.
void FileHandlerAppend::writeAttributes( ) {

   if(! m_update ) {
      bool status = readAttributes( m_local_dset_id, m_global_dset_id ); 
      if( status < 0 ) {
         std::cout << " ERROR Cannot write attributes " << std::endl;
      }

   }
}
// Create or open a dataset in global file
void FileHandlerAppend::createDataset( const char* name , hid_t dtype ) {

   if( m_rank == 0 ) {
 
      // Write the global data into the global file
      m_memspace = H5Screate_simple( m_spatialDimension, m_count, NULL ); 
      
      if( m_groupId != H5P_DEFAULT ) {
         // Dataset is under the sub-group
         m_global_dset_id = H5Dopen ( m_groupId, name, H5P_DEFAULT );
         if( m_global_dset_id < 0 ) {
            m_global_dset_id = H5Dcreate( m_groupId, name, dtype, m_memspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
         }
      } else {
         // Dataset is under the main group
         m_global_dset_id = H5Dopen ( m_globalFileId, name, H5P_DEFAULT ); 
         if( m_global_dset_id < 0 ) {
            m_global_dset_id = H5Dcreate( m_globalFileId, name, dtype, m_memspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
         }
      }
      
      if( m_global_dset_id < 0 ) {
         std::cout << " ERROR Cannot create the dataset " << name << std::endl;
      }
      
      m_filespace = H5Dget_space( m_global_dset_id );
      H5Sselect_hyperslab( m_filespace, H5S_SELECT_SET, m_offset, NULL,m_count, NULL );
   }
}
