#include "manualDesignPointManager.h"

#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include <cmath>

namespace
{
double s_minManualPointDistance = 0.01;
}

namespace casaWizard
{

namespace ua
{

ManualDesignPointManager::ManualDesignPointManager() :
  influentialParameterValues_{},
  m_completed{},
  numberOfParameters_{0}
{
}

void ManualDesignPointManager::addDesignPoint()
{
  addDesignPoint(QVector<double>(numberOfParameters_));
}

bool ManualDesignPointManager::addDesignPoint(const QVector<double> newPoint)
{
  if (newPoint.size() != numberOfParameters_)
  {
    return false;
  }

  if (pointTooCloseToExistingPoints(newPoint))
  {
     return false;
  }

  influentialParameterValues_.append(newPoint);
  m_completed.append(false);
  return true;
}

bool ManualDesignPointManager::pointTooCloseToExistingPoints(const QVector<double> point) const
{
   for (const auto & existingPoint : influentialParameterValues_)
   {
      if (existingPoint.size() != point.size())
      {
         return true;
      }

      double distance(0);
      for (int i = 0; i < existingPoint.size(); i++)
      {
         double d = existingPoint[i]-point[i];
         distance += d*d;
      }
      distance = std::sqrt(distance);

      if (distance < s_minManualPointDistance)
      {
         return true;
      }
   }
   return false;
}

void ManualDesignPointManager::removeDesignPoint(const int index)
{
  if (index > m_completed.size() || m_completed[index])
  {
    return;
  }

  influentialParameterValues_.remove(index);
  m_completed.remove(index);
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
       m_completed[designPoint])
  {
    return;
  }

  influentialParameterValues_[designPoint][parameterIndex] = value;
}

int ManualDesignPointManager::numberOfCasesToRun() const
{
   int result  = 0;
   for (const bool completed : m_completed)
   {
      result += completed ? 0 : 1;
   }

   return result;
}

QVector<QVector<double>> ManualDesignPointManager::parameters() const
{
  return influentialParameterValues_;
}

QVector<bool> ManualDesignPointManager::completed() const
{
  return m_completed;
}

void ManualDesignPointManager::completeAll()
{
  for (bool& b : m_completed)
  {
    b = true;
  }
}

void ManualDesignPointManager::writeToFile(ScenarioWriter& writer) const
{
  writer.writeValue("ManualDesignPointManagerVersion", 0);
  writer.writeValue("manualDesignPoints", influentialParameterValues_);  
  writer.writeValue("manualDesignPointParameters", numberOfParameters_);
  writer.writeValue("manualDesignPointCompleted", m_completed);
}

void ManualDesignPointManager::readFromFile(const ScenarioReader& reader)
{
  influentialParameterValues_ = reader.readVector<QVector<double>>("manualDesignPoints");
  numberOfParameters_ = reader.readInt("manualDesignPointParameters");
  m_completed = reader.readVector<bool>("manualDesignPointCompleted");
}

void ManualDesignPointManager::clear()
{
  influentialParameterValues_.clear();  
  numberOfParameters_ = 0;
  m_completed.clear();
}

} // namespace ua

} // namespace casaWizard
