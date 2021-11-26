// Script to run the uncertainty analysis
#pragma once

#include "model/script/casaScript.h"

#include <QMap>

class QFile;
class QTextStream;

namespace casaWizard
{

struct CalibrationTarget;
struct Well;

namespace ua
{

class Proxy;
class UAScenario;
struct PredictionTarget;

class UAScript : public CasaScript
{
public:
  explicit UAScript(const UAScenario& scenario);
  const CasaScenario& scenario() const override;
  QString workingDirectory() const override;

protected:
  const UAScenario& uaScenario() const;
  QString writeResponse(const QString& doeList) const;
  QString writeExportDataTxt(const QString& exportDataType, const QString& textFileName,
                                   const QString& doeList = "", const QString& proxyName = "") const;
  void writeTargets(QTextStream& out, bool prediction = true) const;
  QString writeAddDesignPoint(const QVector<double> parameters) const;

  QString proxyName() const;
  QMap<int, QString> proxyRSPOrderNumberToTextMap() const;

private:
  bool validateScenario() const override;
  const UAScenario& scenario_;
  const Proxy& proxy_;

  QString writeCalibrationTarget(const CalibrationTarget& target, const Well* well) const;
  QString writePredictionTarget(const PredictionTarget& predictionTarget) const;
};

} // namespace ua

} // namespace casaWizard
