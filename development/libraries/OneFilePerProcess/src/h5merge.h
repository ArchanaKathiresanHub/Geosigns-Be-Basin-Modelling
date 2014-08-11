#ifndef ONEFILEPERPROCESS_H5MERGE_H
#define ONEFILEPERPROCESS_H5MERGE_H

#include <string>

bool copyFile( std::string & dstPath, std::string & currentPath );
bool mergeFiles( MPI_Comm comm, const std::string& fileName, const std::string &tempDirName, const bool reuse );

#endif
