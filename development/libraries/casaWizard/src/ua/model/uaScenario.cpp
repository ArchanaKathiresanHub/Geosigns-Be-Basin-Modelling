#include "uaScenario.h"

#include "doeOption.h"
#include "influentialParameter.h"
#include "model/calibrationTargetManager.h"
#include "model/doeOptionImpl.h"
#include "model/functions/rmseCalibrationTargets.h"
#include "model/input/cmbMapReader.h"
#include "model/input/projectReader.h"
#include "model/SurfaceToDepthConverter.h"
#include "model/logger.h"
#include "model/output/runCaseSetFileManager.h"
#include "model/output/workspaceGenerator.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>

namespace casaWizard
{

namespace
{
int s_stateFileIncrement(0); //To ensure that two state files never have the same name, even if created within the same second.
}

namespace ua
{

UAScenario::UAScenario(ProjectReader* projectReader) :
      CasaScenario(projectReader),
      m_stateFileNameDoEBase{"casaStateDoE.txt"},
      m_stateFileNameDoE{"casaStateDoE.txt"},
      m_stateFileNameQC{"casaStateQC.txt"},
      m_stateFileNameMCMC{"casaStateMCMC.txt"},
      m_runCasesObservablesTextFileName{"runCasesObservables.txt"},
      m_doeIndicesTextFileName{"doeIndices.txt"},
      m_proxyEvaluationObservablesTextFileName{"proxyEvalObservables.txt"},
      m_proxyQualityEvaluationTextFileName{"proxyEvalQuality.txt"},
      m_doeTextFileName{"doeResults.txt"},
      m_simStatesTextFileName{"RunCasesSimulationStates.txt"},
      m_mcTextFileName{"mcResults.txt"},
      m_proxy{},
      m_cmbMapReader(new CMBMapReader()),
      m_toDepthConverter(new SurfaceToDepthConverter(CasaScenario::projectReader(),*m_cmbMapReader)),
      m_influentialParameterManager{CasaScenario::projectReader()},
      m_predictionTargetManager{CasaScenario::projectReader(),*m_toDepthConverter},
      m_monteCarloDataManager{},
      m_manualDesignPointManager{},
      m_runCaseSetFileManager{},
      m_doeOptions{DoeOption::getDoeOptions()},
      m_targetQCs{},
      m_isDoeOptionSelected{QVector<bool>(m_doeOptions.size(), false)},
      m_isQcDoeOptionSelected{},
      m_isStageComplete{},
      m_isStageUpToDate("IsStageUpToDate",true),
      m_subSamplingFactor{1},
      m_baseCaseSubSamplingFactor{1}
{
   //Select the base case by default:
   selectDoEOptionBaseCase();
}

UAScenario::~UAScenario()
{
   for (DoeOption* doe : m_doeOptions)
   {
      delete doe;
   }
}

void UAScenario::selectDoEOptionBaseCase()
{
   for (int i = 0; i < m_doeOptions.size(); i++)
   {
      if (m_doeOptions[i]->name() == "BaseCase")
      {
         setIsDoeOptionSelected(i,true);
         break;
      }
   }
}

const Proxy& UAScenario::proxy() const
{
   return m_proxy;
}

double UAScenario::responseSurfacesRsmeBestMC() const
{
   return functions::rmseCalibrationTargets(calibrationTargetDataBestMC(),
                                            calibrationTargetManager(),
                                            mcmcSettings().standardDeviationFactor());
}

void UAScenario::setNumberOfManualDesignPoints()
{
   if (m_manualDesignPointManager.numberOfVisiblePoints() == 0)
   {
      changeUserDefinedPointStatus(false);
   }

   const int n = m_manualDesignPointManager.numberOfVisiblePoints();
   for (DoeOption* option : m_doeOptions)
   {
      if (dynamic_cast<DoeUserDefined*>(option))
      {
         option->setArbitraryNDesignPoints(n);
      }
   }
}

void UAScenario::changeUserDefinedPointStatus(const bool status)
{
   const int points = m_manualDesignPointManager.numberOfVisiblePoints();

   if ((points != 0 &&
        !status)      ||
       (points == 0  &&
        status))
   {
      return;
   }

   int pos = 0;

   for (DoeOption* option : m_doeOptions)
   {
      if (dynamic_cast<DoeUserDefined*>(option))
      {
         setIsDoeOptionSelected(pos, status);
         return;
      }
      pos++;
   }
}

void UAScenario::obtainTimeSeriesMonteCarloData(const int targetIndex, QVector<double>& snapshotAges,
                                                QMap<QString,QVector<double> >& bestMatchedValuesPerProperty,
                                                QMap<QString,QVector<QVector<double>>>& currentPredTargetMatrixPerProperty) const
{
   snapshotAges = projectReader().agesFromMajorSnapshots();

   const MonteCarloDataManager& monteCarloData = monteCarloDataManager();
   const QVector<QVector<double>>& predTargetMatrix = monteCarloData.predictionTargetMatrix();
   const QVector<const PredictionTarget*> targets = predictionTargetManager().predictionTargets();

   int propertyCount = 0;
   for (const QString& propertyName : predictionTargetManager().predictionTargetOptions())
   {
      if (targets[targetIndex]->properties().contains(propertyName))
      {
         for (int snapshotIndex = 0; snapshotIndex < snapshotAges.size(); snapshotIndex++)
         {
            const int matrixIndex = predictionTargetManager().getIndexInPredictionTargetMatrix(targetIndex, snapshotIndex, propertyName);
            currentPredTargetMatrixPerProperty[propertyName].append(predTargetMatrix[matrixIndex]);
         }
      }
      propertyCount++;
   }

   for (const QString& propertyName : predictionTargetManager().predictionTargetOptions())
   {
      if (targets[targetIndex]->properties().contains(propertyName))
      {
         for (const QVector<double>& predTarget : currentPredTargetMatrixPerProperty[propertyName])
         {
            bestMatchedValuesPerProperty[propertyName].push_back(predTarget[0]);
         }
      }
   }
}

void UAScenario::obtainMonteCarloDataForTimeStep(const int targetIndex, const int timeStep, QVector<QVector<double> >& data) const
{
   const MonteCarloDataManager& monteCarloData = monteCarloDataManager();
   const PredictionTarget* targetAtIndex = predictionTargetManager().predictionTargets()[targetIndex];

   int propertyCounter = 0;
   for (const QString& predictionTargetOption : predictionTargetManager().predictionTargetOptions())
   {
      if (targetAtIndex->properties().contains(predictionTargetOption))
      {
         const int matrixIndex = predictionTargetManager().getIndexInPredictionTargetMatrix(targetIndex, timeStep, predictionTargetOption);
         data.append(monteCarloData.predictionTargetMatrix()[matrixIndex]);
         propertyCounter++;
      }
      else
      {
         data.append(QVector<double>());
      }
   }
}

int UAScenario::subSamplingFactor() const
{
   return m_subSamplingFactor;
}

void UAScenario::setSubSamplingFactor(const int subSampling)
{
   m_subSamplingFactor = subSampling;
}

int UAScenario::baseSubSamplingFactor() const
{
   return m_baseCaseSubSamplingFactor;
}

void UAScenario::initializeBaseSubSamplingFactor()
{
   m_baseCaseSubSamplingFactor = projectReader().subSamplingFactor();
}

void UAScenario::setProxyOrder(int order)
{
   m_proxy.setOrder(order);
}

void UAScenario::setProxyKrigingMethod(const QString& krigingMethod)
{
   m_proxy.setKrigingMethod(krigingMethod);
}

QVector<DoeOption*> UAScenario::doeSelected() const
{
   QVector<DoeOption*> doeOptionsSelected;

   int i = 0;
   for (const bool& isSelected : m_isDoeOptionSelected)
   {
      if (isSelected)
      {
         doeOptionsSelected.push_back(m_doeOptions[i]);
      }
      ++i;
   }

   return doeOptionsSelected;
}

QStringList UAScenario::doeOptionSelectedNames() const
{
   QStringList doeOptionSelectedNames;

   int i = 0;
   for (const bool& isSelected : m_isDoeOptionSelected)
   {
      if (isSelected)
      {
         doeOptionSelectedNames.append(m_doeOptions[i]->name());
      }
      ++i;
   }

   return doeOptionSelectedNames;
}

QStringList UAScenario::qcDoeOptionSelectedNames() const
{
   QStringList qcDoeOptionSelectedNames;

   int i = 0;
   for (const bool& isSelected : m_isQcDoeOptionSelected)
   {
      if (isSelected)
      {
         qcDoeOptionSelectedNames.append(doeOptionSelectedNames()[i]);
      }
      ++i;
   }

   return qcDoeOptionSelectedNames;
}

QVector<bool> UAScenario::isDoeOptionSelected() const
{
   return m_isDoeOptionSelected;
}

QVector<DoeOption*> UAScenario::doeOptions() const
{
   return m_doeOptions;
}

void UAScenario::setIsDoeOptionSelected(const int row, const bool isSelected)
{
   m_isDoeOptionSelected[row] = isSelected;

   m_isQcDoeOptionSelected.clear();
   for (const bool& isDoeInDoeTabSelected : m_isDoeOptionSelected)
   {
      if (isDoeInDoeTabSelected)
      {
         m_isQcDoeOptionSelected.append(true);
      }
   }
}

QVector<int> UAScenario::indicesOfSelectedQCDoeOptionsInSelectedDoeOptions()
{
   const QStringList namesDoeOptionsSelected = doeOptionSelectedNames();
   QVector<int> indQCDoeOptionsSelInDoeOptionTable;
   for (const QString& qcDoeOptionName : qcDoeOptionSelectedNames())
   {
      QStringList::const_iterator it = std::find(namesDoeOptionsSelected.begin(), namesDoeOptionsSelected.end(), qcDoeOptionName);

      if (it == namesDoeOptionsSelected.end())
      {
         indQCDoeOptionsSelInDoeOptionTable.push_back(-1);
         continue;
      }
      indQCDoeOptionsSelInDoeOptionTable.push_back(std::distance<QStringList::const_iterator>(namesDoeOptionsSelected.begin(), it));
   }

   return indQCDoeOptionsSelInDoeOptionTable;
}

QVector<bool> UAScenario::isQcDoeOptionSelected() const
{
   return m_isQcDoeOptionSelected;
}

void UAScenario::setIsQcDoeOptionSelected(const int row, const bool isSelected)
{
   m_isQcDoeOptionSelected[row] = isSelected;
}

void UAScenario::updateDoeArbitraryNDesignPoints(const int row, const int nDesignPoints)
{
   m_doeOptions[row]->setArbitraryNDesignPoints(nDesignPoints);
}

void UAScenario::updateDoeConstantNumberOfDesignPoints(const int totalNumberOfInfluentialParameters)
{
   for (DoeOption* doe: m_doeOptions)
   {
      doe->calculateNDesignPoints(totalNumberOfInfluentialParameters);
   }
}

QString UAScenario::stateFileNameDoE() const
{
   return m_stateFileNameDoE;
}

QString UAScenario::updateStateFileNameDoE()
{
   QString newStateFileName = m_stateFileNameDoEBase;

   QFileInfo fi(newStateFileName);
   newStateFileName = fi.baseName();
   newStateFileName = newStateFileName
                      + "-"
                      + workspaceGenerator::getTimeStamp()
                      + "_"
                      + QString::number(s_stateFileIncrement)
                      + ".txt";
   s_stateFileIncrement++;
   setStateFileNameDoE(newStateFileName);
   return newStateFileName;
}

void UAScenario::setStateFileNameDoE(const QString &stateFilePathDoE)
{
   m_stateFileNameDoE = stateFilePathDoE;
}

QString UAScenario::stateFileNameQC() const
{
   return m_stateFileNameQC;
}

void UAScenario::setStateFileNameQC(const QString& stateFileNameQC)
{
   m_stateFileNameQC = stateFileNameQC;
}

QString UAScenario::stateFileNameMCMC() const
{
   return m_stateFileNameMCMC;
}

void UAScenario::setStateFileNameMCMC(const QString& stateFilePathMCMC)
{
   m_stateFileNameMCMC = stateFilePathMCMC;
}

QString UAScenario::doeTextFileName() const
{
   return m_doeTextFileName;
}

QString UAScenario::simStatesTextFileName() const
{
   return m_simStatesTextFileName;
}

QString UAScenario::runCasesObservablesTextFileName() const
{
   return m_runCasesObservablesTextFileName;
}

QString UAScenario::doeIndicesTextFileName() const
{
   return m_doeIndicesTextFileName;
}

QString UAScenario::proxyEvaluationObservablesTextFileName() const
{
   return m_proxyEvaluationObservablesTextFileName;
}

QString UAScenario::proxyQualityEvaluationTextFileName() const
{
   return m_proxyQualityEvaluationTextFileName;
}

InfluentialParameterManager& UAScenario::influentialParameterManager()
{
   return m_influentialParameterManager;
}

const InfluentialParameterManager& UAScenario::influentialParameterManager() const
{
   return m_influentialParameterManager;
}

PredictionTargetManager& UAScenario::predictionTargetManager()
{
   return m_predictionTargetManager;
}

const PredictionTargetManager& UAScenario::predictionTargetManager() const
{
   return m_predictionTargetManager;
}

MonteCarloDataManager& UAScenario::monteCarloDataManager()
{
   return m_monteCarloDataManager;
}

const MonteCarloDataManager& UAScenario::monteCarloDataManager() const
{
   return m_monteCarloDataManager;
}

ManualDesignPointManager& UAScenario::manualDesignPointManager()
{
   return m_manualDesignPointManager;
}

const ManualDesignPointManager& UAScenario::manualDesignPointManager() const
{
   return m_manualDesignPointManager;
}

RunCaseSetFileManager& UAScenario::runCaseSetFileManager()
{
   return m_runCaseSetFileManager;
}

const RunCaseSetFileManager& UAScenario::runCaseSetFileManager() const
{
   return m_runCaseSetFileManager;
}

void UAScenario::setMcmcSettings(const McmcSettings& mcmcSettings)
{
   m_mcmcSettings = mcmcSettings;
}

const McmcSettings& UAScenario::mcmcSettings() const
{
   return m_mcmcSettings;
}

QVector<TargetQC> UAScenario::targetQCs() const
{
   return m_targetQCs;
}

void UAScenario::clearTargetQCs()
{
   m_targetQCs.clear();
}

void UAScenario::setTargetQCs(const QVector<TargetQC>& targetQCs)
{
   m_targetQCs = targetQCs;
}

void UAScenario::setOptimalValuesTargetQCs(const QVector<double>& values, const QVector<QString> colNames)
{
   if (colNames.size() == m_targetQCs.size() && values.size() == colNames.size())
   {
      for (int i = 0; i < m_targetQCs.size(); i++)
      {
         for (int j = 0; j < colNames.size(); j++)
         {
            TargetQC& targetQc = m_targetQCs[i];
            if (targetQc.identifier() == colNames[j])
            {
               targetQc.setValOptimalSim(values.at(j));
               break;
            }
         }
      }
   }
   else
   {
      Logger::log() << "Mismatch in optimal values and targetQCs size. Could not store optimal values." << Logger::endl();
   }
}

// This method returns the calibration target values of best matching MCMC data point (which is the first one)
QVector<double> UAScenario::calibrationTargetDataBestMC() const
{
   QVector<double> calibrationTargetBestMCData;
   const QVector<QVector<double>> mcDataCalibration = m_monteCarloDataManager.calibrationTargetMatrix();
   const int amountOfActiveCalibTargets = calibrationTargetManager().amountOfActiveCalibrationTargets();
   if (mcDataCalibration.isEmpty() || mcDataCalibration.size() != amountOfActiveCalibTargets)
   {
      return calibrationTargetBestMCData;
   }

   for (int i = 0; i < amountOfActiveCalibTargets; ++i)
   {
      calibrationTargetBestMCData.push_back(m_monteCarloDataManager.calibrationTargetMatrix()[i][0]);
   }
   return calibrationTargetBestMCData;
}

// This method returns the prediction target values of best matching MCMC data point (which is the first one)
QVector<double> UAScenario::predictionTargetDataBestMC() const
{
   if (calibrationTargetManager().amountOfActiveCalibrationTargets() == 0)
   {
      return {}; //Best matching point is only defined if there is calibration data. Otherwise RMSE is just zero for all cases.
   }

   QVector<double> predictionTargetBestMCData;
   const QVector<QVector<double>> mcDataPrediction = m_monteCarloDataManager.predictionTargetMatrix();
   const int amountOfPredictionTargets = m_predictionTargetManager.amountOfPredictionTargetWithTimeSeriesAndProperties();
   if (mcDataPrediction.isEmpty() || mcDataPrediction.size() != amountOfPredictionTargets)
   {
      return predictionTargetBestMCData;
   }

   for (int i = 0; i < amountOfPredictionTargets; ++i)
   {
      predictionTargetBestMCData.push_back(mcDataPrediction[i][0]);
   }
   return predictionTargetBestMCData;
}

QVector<int> UAScenario::predictionDataObservablesIndexRange() const
{
   const int endCalibration = calibrationTargetManager().amountOfActiveCalibrationTargets();
   const int endPrediction = endCalibration + predictionTargetManager().amountOfPredictionTargetWithTimeSeriesAndProperties();

   QVector<int> range;
   for (int i = endCalibration; i < endPrediction; i++)
   {
      range.push_back(i);
   }
   return range;
}

QVector<int> UAScenario::calibrationDataObservablesIndexRange() const
{
   const int endCalibration = calibrationTargetManager().amountOfActiveCalibrationTargets();

   QVector<int> range;
   for (int i = 0; i < endCalibration; i++)
   {
      range.push_back(i);
   }
   return range;
}

const QString UAScenario::mcTextFileName() const
{
   return m_mcTextFileName;
}

void UAScenario::setMCtextFileName(const QString& mcTextFileName)
{
   m_mcTextFileName = mcTextFileName;
}

QVector<InfluentialParameter*> UAScenario::influentialParametersWithRunData()
{
   const int sizeRunInfluentialParameters = m_monteCarloDataManager.influentialParameterMatrix().size();

   if (sizeRunInfluentialParameters > m_influentialParameterManager.influentialParameters().size())
   {
      Logger::log() << "Incompatible data size of the found influential parameters and the selected ones." << Logger::endl();
      return m_influentialParameterManager.influentialParameters();
   }

   return m_influentialParameterManager.influentialParameters().mid(0, sizeRunInfluentialParameters);
}

void UAScenario::writeToFile(ScenarioWriter& writer) const
{
   CasaScenario::writeToFile(writer);
   writer.writeValue("UAScenarioVersion", 9);
   m_influentialParameterManager.writeToFile(writer);
   m_predictionTargetManager.writeToFile(writer);
   m_monteCarloDataManager.writeToFile(writer);
   m_manualDesignPointManager.writeToFile(writer);
   m_runCaseSetFileManager.writeToFile(writer);
   m_isStageComplete.writeToFile(writer);
   m_isStageUpToDate.writeToFile(writer);
   m_proxy.writeToFile(writer);
   m_mcmcSettings.writeToFile(writer);

   writer.writeValue("targetQC", m_targetQCs);

   writer.writeValue("isDoeSelected", m_isDoeOptionSelected);
   writer.writeValue("isQcDoeSelected", m_isQcDoeOptionSelected);
   QVector<int> nDesignPoints;
   for (const DoeOption* option : m_doeOptions)
   {
      nDesignPoints.append(option->nDesignPoints());
   }
   writer.writeValue("nDesignPoints", nDesignPoints);
   writer.writeValue("SubSampling", m_subSamplingFactor);
   writer.writeValue("BaseSubSampling", m_baseCaseSubSamplingFactor);
   writer.writeValue("stateFileNameDoE",m_stateFileNameDoE);
}

void UAScenario::readFromFile(const ScenarioReader& reader)
{
   const int version = reader.readInt("UAScenarioVersion");
   CasaScenario::readFromFile(reader);
   m_influentialParameterManager.readFromFile(reader);
   m_predictionTargetManager.readFromFile(reader);
   m_monteCarloDataManager.readFromFile(reader);
   m_proxy.readFromFile(reader);

   m_targetQCs = reader.readVector<TargetQC>("targetQC");
   const QVector<bool> isDoeOptionSelected = reader.readVector<bool>("isDoeSelected");
   const QVector<bool> isQcDoeOptionSelected = reader.readVector<bool>("isQcDoeSelected");
   const QVector<int> nDesignPoints = reader.readVector<int>("nDesignPoints");
   const int iLast = (nDesignPoints.size()<m_doeOptions.size()) ? nDesignPoints.size() : m_doeOptions.size();

   m_isQcDoeOptionSelected = isQcDoeOptionSelected;

   for (int i = 0; i < iLast; ++i)
   {
      m_doeOptions[i]->setArbitraryNDesignPoints(nDesignPoints[i]);
   }

   updateDoeConstantNumberOfDesignPoints(m_influentialParameterManager.totalNumberOfInfluentialParameters());
   if (version > 0)
   {
      m_manualDesignPointManager.readFromFile(reader);
   }
   else
   {
      m_manualDesignPointManager.clear();
      m_manualDesignPointManager.addInfluentialParameter(influentialParameterManager().influentialParameters().size());
   }

   if (version > 1)
   {
      m_isStageComplete.readFromFile(reader);
   }
   else
   {
      m_isStageComplete.setAllToTrue();
   }

   if (version > 2)
   {
      m_runCaseSetFileManager.readFromFile(reader);
   }
   if (version > 3)
   {
      m_subSamplingFactor = reader.readInt("SubSampling");
   }
   if (version > 4)
   {
      m_isStageUpToDate.readFromFile(reader);
   }
   else
   {
      m_isStageUpToDate.setAllToTrue();
   }
   if (version > 5)
   {
      m_baseCaseSubSamplingFactor = reader.readInt("BaseSubSampling");
   }
   if (version > 6)
   {
      m_stateFileNameDoE = reader.readString("stateFileNameDoE");
   }
   if (version > 7)
   {
      m_mcmcSettings.readFromFile(reader);
   }

   if (version < 9)
   {
      if (isDoeOptionSelected.size()+1 == m_doeOptions.size())
      {
         m_isDoeOptionSelected.resize(0);
         m_isDoeOptionSelected.push_back(false);
         m_isDoeOptionSelected.append(isDoeOptionSelected);
      }
   }
   else
   {
      if (isDoeOptionSelected.size() == m_doeOptions.size())
      {
         m_isDoeOptionSelected = isDoeOptionSelected;
      }
   }
}

void UAScenario::clear()
{
   CasaScenario::clear();

   m_influentialParameterManager.clear();
   m_predictionTargetManager.clear();
   m_monteCarloDataManager.clear();
   m_manualDesignPointManager.clear();
   m_runCaseSetFileManager.clear();
   m_proxy.clear();
   m_mcmcSettings.clear();

   m_targetQCs.clear();
   m_isDoeOptionSelected.fill(false);
   m_isQcDoeOptionSelected.clear();
   for ( DoeOption* option : m_doeOptions)
   {
      option->setArbitraryNDesignPoints(0);
   }
   updateDoeConstantNumberOfDesignPoints(m_influentialParameterManager.totalNumberOfInfluentialParameters());

   m_isStageComplete.clear();
   m_isStageUpToDate.clear();

   //By default the base case should be selected:
   selectDoEOptionBaseCase();
}

QString UAScenario::iterationDirName() const
{
   return m_runCaseSetFileManager.iterationDirName();
}

void UAScenario::updateIterationDir()
{
   m_runCaseSetFileManager.setIterationPath(workingDirectory()+ "/" + runLocation());
}

bool UAScenario::iterationDirExists() const
{
   return !m_runCaseSetFileManager.isIterationDirDeleted(workingDirectory()+ "/" + runLocation());
}

void UAScenario::loadProject3dFile() const
{
   if (m_cmbMapReader)
   {
      m_cmbMapReader->load(project3dPath().toStdString());
   }
   CasaScenario::loadProject3dFile();
}

const CMBMapReader& UAScenario::mapReader() const
{
   return *m_cmbMapReader;
}

bool UAScenario::isStageComplete(const StageTypesUA& stageType) const
{
   return m_isStageComplete.isTrue(stageType);
}

void UAScenario::setStageComplete(const StageTypesUA& stageType, bool isComplete)
{
   m_isStageComplete.setStageState(stageType, isComplete);
}

bool UAScenario::isStageUpToDate(const StageTypesUA& stageType) const
{
   return m_isStageUpToDate.isTrue(stageType);
}

void UAScenario::setStageUpToDate(const StageTypesUA& stageType, bool isUpToDate)
{
   m_isStageUpToDate.setStageState(stageType, isUpToDate);
}

void UAScenario::copyToIterationDir(const QString& fileName) const
{
   const QString source = workingDirectory() + "/" + fileName;
   QString idn =  iterationDirName();
   const QString target = workingDirectory() + "/" + runLocation() + "/" + idn + "/" + fileName;

   if (QFile::exists(target))
   {
      QFile::remove(target);
   }

   if (QFile::copy(source, target))
   {
      QFile::remove(source);
   }
}

} // namespace ua

} // namespace casaWizard
