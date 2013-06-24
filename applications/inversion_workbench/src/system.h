#ifndef INVERSION_SYSTEM_H
#define INVERSION_SYSTEM_H

#include <string>

#include "formattingexception.h"

struct SystemException : formattingexception :: BaseException< SystemException > {};

// returns a string describing the contents in human readable form
std::string describePath( const std::string & path);

// returns true iff path exists and is directory
bool directoryExists( const std::string & path );

// returns true iff directory is empty. If it is not a directory, an exception will be raised.
bool directoryIsEmpty( const std::string & path );

// returns the path of the directory that contains the given path. It is not necessary that the given
// path exists.
std::string getParentDirectory(const std::string & path);

// copy file from source to target. It is an exception when the source does not exist or when the target
// does exist. Directories are not allowed as source or target
void copyFile(const std::string & sourceFile, const std::string & targetFile);

#endif
