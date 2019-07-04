#ifndef HPC_FCBENCH_FASTCAULDRONENVIRONMENT_H
#define HPC_FCBENCH_FASTCAULDRONENVIRONMENT_H

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <iosfwd>

#include "FormattingException.h"

namespace DataAccess { namespace Interface {
   class ProjectHandle;
   class ObjectFactory;
} }

namespace hpc
{

class Project3DParameter;
class CmdLineParameter;
class Path;

/// A 'FastCauldronEnvironment' object knows how to apply parameters to a project file and the command line 
/// and how to start fastcauldron.
class FastCauldronEnvironment 
{
public:
   typedef std::string VersionID;

   struct Exception : formattingexception :: BaseException< Exception > {};

   /// To create a FastcauldronEnvironment object, a configuration file 
   /// has to be read first by constructing a Configuration object. 
   /// The configuration file has a specific layout, e.g.:
   /// 
   /// [2012.1008]
   /// mpiexec -np {PROCS} {MPI_PARAMS} fastcauldron -v2012.1008 -project {INPUT} -save {OUTPUT} {FC_PARAMS}
   ///
   /// [BUILD]
   /// mpiexec -np {PROCS} {MPI_PARAMS} ../../fastcauldron/fastcauldron -project {INPUT} -save {OUTPUT} {FC_PARAMS}
   ///
   /// ...
   ///
   /// The entries which are bounded by right-angled brackets [] mark the
   /// beginning of the script template for a specific version. It can be any
   /// string, although it can't contain an angled bracket and it should fit
   /// on one line.
   ///
   /// The parameters between curly braces {} are replaced with generated
   /// values:
   /// {PROCS}      - Number of processors
   /// {MPI_PARAMS} - Extra parameters to mpirun
   /// {INPUT}      - The input project file name
   /// {OUTPUT}     - The output project file name
   /// {FC_PARAMS}  - Extra parameters to fastcauldron
   ///
   class Configuration
   { 
   public:
      explicit Configuration(const Path & configFile);
      explicit Configuration( std::istream & configFile );

      /// Returns the template script for a specific version
      std::string getRunTemplate( const VersionID & version) const;

      /// Returns the template script for a specific version and replaces the placeholders/markers
      /// with their values.
      std::string getRunScript( const VersionID & version, 
            int numberOfProcessors, const std::vector<std::string> & mpiCmdLineParams,
            const std::string & inputProject, const std::string & outputProject,
            const std::vector<std::string> & fcCmdLineParams
            ) const ;

   private:
      /// reads the configuration file
      void readTemplates(std::istream & );

      std::map< VersionID, std::string > m_runTemplates;
   };


   FastCauldronEnvironment(const Configuration & configuration, const std::string & id, const Path & projectFile, int processors, const VersionID & version);

   void applyProjectParameter( const Project3DParameter & param, const std::string & value);
   void applyMpiCmdLineParameter( const CmdLineParameter & param, const std::vector<std::string> & values);
   void applyCauldronCmdLineParameter( const CmdLineParameter & param, const std::vector<std::string> & values);

   // outputs the fastcauldron environment (command line params, project settings) to the file system
   // while returning true iff any changes were made. Typically, no changes will be made if the
   // job has been ran before.
   bool generateJob( const Path & directory);

   // returns which script has to be run where (working directory) to do the job
   static void commandToRunJob(const Path & directory,
         const std::string & id, 
         std::shared_ptr<Path> & workingDir, 
         std::string & command
         );

   // returns whether the job has been ran before
   static bool jobHasRanBefore(const Path & directory, const std::string & id);

private:
   FastCauldronEnvironment( const FastCauldronEnvironment & ); // copying prohibited
   FastCauldronEnvironment & operator=(const FastCauldronEnvironment & ); // assignment prohibited

   static std::ostream & hostInformationScript( std::ostream & output);


   Configuration m_configuration;
   std::string m_id;
   int m_processors;
   std::vector< std::string > m_mpiCmdLine;
   std::vector< std::string > m_cauldronCmdLine;
   std::shared_ptr<DataAccess::Interface::ProjectHandle>  m_project;
   std::shared_ptr<Path> m_projectSourceDir;
   std::shared_ptr<DataAccess::Interface::ObjectFactory>  m_factory;   
   VersionID m_version;
};  

}

#endif
