#include "track1dScript.h"

namespace casaWizard
{

namespace sac
{

Track1DScript::Track1DScript(const QString& baseDirectory, const QString& projectName) :
  RunScript(baseDirectory),
  xCoordinate_{},
  yCoordinate_{},
  propertiesCauldronName_{},
  propertiesUserName_{},
  runFolders_{},
  projectName_{projectName}
{
}

void Track1DScript::addWell(const double x, const double y, const QString& propertyCauldronName, const QString& propertiesUserName,
                            const QString& runFolder)
{
  xCoordinate_.append(x);
  yCoordinate_.append(y);
  propertiesCauldronName_.append(propertyCauldronName);
  propertiesUserName_.append(propertiesUserName);
  runFolders_.append(runFolder);
}

void Track1DScript::addCommandProject(const int call, const QString& projectName)
{
  addCommand("track1d -coordinates " + QString::number(xCoordinate_[call], 'f') +
             "," + QString::number(yCoordinate_[call],'f') +
             " -properties " + propertiesCauldronName_[call] +
             " -age 0" +
             " -project " + projectName + ".project3d" +
             " -save wellTrajectory-" + projectName + "-" + propertiesUserName_[call] + ".csv" +
             " -lean",
             runFolders_[call]);
}

bool Track1DScript::generateCommands()
{
  const int nCalls = xCoordinate_.size();
  for (int iCall = 0; iCall < nCalls; ++iCall)
  {
    addCommandProject(iCall, projectName_);    
  }
  return true;
}



} // namespace sac

} // namespace casaWizard
