#include "system.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>

#include <boost/shared_ptr.hpp>

bool directoryExists( const std::string & path )
{
   struct stat s;
   int r = stat(path.c_str(), &s);

   if (r==0 && S_ISDIR( s.st_mode ))
      return true;
   else
      return false;
}

bool directoryIsEmpty( const std::string & path )
{
   static const std::string currentDir = ".";
   static const std::string parentDir = "..";

   boost::shared_ptr<DIR> dir( opendir(path.c_str()), closedir );
   
   if (!dir)
      throw SystemException() << "Cannot open directory '" << path 
         << "', because: " << std::strerror(errno);

   while (struct dirent * d = readdir(dir.get()))
      if ( d->d_name != currentDir && d->d_name != parentDir)
         return false;

   return true;
}

std::string describePath( const std::string & path)
{
  struct stat s;
  int r = stat(path.c_str(), &s);

  if (r == -1)
  {
     if ( errno == ENOENT || errno == ENOTDIR ) 
        return "does not exist";
     else if (errno == EACCES)
        return "requires more access permissions";
     else if (errno == ELOOP)
        return "requires traversal of too many symbolic links";
     else if (errno == ENAMETOOLONG)
        return "has file name that is too long";
     else
        throw SystemException() << "Cannot 'stat' path '" << path << "', because: " << std::strerror(errno);
  }
  else
  {
     if (S_ISREG(s.st_mode))
        return "is a regular file";
     else if (S_ISDIR(s.st_mode))
        return "is a directory";
     else 
        return "is a special file";
  }
}

std::string getParentDirectory( const std::string & path )
{
   const std::string::size_type lastSlash = path.rfind("/");

   if ( lastSlash == path.size() )
   { // the path ends with a '/'
      return getParentDirectory(path.substr(0, path.size() - 1));
   }
   else if ( lastSlash == std::string::npos )
   { // the path does not contain a '/'
      return ".";
   }
   else
   { // return everything up to (exclusive) the last '/'
     return path.substr(0, lastSlash); 
   }
}

void copyFile(const std::string & sourceFile, const std::string & targetFile)
{
   std::ifstream input(sourceFile.c_str());

   if (!input)
      throw SystemException() << "Cannot open source file '" << sourceFile << "'";

   std::ofstream output(targetFile.c_str());

   if (!output)
      throw SystemException() << "Cannot open destination location '" << targetFile << "'";

   char buffer[1<<16];
   while ( !input.eof() )
   {
      input.read(buffer, sizeof(buffer));
      std::streamsize n = input.gcount();
      output.write(buffer, n);
   }

   if ( !output)
      throw SystemException() << "Error while copying file '" 
         << sourceFile << "' to '" << targetFile << "'";
}
