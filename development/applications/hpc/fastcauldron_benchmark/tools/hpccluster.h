#ifndef HPC_FCBENCH_HPCCLUSTER_H
#define HPC_FCBENCH_HPCCLUSTER_H

#include <string>
#include <vector>

#include "configurationfile.h"

#include <memory>

namespace hpc
{
 
class Path;

class HPCCluster
{
public:
   virtual ~HPCCluster() {}

   template <class It>
   void submitMpiJob(It begin, It end) 
   {
      this->submitMpiJob( std::vector< std::pair< Processors, ID > >( begin, end));
   }

   typedef int Processors;
   typedef std::string ID;

   virtual void submitMpiJob(const std::vector< std::pair< Processors, ID > > & jobs) = 0;
   virtual void wait() = 0;

   int processorsPerHost()  const
   { return m_processorsPerHost; }

protected:
   HPCCluster(int processorsPerHost)
      : m_processorsPerHost(processorsPerHost)
   {}

private:
   int m_processorsPerHost;
};


class DelayedLsfCluster : private ConfigurationFile, public HPCCluster
{
public:
   DelayedLsfCluster(const Path & configFile, const Path & workingDirectory);

   virtual void submitMpiJob(const std::vector< std::pair< Processors, ID > > & jobs);
   virtual void wait();

private:
   static std::string randomAllBenchmarkId() ;

   std::string m_clusterName;
   std::shared_ptr<Path> m_directory;
   std::string m_lsfProject;
   std::string m_allBenchmarkId;
   std::shared_ptr<Path> m_jobDir;
   int m_estimatedJobDuration;
};


}

#endif
