#include "fastcauldronenvironment.h"
#include "cmdlineparameter.h"
#include "project3dparameter.h"
#include "projectdependencies.h"
#include "system.h"

#include <iostream>

#include "Interface/ProjectHandle.h"

namespace hpc
{

FastCauldronEnvironment
   :: FastCauldronEnvironment( const std::string & id, const std::string & projectFile, int processors)
   : m_id(id)
   , m_processors(processors)
   , m_mpiCmdLine()
   , m_cauldronCmdLine()
   , m_project( DataAccess::Interface::OpenCauldronProject( projectFile, "r"))
   , m_projectSourceDir( dirname(projectFile))
{}

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
                     << MPICmdLineTools().cpuinfo() << "; "
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
   :: jobHasRanBefore(const std::string & directory, const std::string & id)
{
   const std::string myDir = directory + pathSeparator + id;
   const std::string outputFile = myDir + pathSeparator + "output";

   if (getFileType(directory) == FT_NotExists)
      return false;

   if (getFileType(directory) != FT_Directory)
      throw Exception() << "'" << directory << "' exists but is not a directory.";

   if (getFileType(myDir) == FT_NotExists)
      return false;

   if (getFileType(myDir) != FT_Directory)
      throw Exception() << "'" << myDir << "' exists but is not a directory.";

   FileType fileType = getFileType(outputFile);

   if (fileType != FT_Regular && fileType != FT_NotExists)
      throw Exception() << "File '" << outputFile << "' exists and is not a regular file";
   
   return fileType == FT_Regular;
}

void 
FastCauldronEnvironment
   :: commandToRunJob( const std::string & directory, const std::string & id, std::string & workingDir, std::string & command)
{
  workingDir = directory + pathSeparator + id;
  command = "bash runcauldron.sh > output";
}

bool
FastCauldronEnvironment
   :: generateJob( const std::string & directory )
{
   const std::string myDir = directory + pathSeparator + m_id;

   if ( getFileType(directory) == FT_NotExists)
      mkdir( directory );

   if (getFileType(directory) != FT_Directory)
      throw Exception() << "'" << directory << "' exists but is not a directory.";

   if (getFileType(myDir) == FT_NotExists)
      mkdir( myDir );

   if (getFileType(myDir) != FT_Directory)
      throw Exception() << "'" << myDir << "' exists but is not a directory.";

   if (jobHasRanBefore(directory, m_id))
      return false;

   // save the project file
   m_project->saveToFile(myDir + pathSeparator + "Project.project3d");

   {  // copy files on which the project depends
      ProjectDependencies deps = getProjectDependencies(m_project->getDataBase());

      if ( !deps.outputMaps.empty() || !deps.snapshots.empty())
      {
         throw Exception() << "Insuitable project, because it already contains output.";
      }

      for (unsigned i = 0; i < deps.inputMaps.size(); ++i)
         copyFile( m_projectSourceDir + pathSeparator + deps.inputMaps[i], myDir + pathSeparator + deps.inputMaps[i]);
   }

   {  // construct the file that runs cauldron
      std::ofstream commandFile( (myDir + pathSeparator + "runcauldron.sh").c_str() );
      commandFile 
         << "#!/bin/bash\n"
         << '\n'
         // output host information
         << "echo /////////////////// HOST INFORMATION BEFORE RUN //////////////////////\n"
         << hostInformationScript
         << "\n"
         // execute fastcauldron
         << "echo\n"
         << "echo /////////////////// FASTCAULDRON OUTPUT //////////////////////////////\n"
         << "mpirun -np " << m_processors << ' ' ;
      
      for (unsigned i = 0; i < m_mpiCmdLine.size(); ++i)
         commandFile << m_mpiCmdLine[i] << ' ';

      commandFile 
         << "fastcauldron "
         << "-project Project.project3d "  // the project file
         << "-save Project_output.project3d " // the output project file
         << "-debug1 -log_summary "       // flags to fastcauldron that print a lot of performance data
         ;

      for (unsigned i = 0 ; i < m_cauldronCmdLine.size(); ++i)
         commandFile << m_cauldronCmdLine[i] << ' ';

      commandFile
         << "\necho\n"
         << "echo /////////////////// HOST INFORMATION AFTER RUN //////////////////////\n"
         << hostInformationScript
         << "\n";

      commandFile << std::endl;
   } 

   return true;
}


}


