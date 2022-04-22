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
  explicit PredictionTargetTable(QWidget* parent = 0);

  void updateTable(const QVector<const PredictionTarget*> predictionTargets,
                   const QVector<QString>& predictionTargetOptions,
                   const QVector<bool>& hasTimeSeriesForPredictionTargets,
                   const QStringList validLayerNames);

  const QTableWidget* tableWidgetSurfaceTargets() const;
  const QPushButton* pushButtonAddSurfaceTarget() const;
  const QPushButton* pushButtonDelSurfaceTarget() const;
  const QPushButton* pushButtonCopySurfaceTarget() const;

  int checkBoxColumnNumber() const;

signals:
  void activePropertyCheckBoxChanged(int, int, QString);
  void targetHasTimeSeriesChanged(int, int);

private:
  QTableWidget* tableWidgetTargets_;
  QPushButton* pushButtonAddTarget_;
  QPushButton* pushButtonDelTarget_;
  QPushButton* pushButtonCopyTarget_;
  int checkBoxColumnNumber_;
  QVector<int> propertyColumns_;
};

} // namespace ua

} // namespace casaWizard
