// Script to run the uncertainty analysis
#pragma once

#include "uaScript.h"

class QFile;

namespace casaWizard
{

namespace ua
{

class Proxy;
class UAScenario;

class McmcScript : public UAScript
{
public:
  explicit McmcScript(const UAScenario& scenario);
  QString scriptFilename() const final;

private:
  void writeScriptContents(QFile& file) const final;
  QString writeMC(const QString& MCType, const QString& nSamples, const QString& nMaximumIterations, const QString& proxyEvaluationType,
                  const QString& distributionTypePDFVariables, const QString& standardDeviationFactor) const;
};

} // namespace ua

} // namespace casaWizard
