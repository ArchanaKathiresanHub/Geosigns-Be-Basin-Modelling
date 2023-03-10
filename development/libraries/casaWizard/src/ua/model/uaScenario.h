// Data class for the UA Thermal Wizard
#pragma once

#include "influentialParameterManager.h"
#include "manualDesignPointManager.h"
#include "model/casaScenario.h"
#include "model/McmcSettings.h"
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
   QString updateStateFileNameDoE();
   void setStateFileNameDoE(const QString& stateFileNameDoE);

   QString stateFileNameQC() const;
   void setStateFileNameQC(const QString& stateFileNameQC);

   QString stateFileNameMCMC() const;
   void setStateFileNameMCMC(const QString& stateFileNameMCMC);

   const QString mcTextFileName() const;
   void setMCtextFileName(const QString& mcTextFileName);

   QString doeTextFileName() const;
   QString simStatesTextFileName() const;
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

   void setMcmcSettings(const McmcSettings& mcmcSettings);
   const McmcSettings& mcmcSettings() const;

   void copyToIterationDir(const QString& fileName) const;
   void updateIterationDir();
   bool iterationDirExists() const;

   QVector<TargetQC> targetQCs() const;
   void clearTargetQCs();
   void setTargetQCs(const QVector<TargetQC>& targetQCs);
   void setOptimalValuesTargetQCs(const QVector<double>& values, const QVector<QString> colNames);

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

   double responseSurfacesRsmeBestMC() const;
   void setNumberOfManualDesignPoints();

   void writeToFile(ScenarioWriter& writer) const override;
   void readFromFile(const ScenarioReader& reader) override;
   void clear() override;
   QString iterationDirName() const override;

   void loadProject3dFile() const override;
   const CMBMapReader& mapReader() const;

   void setStageComplete(const StageTypesUA& stageType, bool isComplete = true);
   bool isStageComplete(const StageTypesUA& stageType) const;

   bool isStageUpToDate(const StageTypesUA& stageType) const;
   void setStageUpToDate(const StageTypesUA& stageType, bool isUpToDate = true);

   void changeUserDefinedPointStatus(const bool status);

   void obtainTimeSeriesMonteCarloData(const int targetIndex, QVector<double>& snapshotAges,
                                       QMap<QString, QVector<double>>& bestMatchedValuesPerProperty,
                                       QMap<QString, QVector<QVector<double>>>& currentPredTargetMatrixPerProperty) const;

   void obtainMonteCarloDataForTimeStep(const int targetIndex, const int timeStep, QVector<QVector<double> >& data) const;

   int subSamplingFactor() const;
   void setSubSamplingFactor(const int subSamplingFactor);

   int baseSubSamplingFactor() const;
   void initializeBaseSubSamplingFactor();

private:
   void selectDoEOptionBaseCase();

   QString m_stateFileNameDoEBase;
   QString m_stateFileNameDoE;
   QString m_stateFileNameQC;
   QString m_stateFileNameMCMC;
   const QString m_runCasesObservablesTextFileName;
   const QString m_doeIndicesTextFileName;
   const QString m_proxyEvaluationObservablesTextFileName;
   const QString m_proxyQualityEvaluationTextFileName;
   QString m_doeTextFileName;
   QString m_simStatesTextFileName;
   QString m_mcTextFileName;
   Proxy m_proxy;
   std::unique_ptr<CMBMapReader> m_cmbMapReader;
   std::unique_ptr<ToDepthConverter> m_toDepthConverter;
   InfluentialParameterManager m_influentialParameterManager;
   PredictionTargetManager m_predictionTargetManager;
   MonteCarloDataManager m_monteCarloDataManager;
   ManualDesignPointManager m_manualDesignPointManager;
   RunCaseSetFileManager m_runCaseSetFileManager;
   McmcSettings m_mcmcSettings;

   QVector<DoeOption*> m_doeOptions;

   QVector<TargetQC> m_targetQCs;
   QVector<bool> m_isDoeOptionSelected;
   QVector<bool> m_isQcDoeOptionSelected;

   StageStateUA m_isStageComplete;
   StageStateUA m_isStageUpToDate; //Tracks if the state of the stage is consistent with that if preceding stages.
   int m_subSamplingFactor;
   int m_baseCaseSubSamplingFactor;
};

} // namespace ua

} // namespace casaWizard
