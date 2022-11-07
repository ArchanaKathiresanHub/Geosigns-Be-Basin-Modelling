// Data container for the objective function
#pragma once

#include "objectiveFunctionValue.h"
#include "writable.h"

#include <QMap>
#include <QVector>
#include <QStringList>

namespace casaWizard
{

class ObjectiveFunctionManager : public Writable
{
public:
  explicit ObjectiveFunctionManager();

  void setValue(const int row, const int col, const double value);
  void setVariables(const QStringList& variables, const QMap<QString, QString>& userNameToCauldronNameMapping);
  void setEnabledState(const bool state, const int row);

  double absoluteError(const int index) const;
  double relativeError(const int index) const;
  double weight(const int index) const;
  bool enabled(const int index) const;

  int indexOfUserName(const QString& variable) const;
  int indexOfCauldronName(const QString& variable) const;
  QStringList variablesCauldronNames() const;
  QStringList variablesUserNames() const;
  QStringList enabledVariablesUserNames() const;

  void writeToFile(ScenarioWriter& writer) const override;
  void readFromFile(const ScenarioReader& reader) override;
  void clear() override;

  const QVector<ObjectiveFunctionValue>& values() const;
  void setValues(const QVector<ObjectiveFunctionValue>& values);

  void setUserNameToCauldronNameMapping(const QMap<QString, QString>& mapping);

private:
  ObjectiveFunctionManager(const ObjectiveFunctionManager&) = delete;
  ObjectiveFunctionManager& operator=(ObjectiveFunctionManager) = delete;
  ObjectiveFunctionValue createObjectiveFunctionValue(const QString& variableUserName) const;

  QVector<ObjectiveFunctionValue> values_;
  QMap<QString, QString> userNameToCauldronNameMapping_;
};

} // namespace casaWizard
