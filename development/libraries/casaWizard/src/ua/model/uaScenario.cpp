#include "uaScenario.h"

#include "doeOption.h"
#include "influentialParameter.h"
#include "model/calibrationTargetManager.h"
#include "model/doeOptionImpl.h"
#include "model/functions/rmseCalibrationTargets.h"
#include "model/input/projectReader.h"
#include "model/logger.h"
#include "model/output/runCaseSetFileManager.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>

namespace casaWizard
{

namespace ua
{

UAScenario::UAScenario(std::unique_ptr<ProjectReader> projectReader) :
  CasaScenario(std::move(projectReader)),
  stateFileNameDoE_{"casaStateDoE.txt"},
  stateFileNameQC_{"casaStateQC.txt"},
  stateFileNameMCMC_{"casaStateMCMC.txt"},
  runCasesObservablesTextFileName_{"runCasesObservables.txt"},
  doeIndicesTextFileName_{"doeIndices.txt"},
  proxyEvaluationObservablesTextFileName_{"proxyEvalObservables.txt"},
  proxyQualityEvaluationTextFileName_{"proxyEvalQuality.txt"},
  doeTextFileName_{"doeResults.txt"},
  mcTextFileName_{"mcResults.txt"},
  proxy_{},
  influentialParameterManager_{CasaScenario::projectReader()},
  predictionTargetManager_{CasaScenario::projectReader()},
  doeOptions_{DoeOption::getDoeOptions()},
  targetQCs_{},
  isDoeOptionSelected_{QVector<bool>(doeOptions_.size(), false)},
  isQcDoeOptionSelected_{},
  isStageComplete_{},
  monteCarloDataManager_{},
  manualDesignPointManager_{},
  runCaseSetFileManager_{}
{
  calibrationTargetManager().setObjectiveFunctionVariables(predictionTargetManager_.predictionTargetOptions());
  calibrationTargetManager().setObjectiveFunction(0, 0, 5);
  calibrationTargetManager().setObjectiveFunction(0, 1, 0.05);
  calibrationTargetManager().setObjectiveFunction(1, 0, 0.1);
}

UAScenario::~UAScenario()
{
  for (DoeOption* doe : doeOptions_)
  {
    delete doe;
  }
}

const Proxy& UAScenario::proxy() const
{
  return proxy_;
}

double UAScenario::responseSurfacesL2NormBestMC() const
{
  return functions::rmseCalibrationTargets(calibrationTargetDataBestMC(), calibrationTargetManager());
}

void UAScenario::setNumberOfManualDesignPoints()
{
  if (manualDesignPointManager_.numberOfPoints() == 0)
  {
    changeUserDefinedPointStatus(false);
  }

  const int n = manualDesignPointManager_.numberOfPoints();
  for (DoeOption* option : doeOptions_)
  {
    if (dynamic_cast<DoeUserDefined*>(option))
    {
      option->setArbitraryNDesignPoints(n);
    }
  }
}

void UAScenario::changeUserDefinedPointStatus(const bool status)
{
  const int points = manualDesignPointManager_.numberOfPoints();

  if ((points != 0 &&
     !status)      ||
     (points == 0  &&
     status))
  {
    return;
  }

  unsigned int pos = 0;

  for (DoeOption* option : doeOptions_)
  {
    if (dynamic_cast<DoeUserDefined*>(option))
    {
      setIsDoeOptionSelected(pos, status);
      return;
    }
    pos++;
  }
}

void UAScenario::setProxyOrder(int order)
{
  proxy_.setOrder(order);
}

void UAScenario::setProxyKrigingMethod(const QString& krigingMethod)
{
  proxy_.setKrigingMethod(krigingMethod);
}

QVector<DoeOption*> UAScenario::doeSelected() const
{
  QVector<DoeOption*> doeOptionsSelected;

  int i = 0;
  for (const bool& isSelected : isDoeOptionSelected_)
  {
    if (isSelected)
    {
      doeOptionsSelected.push_back(doeOptions_[i]);
    }
    ++i;
  }

  return doeOptionsSelected;
}

QStringList UAScenario::doeOptionSelectedNames() const
{
  QStringList doeOptionSelectedNames;

  int i = 0;
  for (const bool& isSelected : isDoeOptionSelected_)
  {
    if (isSelected)
    {
      doeOptionSelectedNames.append(doeOptions_[i]->name());
    }
    ++i;
  }

  return doeOptionSelectedNames;
}

QStringList UAScenario::qcDoeOptionSelectedNames() const
{
  QStringList qcDoeOptionSelectedNames;

  int i = 0;
  for (const bool& isSelected : isQcDoeOptionSelected_)
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
  return isDoeOptionSelected_;
}

QVector<DoeOption*> UAScenario::doeOptions() const
{
  return doeOptions_;
}

void UAScenario::setIsDoeOptionSelected(const int row, const bool isSelected)
{
  isDoeOptionSelected_[row] = isSelected;

  isQcDoeOptionSelected_.clear();
  int i = 0;
  for (const bool& isDoeInDoeTabSelected : isDoeOptionSelected_)
  {
    if (isDoeInDoeTabSelected)
    {
      isQcDoeOptionSelected_.append(false);
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
  return isQcDoeOptionSelected_;
}

void UAScenario::setIsQcDoeOptionSelected(const int row, const bool isSelected)
{
  isQcDoeOptionSelected_[row] = isSelected;
}

void UAScenario::updateDoeArbitraryNDesignPoints(const int row, const int nDesignPoints)
{
  doeOptions_[row]->setArbitraryNDesignPoints(nDesignPoints);
}

void UAScenario::updateDoeConstantNumberOfDesignPoints(const int totalNumberOfInfluentialParameters)
{
  for (DoeOption* doe: doeOptions_)
  {
    doe->calculateNDesignPoints(totalNumberOfInfluentialParameters);
  }
}

QString UAScenario::stateFileNameDoE() const
{
  return stateFileNameDoE_;
}

void UAScenario::setStateFileNameDoE(const QString &stateFilePathDoE)
{
  stateFileNameDoE_ = stateFilePathDoE;
}

QString UAScenario::stateFileNameQC() const
{
  return stateFileNameQC_;
}

void UAScenario::setStateFileNameQC(const QString& stateFileNameQC)
{
  stateFileNameQC_ = stateFileNameQC;
}

QString UAScenario::stateFileNameMCMC() const
{
  return stateFileNameMCMC_;
}

void UAScenario::setStateFileNameMCMC(const QString& stateFilePathMCMC)
{
  stateFileNameMCMC_ = stateFilePathMCMC;
}

QString UAScenario::doeTextFileName() const
{
  return doeTextFileName_;
}

QString UAScenario::runCasesObservablesTextFileName() const
{
  return runCasesObservablesTextFileName_;
}

QString UAScenario::doeIndicesTextFileName() const
{
  return doeIndicesTextFileName_;
}

QString UAScenario::proxyEvaluationObservablesTextFileName() const
{
  return proxyEvaluationObservablesTextFileName_;
}

QString UAScenario::proxyQualityEvaluationTextFileName() const
{
  return proxyQualityEvaluationTextFileName_;
}

InfluentialParameterManager& UAScenario::influentialParameterManager()
{
  return influentialParameterManager_;
}

const InfluentialParameterManager& UAScenario::influentialParameterManager() const
{
  return influentialParameterManager_;
}

PredictionTargetManager& UAScenario::predictionTargetManager()
{
  return predictionTargetManager_;
}

const PredictionTargetManager& UAScenario::predictionTargetManager() const
{
  return predictionTargetManager_;
}

MonteCarloDataManager& UAScenario::monteCarloDataManager()
{
  return monteCarloDataManager_;
}

const MonteCarloDataManager& UAScenario::monteCarloDataManager() const
{
  return monteCarloDataManager_;
}

ManualDesignPointManager& UAScenario::manualDesignPointManager()
{
  return manualDesignPointManager_;
}

const ManualDesignPointManager& UAScenario::manualDesignPointManager() const
{
  return manualDesignPointManager_;
}

RunCaseSetFileManager& UAScenario::runCaseSetFileManager()
{
  return runCaseSetFileManager_;
}

const RunCaseSetFileManager& UAScenario::runCaseSetFileManager() const
{
  return runCaseSetFileManager_;
}

QVector<TargetQC> UAScenario::targetQCs() const
{
  return targetQCs_;
}

void UAScenario::setTargetQCs(const QVector<TargetQC>& targetQCs)
{
  targetQCs_.clear();
  targetQCs_ = targetQCs;
}

// This method returns the calibration target values of best matching MCMC data point (which is the first one)
QVector<double> UAScenario::calibrationTargetDataBestMC() const
{
  QVector<double> calibrationTargetBestMCData;
  const QVector<QVector<double>> mcDataCalibration = monteCarloDataManager_.calibrationTargetMatrix();
  const int amountOfActiveCalibTargets = calibrationTargetManager().amountOfActiveCalibrationTargets();
  if (mcDataCalibration.isEmpty() || mcDataCalibration.size() != amountOfActiveCalibTargets)
  {
    return calibrationTargetBestMCData;
  }

  for (int i = 0; i < amountOfActiveCalibTargets; ++i)
  {
    calibrationTargetBestMCData.push_back(monteCarloDataManager_.calibrationTargetMatrix()[i][0]);
  }
  return calibrationTargetBestMCData;
}


const QString UAScenario::mcTextFileName() const
{
  return mcTextFileName_;
}

void UAScenario::setMCtextFileName(const QString& mcTextFileName)
{
  mcTextFileName_ = mcTextFileName;
}

QVector<InfluentialParameter*> UAScenario::influentialParametersWithRunData()
{
  const int sizeRunInfluentialParameters = monteCarloDataManager_.influentialParameterMatrix().size();

  if (sizeRunInfluentialParameters > influentialParameterManager_.influentialParameters().size())
  {
    Logger::log() << "Incompatible data size of the found influential parameters and the selected ones." << Logger::endl();
    return influentialParameterManager_.influentialParameters();
  }

  return influentialParameterManager_.influentialParameters().mid(0, sizeRunInfluentialParameters);
}

void UAScenario::writeToFile(ScenarioWriter& writer) const
{
  CasaScenario::writeToFile(writer);
  writer.writeValue("UAScenarioVersion", 3);
  influentialParameterManager_.writeToFile(writer);
  predictionTargetManager_.writeToFile(writer);
  monteCarloDataManager_.writeToFile(writer);
  manualDesignPointManager_.writeToFile(writer);
  runCaseSetFileManager_.writeToFile(writer);
  isStageComplete_.writeToFile(writer);
  proxy_.writeToFile(writer);

  writer.writeValue("targetQC", targetQCs_);

  writer.writeValue("isDoeSelected", isDoeOptionSelected_);
  writer.writeValue("isQcDoeSelected", isQcDoeOptionSelected_);
  QVector<int> nDesignPoints;
  for (const DoeOption* option : doeOptions_)
  {
    nDesignPoints.append(option->nDesignPoints());
  }
  writer.writeValue("nDesignPoints", nDesignPoints);
}

void UAScenario::readFromFile(const ScenarioReader& reader)
{
  const int version = reader.readInt("UAScenarioVersion");
  CasaScenario::readFromFile(reader);
  influentialParameterManager_.readFromFile(reader);
  predictionTargetManager_.readFromFile(reader);
  monteCarloDataManager_.readFromFile(reader);
  proxy_.readFromFile(reader);

  targetQCs_ = reader.readVector<TargetQC>("targetQC");
  const QVector<bool> isDoeOptionSelected = reader.readVector<bool>("isDoeSelected");
  const QVector<bool> isQcDoeOptionSelected = reader.readVector<bool>("isQcDoeSelected");
  const QVector<int> nDesignPoints = reader.readVector<int>("nDesignPoints");
  const int iLast = (nDesignPoints.size()<doeOptions_.size()) ? nDesignPoints.size() : doeOptions_.size();

  for (int i = 0; i < iLast; ++i)
  {
    doeOptions_[i]->setArbitraryNDesignPoints(nDesignPoints[i]);
  }

  if (isDoeOptionSelected.size() == doeOptions_.size())
  {
    isDoeOptionSelected_ = isDoeOptionSelected;
    isQcDoeOptionSelected_ = isQcDoeOptionSelected;
  }

  updateDoeConstantNumberOfDesignPoints(influentialParameterManager_.totalNumberOfInfluentialParameters());
  if (version > 0)
  {
    manualDesignPointManager_.readFromFile(reader);
  }
  else
  {
    manualDesignPointManager_.clear();
    manualDesignPointManager_.addInfluentialParameter(influentialParameterManager().influentialParameters().size());
  }

  if (version > 1)
  {
    isStageComplete_.readFromFile(reader);
  }
  else
  {
    isStageComplete_.setAllToCompleted();
  }

  if (version > 2)
  {
    runCaseSetFileManager_.readFromFile(reader);
  }
}

void UAScenario::clear()
{
  CasaScenario::clear();

  influentialParameterManager_.clear();
  predictionTargetManager_.clear();
  monteCarloDataManager_.clear();
  manualDesignPointManager_.clear();
  runCaseSetFileManager_.clear();
  proxy_.clear();

  targetQCs_.clear();
  isDoeOptionSelected_.fill(false);
  isQcDoeOptionSelected_.clear();
  for ( DoeOption* option : doeOptions_)
  {
    option->setArbitraryNDesignPoints(0);
  }
  updateDoeConstantNumberOfDesignPoints(influentialParameterManager_.totalNumberOfInfluentialParameters());

  isStageComplete_.clear();
}

QString UAScenario::iterationDirName() const
{
  const QString iterationPath = workingDirectory() + "/" + runLocation();

  const QDir dir(iterationPath);
  QDateTime dateTime = QFileInfo(dir.path()).lastModified();

  QString dirName{""};
  for (const QString entry : dir.entryList())
  {
    if (entry.toStdString().find("Iteration_") == 0)
    {
      const QFileInfo info{dir.path() + "/" + entry};
      if (info.lastModified() >= dateTime)
      {
        dateTime = info.lastModified();
        dirName = entry;
      }
    }
  }

  return dirName;
}

const StageCompletionUA& UAScenario::isStageComplete() const
{
  return isStageComplete_;
}

StageCompletionUA& UAScenario::isStageComplete()
{
  return isStageComplete_;
}

} // namespace ua

} // namespace casaWizard
