#ifndef FLUIDCONTACTSPROPERTY_H_INCLUDED
#define FLUIDCONTACTSPROPERTY_H_INCLUDED

#include "Project.h"

#include <vector>

class MiVolumeMeshCurvilinear;
class SoLineSet;

SoLineSet* buildIsoLines(
  const MiVolumeMeshCurvilinear& mesh, 
  const std::vector<Project::Trap>& traps);


#endif
