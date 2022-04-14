#include "predictionTargetManager.h"

#include "model/input/projectReader.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"
#include "model/targetParameterMapCreator.h"

#include <cassert>
#include <numeric>

namespace casaWizard
{

namespace ua
{

namespace
{

template <class TargetType>
void addTargets(QVector<const PredictionTarget*>& allTargets, QVector<TargetType> typeTargets, QVector<bool> hasTimeSeries, QVector<double> snapshots)
{
  if (hasTimeSeries.isEmpty())
  {
    return;
  }

  assert(hasTimeSeries.size() == typeTargets.size());

  if (hasTimeSeries.size() != typeTargets.size())
  {
    return;
  }

  int i = 0;
  for (const TargetType& target : typeTargets)
  {
    if (hasTimeSeries[i])
    {
      for (const double age : snapshots)
      {
        TargetType* tmpPredTar = new TargetType(target);
        tmpPredTar->setAge(age);
        allTargets.push_back(tmpPredTar);
      }
    }
    else
    {
      TargetType* newTarget = new TargetType(target);
      allTargets.push_back(newTarget);
    }
    ++i;
  }
}

}


QStringList PredictionTargetManager::targetVariables_{"Temperature","VRe"};

PredictionTargetManager::PredictionTargetManager(const ProjectReader& projectReader) :
  depthTargets_{},
  surfaceTargets_{},
  predictionTargetsAllTimes_{},
  depthTargetHasTimeSeries_{},
  surfaceTargetHasTimeSeries_{},
  projectReader_{projectReader}
{
}

PredictionTargetManager::~PredictionTargetManager()
{
  clearMemory();
}

QVector<PredictionTargetDepth> PredictionTargetManager::depthTargets() const
{
  return depthTargets_;
}

QVector<PredictionTargetSurface> PredictionTargetManager::surfaceTargets() const
{
  return surfaceTargets_;
}

QVector<const PredictionTarget*> PredictionTargetManager::predictionTargets() const
{
  QVector<const PredictionTarget*> allTargets;
  for (const PredictionTargetDepth& target : depthTargets_)
  {
    allTargets.push_back(&target);
  }
  for (const PredictionTargetSurface& target : surfaceTargets_)
  {
    allTargets.push_back(&target);
  }
  return allTargets;
}

QVector<const PredictionTarget*> PredictionTargetManager::predictionTargetInTimeSeries(const int row) const
{
  return predictionTargetsIncludingTimeSeries().mid(indexCumulativePredictionTarget(row), sizeOfPredictionTargetWithTimeSeries(row));
}

int PredictionTargetManager::indexCumulativePredictionTarget(const int row) const
{
  const QVector<int> sizesPredTargets = sizeOfPredictionTargetsWithTimeSeries();

  return (row > 0
          ? std::accumulate(sizesPredTargets.begin(), sizesPredTargets.begin() + row, 0)
          : 0);
}

QStringList PredictionTargetManager::validLayerNames() const
{
  return projectReader_.layerNames();
}

QVector<const PredictionTarget*> PredictionTargetManager::predictionTargetsIncludingTimeSeries() const
{
  return predictionTargetsAllTimes_;
}

void PredictionTargetManager::setPredictionTargetsAllTimes()
{
  for (const PredictionTarget* target : predictionTargetsAllTimes_)
  {
    delete target;
  }
  predictionTargetsAllTimes_.clear();

  QVector<double> snapshots = projectReader_.agesFromMajorSnapshots();

  addTargets<PredictionTargetDepth>(predictionTargetsAllTimes_, depthTargets_, depthTargetHasTimeSeries_, snapshots);
  addTargets<PredictionTargetSurface>(predictionTargetsAllTimes_, surfaceTargets_, surfaceTargetHasTimeSeries_, snapshots);
}

void PredictionTargetManager::clearMemory()
{
  for (const PredictionTarget* targetTime : predictionTargetsAllTimes_)
  {
    delete targetTime;
  }
}

void PredictionTargetManager::setDepthTarget(int row, int column, const QString& text)
{
  if (text.isEmpty() || row < 0 || row >= depthTargets_.size())
  {
    return;
  }

  switch(column)
  {
    case 0:
    {
      depthTargets_[row].setProperty(text);
      break;
    }
    case 1:
    {
      depthTargets_[row].setX(text.toDouble());
      break;
    }
    case 2:
    {
      depthTargets_[row].setY(text.toDouble());
      break;
    }
    case 3:
    {
      depthTargets_[row].setZ(text.toDouble());
      break;
    }
  }
  setPredictionTargetsAllTimes();
}

void PredictionTargetManager::setSurfaceTarget(int row, int column, const QString& text)
{
  if (text.isEmpty())
  {
    return;
  }
  if (row < 0 || row >= surfaceTargets_.size())
  {
    return;
  }
  switch(column)
  {
    case 0:
    {
      surfaceTargets_[row].setProperty(text);
      break;
    }
    case 1:
    {
      surfaceTargets_[row].setX(text.toDouble());
      break;
    }
    case 2:
    {
      surfaceTargets_[row].setY(text.toDouble());
      break;
    }
    case 3:
    {
      surfaceTargets_[row].setLayerName(text);
      break;
    }
  }
  setPredictionTargetsAllTimes();
}


void PredictionTargetManager::addDepthTarget(const QString& property, const double x, const double y, const double z, const double age)
{
  PredictionTargetDepth target(property, x, y, z, age);
  depthTargets_.append(target);
  depthTargetHasTimeSeries_.push_back(false);
  setPredictionTargetsAllTimes();
}

void PredictionTargetManager::addSurfaceTarget(const QString& property, const double x, const double y, QString layer, const double age)
{  
  const QStringList layers = projectReader_.layerNames();
  if (!layers.contains(layer))
  {
    if (layers.empty())
    {
      return;
    }
    else
    {
      layer = layers[0];
    }
  }
  PredictionTargetSurface target(property, x, y, layer, age);
  surfaceTargets_.append(target);
  surfaceTargetHasTimeSeries_.push_back(false);
  setPredictionTargetsAllTimes();
}

void PredictionTargetManager::copyDepthTarget(int index)
{
  if (index<0 || index>=depthTargets_.size())
  {
    return;
  }
  depthTargets_.append(depthTargets_[index]);
  depthTargetHasTimeSeries_.push_back(depthTargetHasTimeSeries_[index]);
  setPredictionTargetsAllTimes();
}

void PredictionTargetManager::copySurfaceTarget(int index)
{
  if (index<0 || index>=surfaceTargets_.size())
  {
    return;
  }
  surfaceTargets_.append(surfaceTargets_[index]);
  surfaceTargetHasTimeSeries_.push_back(surfaceTargetHasTimeSeries_[index]);
  setPredictionTargetsAllTimes();
}

void PredictionTargetManager::removeDepthTarget(int index)
{
  if (index<0 || index>=depthTargets_.size())
  {
    return;
  }
  depthTargets_.remove(index);
  depthTargetHasTimeSeries_.remove(index);
  setPredictionTargetsAllTimes();
}

void PredictionTargetManager::removeSurfaceTarget(int index)
{
  if (index<0 || index>=surfaceTargets_.size())
  {
    return;
  }
  surfaceTargets_.remove(index);
  surfaceTargetHasTimeSeries_.remove(index);
  setPredictionTargetsAllTimes();
}

QStringList PredictionTargetManager::predictionTargetOptions()
{
  return targetVariables_;
}

void PredictionTargetManager::writeToFile(ScenarioWriter& writer) const
{
  const int version = 1;
  writer.writeValue("PredictionTargetManagerVersion", version);
  writer.writeValue("depthTarget", depthTargets_);
  writer.writeValue("depthTargetHasTimeSeries", depthTargetHasTimeSeries_);
  writer.writeValue("surfaceTarget", surfaceTargets_);
  writer.writeValue("surfaceTargetHasTimeSeries", surfaceTargetHasTimeSeries_);
}

void PredictionTargetManager::readFromFile(const ScenarioReader& reader)
{
  const int version = reader.readInt("PredictionTargetManagerVersion");
  depthTargets_ = reader.readVector<PredictionTargetDepth>("depthTarget");

  if (version > 0)
  {
    depthTargetHasTimeSeries_ = reader.readVector<bool>("depthTargetHasTimeSeries");
    surfaceTargets_ = reader.readVector<PredictionTargetSurface>("surfaceTarget");
    surfaceTargetHasTimeSeries_ = reader.readVector<bool>("surfaceTargetHasTimeSeries");
    setPredictionTargetsAllTimes();
  }
}

void PredictionTargetManager::clear()
{
  clearMemory();
  depthTargets_.clear();
  surfaceTargets_.clear();
  predictionTargetsAllTimes_.clear();
  depthTargetHasTimeSeries_.clear();
  surfaceTargetHasTimeSeries_.clear();
}

QVector<bool> PredictionTargetManager::depthTargetHasTimeSeries() const
{
  return depthTargetHasTimeSeries_;
}

QVector<bool> PredictionTargetManager::surfaceTargetHasTimeSeries() const
{
  return surfaceTargetHasTimeSeries_;
}

void PredictionTargetManager::setDepthTargetHasTimeSeries(const int row, const bool isSelected)
{
  if (row < 0 || row >= depthTargetHasTimeSeries_.size())
  {
    return;
  }
  depthTargetHasTimeSeries_[row] = isSelected;
  setPredictionTargetsAllTimes();
}

void PredictionTargetManager::setSurfaceTargetHasTimeSeries(const int row, const bool isSelected)
{
  if (row < 0 || row >= surfaceTargetHasTimeSeries_.size())
  {
    return;
  }
  surfaceTargetHasTimeSeries_[row] = isSelected;
  setPredictionTargetsAllTimes();
}

int PredictionTargetManager::amountAtAge0() const
{
  return depthTargets_.size() + surfaceTargets_.size();
}

int PredictionTargetManager::amountIncludingTimeSeries() const
{
  const int nMajorSnapShots = projectReader_.agesFromMajorSnapshots().size();
  int sizeTotalPredTargets = 0;
  for (const bool hasTimeSeries : depthTargetHasTimeSeries_)
  {
    sizeTotalPredTargets += hasTimeSeries ? nMajorSnapShots : 1;
  }

  for (const bool hasTimeSeries : surfaceTargetHasTimeSeries_)
  {
    sizeTotalPredTargets += hasTimeSeries ? nMajorSnapShots : 1;
  }

  return sizeTotalPredTargets;
}

int PredictionTargetManager::sizeOfPredictionTargetWithTimeSeries(const int row) const
{
  QVector<int> sizes = sizeOfPredictionTargetsWithTimeSeries();
  const int n = sizes.size();
  if (n == 0 || row < 0 || row >= n )
  {
    return 0;
  }

  return sizeOfPredictionTargetsWithTimeSeries()[row];
}

// Returning vector contains 1 by default and the number of snapshots if the prediction target has a time series
QVector<int> PredictionTargetManager::sizeOfPredictionTargetsWithTimeSeries() const
{
  const int numberSnapshots = projectReader_.agesFromMajorSnapshots().size();
  const int numberDepthTargets = depthTargets_.size();
  const int numberSurfaceTargets = surfaceTargets_.size();

  assert(numberDepthTargets == depthTargetHasTimeSeries_.size());
  assert(numberSurfaceTargets == surfaceTargetHasTimeSeries_.size());

  QVector<int> sizePredTargets(numberDepthTargets + numberSurfaceTargets, 1);

  for (int i = 0; i < numberDepthTargets; ++i)
  {
    if (depthTargetHasTimeSeries_[i])
    {
      sizePredTargets[i] = numberSnapshots;
    }
  }

  for (int j = 0; j < numberSurfaceTargets; ++j)
  {
    if (surfaceTargetHasTimeSeries_[j])
    {
      sizePredTargets[j + numberDepthTargets] = numberSnapshots;
    }
  }
  return sizePredTargets;
}

} // namespace ua

} // namespace casaWizard
