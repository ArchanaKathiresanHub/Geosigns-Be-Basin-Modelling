#ifndef HPC_FCBENCH_VARIABLEDEFINITIONS_H
#define HPC_FCBENCH_VARIABLEDEFINITIONS_H

#include <string>
#include <map>
#include <vector>

namespace hpc
{

class Path;

class VariableDefinitions
{
public:
   typedef std::string Name;
   typedef std::string Description;
   typedef std::string ShellScript;
   struct Definition
   {
      Name name;
      Description description;
      ShellScript script;
   };

   VariableDefinitions(const Path & variableDefinitionsFile );

   const std::vector< Definition > & definitions() const
   { return m_definitions; }

private:
   std::vector< Definition > m_definitions;
};

}

#endif
