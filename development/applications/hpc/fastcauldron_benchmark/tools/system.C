#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "system.h"

#include <fstream>
#include <sstream>
#include <string>
#include <cstdio>
#include <vector>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <stdlib.h>
#include <libgen.h>

#include <boost/shared_ptr.hpp>

namespace hpc
{
  const std::string pathSeparator = "/";

  void mkdir(const std::string & directory)
  {
     int r = ::mkdir(directory.c_str(), 0777);

     if (r == -1)
        throw SystemException() << "Could not create directory '" << directory << "', because: " 
           << strerror(errno);
  }

  std::string dirname( const std::string & path)
  {
     std::vector<char> charArray(path.begin(), path.end());
     charArray.push_back('\0');

     return ::dirname( &charArray[0] );
  }

  void copyFile(const std::string & src, const std::string & dst)
  {
     std::ifstream input(src.c_str());

     if (!input)
        throw SystemException() << "Cannot open source file '" << src << "'";

     std::ofstream output(dst.c_str());

     if (!output)
        throw SystemException() << "Cannot open destination location '" << dst << "'";

     char buffer[1<<16];
     while ( !input.eof() )
     {
        input.read(buffer, sizeof(buffer));
        std::streamsize n = input.gcount();
        output.write(buffer, n);
     }

     if ( !output)
        throw SystemException() << "Error while copying file '" << src << "' to '" << dst << "'";
  }

  FileType getFileType(const std::string & fileName)
  {
     struct stat fileInfo;
     int r = stat( fileName.c_str(), &fileInfo);

     if (r==-1)
     {
        switch(errno)
        {
           case ENOENT: return FT_NotExists;
           default: throw SystemException() << "Cannot stat file, because " << strerror(errno);
        }
     }
    
     if (S_ISREG(fileInfo.st_mode))
        return FT_Regular;     
     else if (S_ISDIR(fileInfo.st_mode))
        return FT_Directory;
     else
        return FT_Other;
  }

   std::string canonicalPath(const std::string & path)
   {
      boost::shared_ptr<void> canonPath( canonicalize_file_name( path.c_str() ), &::free );

      if (!canonPath)
         throw SystemException() << "Cannot canonicalize path '" << path << "', because " << strerror(errno);

      return static_cast<const char *>(canonPath.get());
   }

   MPICmdLineTools
      :: MPICmdLineTools()
      : m_root("/nfs/rvl/apps/3rdparty/intel/impi/4.1.0.030/intel64")
   { }

   std::string
   MPICmdLineTools
      :: cpuinfo() const
   {
      return m_root + "/bin/cpuinfo";
   }

   std::string
   MPICmdLineTools
      :: loadEnv() const
   {
      return "source " + m_root + "/bin/mpivars.sh";
   }

   std::string
   MPICmdLineTools
      :: startEnv(const std::string & hostsFile, int numberOfHosts) const
   {
      std::ostringstream command;
      command 
         << m_root
         << "/bin/mpdboot -f " 
         << hostsFile
         << " -n " 
         << numberOfHosts;
      return command.str();
   }

   std::string
   MPICmdLineTools
      :: stopEnv() const
   {
      return m_root + "/bin/mpdallexit";
   }

   
}
