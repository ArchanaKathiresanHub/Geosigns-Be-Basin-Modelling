#ifndef ONEFILEPERPROCESS_H5MERGE_H
#define ONEFILEPERPROCESS_H5MERGE_H

#include <string>
#include "mpi.h"

class FileHandler;

enum MergeOption { CREATE, REUSE, APPEND, UNKNOWN };

bool copyFile( const std::string & dstPath, const std::string & currentPath );

bool mergeFiles( FileHandler * aFileHandler, const bool appendRank = true ) ;

/// \brief Interface to allocate file handler
FileHandler * allocateFileHandler ( MPI_Comm comm, const std::string & fileName, const std::string & tempDirName, MergeOption anOption );

#endif
