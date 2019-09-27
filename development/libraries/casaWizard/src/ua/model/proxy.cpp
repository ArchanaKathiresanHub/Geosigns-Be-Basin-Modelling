#include "proxy.h"

#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include <QStringList>

namespace casaWizard
{

namespace ua
{

const int defaultOrder{Proxy::orderOptions()[2]};
const QString defaultKrigingMethod{Proxy::krigingMethodOptions()[0]};

Proxy::Proxy() :
  order_{defaultOrder},
  krigingMethod_{defaultKrigingMethod}
{
}

const QVector<int> Proxy::orderOptions()
{
  return {-1, 0, 1, 2, 3};
}

const QStringList Proxy::krigingMethodOptions()
{
  return {"No", "Local", "Global"};
}

int Proxy::order() const
{
  return order_;
}

void Proxy::setOrder(int order)
{
  order_ = order;
}

QString Proxy::krigingMethod() const
{
  return krigingMethod_;
}

void Proxy::setKrigingMethod(const QString& krigingMethod)
{
  krigingMethod_ = krigingMethod;
}

void Proxy::writeToFile(ScenarioWriter& writer) const
{
  writer.writeValue("proxyOrder", order_);
  writer.writeValue("proxyKrigingMethod", krigingMethod_);
}

void Proxy::readFromFile(const ScenarioReader& reader)
{
  order_ = reader.readInt("proxyOrder");
  krigingMethod_ = reader.readString("proxyKrigingMethod");
}

void Proxy::clear()
{
  order_ = defaultOrder;
  krigingMethod_ = defaultKrigingMethod;
}

} // namespace ua

} // namespace casaWizard
