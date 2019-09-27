/* Class responsible to create a workspace in a new location. The new location will be chosen by the user.
All necessary files will be copied to the new location within this class. The new workspace path will be set in the controller
using casaScenario_.setWorkingDirectory(NEW WORKSPACE LOCATION);*/
#pragma once

class QString;

namespace casaWizard
{

namespace workspaceGenerator
{
bool createWorkspace(const QString& rootPath,const QString& newFolder );
bool copyDir(const QString& currentPath,const QString& newWorkspace);
QString getSuggestedWorkspace(const QString& currentPath);

} // namespace workspaceGenerator

} // namespace casaWizard
