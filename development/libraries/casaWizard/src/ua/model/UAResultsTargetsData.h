//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "predictionTarget.h"

#include <QString>
#include <QVector>

namespace casaWizard
{

namespace ua
{

struct UAResultsTargetData
{
   UAResultsTargetData():
      targetIndex(-1),
      x(0),
      y(0),
      z(0),
      hasTimeSeriesData(false)
   {}

   explicit UAResultsTargetData(const PredictionTarget& predictionTarget, bool hasTimeSeries = false):
      targetIndex(-1),
      locationName(predictionTarget.locationName()),
      x(predictionTarget.x()),
      y(predictionTarget.y()),
      z(predictionTarget.z()),
      surfaceName(predictionTarget.surfaceName()),
      hasTimeSeriesData(hasTimeSeries)
   {}

   int targetIndex;
   QString locationName;
   double x;
   double y;
   double z;
   QString surfaceName;
   QVector<bool> propertyStates; //enabled or disabled
   bool hasTimeSeriesData;
};

class UAResultsTargetsData
{
public:
   UAResultsTargetsData();

   UAResultsTargetsData(const QVector<const PredictionTarget*> predictionTargets,
                        const QVector<QString>& targetProperties,
                        const QVector<bool>& hasTimeSeries);

   const QVector<UAResultsTargetData>& targetData() const;

   void setData(const QVector<const PredictionTarget*> predictionTargets,
                const QVector<QString>& targetProperties,
                const QVector<bool>& hasTimeSeries);

   void sortData(int column);

   int targetIndex(int rowIdx) const;
   int rowIndex(int targetIdx) const;

   QVector<int> tableRowsWithoutTimeSeries() const;

private:
   QVector<UAResultsTargetData> m_targetData;
   QVector<QString> m_targetProperties;
   int m_sortedByCol;
   int m_selectedRow;
};

} //ua
} //casaWizard

