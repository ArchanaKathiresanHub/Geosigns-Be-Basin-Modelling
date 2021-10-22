//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QString>
#include <QMap>

namespace casaWizard
{

struct ImportOptions
{
  QString depthUserPropertyName = "";
  QMap<QString, double> userPropertyNameToUnitConversion = {};
  bool correctForElevation = false;
  double elevationCorrection = 0.0;
  QString elevationCorrectionUnit = "";
  int depthColumn = 0;
  double undefinedValue = -99999;
};

} // namespace casaWizard
