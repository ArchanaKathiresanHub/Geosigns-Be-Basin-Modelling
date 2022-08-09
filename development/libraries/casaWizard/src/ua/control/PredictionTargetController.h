//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Controller for the table containing the prediction target. It contains the x, y coordinates, the layer (the top will be tracked) and the variable (like temperature)
#pragma once

#include <QObject>

class QTableWidgetItem;

namespace casaWizard
{

class CalibrationTargetManager;

namespace ua
{

class PredictionTargetTable;
class PredictionTargetManager;
class ProjectReader;

class PredictionTargetController : public QObject
{
   Q_OBJECT

public:
   explicit PredictionTargetController(PredictionTargetTable* table,
                                       PredictionTargetManager& manager,
                                       CalibrationTargetManager& calibrationTargetManager,
                                       QObject* parent);

   void refreshAndEmitDataChanged();
   void enableTargetsAtWellLocationsButton(bool enable);

signals:
  void dataChanged();

public slots:
   void slotRefresh();

private slots:
   void slotPushButtonAddPredictionTargetClicked();
   void slotPushButtonAddTargetWellLocationsClicked();
   void slotPushButtonAddPredictionTargetsClicked();
   void slotPushButtonDelPredictionTargetClicked();
   void slotPushButtonCopyPredictionTargetClicked();
   void slotTableWidgetPredictionTargetsItemChanged(QTableWidgetItem* item);
   void slotActivePropertyCheckBoxChanged(int state, int row, QString property);
   void slotTargetHasTimeSeriesChanged(int state, int row);

private:
   PredictionTargetTable* m_table;
   PredictionTargetManager& m_predictionTargetManager;
   const CalibrationTargetManager& m_calibrationTargetManager;
   QVector<int> getSelectedRows();
};

} // namespace ua

} // namespace casaWizard
