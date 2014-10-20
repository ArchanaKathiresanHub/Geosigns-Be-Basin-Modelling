#include "mpi.h"
#include "strings.h"
#include "stdlib.h"
#include <sstream>
#include <iostream>

#include "hdf5.h"
#include "fileHandlerReuse.h"

void FileHandlerReuse::openLocalFile( hid_t fileAccessPList ) {

   // only rank 0 needs RDWR access?
   if(  m_rank != 0 ) {
      m_localFileId = H5Fopen( m_fileName.c_str(), H5F_ACC_RDONLY, fileAccessPList );
   } else {
      m_localFileId = H5Fopen( m_fileName.c_str(), H5F_ACC_RDWR, fileAccessPList );
   }
}

void FileHandlerReuse::closeSpaces( ) {

   // filespace and memspace will be used by rank 0
   if( m_rank != 0 ) {
      H5Sclose( m_filespace );
      H5Sclose( m_memspace );
   }
}

void FileHandlerReuse::createGroup( const char* name ) {
   
   if( m_rank == 0 ) {       
      // Do we need to open a group?
      m_groupId = H5P_DEFAULT;
   }
  
}
 
