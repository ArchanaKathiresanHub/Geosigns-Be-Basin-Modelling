//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//


// Manager for the prediction targets
#pragma once

#include "predictionTargetDepth.h"
#include "predictionTargetSurface.h"
#include "model/writable.h"

#include <QVector>

#include <memory>

namespace casaWizard
{

class ProjectReader;
class ToDepthConverter;

namespace ua
{

class PredictionTargetManager : public Writable
{
public:
   PredictionTargetManager(const ProjectReader& projectReader, const ToDepthConverter& toDepthConverter);
   ~PredictionTargetManager() override;

   const QVector<const PredictionTarget*> predictionTargets() const;
   const QVector<const PredictionTarget*> predictionTargetsIncludingTimeSeries() const;
   const QVector<const PredictionTarget*> predictionTargetInTimeSeries(const int row) const;

   void addDepthTarget(const double x, const double y, const double z, const QVector<QString>& properties = s_targetVariables, const double age = 0.0, QString locationName = "");
   void addSurfaceTarget(const double x, const double y, QString surface = "", const QVector<QString>& properties = s_targetVariables, const double age = 0.0, QString locationName = "");

   void copyTargets(const QVector<int>& indices);
   void removeTargets(QVector<int> indices);
   void setTarget(int row, int column, const QString& text);

   static QVector<QString> predictionTargetOptions();
   void writeToFile(ScenarioWriter& writer) const override;
   void readFromFile(const ScenarioReader& reader) override;
   void clear() override;

   void setTargetHasTimeSeries(const int row, const bool isSelected);

   int amountAtAge0() const;
   int sizeOfPredictionTargetWithTimeSeries(const int row) const;
   int indexCumulativePredictionTarget(const int row) const;

   void setTargetActiveProperty(const bool active, const int row, const QString& property);

   QStringList validSurfaceNames() const;

   QVector<bool> targetHasTimeSeries() const;
   int amountOfPredictionTargetWithTimeSeriesAndProperties() const;

   void setPropertyActiveForAllTargets(const QString& propertyName);
   void setTimeSeriesActiveForAllTargets();

   int getIndexInPredictionTargetMatrix(const int predictionTargetRow, const int snapshotIndex, const QString& propertyName) const;
private:
   PredictionTargetManager(const PredictionTargetManager&) = delete;
   PredictionTargetManager& operator=(PredictionTargetManager) = delete;

   int indexCumulativePredictionTargetIncludingProperties(const int row) const;
   int sizeOfPredictionTargetsWithTimeSeriesIncludingProperties(const int row) const;
   QVector<int> sizeOfPredictionTargetsWithTimeSeriesIncludingProperties() const;
   QVector<int> sizeOfPredictionTargetsWithTimeSeries() const;
   void setPredictionTargetsAllTimes();
   void clearMemory();
   void addTargets(QVector<double> snapshots);

   static QVector<QString> s_targetVariables;

   QVector<std::shared_ptr<PredictionTarget>> m_predictionTargets;
   QVector<const PredictionTarget*> predictionTargetsAllTimes_;
   QVector<bool> m_targetHasTimeSeries;
   const ProjectReader& m_projectReader;
   const ToDepthConverter& m_toDepthConverter;
   int m_identifier;
};

} // namespace ua

} // namespace casaWizard

