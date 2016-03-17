#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "system.h"

#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
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
   Path
      :: Path(const std::string & path)
      : m_path(path)
   {}        

   boost::shared_ptr<Path>
   Path
      :: clone() const
   {
      return boost::shared_ptr<Path>( this->doClone() );
   }


   const std::string & 
   Path
      :: pathSeparator() const
   {
#ifdef _WIN32
      static std::string pathSeparator = "\\";
#else
      static std::string pathSeparator = "/";
#endif
      return pathSeparator;
   }

   
   void 
   Path
      :: makeDirectory() const
   {
      int r = ::mkdir(m_path.c_str(), 0777);
 
      if (r == -1)
         throw Exception() << "Could not create directory '" << m_path << "', because: " 
            << strerror(errno);
   }

   boost::shared_ptr<Path>
   Path    
      :: getParentDirectory() const
   {
      // dirname  (3) needs a writeable character buffer, so therefore copy it
      std::vector<char> charArray(m_path.begin(), m_path.end());
      charArray.push_back('\0');

      // and return the result
      return boost::shared_ptr<Path>(new Path(::dirname( &charArray[0] )));
   }

   boost::shared_ptr<Path>
   Path
      :: getDirectoryEntry( const std::string & file) const
   {
      return boost::shared_ptr<Path>(new Path( m_path + pathSeparator() + file));
   }
   

   void 
   Path
      :: copyTo(const Path & dst) const
   {
      std::ifstream input(m_path.c_str());

      if (!input)
         throw Exception() << "Cannot open source file '" << m_path << "'";

      std::ofstream output(dst.m_path.c_str());

      if (!output)
         throw Exception() << "Cannot open destination file '" << dst << "'";

      char buffer[1<<16];
      while ( !input.eof() )
      {
         input.read(buffer, sizeof(buffer));
         std::streamsize n = input.gcount();
         output.write(buffer, n);
      }

      if ( !output)
         throw Exception() << "Error while copying file '" << m_path << "' to '" << dst.m_path << "'";
   }

   Path :: FileType 
   Path 
      :: getFileType() const
   {
      struct stat fileInfo;
      int r = stat( m_path.c_str(), &fileInfo);

      if (r==-1)
      {
         switch(errno)
         {
            case ENOENT: return NotExists;
            default: throw Exception() << "Cannot stat file '" << m_path << "', because " << strerror(errno);
         }
      }
    
      if (S_ISREG(fileInfo.st_mode))
         return Regular;     
      else if (S_ISDIR(fileInfo.st_mode))
         return Directory;
      else
         return Other;
   }

   std::string 
   Path
      :: getCanonicalPath() const
   {
      boost::shared_ptr<void> canonPath( canonicalize_file_name( m_path.c_str() ), &::free );

      if (!canonPath)
         throw Exception() << "Cannot canonicalize path '" << m_path << "', because " << strerror(errno);

      return static_cast<const char *>(canonPath.get());
   }

   std::string
   Path
      :: getPath() const
   {
      return m_path;
   }


   boost::shared_ptr<std::ostream>
   Path
      :: writeFile() const
   {
      boost::shared_ptr<std::ostream> outputFile( new std::ofstream( m_path.c_str() ));
      if (outputFile->fail())
         throw Exception() << "Cannot open file '" << m_path << "' for output";

      return outputFile;
   }

   boost::shared_ptr<std::istream>
   Path
      :: readFile() const
   {
      boost::shared_ptr<std::istream> inputFile( new std::ifstream( m_path.c_str() ));

      if (inputFile->fail())
         throw Exception() << "Cannot open file '" << m_path << "' for input";

      return inputFile;
   }

   Path *
   Path
      :: doClone() const
   {
      return new Path(*this);
   }

   std::ostream & operator<<( std::ostream & output, const Path & path)
   {
      return output << path.getPath();
   }
}
