#ifndef XCAULDRON_PROJECTDEPENDENCIES_H
#define XCAULDRON_PROJECTDEPENDENCIES_H

#include <vector>
#include <string>

namespace database
{
   class Database;
}

struct ProjectDependencies
{
   std::vector< std::string > inputMaps, related, outputMaps, snapshots;
};

ProjectDependencies getProjectDependencies( database::Database * projBase );

#endif
