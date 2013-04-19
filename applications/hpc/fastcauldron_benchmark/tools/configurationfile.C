#include "configurationfile.h"
#include "parser.h"

#include <fstream>
#include <sstream>

namespace hpc
{

ConfigurationFile
   :: ConfigurationFile( const std::string & file)
   : m_nameValueMap()
{
   std::ifstream input(file.c_str());

   std::string line;
   while (getline( input, line ) )
   {
      if (line.empty())
         continue;

      Parser parser(line);
      m_nameValueMap.insert( std::make_pair( parser.nextToken(), parser.nextToken() ));
   }
}


const std::string &
ConfigurationFile
   :: getString( const std::string & name) const
{
   std::map< std::string, std::string > :: const_iterator i
      = m_nameValueMap.find( name );

   if (i == m_nameValueMap.end())
      throw Exception() << "Could not find value of '" << name << "' in configuration file";

   return i->second;
}

int 
ConfigurationFile
   :: getInt( const std::string & name) const
{
   std::istringstream input( getString(name) );

   int result = 0;
   input >> result;

   if (!input)
      throw Exception() << "Value '" << getString(name) << "' of '" << name << "' could not be parsed as integer";

   return result;
}



}
