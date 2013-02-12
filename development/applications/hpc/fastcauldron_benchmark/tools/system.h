#ifndef HPC_FCBENCH_SYSTEM_H
#define HPC_FCBENCH_SYSTEM_H

#include "formattingexception.h"

namespace hpc
{  // wrapper around system calls

   struct SystemException : formattingexception::BaseException<SystemException> {};

   extern const std::string pathSeparator;

   void mkdir(const std::string & directory);
   std::string dirname( const std::string & path);

   void copyFile(const std::string & src, const std::string & dest);

   enum FileType { FT_Regular, FT_Directory, FT_Other, FT_NotExists };
   FileType getFileType( const std::string & fileName);

   std::string canonicalPath(const std::string & path);


   // Access Intel MPI Command line Tools
   class MPICmdLineTools
   {
   public:
      MPICmdLineTools();

      std::string cpuinfo() const;
      std::string loadEnv() const;
      std::string startEnv(const std::string & hostsFile, int numberOfHosts ) const;
      std::string stopEnv() const;

   private:
      std::string m_root;
   };

 }

#endif
