#ifndef HPC_FCBENCH_TEST_TEST_PROJECT3D_H
#define HPC_FCBENCH_TEST_TEST_PROJECT3D_H

#include <string>
#include <boost/shared_ptr.hpp>

#include "FormattingException.h"

namespace DataAccess { namespace Interface {
   class ProjectHandle;
   class ObjectFactory;
} }

namespace hpc
{

   class TestProject3DFile
   {
   public:
      TestProject3DFile(const std::string & fileName);
      ~TestProject3DFile();

      struct WriteException : formattingexception :: BaseException< WriteException > {};

      DataAccess::Interface :: ProjectHandle * project() ;


   private:
      void write(const std::string &) ;

      static const char * s_contents;

      std::string m_fileName;
      boost::shared_ptr<DataAccess::Interface::ProjectHandle> m_project;
      boost::shared_ptr<DataAccess::Interface::ObjectFactory> m_factory;
   };

}

#endif
