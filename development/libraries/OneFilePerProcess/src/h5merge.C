
#include "hdf5.h"

#include "h5merge.h"
#include "fileHandler.h"
#include "fileHandlerReuse.h"
#include "fileHandlerAppend.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

bool copyFile( const std::string & dstPath, const std::string & curPath )    
{
   bool status = true;
   const size_t bufSize = 8 * 1024 * 1024;
   char buf[ bufSize ];
   size_t size;
   
   int source = open( curPath.c_str(), O_RDONLY, 0 );
   int dest = open( dstPath.c_str(),  O_CREAT | O_WRONLY | O_TRUNC, 0644 );
   
   if( source < 0 || dest < 0 ) {
      perror(" CopyFile: ");

      status = false;
   }
   if( status ) {
      while (( size = read(source, buf, bufSize )) > 0) {
         if( write(dest, buf, size) < 0 ) {
            perror(" CopyFile: ");
            status = false;
            break;  
         }
      }
   }
   
   close(source);
   close(dest);

   return status;
}


bool mergeFiles ( FileHandler * aFileHandler ) {

   if( aFileHandler != NULL ) {
      bool status = aFileHandler->mergeFiles();
      
      delete aFileHandler;
      return status;
   } 
   return false;

}

FileHandler * allocateFileHandler ( MPI_Comm comm, const std::string & fileName, const std::string & tempDirName, MergeOption anOption ) {
   
   if( anOption == REUSE ) return new FileHandlerReuse( comm, fileName, tempDirName );
   else if( anOption == APPEND ) return new FileHandlerAppend( comm, fileName, tempDirName );
   else return new FileHandler( comm, fileName, tempDirName );

}
