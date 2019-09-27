#include "targetParameterMapCreator.h"

namespace casaWizard
{

namespace targetParameterMapCreator
{

QMap<QString, QString> targetParametersMap()
{
  QMap<QString, QString> targetParameterMap;

  targetParameterMap.insert("Temperature", "[degrees C]");
  targetParameterMap.insert("VRe", "[%]");

  return targetParameterMap;
}

QString lookupSIUnit(const QString& key)
{
 return targetParametersMap()[key];
}

} // namespace targetParameterMapCreator

} // namespace casaWizard
