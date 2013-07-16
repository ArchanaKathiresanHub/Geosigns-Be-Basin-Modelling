#include "variabledefinitions.h"
#include "parser.h"
#include "system.h"

#include <fstream>

namespace hpc
{

   VariableDefinitions
      :: VariableDefinitions(const Path & variableDefinitionsFile)
      : m_definitions()
   {
      boost::shared_ptr<std::istream> input = variableDefinitionsFile.readFile();

      std::string line;
      while (getline(*input, line))
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
