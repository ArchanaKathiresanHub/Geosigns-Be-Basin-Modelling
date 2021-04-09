#include "objectiveFunctionManager.h"

#include "scenarioReader.h"
#include "scenarioWriter.h"

#include <QMap>
#include <QPair>
#include <QString>

namespace casaWizard
{

ObjectiveFunctionManager::ObjectiveFunctionManager() :
  values_{}
{
}

void ObjectiveFunctionManager::setVariables(const QStringList& variables)
{
  bool change = false;
  QStringList currentVariables;
  if (!values_.empty())
  {
    auto it = values_.begin();
    while (it != values_.end())
    {
      const QString currentVariable = (*it).variable();
      if (!variables.contains(currentVariable))
      {
        it = values_.erase(it);
        change = true;
      }
      else
      {
        currentVariables << currentVariable;
        ++it;
      }
    }
  }

  for (const QString& variable: variables)
  {
    if (!currentVariables.contains(variable))
    {
      const ObjectiveFunctionValue val = createObjectiveFunctionValue(variable);
      values_.push_back(val);
      currentVariables << variable;
      change = true;
    }
  }

  if (change && !values_.empty())
  {
    for (ObjectiveFunctionValue& val : values_)
    {
      val.setWeight(1.0/values_.size());
    }
  }
}

double ObjectiveFunctionManager::absoluteError(const int index) const
{
  return values_[index].absoluteError();
}

double ObjectiveFunctionManager::relativeError(const int index) const
{
  return values_[index].relativeError();
}

double ObjectiveFunctionManager::weight(const int index) const
{
  return values_[index].weight();
}

void ObjectiveFunctionManager::setValue(const int row, const int col, const double value)
{
  if (row >= values_.size() || row < 0)
  {
    return;
  }
  switch(col)
  {
    case 0:
      values_[row].setAbsoluteError(value);
      break;
    case 1:
      values_[row].setRelativeError(value);
      break;
    case 2:
      values_[row].setWeight(value);
      break;
    default:
      break;
  }
}

int ObjectiveFunctionManager::indexOf(const QString& variable) const
{
  int i = 0;
  for (const ObjectiveFunctionValue& val : values_)
  {
    if (val.variable() == variable)
    {
      return i;
    }
    ++i;
  }
  return -1;
}

QStringList ObjectiveFunctionManager::variables() const
{
  QStringList vars;
  for (const ObjectiveFunctionValue& val : values_)
  {
    vars << val.variable();
  }
  return vars;
}

void ObjectiveFunctionManager::writeToFile(ScenarioWriter& writer) const
{
  writer.writeValue("ObjectiveFunctionManagerVersion", 0);
  writer.writeValue("objectiveFunction", values_);
}

void ObjectiveFunctionManager::readFromFile(const ScenarioReader& reader)
{
  values_ = reader.readVector<ObjectiveFunctionValue>("objectiveFunction");
}

void ObjectiveFunctionManager::clear()
{
  values_.clear();
}

const QVector<ObjectiveFunctionValue>& ObjectiveFunctionManager::values() const
{
  return values_;
}

ObjectiveFunctionValue ObjectiveFunctionManager::createObjectiveFunctionValue(const QString &variable) const
{
  QMap<QString, QPair<double, double>> defaultValueMap;
  defaultValueMap.insert(QString("TwoWayTime"), {50.0, 0.0});
  defaultValueMap.insert(QString("BulkDensity"), {20.0, 0.0});
  defaultValueMap.insert(QString("SonicSlowness"), {1.0, 0.05});

  if (defaultValueMap.contains(variable) == 0)
  {
    return ObjectiveFunctionValue(variable);
  }

  const QPair<double, double>& p = defaultValueMap[variable];
  const double defaultAbsoluteError = p.first;
  const double defaultRelativeError = p.second;

  return ObjectiveFunctionValue(variable, defaultAbsoluteError, defaultRelativeError);
}

} // namespace casaWizard
