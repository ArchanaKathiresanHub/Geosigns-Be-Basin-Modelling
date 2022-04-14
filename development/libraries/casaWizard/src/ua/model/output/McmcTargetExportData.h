//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QMap>
#include <QVector>
#include <QString>
#include <QStringList>

#include <set>
#include <math.h>

namespace casaWizard
{

namespace ua
{

struct TargetDataSingleProperty
{
   TargetDataSingleProperty():
      p10(double(NAN)),
      p50(double(NAN)),
      p90(double(NAN)),
      baseSim(double(NAN)),
      baseProxy(double(NAN)),
      optimalSim(double(NAN)),
      optimalProxy(double(NAN))
   {}

   double p10;
   double p50;
   double p90;
   double baseSim;
   double baseProxy;
   double optimalSim;
   double optimalProxy;
};

struct McmcSingleTargetExportData
{
   McmcSingleTargetExportData():
      locationName("Unknown"),
      xCor(double(NAN)),
      yCor(double(NAN)),
      zCor(double(NAN)),
      stratigraphicSurface("N/A"),
      age(double(NAN))
   {}

   QString locationName;
   double xCor;
   double yCor;
   double zCor;
   QString stratigraphicSurface;
   double age;

   QMap<QString,TargetDataSingleProperty> targetOutputs;
};

struct McmcTargetExportData
{
   std::set<QString> allPropNames; //Using std::set instead of QSet for sorting
   QMap<QString, McmcSingleTargetExportData> targetData;
};

} // namespace ua

} // namespace casaWizard

