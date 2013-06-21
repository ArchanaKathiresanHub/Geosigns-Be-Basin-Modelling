#ifndef HPC_FCBENCH_PARAMETERSETTINGS_H
#define HPC_FCBENCH_PARAMETERSETTINGS_H

#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>


namespace hpc
{

class Project3DParameter;
class CmdLineParameter;

class ParameterSettings
{
public:
   ParameterSettings(); // needed to be public for std::vector

   typedef std::string Name;
   typedef std::string Value;
   typedef std::map< Name, Value > Map;
   static std::vector<ParameterSettings> parse(const std::string & parameterSettingsFile );

   const Map & map() const
   { return m_map; }

private:

   Map m_map;
};


}

#endif
