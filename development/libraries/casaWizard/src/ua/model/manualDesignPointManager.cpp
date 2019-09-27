#include "manualDesignPointManager.h"

#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

namespace casaWizard
{

namespace ua
{

ManualDesignPointManager::ManualDesignPointManager() :
  influentialParameterValues_{},
  completed_{},
  numberOfParameters_{0}
{
}

void ManualDesignPointManager::addDesignPoint()
{
  addDesignPoint(QVector<double>(numberOfParameters_));
}

void ManualDesignPointManager::addDesignPoint(const QVector<double> newPoint)
{
  if (newPoint.size() != numberOfParameters_)
  {
    return;
  }
  influentialParameterValues_.append(newPoint);
  completed_.append(false);
}

void ManualDesignPointManager::removeDesignPoint(const int index)
{
  if (index > completed_.size() || completed_[index])
  {
    return;
  }

  influentialParameterValues_.remove(index);
  completed_.remove(index);
}

void ManualDesignPointManager::addInfluentialParameter(const int amount)
{
  numberOfParameters_ += amount;
  for (int i = 0; i<amount; ++i)
  {
    for (QVector<double>& v : influentialParameterValues_)
    {
      v.append(0);
    }
  }
}

void ManualDesignPointManager::removeInfluentialParameter(const int index)
{
  if (index >= numberOfParameters_ || index < 0)
  {
    return;
  }

  for (QVector<double>& v : influentialParameterValues_)
  {
    v.remove(index);
  }
  numberOfParameters_--;
}

int ManualDesignPointManager::numberOfPoints() const
{
  return influentialParameterValues_.size();
}

int ManualDesignPointManager::numberOfParameters() const
{
  return numberOfParameters_;
}

QVector<double> ManualDesignPointManager::getDesignPoint(const int index) const
{
  if (index < numberOfPoints())
  {
    return influentialParameterValues_[index];
  }
  return QVector<double>();
}

void ManualDesignPointManager::setParameterValue(const int designPoint, const int parameterIndex, const double value)
{
  if ( designPoint >= numberOfPoints() ||
       parameterIndex >= numberOfParameters_ ||
       completed_[designPoint])
  {
    return;
  }

  influentialParameterValues_[designPoint][parameterIndex] = value;
}

QVector<QVector<double>> ManualDesignPointManager::parameters() const
{
  return influentialParameterValues_;
}

QVector<bool> ManualDesignPointManager::completed() const
{
  return completed_;
}

void ManualDesignPointManager::completeAll()
{
  for (bool& b : completed_)
  {
    b = true;
  }
}

void ManualDesignPointManager::writeToFile(ScenarioWriter& writer) const
{
  writer.writeValue("ManualDesignPointManagerVersion", 0);
  writer.writeValue("manualDesignPoints", influentialParameterValues_);  
  writer.writeValue("manualDesignPointParameters", numberOfParameters_);
  writer.writeValue("manualDesignPointCompleted", completed_);
}

void ManualDesignPointManager::readFromFile(const ScenarioReader& reader)
{
  influentialParameterValues_ = reader.readVector<QVector<double>>("manualDesignPoints");
  numberOfParameters_ = reader.readInt("manualDesignPointParameters");
  completed_ = reader.readVector<bool>("manualDesignPointCompleted");
}

void ManualDesignPointManager::clear()
{
  influentialParameterValues_.clear();  
  numberOfParameters_ = 0;
  completed_.clear();
}

} // namespace ua

} // namespace casaWizard
