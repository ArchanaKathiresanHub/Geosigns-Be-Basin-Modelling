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

private:
  void writeScriptContents(QFile& file) const override;
  QString writeMC(const QString& MCType, const QString& nSamples, const QString& nMaximumIterations, const QString& proxyEvaluationType,
                  const QString& distributionTypePDFVariables, const QString& standardDeviationFactor) const;
};

} // namespace ua

} // namespace casaWizard
