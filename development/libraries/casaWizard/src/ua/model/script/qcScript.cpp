#include "qcScript.h"

#include "model/logger.h"
#include "model/uaScenario.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>

namespace casaWizard
{

namespace ua
{

QCScript::QCScript(UAScenario& scenario) :
  UAScript{scenario}
{
}

QString QCScript::scriptFilename() const
{
  return QString("qcScript.casa");
}

void QCScript::writeScriptContents(QFile& file) const
{
  QTextStream out(&file);

  out << writeBaseProject(uaScenario().project3dFilename());
  out << writeLoadState(uaScenario().runLocation() + "/" + uaScenario().iterationDirName() + "/" + uaScenario().stateFileNameDoE());

  writeTargets(out);

  const QString doeList = writeDoeTextList(uaScenario().qcDoeOptionSelectedNames());
  out << writeResponse(doeList);

  out << writeExportDataTxt("DoeIndices", uaScenario().doeIndicesTextFileName());
  out << writeExportDataTxt("RunCasesSimulationStates", uaScenario().simStatesTextFileName());
  out << writeExportDataTxt("RunCasesObservables", uaScenario().runCasesObservablesTextFileName());
  out << writeExportDataTxt("ProxyEvalObservables", uaScenario().proxyEvaluationObservablesTextFileName(), doeList, proxyName());
  out << writeExportDataTxt("ProxyEvaluateQuality", uaScenario().proxyQualityEvaluationTextFileName(), doeList, proxyName());
  out << writeSaveState(uaScenario().stateFileNameQC());
}

} // namespace ua

} // namespace casaWizard
