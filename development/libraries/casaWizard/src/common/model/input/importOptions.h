//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QMap>
#include <QPair>
#include <QString>
#include <QVector>

namespace casaWizard
{

struct ImportOptionsLAS
{
  // Properties for all files
  QString depthUserPropertyName = "";
  QMap<QString, double> userPropertyNameToUnitConversion = {};
  bool allFilesHaveElevation = true;
  bool singleFile = true;

  // Properties which differ per file
  bool correctForElevation = false;
  double elevationCorrection = 0.0;
  QString elevationCorrectionUnit = "";
  double referenceCorrection = 0.0;
  QString referenceCorrectionUnit = "";
  int depthColumn = 0;
  double undefinedValue = -99999;
  bool wrapped = false;
  double lasVersion = -1.0;
};

struct ImportOptionsVSET
{  
  QString wellIdentifierName = "XXX";
  int distance = 1e4;
  QVector<QPair<double,double>> xyPairs = {};
  int interval = 1e2;
  double undefinedValue = -99999;
  bool depthNotTWT = true;  
};

} // namespace casaWizard
