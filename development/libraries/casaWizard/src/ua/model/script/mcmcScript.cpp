#include "mcmcScript.h"

#include "model/uaScenario.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>

namespace casaWizard
{

namespace ua
{

McmcScript::McmcScript(UAScenario& scenario) :
  UAScript{scenario},
  m_mcmcSettings(scenario.mcmcSettings())
{}

QString McmcScript::scriptFilename() const
{
  return QString("mcmcScript.casa");
}

void McmcScript::writeScriptContents(QFile& file) const
{
  QTextStream out(&file);

  out << writeBaseProject(uaScenario().project3dFilename());
  out << writeLoadState(uaScenario().runLocation() + "/" + uaScenario().iterationDirName() + "/" + uaScenario().stateFileNameQC());

  const QString doeList = writeDoeTextList(uaScenario().qcDoeOptionSelectedNames());

  const QString MCType = "MCMC"; // MC/MCMC/MCSolver
  const QString nSamples = QString::number(m_mcmcSettings.nSamples());
  const QString nMaximumIterations = "100";
  const QString proxyEvaluationType = "Polynomial"; // Polynomial/GlobalKriging
  const QString distributionTypePDFVariables = "NoPrior"; // NoPrior/MarginalPrior/MultivariatePrior
  const QString standardDeviationFactor = QString::number(m_mcmcSettings.standardDeviationFactor());

  out << writeMC(MCType, nSamples, nMaximumIterations, proxyEvaluationType, distributionTypePDFVariables, standardDeviationFactor);
  out << writeExportDataTxt("DoEParameters", uaScenario().doeTextFileName(), doeList);

  if (uaScenario().mcTextFileName().isEmpty())
  {
    throw std::runtime_error("MC text file name not found. Failed to write casa script");
  }

  out << writeExportDataTxt("MCResults", uaScenario().mcTextFileName(), doeList);
  out << writeSaveState(uaScenario().stateFileNameMCMC());
}

QString McmcScript::writeMC(const QString& MCType, const QString& nSamples, const QString& nMaximumIterations, const QString& proxyEvaluationType,
                            const QString& distributionTypePDFVariables, const QString& standardDeviationFactor) const
{
  return QString("montecarlo " + proxyName() + " " + MCType + " " + nSamples + " " + nMaximumIterations + " " +
                 proxyEvaluationType + " " + distributionTypePDFVariables + " " + standardDeviationFactor + "\n");
}

} // namespace ua

} // namespace casaWizard
