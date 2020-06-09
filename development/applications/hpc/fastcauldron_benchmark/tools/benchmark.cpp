#include "fastcauldronenvironment.h"
#include "project3dparameter.h"
#include "cmdlineparameter.h"
#include "parameterdefinitions.h"
#include "parametersettings.h"
#include "hpccluster.h"
#include "resultstabulator.h"
#include "variabledefinitions.h"
#include "system.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <tr1/array>
#include <map>
#include <set>

namespace hpc
{

   void benchmark( const Path & parameterDefinitionsFile, const Path & settingsFile, const Path & variableDefinitionsFile, const Path & environment, const Path & workingDir, HPCCluster & cluster )
   {
      std::tr1::array<std::string, 4> mandatorySettings = { { "ID", "Project", "Processors", "CauldronVersion" } };

      struct MandatorySetting { static
         std::string getValue(const ParameterSettings::Map & map, const std::string & parameter)
         {
            ParameterSettings::Map::const_iterator s = map.find(parameter);
            if (s == map.end())
            {
               std::cerr
                  << "Warning: Non-optional parameter '" << parameter << "' is missing in settings."
                  << std::endl;
               return std::string();
            }
            return s->second;
         }
      };

      // read the Fastcauldron environment configuration
      FastCauldronEnvironment::Configuration fcConfig(environment);

      // read parameter definitions
      ParameterDefinitions pd( parameterDefinitionsFile );

      // read the parameter settings
      std::vector<ParameterSettings> settings = ParameterSettings::parse(settingsFile);

      // write the results tabulator script
      writeResultsTabulatorScript(
           *workingDir.getDirectoryEntry("tabulate_results.sh")->writeFile(),
           pd,
           VariableDefinitions(variableDefinitionsFile),
           settings,
           workingDir.getCanonicalPath()
           );

      // write the directory structure with project files, scripts, etc...
      typedef int         Processors;
      typedef std::string ID;
      typedef std::map<Processors, ID > ProcList;
      std::multimap< Processors, ID > processorsPerSetting;
      std::set<ID> ids;

      for (unsigned i = 0; i < settings.size(); ++i)
      {
         std::string projectFile = MandatorySetting :: getValue(settings[i].map(), "Project");
         std::string settingID = MandatorySetting :: getValue(settings[i].map(), "ID");
         int processors = -1;
         {
           std::istringstream input(MandatorySetting :: getValue(settings[i].map(), "Processors"));
           input >> processors;
         }
         std::string version = MandatorySetting::getValue(settings[i].map(), "CauldronVersion");

         if (projectFile.empty() || settingID.empty() || processors < 1)
            continue;

         if ( !ids.insert( settingID ).second)
         {
            std::cerr << "Warning: Multiple benchmarks have the same ID '" << settingID << "'" << std::endl;
            continue;
         }

         try
         {

            FastCauldronEnvironment env( fcConfig, settingID, projectFile, processors, version );

            typedef ParameterSettings::Map::const_iterator It;
            for (It s = settings[i].map().begin(); s != settings[i].map().end(); ++s)
            {
               const std::string & name = s->first;
               const std::string & value = s->second;

               if (std::find(mandatorySettings.begin(), mandatorySettings.end(), name) != mandatorySettings.end())
                  continue;

               typedef ParameterDefinitions :: ProjectParamMap :: const_iterator P;
               typedef ParameterDefinitions :: CmdLineParamMap :: const_iterator C;

               {
                  P p = pd.projectParameters().find( name );
                  if ( p != pd.projectParameters().end())
                     env.applyProjectParameter( *p->second, value );
               }

               {
                  C c = pd.mpiCmdLineParameters().find( name );
                  if (c != pd.mpiCmdLineParameters().end())
                     env.applyMpiCmdLineParameter( *c->second, std::vector<std::string>(1,value));
               }

               {
                  C c = pd.cauldronCmdLineParameters().find( name );
                  if (c != pd.cauldronCmdLineParameters().end())
                     env.applyCauldronCmdLineParameter( *c->second, std::vector<std::string>(1, value));
               }
            }

            if (env.generateJob(workingDir))
               processorsPerSetting.insert( std::make_pair( processors , settingID ) );
         }
         catch(std::exception & e)
         {
            std::cerr << "Warning: Cannot generate benchmark for setting '" << settingID << "', because "
              << e.what() << std::endl;
         }
      }

      if (processorsPerSetting.empty())
      {
         std::cerr << "Warning: No benchmark runs are prepared" << std::endl;
         return;
      }

      // schedule benchmark jobs in the cluster
      int procsPerHost = cluster.processorsPerHost();
      int maxProcs = processorsPerSetting.rbegin()->first;

      for (int hosts = 0; hosts < (maxProcs + procsPerHost - 1) / procsPerHost; ++hosts)
      {
         ProcList::const_iterator begin = processorsPerSetting.upper_bound( hosts*procsPerHost);
         ProcList::const_iterator end = processorsPerSetting.upper_bound( (hosts+1)*procsPerHost);

         cluster.submitMpiJob( begin, end );
      }

      // wait on results
      cluster.wait();



   }
} // namespace hpc

int main(int argc, char ** argv)
{
   using namespace hpc;
   if (argc < 6)
   {
      std::cerr << "Usage: " << argv[0] << " PARAMETERDEFINITIONS SETTINGS VARIABLEDEFS ENVIRONMENT CLUSTERCONF WORKDIR" << std::endl;
      return 1;
   }

   Path parameterDefinitions(argv[1]);
   Path parameterSettings(argv[2]);
   Path variableDefinitions(argv[3]);
   Path environment(argv[4]);
   Path clusterConfiguration(argv[5]);
   Path workingDirectory(argv[6]);

   DelayedLsfCluster cluster(clusterConfiguration, workingDirectory );
   benchmark( parameterDefinitions, parameterSettings, variableDefinitions, environment, workingDirectory, cluster);

   return 0;
}
