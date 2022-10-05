// Script to run the uncertainty analysis
#pragma once

#include "uaScript.h"

class QFile;

namespace casaWizard
{

namespace ua
{

class McmcSettings;
class Proxy;
class UAScenario;

class McmcScript : public UAScript
{
public:
  explicit McmcScript(UAScenario& scenario);
  QString scriptFilename() const final;

private:
  void writeScriptContents(QFile& file) const final;
  QString writeMC(const QString& MCType, const QString& nSamples, const QString& nMaximumIterations, const QString& proxyEvaluationType,
                  const QString& distributionTypePDFVariables, const QString& standardDeviationFactor) const;

   const McmcSettings& m_mcmcSettings;
};

} // namespace ua

} // namespace casaWizard
