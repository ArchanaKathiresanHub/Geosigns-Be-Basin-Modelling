// Script to run the design of experiment
#pragma once

#include "model/script/uaScript.h"

class QFile;

namespace casaWizard
{

namespace ua
{

class DoeOption;
class UAScenario;
struct InfluentialParameter;

class DoEScript : public UAScript
{
public:
  explicit DoEScript(UAScenario& scenario);

private:
  void writeScriptContents(QFile& file) const override;
  bool validateScenario() const override;
  QString scriptFilename() const override;

  QString writeInfluentialParameter(const InfluentialParameter* influentialParameterSelected) const;
  QString writeDOE(const QVector<DoeOption*>& doeOptions) const;
};

} // namespace ua

} // namespace casaWizard
