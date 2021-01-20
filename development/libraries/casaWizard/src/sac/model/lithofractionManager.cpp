//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "lithofractionManager.h"

#include "model/logger.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

namespace casaWizard
{

namespace sac
{

const double minPercentage{0.01};
const double maxPercentage{99.99};
const double minFraction{0.001};
const double maxFraction{0.999};

LithofractionManager::LithofractionManager() :
  lithofractions_{},
  optimizedLithofractions_{}
{
}

const QVector<Lithofraction>& LithofractionManager::lithofractions() const
{
  return lithofractions_;
}

void LithofractionManager::setLithofractionLayerName(int index, const QString& name)
{
  lithofractions_[index].setLayerName(name);
}

void LithofractionManager::setLithofractionFirstComponent(int index, int percent)
{
  if( percent < 0 || percent > 2)
  {
    Logger::log() <<  "First percentage of lithofraction can only be 0, 1, 2." << Logger::endl();
  }
  lithofractions_[index].setFirstComponent(percent);
  checkLithofractionPercentWarning(index);
}

void LithofractionManager::setLithofractionFirstMinPercentage(int index, double value)
{
  if (value < minPercentage || value > maxPercentage)
  {
    Logger::log() << "Warning: Minimum percentage should be between "
                  << minPercentage << " and "
                  << maxPercentage << ". Reset to "
                  << minPercentage << Logger::endl();
    value = minPercentage;
  }
  lithofractions_[index].setMinPercentageFirstComponent(value);
}

void LithofractionManager::setLithofractionFirstMaxPercentage(int index, double value)
{
  if (value < minPercentage || value > maxPercentage)
  {
    Logger::log() << "Warning: Maximum percentage should be between "
                  << minPercentage << " and "
                  << maxPercentage << ". Reset to "
                  << maxPercentage << Logger::endl();
    value = maxPercentage;
  }
  lithofractions_[index].setMaxPercentageFirstComponent(value);
}

void LithofractionManager::setLithofractionSecondComponent(int index, int percent)
{
  if( percent < 0 || percent > 3)
  {
    Logger::log() << "First percentage of lithofraction can only be 0, 1, 2, 3." << Logger::endl();
  }
  lithofractions_[index].setSecondComponent(percent);
  checkLithofractionPercentWarning(index);
}

void LithofractionManager::checkLithofractionPercentWarning(int index) const
{
  const int p1 = lithofractions_[index].firstComponent();
  const int p2 = lithofractions_[index].secondComponent();
  if (p1 == p2)
  {
    Logger::log() << "Warning: You used " << Lithofraction::percentNames[p1] << " for both the first and second lithofraction in layer "
                  << lithofractions_[index].layerName() << " (row " << (index+1) << ")." << Logger::endl();
  }
  else if (p1 > p2)
  {
    Logger::log() << "Warning: You used " << Lithofraction::percentNames[p1] << " before " << Lithofraction::percentNames[p2] << " in layer "
                  << lithofractions_[index].layerName() << " (row " << (index+1) << ")."
                  << " Consider changing the order." << Logger::endl();

  }
}

void LithofractionManager::setLithofractionSecondMinFraction(int index, double value)
{
  if (value < minFraction || value > maxFraction)
  {
    Logger::log() << "Warning: Minimum of second lithofraction should be between "
                  << minFraction << " and "
                  << maxFraction << ". Reset to "
                  << minFraction << Logger::endl();
    value = minFraction;
  }
  lithofractions_[index].setMinFractionSecondComponent(value);
}

void LithofractionManager::setLithofractionSecondMaxFraction(int index, double value)
{
  if (value < minFraction || value > maxFraction)
  {
    Logger::log() << "Warning: Maximum of second lithofraction should be between "
                  << minFraction << " and "
                  << maxFraction << ". Reset to "
                  << minFraction << Logger::endl();
    value = maxFraction;
  }
  lithofractions_[index].setMaxFractionSecondComponent(value);
}

void LithofractionManager::setLithoFractionDoFirstOptimization(int index, bool doFirstOptimization)
{
  lithofractions_[index].setDoFirstOptimization(doFirstOptimization);
}

void LithofractionManager::setLithoFractionDoSecondOptimization(int index, bool doSecondOptimization)
{
  lithofractions_[index].setDoSecondOptimization(doSecondOptimization);
}

void LithofractionManager::addLithofraction(const QString& name)
{
  Lithofraction newLithofraction
  {
    name,
    0,
    minPercentage,
    maxPercentage,
    1,
    minFraction,
    maxFraction
  };

  lithofractions_.push_back(newLithofraction);
}

void LithofractionManager::removeLithofraction(int index)
{
  lithofractions_.remove(index);
}

const QVector<OptimizedLithofraction>& LithofractionManager::optimizedLithofractions() const
{
  return optimizedLithofractions_;
}

QVector<OptimizedLithofraction> LithofractionManager::optimizedInWell(const int wellIndex) const
{
  QVector<OptimizedLithofraction> optimizedInWell;
  for (const OptimizedLithofraction& opt : optimizedLithofractions_)
  {
    if (opt.wellId() == wellIndex)
    {
      optimizedInWell.push_back(opt);
    }
  }
  return optimizedInWell;
}

void LithofractionManager::addOptimizedLithofraction(const OptimizedLithofraction& optimal)
{
  optimizedLithofractions_.push_back(optimal);
}

void LithofractionManager::clearOptimizedLithofractions()
{
  optimizedLithofractions_.clear();
}

void LithofractionManager::writeToFile(ScenarioWriter& writer) const
{
  writer.writeValue("LithofractionManagerVersion", 0);
  writer.writeValue("lithofractions", lithofractions_);
  writer.writeValue("optimizedLithofractions", optimizedLithofractions_);
}

void LithofractionManager::readFromFile(const ScenarioReader& reader)
{
  lithofractions_ = reader.readVector<Lithofraction>("lithofractions");
  optimizedLithofractions_ = reader.readVector<OptimizedLithofraction>("optimizedLithofractions");
}

void LithofractionManager::clear()
{
  lithofractions_.clear();
  optimizedLithofractions_.clear();
}


}  // namespace sac

}  // namespace casaWizard
