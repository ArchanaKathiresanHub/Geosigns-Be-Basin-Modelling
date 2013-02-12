#include "hpccluster.h"
#include "system.h"
#include "fastcauldronenvironment.h"

#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <iostream>

namespace hpc
{

std::string
DelayedLsfCluster
   :: randomAllBenchmarkId()
{
  static bool initialized = false;
  if (!initialized)
     ::srandom( std::time(0));

  std::ostringstream id;
  id << "cauldron_benchmark_" << std::setfill('0') << std::setw(8) << std::hex << static_cast<int32_t>(::random());
  return id.str();
}

DelayedLsfCluster
   :: DelayedLsfCluster(const std::string & configFile, const std::string & workingDirectory)
   : ConfigurationFile(configFile)
   , HPCCluster(ConfigurationFile::getInt("ProcessorsPerHost"))
   , m_clusterName(ConfigurationFile::getString("ClusterName"))
   , m_directory()
   , m_lsfProject(ConfigurationFile::getString("LSFProjectName"))
   , m_allBenchmarkId(randomAllBenchmarkId())
   , m_jobDir()                                          
   , m_estimatedJobDuration(ConfigurationFile::getInt("EstimatedJobDuration"))
{
   if (getFileType(workingDirectory) != FT_Directory)
     mkdir( workingDirectory );

   m_directory = canonicalPath(workingDirectory);

   if (getFileType(m_directory + pathSeparator + m_clusterName + "_jobs") != FT_Directory)
      mkdir( m_directory + pathSeparator +  m_clusterName + "_jobs");
         
   m_jobDir = m_directory + pathSeparator + m_clusterName + "_jobs" + pathSeparator + m_allBenchmarkId;

   mkdir( m_jobDir );
   std::cout << "Generating LSF scripts in directory: " << m_jobDir << std::endl;
}


void 
DelayedLsfCluster
   :: submitMpiJob(const std::vector< std::pair< Processors, ID > > & jobs)
{
   if (jobs.empty())
      return;

   Processors maxP = 0;
   for (unsigned i = 0; i < jobs.size(); ++i)
      maxP = std::max(maxP, jobs[i].first );

   int nHosts = (maxP + processorsPerHost() - 1)/ processorsPerHost();

   std::ostringstream scriptName;
   scriptName << m_jobDir << pathSeparator << "lsf_submit_" << nHosts;

   std::ofstream lsfscript(scriptName.str().c_str());
   lsfscript 
      << "#!/bin/bash\n"
      << "#BSUB -P " << m_lsfProject << '\n'     // the project to bill it on
      << "#BSUB -We " << m_estimatedJobDuration << ":00\n"   // estimate that it will take three days
      << "#BSUB -J " << m_allBenchmarkId << '\n' // job id
      << "#BSUB -n " << maxP << '\n'             // number of processors 
      << "#BSUB -x\n"                            // exclusive mode, so that we get reliable measurements
      << "#BSUB -cwd " << m_jobDir << '\n'       // working directory
      << "#BSUB -o " << scriptName.str() << "_stdout"
      << "\n"
      << "function FAIL()\n"
      << "{\n"
      << "  echo An error occurred: $1\n"
      << "  return 1\n"
      << "}\n"
      << "\n"
      << "\n"
      << "HOSTS=$(( `echo $LSB_MCPU_HOSTS | wc -w` / 2 ))\n"
      << "PROCS=`echo $LSB_HOSTS | wc -w`\n"
      << "\n"
      << "echo ============== INIT ==========================\n"
      << "echo Loading MPI environment\n"
      << MPICmdLineTools().loadEnv() << '\n'
      << "echo ============== DEBUGINFO ======================\n"
      << "echo HOSTS=$HOSTS\n"
      << "echo PROCS=$PROCS\n"
      << "echo =======================================\n"
      << "echo RUNNING MPI PROCESS\n"
      << "echo ---------------------------------------\n"
      ;

   for (unsigned job = 0; job < jobs.size(); ++job)
   {
      std::string command;
      std::string workingDirectory;
      FastCauldronEnvironment::commandToRunJob(m_directory, jobs[job].second, workingDirectory, command);

      lsfscript
         << "echo --------- RUNNING " << std::setw(10) << jobs[job].second << " ----------\n"
         << "pushd " << workingDirectory << '\n'
         << command << '\n'
         << "popd\n";
   }

   lsfscript
      << "echo =======================================\n"
      << "echo EXITING\n"
      ;
}

void 
DelayedLsfCluster
   :: wait()
{
   std::ostringstream scriptName;
   scriptName << m_jobDir << pathSeparator << "lsf_wait";

   std::ofstream lsfscript(scriptName.str().c_str());
   lsfscript
      << "#!/bin/bash\n"
      << "#BSUB -P " << m_lsfProject << '\n'     // the project to bill it on
      << "#BSUB -We 1\n"                       // the work done by the job is trivial (so 1 minute)
      << "#BSUB -J wait_on_" << m_allBenchmarkId << '\n' // job id
      << "#BSUB -cwd " << m_jobDir << '\n' // working directory
      << "#BSUB -w 'ended(" << m_allBenchmarkId << ")'\n" // dependency on all running jobs of this benchmark
      << "echo DONE\n"
      ;

}


}
