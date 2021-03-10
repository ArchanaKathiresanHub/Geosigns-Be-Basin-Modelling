#include "depthConversionScript.h"

#include "model/logger.h"
#include "model/sacScenario.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>

namespace casaWizard
{

namespace sac
{

DepthConversionScript::DepthConversionScript(const SACScenario& scenario, const QString& baseDirectory) :
  RunScript(baseDirectory),
  scenario_{scenario},
  scriptFilename_{scenario.workingDirectory() + "/T2Z_step2/runt2z.sh"}
{
}

bool DepthConversionScript::generateCommands()
{
  if (scenario_.clusterName().toLower() == "local")
  {
    addCommand("fastdepthconversion"
               " -project " + QFileInfo(scenario_.project3dPath()).fileName() + " -temperature -onlyat 0"
               " -referenceSurface " + QString::number(scenario_.referenceSurface()) +
               " -endSurface " + QString::number(scenario_.lastSurface()));
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

    addCommand("bsub -K < " + scriptFilename_);
  }
  return true;
}

void DepthConversionScript::writeScriptContents(QFile& file) const
{
  QTextStream out(&file);

  out << QString("#!/bin/bash -lx\n");

  out << QString("#BSUB -P cldrn\n");
  out << QString("#BSUB -W 0:30\n");
  out << QString("#BSUB -J \"Fastcauldron T2Z conversion run\"\n");
  out << QString("#BSUB -n " + QString::number(scenario_.numberCPUs()) + "\n");
  out << QString("#BSUB -o output.log\n");
  out << QString("#BSUB -x\n");
  out << QString("#BSUB -cwd " + scenario_.workingDirectory() +"T2Z_step2\n");

  out << QString("mpirun_wrap.sh -n " + QString::number(scenario_.numberCPUs()) +
                 " -outfile-pattern 'fastdepthconversion-output-rank-%r.log'"
                 " fastdepthconversion"
                 " -project " + QFileInfo(scenario_.project3dPath()).fileName() + " -temperature -onlyat 0" +
                 " -referenceSurface " + QString::number(scenario_.referenceSurface()) +
                 " -endSurface " + QString::number(scenario_.lastSurface()) +
                 " -noofpp" +
                 "\n");
}

} // namespace sac

} // namespace casaWizard
