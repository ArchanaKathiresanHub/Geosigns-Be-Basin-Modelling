//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QWidget>

class QPushButton;
class QTableWidget;

namespace casaWizard
{

namespace ua
{

class PredictionTarget;

class PredictionTargetTable : public QWidget
{
   Q_OBJECT

public:
   explicit PredictionTargetTable(QWidget* parent = nullptr);

   void updateTable(const QVector<const PredictionTarget*> predictionTargets,
                    const QVector<QString>& predictionTargetOptions,
                    const QVector<bool>& hasTimeSeriesForPredictionTargets,
                    const QStringList validLayerNames);

   void updateTableAtRow(const PredictionTarget* predictionTarget, const int row);

   const QTableWidget* tableWidgetTargets() const;

   void setTargetsAtWellLocationsButtonEnabled(bool state);

   const QPushButton* pushButtonAddTargetsAtWellLocations() const;
   const QPushButton* pushButtonDelTarget() const;
   const QPushButton* pushButtonCopyTarget() const;
   const QPushButton* pushButtonAddPredictionTargets() const;
   const QPushButton* pushButtonAddTarget() const;

   int checkBoxColumnNumber() const;

signals:
   void activePropertyCheckBoxChanged(int, int, QString);
   void targetHasTimeSeriesChanged(int, int);

private:
   QTableWidget* m_tableWidgetTargets;
   QPushButton* m_pushButtonAddTarget;
   QPushButton* m_pushButtonAddTargetsAtWellLocations;
   QPushButton* m_pushButtonDelTarget;
   QPushButton* m_pushButtonCopyTarget;
   QPushButton* m_pushButtonAddPredictionTargets;
   int m_checkBoxColumnNumber;
   QVector<int> m_propertyColumns;
};

} // namespace ua

} // namespace casaWizard
