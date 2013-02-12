#include "parametersettings.h"
#include "parser.h"

#include <fstream>

namespace hpc
{
ParameterSettings
   :: ParameterSettings()
   : m_map()
{}     

std::vector<ParameterSettings>
ParameterSettings
   :: parse( const std::string & parameterSettingsFile )
{
  std::ifstream file(parameterSettingsFile.c_str());

  std::vector<ParameterSettings> result;
  ParameterSettings oneSetting;
  std::string line;
  while (getline(file, line))
  {
     // empty lines are record separators
     if (line.empty())
     {
        result.push_back( oneSetting );
        oneSetting.m_map.clear();
     }
     else
     {
        Parser parser(line);
        oneSetting.m_map[ parser.nextToken() ] = parser.nextToken() ;
     }
  }

  return result;
}
   

}
