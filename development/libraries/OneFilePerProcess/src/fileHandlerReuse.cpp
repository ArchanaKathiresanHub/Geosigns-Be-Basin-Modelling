#include "mpi.h"
#include "strings.h"
#include "stdlib.h"
#include <sstream>
#include <iostream>

#include "hdf5.h"
#include "fileHandlerReuse.h"

void FileHandlerReuse::openLocalFile( hid_t fileAccessPList ) {

   // only rank 0 needs RDWR access?
   if( getRank() != 0 ) {
      setLocalFileId ( H5Fopen( getFileName(), H5F_ACC_RDONLY, fileAccessPList ));
   } else {
      setLocalFileId ( H5Fopen( getFileName(), H5F_ACC_RDWR, fileAccessPList ));
   }
}

void FileHandlerReuse::closeSpaces( ) {

   // filespace and memspace will be used by rank 0
   if( getRank() != 0 ) {
      H5Sclose( getFilespace() );
      H5Sclose( getMemspace() );
   }
}

void FileHandlerReuse::createGroup( const char* name ) {
   
   if( getRank() == 0 ) {       
      // Do we need to open a group?
      setGroupId ( H5P_DEFAULT );
   }
  
}
 
