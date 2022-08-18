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
      z(0)
   {}

   explicit UAResultsTargetData(const PredictionTarget& predictionTarget):
      targetIndex(-1),
      locationName(predictionTarget.locationName()),
      x(predictionTarget.x()),
      y(predictionTarget.y()),
      z(predictionTarget.z()),
      surfaceName(predictionTarget.surfaceName())
   {}

   int targetIndex;
   QString locationName;
   double x;
   double y;
   double z;
   QString surfaceName;
   QVector<bool> propertyStates; //enabled or disabled
};

class UAResultsTargetsData
{
public:
   UAResultsTargetsData();

   UAResultsTargetsData(const QVector<const PredictionTarget*> predictionTargets,
                        const QVector<QString>& targetProperties);

   const QVector<UAResultsTargetData>& targetData() const;

   void setData(const QVector<const PredictionTarget*> predictionTargets,
           const QVector<QString>& targetProperties);

   void sortData(int column);

   int targetIndex(int rowIdx) const;
   int rowIndex(int targetIdx) const;

private:
   QVector<UAResultsTargetData> m_targetData;
   QVector<QString> m_targetProperties;
   int m_sortedByCol;
};

} //ua
} //casaWizard

