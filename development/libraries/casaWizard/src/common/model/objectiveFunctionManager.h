// Data container for the objective function
#pragma once

#include "objectiveFunctionValue.h"
#include "writable.h"

#include <QVector>
#include <QStringList>

namespace casaWizard
{

class ObjectiveFunctionManager : public Writable
{
public:
  ObjectiveFunctionManager();

  void setValue(const int row, const int col, const double value);
  void setVariables(const QStringList& variables);

  double absoluteError(const int index) const;
  double relativeError(const int index) const;
  double weight(const int index) const;

  int indexOf(const QString& variable) const;
  QStringList variables() const;

  void writeToFile(ScenarioWriter& writer) const override;
  void readFromFile(const ScenarioReader& reader) override;
  void clear() override;

private:
  ObjectiveFunctionManager(const ObjectiveFunctionManager&) = delete;
  ObjectiveFunctionManager& operator=(ObjectiveFunctionManager) = delete;
  ObjectiveFunctionValue createObjectiveFunctionValue(const QString& variable) const;

  QVector<ObjectiveFunctionValue> values_;
};

} // namespace casaWizard
