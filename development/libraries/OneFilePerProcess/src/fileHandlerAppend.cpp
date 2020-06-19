#include "mpi.h"
#include "strings.h"
#include "stdlib.h"
#include <sstream>
#include <iostream>

#include "hdf5.h"
#include "fileHandlerAppend.h"

FileHandlerAppend::FileHandlerAppend( MPI_Comm comm, const std::string & fileName, const std::string & tempDirName ):
      FileHandler( comm, fileName, tempDirName) {
};

void FileHandlerAppend::openGlobalFile () {

   if( getRank() == 0 ) {
      // Update existing file
      setGlobalFileId ( H5Fopen( getFileName(), H5F_ACC_RDWR, H5P_DEFAULT ));
      //If it doesn't exist create one
      if(  getGlobalFileId()  < 0 ) {
         setGlobalFileId ( H5Fcreate( getFileName(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT ));
      }
   }
}

//  Open or create a group in the global file
void FileHandlerAppend::createGroup( const char* name ) {

   if( getRank() == 0 ) {
      // Open existing group for update
      setGroupId( H5Gopen( getGlobalFileId(), name, H5P_DEFAULT ));
      // create a group if it doesn't exist
      if( getGroupId() < 0 ) {
         setGroupId ( H5Gcreate( getGlobalFileId(), name,  H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT ));
         // Unset update flag because the file is created
         m_update = false;
      }
      if( getGroupId() < 0 ) {
         std::cout << " ERROR Cannot open or create the group " << name << std::endl;
      }
   }
}
// Writes attributes into created global file.
void FileHandlerAppend::writeAttributes( ) {

   if(! m_update ) {
      herr_t status = readAttributes( getLocalDsetId(), getGlobalDsetId() );
      if( status < 0 ) {
         std::cout << " ERROR Cannot write attributes " << std::endl;
      }

   }
}
// Create or open a dataset in global file
void FileHandlerAppend::createDataset( const char* name , hid_t dtype ) {

   if( getRank() == 0 ) {

      // Write the global data into the global file
      setMemspace( H5Screate_simple( getSpatialDimension(), m_count, NULL ));

      if( getGroupId() != H5P_DEFAULT ) {
         // Dataset is under the sub-group
         setGlobalDsetId( H5Dopen ( getGroupId(), name, H5P_DEFAULT ));
         if( getGlobalDsetId() < 0 ) {
            setGlobalDsetId( H5Dcreate( getGroupId(), name, dtype, getMemspace(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT ));
         }
      } else {
         // Dataset is under the main group
         setGlobalDsetId ( H5Dopen ( getGlobalFileId(), name, H5P_DEFAULT ));
         if( getGlobalDsetId() < 0 ) {
            setGlobalDsetId( H5Dcreate( getGlobalFileId(), name, dtype, getMemspace(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT ));
         }
      }

      if( getGlobalDsetId() < 0 ) {
         std::cout << " ERROR Cannot create the dataset " << name << std::endl;
      }

      setFilespace ( H5Dget_space( getGlobalDsetId() ));
      H5Sselect_hyperslab( getFilespace(), H5S_SELECT_SET, m_offset, NULL, m_count, NULL );
   }
}
