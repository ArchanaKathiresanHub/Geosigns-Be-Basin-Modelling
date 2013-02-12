#ifndef HPC_FCBENCH_FASTCAULDRONENVIRONMENT_H
#define HPC_FCBENCH_FASTCAULDRONENVIRONMENT_H

#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include <iosfwd>

#include "formattingexception.h"

namespace DataAccess { namespace Interface {
   class ProjectHandle;
} }

namespace hpc
{

class Project3DParameter;
class CmdLineParameter;

class FastCauldronEnvironment 
{
public:
   FastCauldronEnvironment(const std::string & id, const std::string & projectFile, int processors);

   struct Exception : formattingexception::BaseException< Exception > {};

   void applyProjectParameter( const Project3DParameter & param, const std::string & value);
   void applyMpiCmdLineParameter( const CmdLineParameter & param, const std::vector<std::string> & values);
   void applyCauldronCmdLineParameter( const CmdLineParameter & param, const std::vector<std::string> & values);

   // outputs the fastcauldron environment (command line params, project settings) to the file system
   // while returning true iff any changes were made. Typically, no changes will be made if the
   // job has been ran before.
   bool generateJob(const std::string & directory);

   // returns which script has to be run where (working directory) to do the job
   static void commandToRunJob(const std::string & directory, 
         const std::string & id, 
         std::string & workingDir, 
         std::string & command
         );

   // returns whether the job has been ran before
   static bool jobHasRanBefore(const std::string & directory, const std::string & id);

private:
   FastCauldronEnvironment( const FastCauldronEnvironment & ); // copying prohibited
   FastCauldronEnvironment & operator=(const FastCauldronEnvironment & ); // assignment prohibited

   static std::ostream & hostInformationScript( std::ostream & output);

   std::string m_id;
   int m_processors;
   std::vector< std::string > m_mpiCmdLine;
   std::vector< std::string > m_cauldronCmdLine;
   boost::shared_ptr<DataAccess::Interface::ProjectHandle>  m_project;
   std::string m_projectSourceDir;
};  

}

#endif
