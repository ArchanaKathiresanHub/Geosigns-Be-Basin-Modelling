#ifndef ONEFILEPERPROCESS_FILEHANDLER_REUSE_H
#define ONEFILEPERPROCESS_FILEHANDLER_REUSE_H

#include <string>
#include <vector>
#include "fileHandler.h"

class FileHandlerReuse : public FileHandler {

public:  
   FileHandlerReuse( MPI_Comm comm, const std::string & fileName, const std::string & tempDirName ) :
      FileHandler( comm, fileName, tempDirName) {} ;  

   virtual ~FileHandlerReuse() {};

   void  openGlobalFile( );
   void  openLocalFile( hid_t fileAccessPList );
   void  createGroup( const char* name );
   hid_t closeGlobalFile();
   void  closeSpaces();
   void  closeGlobalDset() {};
   void  writeAttributes() {};
   void  createDataset( const char* name , hid_t dtype );
};

inline hid_t FileHandlerReuse::closeGlobalFile() {
   return 0;

}
inline void FileHandlerReuse::openGlobalFile () {

   if( getRank() == 0 ) {
      setGlobalFileId ( getLocalFileId()); 
   }
}
inline void FileHandlerReuse::createDataset( const char* name , hid_t dtype ) {

    if( getRank() == 0 ) {
       setGlobalDsetId( getLocalDsetId());
    }
}


#endif
