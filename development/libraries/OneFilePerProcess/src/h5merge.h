#ifndef ONEFILEPERPROCESS_H5MERGE_H
#define ONEFILEPERPROCESS_H5MERGE_H

#include <string>
#include "mpi.h"

class FileHandler;

enum mergeOption { CREATE, REUSE, APPEND, UNKNOWN };

bool copyFile( std::string & dstPath, std::string & currentPath );
bool mergeFiles( FileHandler * aFileHandler ) ;//MPI_Comm comm, const std::string& fileName, const std::string &tempDirName, const bool reuse );
bool appendFiles( MPI_Comm comm, const std::string & fileName, const std::string & tempDirName );
FileHandler * allocateFileHandler ( MPI_Comm comm, const std::string & fileName, const std::string & tempDirName, mergeOption anOption );

#endif
