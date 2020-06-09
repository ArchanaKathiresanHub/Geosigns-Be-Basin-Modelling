#include "parametersettings.h"
#include "parser.h"
#include "system.h"

#include <fstream>

namespace hpc
{
ParameterSettings
   :: ParameterSettings()
   : m_map()
{}     

std::vector<ParameterSettings>
ParameterSettings
   :: parse( const Path & parameterSettingsFile )
{
  std::shared_ptr<std::istream> file = parameterSettingsFile.readFile();

  std::vector<ParameterSettings> result;
  std::vector< ParameterSettings > currentSettings(1);
  std::string line;
  while (getline(*file, line))
  {
     // empty lines are record separators
     if (line.empty())
     {
        if (currentSettings.size() >= 1 && !currentSettings[0].m_map.empty())
        {
           // this is a record separator
           // first make the ID's unique
           if (currentSettings.size() > 1)
           {
              for (unsigned i = 0; i < currentSettings.size(); ++i)
              {
                 std::ostringstream id( currentSettings[i].m_map["ID"], std::ios_base::app );
                 id << "-" << i;
                 currentSettings[i].m_map["ID"] = id.str();
              }
           }

           // and add everything
           result.insert( result.end(), currentSettings.begin(), currentSettings.end());
        }

        currentSettings.clear();
        currentSettings.resize(1);
     }
     else
     {
        Parser parser(line);
        std::string name = parser.nextToken();
        std::string v = parser.nextToken();

        if (v == "{")
        { // then parse a range of values
           std::vector< std::string > values;
           while ( v != "}")
           {
             values.push_back( parser.nextToken() );
             v = parser.nextToken();
             if (v != "," && v != "}")
                throw ParseException() << "Syntax error in specification of range in parameter setting";
           }

           // and then as resulting set of settings you want to have the cross-product.
           std::vector<ParameterSettings> newCurrentSettings;

           for (unsigned i = 0; i < currentSettings.size(); ++i)
           {
              for (unsigned j = 0; j < values.size(); ++j)
              {
                 newCurrentSettings.push_back( currentSettings[i] );
                 newCurrentSettings.back().m_map[name] = values[j];
              }
           }
           currentSettings.swap( newCurrentSettings );
        }
        else
        { // just add the value to the settings
           for (unsigned i = 0; i < currentSettings.size(); ++i)
              currentSettings[i].m_map[ name ] = v;
        }
     }
  }

  return result;
}
   

}
