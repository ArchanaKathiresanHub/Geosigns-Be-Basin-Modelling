#include "depthConversionScript.h"

#include "model/logger.h"
#include "model/sacScenario.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QCoreApplication>

namespace casaWizard
{

namespace sac
{

DepthConversionScript::DepthConversionScript(const SACScenario& scenario, const QString& baseDirectory) :
  RunScript(baseDirectory),
  scenario_{scenario},
  scriptFilename_{baseDirectory + "/runt2z.sh"}
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

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
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

  out << QString("#!/bin/bash -lx\n");
  out << QString("cat > runt2zCluster.sh << EOF\n");
  out << QString("#BSUB -P cldrn\n");
  out << QString("#BSUB -We 3:00\n");
  out << QString("#BSUB -J \"Fastcauldron T2Z conversion run\"\n");
  out << QString("#BSUB -n " + QString::number(scenario_.t2zNumberCPUs()) + "\n");
  out << QString("#BSUB -o output.log\n");
  out << QString("#BSUB -cwd " + baseDirectory() + "\n");

  std::string applicationPath = QCoreApplication::applicationDirPath().toStdString();
  std::size_t index = applicationPath.find("/apps/sss");
  if (index != std::string::npos)
  {
    applicationPath = applicationPath.substr(index);
  }

  out << QString("export PATH=" + QString::fromStdString(applicationPath) + ":$PATH\n" +
                 "source setupEnv.sh\n");
  out << getDepthConversionCommand() + "\n";
  out << QString("EOF\n\n");
  out << QString("bsub -Is < runt2zCluster.sh\n");
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
                   " -noofpp" +
                   " -preserveErosion" +
                   " -noCalculatedTWToutput");
}

} // namespace sac

} // namespace casaWizard
