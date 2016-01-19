#ifndef HPC_FCBENCH_SYSTEM_H
#define HPC_FCBENCH_SYSTEM_H

#include "FormattingException.h"

#include <boost/shared_ptr.hpp>

namespace hpc
{  
   /// A Wrapper around system calls. All methods are virtual, so that they can be mocked in Unit Tests.
   class Path
   {
   public:
      struct Exception : formattingexception::BaseException<Exception> {};

      Path( const std::string & path);
      boost::shared_ptr<Path> clone() const;

      virtual ~Path() {}

      // the path separator
      virtual const std::string & pathSeparator() const;
      
      /// Make a directory
      virtual void makeDirectory() const ;

      /// Returns the path of the parent directory
      virtual boost::shared_ptr<Path> getParentDirectory() const;

      /// Return a child of the current directory
      virtual boost::shared_ptr<Path> getDirectoryEntry( const std::string & file ) const;

      // Copy a file
      virtual void copyTo(const Path & dst) const ;
   
      enum FileType { Regular, Directory, Other, NotExists };
      // Returns the type (regular, directory, other, non-existant) of the file
      virtual FileType getFileType() const;

      /// Returns the canonical form of the path. Note: the file must exist 
      virtual std::string getCanonicalPath() const ;

      /// Return the path
      virtual std::string getPath() const;

      /// Start writing a file
      virtual boost::shared_ptr<std::ostream> writeFile() const; 

      /// Read a file
      virtual boost::shared_ptr<std::istream> readFile() const; 

   private:
      virtual Path * doClone() const;

      std::string m_path;
   };

   std::ostream & operator<<( std::ostream & output, const Path & path);

}

#endif
