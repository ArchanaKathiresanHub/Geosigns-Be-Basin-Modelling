//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Model to manage the manual design points
// Design points that are marked as completed can not be deleted
#pragma once

#include "ManualDesignPoint.h"
#include "model/writable.h"

#include <QVector>
#include <QMap>

namespace casaWizard
{

namespace ua
{

class ManualDesignPointManager : public Writable
{
public:
   explicit ManualDesignPointManager();

   void addDesignPoint();
   bool addDesignPoint(const QVector<double> newPoint);
   void removeDesignPoint(const int visiblePointIndex);

   void addInfluentialParameter(const int amount = 1);
   void removeInfluentialParameter(const int index);

   int numberOfVisiblePoints() const;
   int numberOfParameters() const;

   void readAndSetCompletionStates(const QString& stateFileName, const QString& doeIndicesFileName, const QStringList& namesDoeOptionsSelected);

   void setCompleted(int idx, bool completionState = true);
   bool isCompleted(int visiblePointIndex) const;
   void setAllIncomplete();
   void removeHiddenPoints();
   void setParameterValue(const int visiblePointIndex, const int parameterIndex, const double value);

   int numberOfCasesToRun() const;

   QVector<QVector<double>> pointsToRun() const;
   QVector<QVector<double>> parameters() const;
   QVector<bool> completed() const;

   void writeToFile(ScenarioWriter& writer) const override;
   void readFromFile(const ScenarioReader& reader) override;
   void clear() override;

private:
   ManualDesignPointManager(const ManualDesignPointManager&) = delete;
   ManualDesignPointManager& operator=(ManualDesignPointManager) = delete;

   void updateVisualIndexToPointIndexMap();
   bool pointTooCloseToExistingPoints(const QVector<double> point) const;

   QMap<int,int> m_visualIndexToPointIndexMap;

   QVector<ManualDesignPoint> m_manualDesignPoints;
   int m_numberOfParameters;
};

} // namespace ua

} // namespace casaWizard
