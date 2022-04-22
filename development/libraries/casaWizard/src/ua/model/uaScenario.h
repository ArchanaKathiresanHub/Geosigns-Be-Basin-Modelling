// Data class for the UA Thermal Wizard
#pragma once

#include "influentialParameterManager.h"
#include "manualDesignPointManager.h"
#include "model/casaScenario.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"
#include "model/stagesUA.h"
#include "model/targetQC.h"
#include "model/output/runCaseSetFileManager.h"
#include "monteCarloDataManager.h"
#include "predictionTargetManager.h"
#include "proxy.h"

namespace casaWizard
{

class ToDepthConverter;
class CMBMapReader;

namespace ua
{

class DoeOption;
class InfluentialParameter;

class UAScenario : public CasaScenario
{
public:
  UAScenario(ProjectReader* projectReader);
  virtual ~UAScenario() override;

  QVector<DoeOption*> doeSelected() const;
  QVector<DoeOption*> doeOptions() const;

  void updateDoeArbitraryNDesignPoints(const int row, const int nDesignPoints);
  void updateDoeConstantNumberOfDesignPoints(const int totalNumberOfInfluentialParameters);

  QString stateFileNameDoE() const;
  void setStateFileNameDoE(const QString& stateFileNameDoE);

  QString stateFileNameQC() const;
  void setStateFileNameQC(const QString& stateFileNameQC);

  QString stateFileNameMCMC() const;
  void setStateFileNameMCMC(const QString& stateFileNameMCMC);

  const QString mcTextFileName() const;
  void setMCtextFileName(const QString& mcTextFileName);

  QString doeTextFileName() const;
  QString runCasesObservablesTextFileName() const;
  QString doeIndicesTextFileName() const;
  QString proxyEvaluationObservablesTextFileName() const;
  QString proxyQualityEvaluationTextFileName() const;

  InfluentialParameterManager& influentialParameterManager();
  const InfluentialParameterManager& influentialParameterManager() const;

  PredictionTargetManager& predictionTargetManager();
  const PredictionTargetManager& predictionTargetManager() const;

  QVector<int> predictionDataObservablesIndexRange() const;
  QVector<int> calibrationDataObservablesIndexRange() const;

  MonteCarloDataManager& monteCarloDataManager();
  const MonteCarloDataManager& monteCarloDataManager() const;

  ManualDesignPointManager& manualDesignPointManager();
  const ManualDesignPointManager& manualDesignPointManager() const;

  RunCaseSetFileManager& runCaseSetFileManager();
  const RunCaseSetFileManager& runCaseSetFileManager() const;

  QVector<TargetQC> targetQCs() const;
  void setTargetQCs(const QVector<TargetQC>& targetQCs);

  QVector<double> calibrationTargetDataBestMC() const;
  QVector<double> predictionTargetDataBestMC() const;
  QVector<InfluentialParameter*> influentialParametersWithRunData();

  QVector<bool> isDoeOptionSelected() const;
  void setIsDoeOptionSelected(const int row, const bool isSelected);
  QVector<int> indicesOfSelectedQCDoeOptionsInSelectedDoeOptions();
  QStringList doeOptionSelectedNames() const;
  QStringList qcDoeOptionSelectedNames() const;

  QVector<bool> isQcDoeOptionSelected() const;
  void setIsQcDoeOptionSelected(const int row, const bool isSelected);

  void setProxyOrder(int order);
  void setProxyKrigingMethod(const QString& krigingMethod);

  const Proxy& proxy() const;

  double responseSurfacesL2NormBestMC() const;
  void setNumberOfManualDesignPoints();

  void writeToFile(ScenarioWriter& writer) const override;
  void readFromFile(const ScenarioReader& reader) override;
  void clear() override;
  QString iterationDirName() const override;

  void loadProject3dFile() const override;
  const CMBMapReader& mapReader() const;

  void setStageComplete(const StageTypesUA& stageType, bool isComplete = true);
  bool isStageComplete(const StageTypesUA& stageType) const;

  void changeUserDefinedPointStatus(const bool status);

  void obtainTimeSeriesMonteCarloData(const int targetIndex, QVector<double>& snapshotAges,
                                QMap<QString, QVector<double>>& bestMatchedValuesPerProperty,
                                QMap<QString, QVector<QVector<double>>>& currentPredTargetMatrixPerProperty) const;

  void obtainMonteCarloDataForTimeStep(const int targetIndex, const int timeStep, QVector<QVector<double> >& data) const;
private:
  QString stateFileNameDoE_;
  QString stateFileNameQC_;
  QString stateFileNameMCMC_;
  const QString runCasesObservablesTextFileName_;
  const QString doeIndicesTextFileName_;
  const QString proxyEvaluationObservablesTextFileName_;
  const QString proxyQualityEvaluationTextFileName_;
  QString doeTextFileName_;
  QString mcTextFileName_;
  Proxy proxy_;
  std::unique_ptr<CMBMapReader> cmbMapReader_;
  std::unique_ptr<ToDepthConverter> m_toDepthConverter;
  InfluentialParameterManager influentialParameterManager_;
  PredictionTargetManager predictionTargetManager_;
  MonteCarloDataManager monteCarloDataManager_;
  ManualDesignPointManager manualDesignPointManager_;
  RunCaseSetFileManager runCaseSetFileManager_;

  QVector<DoeOption*> doeOptions_;

  QVector<TargetQC> targetQCs_;
  QVector<bool> isDoeOptionSelected_;
  QVector<bool> isQcDoeOptionSelected_;

  StageCompletionUA isStageComplete_;
};

} // namespace ua

} // namespace casaWizard
