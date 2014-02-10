#include "Mesh.h"

#include "ProjectHandle.h"

void foo()
{
	std::string filename = "bla";
	std::string access = "r";
	DataAccess::Interface::ProjectHandle* handle = DataAccess::Interface::OpenCauldronProject(filename, access);
  DataAccess::Interface::FormationList* formations = handle->getFormations();
	//DataAccess::Interface::InitializeSerializedIO();
}