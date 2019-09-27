// Function to populate prediction (or calibration) target parameter maps.
#pragma once

#include <QMap>
#include <QString>

namespace casaWizard
{

namespace targetParameterMapCreator
{

  QMap<QString, QString> targetParametersMap();
  QString lookupSIUnit(const QString& key);

} // namespace targetParameterMapCreator

} // namespace casaWizard
