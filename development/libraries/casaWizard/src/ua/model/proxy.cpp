#include "proxy.h"

#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"
#include "model/logger.h"

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
   return {0, 1, 2, 3};
}

const QStringList Proxy::krigingMethodOptions()
{
   return {"No", "Global"};
}

int Proxy::order() const
{ 
   return order_;
}

void Proxy::setOrder(int order)
{
   orderOptions().contains(order) ? order_ = order : order_ = defaultOrder;
}

QString Proxy::krigingMethod() const
{
   return krigingMethod_;
}

void Proxy::setKrigingMethod(const QString& krigingMethod)
{
   krigingMethodOptions().contains(krigingMethod) ? krigingMethod_ = krigingMethod : krigingMethod_ = defaultKrigingMethod;
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

   if (!krigingMethodOptions().contains(krigingMethod_))
   {
      krigingMethod_ = defaultKrigingMethod;
      Logger::log() << "Error: Unknown or deprecated kriging method, valid methods: ";
      for (const QString& str : krigingMethodOptions())
      {
         str != krigingMethodOptions().last() ? Logger::log() << "\"" << str <<  "\"/" :
                                                Logger::log() << "\"" << str <<  "\". Defaulting to \"No\"." << Logger::endl();
      }
   }

   if (!orderOptions().contains(order_)){
      order_ = defaultOrder;
      Logger::log() << "Error: Unknown or deprecated order, valid orders: ";
      for (const QString& str : krigingMethodOptions())
      {
         str != krigingMethodOptions().last() ? Logger::log() << "\"" << str <<  "\"/" :
                                                Logger::log() << "\"" << str <<  "\". Defaulting to \"2\"." << Logger::endl();
      }
   }
}

void Proxy::clear()
{
   order_ = defaultOrder;
   krigingMethod_ = defaultKrigingMethod;
}

} // namespace ua

} // namespace casaWizard
