#include "depthConversionScript.h"

#include "model/logger.h"
#include "model/sacLithologyScenario.h"

#include "SDUWorkLoadManager.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QCoreApplication>

#ifndef _WIN32
#include <sys/stat.h>
#endif

namespace casaWizard
{

namespace sac
{

DepthConversionScript::DepthConversionScript(const SacLithologyScenario& scenario, const QString& baseDirectoryVar, const workloadmanagers::WorkLoadManagerType& workloadManagerType) :
  RunScript(baseDirectoryVar),
  scenario_{scenario},
  scriptFilename_{baseDirectoryVar + "/runt2z.sh"}
{
  try
  {
    workloadManager_ = workloadmanagers::WorkLoadManager::Create(baseDirectory().toStdString() + "/run.sh", workloadManagerType);
  }
  catch (workloadmanagers::WLMException)
  {
    Logger::log() << "The folder for creating a depth conversion script does not exist!" << Logger::endl();
    return;
  }
}

DepthConversionScript::~DepthConversionScript()
{

}

bool DepthConversionScript::generateCommands()
{
  if (runLocally())
  {
    addCommand(getDepthConversionCommand());
  }
  else
  {
    QFile file(scriptFilename_);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text) || !workloadManager_)
    {
      Logger::log() << "- Failed to write depth conversion script, as file can not be opened" << Logger::endl();
      return false;
    }

    Logger::log() << "- Start writing depth conversion file to " << scriptFilename_ << Logger::endl();
    writeScriptContents(file);
    file.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner|
                        QFile::ReadGroup|QFile::ExeGroup|
                        QFile::ReadOther|QFile::ExeOther);
    file.close();
    Logger::log() << "- Finished writing depth conversion file to " << scriptFilename_ << Logger::endl();

    addCommand(scriptFilename_);
  }
  return true;
}

bool DepthConversionScript::runLocally() const
{
  return scenario_.clusterName().toLower() == "local";
}

void DepthConversionScript::writeScriptContents(QFile& file) const
{   
  QTextStream out(&file);

#ifndef _WIN32
  chmod( (baseDirectory().toStdString() + "/run.sh").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
#endif
  out << QString("#!/bin/bash -lx\n");

  std::string applicationPath = QCoreApplication::applicationDirPath().toStdString();
  std::size_t index = applicationPath.find("/apps/sss");
  if (index != std::string::npos)
  {
    applicationPath = applicationPath.substr(index);
  }

  out << QString("export PATH=" + QString::fromStdString(applicationPath) + ":$PATH\n" +
                 "source setupEnv.sh\n");

  out << QString::fromStdString(workloadManager_->JobSubmissionCommand("cldrn", "", -1, "\"Fastcauldron T2Z conversion run\"", "output.log",
                                                                   "err.log", std::to_string(scenario_.t2zNumberCPUs()), "", "", baseDirectory().toStdString(), false, true,
                                                                   getDepthConversionCommand().toStdString(), true)) << "\n";
}

QString DepthConversionScript::getDepthConversionCommand() const
{
  const QString runMode = scenario_.applicationName().split(QString("\""))[1];
  return QString("mpirun_wrap.sh -n " + QString::number(scenario_.t2zNumberCPUs()) +
                   (runLocally() ? "" : " -outfile-pattern 'fastdepthconversion-output-rank-%r.log'") +
                   " fastdepthconversion" +
                   " -project " + scenario_.project3dFilename() + " " + runMode + " -onlyat 0" +
                   " -referenceSurface " + QString::number(scenario_.t2zReferenceSurface()) +
                   " -endSurface " + QString::number(scenario_.t2zLastSurface()) +
                   " -preserveErosion" +
                   " -noCalculatedTWToutput");				   
}

} // namespace sac

} // namespace casaWizard
