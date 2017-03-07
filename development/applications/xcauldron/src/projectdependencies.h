#ifndef XCAULDRON_PROJECTDEPENDENCIES_H
#define XCAULDRON_PROJECTDEPENDENCIES_H

#include <vector>
#include <string>

#include "ProjectFileHandler.h"

struct ProjectDependencies
{
   std::vector< std::string > inputMaps, related, outputMaps, snapshots;
};

ProjectDependencies getProjectDependencies( database::ProjectFileHandlerPtr projBase );

#endif
