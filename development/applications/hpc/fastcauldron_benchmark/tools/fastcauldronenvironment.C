#include "fastcauldronenvironment.h"
#include "cmdlineparameter.h"
#include "project3dparameter.h"
#include "projectdependencies.h"
#include "system.h"
#include "FastCauldronEnvironmentConfigurationTokenizer.h"

#include <iostream>
#include <fstream>

#include "ProjectFileHandler.h"

#include "ProjectHandle.h"
#include "ObjectFactory.h"

namespace hpc
{

FastCauldronEnvironment :: Configuration
   :: Configuration( const Path & configFile)
   : m_runTemplates()
{
   readTemplates( * configFile.readFile() );
}

FastCauldronEnvironment :: Configuration
   :: Configuration( std::istream & configFile )
   : m_runTemplates()
{
   readTemplates( configFile );
}

void
FastCauldronEnvironment :: Configuration
   :: readTemplates( std::istream & input )
{
   const std::string sepMarkerLeft = "[";
   const std::string sepMarkerRight = "]";

   std::string line, script;
   VersionID   version;

   // read the configuration file line by line
   while ( getline(input, line) )
   {
      std::string::size_type left = line.find( sepMarkerLeft );
      std::string::size_type right = line.rfind( sepMarkerRight );

      // if the line is a separator, like "[2012.1008]", then a new template script starts
      if (left != std::string::npos && right != std::string::npos && left == 0 && left < right)
      {
         if (!version.empty())
         {
            m_runTemplates[version] = script;
         }

         script.clear();
         version = line.substr( left + sepMarkerLeft.size(), right - left - sepMarkerRight.size() );
      }
      else // else the current script should be continued
      {
         script.append( line );
         script.push_back( '\n' );
      }
   }

   // add the final script
   if (!version.empty())
      m_runTemplates[version] = script;
}

std::string
FastCauldronEnvironment :: Configuration
   :: getRunTemplate( const VersionID & version) const
{
   typedef std::map< VersionID, std::string > :: const_iterator It;
   It entry = m_runTemplates.find( version );

   if (entry == m_runTemplates.end())
      throw Exception() << "Could not find fastcauldron template run-script for version '" << version << "'";

   return entry->second;
}


std::string
FastCauldronEnvironment :: Configuration
   :: getRunScript( const VersionID & version, int numberOfProcessors,
         const std::vector< std::string > & mpiCmdLineParams,
         const std::string & inputProject, const std::string & outputProject,
         const std::vector< std::string > & fcCmdLineParams) const
{
   std::ostringstream result;

   std::string token, marker;
   FastCauldronEnvironmentConfigurationTokenizer tokens(getRunTemplate(version));

   while ( tokens.hasMore())
   {
      tokens.next( token, marker );
      result << token;

      if (marker.empty())
      {
         // then there was no marker found, probably because end of string was reached
         // anyway, we can just ignore it
      }
      else if (marker == "PROCS")
      {
         result << numberOfProcessors;
      }
      else if (marker == "MPI_PARAMS")
      {
         for (unsigned k = 0; k < mpiCmdLineParams.size(); ++k)
         {
            if (k)
               result << ' ';

            result << mpiCmdLineParams[k] ;
         }
      }
      else if (marker == "INPUT")
      {
         result << inputProject;
      }
      else if (marker == "OUTPUT")
      {
         result << outputProject;
      }
      else if (marker == "FC_PARAMS")
      {
         for (unsigned k = 0; k < fcCmdLineParams.size(); ++k)
         {
            if (k)
               result << ' ';

            result << fcCmdLineParams[k] ;
         }
      }
      else
      {
         throw Exception() << "Unrecognized parameter '" << marker << "' in fastcauldron template run-script for version '" << version << "'";
      }
   }
   return result.str();
}


FastCauldronEnvironment
   :: FastCauldronEnvironment( const Configuration & configuration, const std::string & id, const Path & projectFile, int processors, const std::string & version)
   : m_configuration(configuration)
   , m_id(id)
   , m_processors(processors)
   , m_mpiCmdLine()
   , m_cauldronCmdLine()
   , m_projectSourceDir( projectFile.getParentDirectory() )
   , m_version(version)
{
  m_factory.reset(new DataAccess::Interface::ObjectFactory());
  m_project.reset(DataAccess::Interface::OpenCauldronProject(projectFile.getCanonicalPath(), m_factory.get()));

  if (!m_project)
     throw Exception() << "Could not open project file '" << projectFile << "'";
}

FastCauldronEnvironment::~FastCauldronEnvironment()
{
}

void
FastCauldronEnvironment
   :: applyProjectParameter( const Project3DParameter & param, const std::string & value)
{
   param.writeValue( m_project.get(), value );
}

void
FastCauldronEnvironment
   :: applyMpiCmdLineParameter( const CmdLineParameter & param, const std::vector< std::string > & values)
{
   std::vector< CmdLineParameter::Option > newOptions
      = param.getOptions(values);

   m_mpiCmdLine.insert( m_mpiCmdLine.end(), newOptions.begin(), newOptions.end());
}

void
FastCauldronEnvironment
   :: applyCauldronCmdLineParameter( const CmdLineParameter & param, const std::vector< std::string > & values)
{
   std::vector< CmdLineParameter::Option > newOptions
      = param.getOptions(values);

   m_cauldronCmdLine.insert( m_cauldronCmdLine.end(), newOptions.begin(), newOptions.end());
}


std::ostream &
FastCauldronEnvironment
   :: hostInformationScript( std::ostream & output)
{
   output
         << "echo LSB_HOSTS = \"$LSB_HOSTS\"\n"
         << "echo LSB_MCPU_HOSTS = \"$LSB_MCPU_HOSTS\"\n"
         << "if [ x\"$LSB_HOSTS\" = x ]; then\n"
         << "  export LSB_HOSTS=`hostname`\n"
         << "fi\n"
         << "previousHost=\n"
         << "for host in $LSB_HOSTS\n"
         << "do\n"
         << "   # Only do unique hosts\n"
         << "   if [ x$previousHost = x$host ]; then continue; fi\n"
         << "   previousHost=$host\n"
         << "\n"
         << "   echo ::::::::::::: $host :::::::::::::::\n"
         << "   ssh -oStrictHostKeyChecking=no -oForwardX11=no $host '"
                     << "cat /proc/cpuinfo ; "
                     << "echo; "
                     << "echo ===== Memory Info =====; "
                     << "cat /proc/meminfo; "
                     << "echo; "
                     << "echo ===== Uptime =====; "
                     << "uptime; "
                     << "echo; "
                     << "echo ===== Net statistics =====; "
                     << "cat /proc/net/dev; "
                     << "'\n"
         << "   echo :::::::::::::::::::::::::::::::::::\n"
         << "done\n";
   return output;
}

bool
FastCauldronEnvironment
   :: jobHasRanBefore(const Path & directory, const std::string & id)
{
   const std::shared_ptr<Path> myDir = directory.getDirectoryEntry(id);
   const std::shared_ptr<Path> outputFile = myDir->getDirectoryEntry("output");

   if (directory.getFileType() == Path::NotExists)
      return false;

   if (directory.getFileType() != Path::Directory)
      throw Exception() << "'" << directory << "' exists but is not a directory.";

   if (myDir->getFileType() == Path::NotExists)
      return false;

   if (myDir->getFileType() != Path::Directory)
      throw Exception() << "'" << *myDir << "' exists but is not a directory.";

   Path::FileType fileType = outputFile->getFileType();

   if (fileType != Path::Regular && fileType != Path::NotExists)
      throw Exception() << "File '" << outputFile << "' exists and is not a regular file";

   return fileType == Path::Regular;
}

void
FastCauldronEnvironment
   :: commandToRunJob( const Path & directory, const std::string & id,
         std::shared_ptr<Path> & workingDir, std::string & command )
{
  workingDir = directory.getDirectoryEntry(id);
  command = "bash runcauldron.sh > output";
}

bool
FastCauldronEnvironment
   :: generateJob( const Path & directory )
{
   std::shared_ptr<Path> myDir = directory.getDirectoryEntry( m_id );

   if ( directory.getFileType() == Path::NotExists)
      directory.makeDirectory();

   if ( directory.getFileType() != Path::Directory)
      throw Exception() << "'" << directory << "' exists but is not a directory.";

   if ( myDir->getFileType() == Path::NotExists)
      myDir->makeDirectory();

   if ( myDir->getFileType() != Path::Directory)
      throw Exception() << "'" << *myDir << "' exists but is not a directory.";

   if (jobHasRanBefore(directory, m_id))
      return false;

   // save the project file
   m_project->saveToFile( myDir->getDirectoryEntry( "Project.project3d")->getPath() );

   {  // copy files on which the project depends
      ProjectDependencies deps = getProjectDependencies(m_project->getProjectFileHandler ());

      if ( !deps.outputMaps.empty() || !deps.snapshots.empty())
      {
         throw Exception() << "Insuitable project, because it already contains output.";
      }

      // copy input maps
      for (unsigned i = 0; i < deps.inputMaps.size(); ++i)
      {
         m_projectSourceDir->getDirectoryEntry(deps.inputMaps[i])->copyTo(
               *myDir->getDirectoryEntry( deps.inputMaps[i])
             );
      }

      // copy related projects
      for (unsigned i = 0; i < deps.related.size(); ++i)
      {
         m_projectSourceDir->getDirectoryEntry(deps.related[i])->copyTo(
               *myDir->getDirectoryEntry( deps.related[i])
             );
      }
   }

   // construct the file that runs cauldron
   * myDir->getDirectoryEntry("runcauldron.sh")->writeFile()
         << "#!/bin/bash\n"
         << '\n'
         // output host information
         << "echo /////////////////// HOST INFORMATION BEFORE RUN //////////////////////\n"
         << hostInformationScript
         << "\n"
         << "echo\n"
         << "echo /////////////////// FASTCAULDRON OUTPUT //////////////////////////////\n"
         // setup the environment
         << m_configuration.getRunScript(m_version, m_processors, m_mpiCmdLine, "Project.project3d", "Project_output.project3d", m_cauldronCmdLine)

         << "\necho\n"
         << "echo /////////////////// HOST INFORMATION AFTER RUN //////////////////////\n"
         << hostInformationScript
         << "\n"
         << std::endl;


   return true;
}


}
