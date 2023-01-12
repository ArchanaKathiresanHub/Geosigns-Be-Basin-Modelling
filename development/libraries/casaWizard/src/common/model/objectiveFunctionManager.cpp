#include "objectiveFunctionManager.h"

#include "scenarioReader.h"
#include "scenarioWriter.h"

#include <QPair>
#include <QString>

namespace casaWizard
{

ObjectiveFunctionManager::ObjectiveFunctionManager() :
   values_{},
   userNameToCauldronNameMapping_{}
{
}

void ObjectiveFunctionManager::setVariables(const QStringList& variables, const QMap<QString, QString>& userNameToCauldronNameMapping)
{
   userNameToCauldronNameMapping_ = userNameToCauldronNameMapping;

   bool change = false;
   QStringList currentVariables;
   if (!values_.empty())
   {
      auto it = values_.begin();
      while (it != values_.end())
      {
         const QString currentVariable = (*it).variableUserName();
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
         val.setWeight(1.0);
      }
   }
}

void ObjectiveFunctionManager::setEnabledState(const bool state, const int row)
{
   values_[row].setEnabled(state);
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

bool ObjectiveFunctionManager::enabled(const int index) const
{
   return values_[index].enabled();
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

int ObjectiveFunctionManager::indexOfUserName(const QString& variable) const
{
   int i = 0;
   for (const ObjectiveFunctionValue& val : values_)
   {
      if (val.variableUserName() == variable)
      {
         return i;
      }
      ++i;
   }
   return -1;
}

int ObjectiveFunctionManager::indexOfCauldronName(const QString& variable) const
{
   int i = 0;
   for (const ObjectiveFunctionValue& val : values_)
   {
      if (val.variableCauldronName() == variable)
      {
         return i;
      }
      ++i;
   }
   return -1;
}

QStringList ObjectiveFunctionManager::variablesCauldronNames() const
{
   QStringList vars;
   for (const ObjectiveFunctionValue& val : values_)
   {
      vars << userNameToCauldronNameMapping_.value(val.variableUserName(), "Unknown");
   }
   return vars;
}

QStringList ObjectiveFunctionManager::variablesUserNames() const
{
   QStringList vars;
   for (const ObjectiveFunctionValue& val : values_)
   {
      vars << val.variableUserName();
   }
   return vars;
}

QStringList ObjectiveFunctionManager::enabledVariablesUserNames() const
{
   QStringList vars;
   for (const ObjectiveFunctionValue& val : values_)
   {
      if (val.enabled())
      {
         vars << val.variableUserName();
      }
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
   userNameToCauldronNameMapping_.clear();
   values_.clear();
}

const QVector<ObjectiveFunctionValue>& ObjectiveFunctionManager::values() const
{
   return values_;
}

void ObjectiveFunctionManager::setValues(const QVector<ObjectiveFunctionValue>& values)
{
   values_ = values;
}

void ObjectiveFunctionManager::setUserNameToCauldronNameMapping(const QMap<QString, QString>& mapping)
{
   userNameToCauldronNameMapping_ = mapping;
}

ObjectiveFunctionValue ObjectiveFunctionManager::createObjectiveFunctionValue(const QString& variableUserName) const
{
   QMap<QString, QPair<double, double>> defaultValueMap;
   defaultValueMap.insert(QString("TwoWayTime"), {20.0, 0.0});
   defaultValueMap.insert(QString("GammaRay"), {0.0, 0.1});
   defaultValueMap.insert(QString("BulkDensity"), {50.0, 0.0});
   defaultValueMap.insert(QString("SonicSlowness"), {0.0, 0.05});
   defaultValueMap.insert(QString("Pressure"), {0.0, 0.01});
   defaultValueMap.insert(QString("Temperature"), {10.0, 0.0});
   defaultValueMap.insert(QString("VRe"), {0.2, 0.0});
   defaultValueMap.insert(QString("Velocity"), {0.0, 0.1});

   const QString variableCauldronName = userNameToCauldronNameMapping_.value(variableUserName, "Unknown");

   if (defaultValueMap.contains(variableCauldronName) == 0)
   {
      return ObjectiveFunctionValue(variableCauldronName, variableUserName);
   }

   const QPair<double, double>& p = defaultValueMap[variableCauldronName];
   const double defaultAbsoluteError = p.first;
   const double defaultRelativeError = p.second;
   const bool defaultEnabledState = variableCauldronName != "Velocity" && variableCauldronName != "GammaRay";

   return ObjectiveFunctionValue(variableCauldronName, variableUserName, defaultAbsoluteError, defaultRelativeError, 0.0, defaultEnabledState);
}

} // namespace casaWizard
