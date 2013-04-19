#include "variabledefinitions.h"
#include "parser.h"

#include <fstream>

namespace hpc
{

   VariableDefinitions
      :: VariableDefinitions(const std::string & variableDefinitionsFile)
      : m_definitions()
   {
      std::ifstream input(variableDefinitionsFile.c_str());

      if (!input)
         throw ParseException() << "Cannot open variable definitions file '" << variableDefinitionsFile << "'";

      std::string line;
      while (getline(input, line))
      {
         if (line.empty())
            continue;

         Parser parser(line);
         Name name = parser.nextToken();
         Description description = parser.nextToken();
         ShellScript script = parser.nextToken();

         Definition definition = { name, description, script };
         m_definitions.push_back( definition );
      }
   }


}
