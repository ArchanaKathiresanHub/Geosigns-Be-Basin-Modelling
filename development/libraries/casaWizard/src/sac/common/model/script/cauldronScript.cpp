#include "cauldronScript.h"

#include "model/SacScenario.h"

#include <QFile>
#include <QTextStream>

namespace casaWizard
{

namespace sac
{

class SacScenario;
CauldronScript::CauldronScript(const SacScenario& scenario, const QString& baseDirectory) :
  CasaScript(baseDirectory),
  scenario_{scenario}
{
}

QString CauldronScript::scriptFilename() const
{
  return QString("cauldronScript.casa");
}

QString CauldronScript::workingDirectory() const
{
  return QString(scenario_.workingDirectory());
}

const CasaScenario& CauldronScript::scenario() const
{
  return scenario_;
}

void CauldronScript::writeScriptContents(QFile& file) const
{
  QTextStream out(&file);

  out << writeApp(scenario_.numberCPUs(), scenario_.applicationName() + " \"-allproperties\" \"-onlyat 0\"");
  out << writeBaseProject(scenario_.project3dFilename());
  out << "runPipeline \"" + scenario_.clusterName() + "\" \"Default\"\n";
}

bool CauldronScript::validateScenario() const
{
  return true;
}

}  // namespace sac

}  // namespace casaWizard
