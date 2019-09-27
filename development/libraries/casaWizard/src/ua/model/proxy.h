// Struct storing the proxy response surface construction features
#pragma once

#include "model/writable.h"

#include <QString>
#include <QVector>

class QStringList;

namespace casaWizard
{

namespace ua
{

class Proxy : public Writable
{
public:
  Proxy();

  static const QVector<int> orderOptions();
  static const QStringList krigingMethodOptions();

  int order() const;
  void setOrder(int order);
  QString krigingMethod() const;
  void setKrigingMethod(const QString& krigingMethod);

  void writeToFile(ScenarioWriter& writer) const override;
  void readFromFile(const ScenarioReader& reader) override;
  void clear() override;

private:
  int order_;
  QString krigingMethod_;
};

} // namespace ua

} // namespace casaWizard
