#ifndef FLUIDCONTACTSPROPERTY_H_INCLUDED
#define FLUIDCONTACTSPROPERTY_H_INCLUDED

#include "Project.h"

#include <vector>

class MiVolumeMeshCurvilinear;
class MoLevelColorMapping;
class SoLineSet;

SoLineSet* buildIsoLines(
  const MiVolumeMeshCurvilinear& mesh, 
  const std::vector<Project::Trap>& traps);

std::shared_ptr<MiDataSetIjk<double> > createFluidContactsProperty(
  const std::vector<Project::Trap>& traps,
  const MiDataSetIjk<double>& trapIdProperty,
  const MiVolumeMeshCurvilinear& mesh);

MoLevelColorMapping* createFluidContactsColorMap();

#endif
