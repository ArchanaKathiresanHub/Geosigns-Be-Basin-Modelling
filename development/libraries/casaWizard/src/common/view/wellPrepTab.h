//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QWidget>

class QComboBox;
class QPushButton;
class QSpinBox;
class EmphasisButton;

namespace casaWizard
{

class CalibrationTargetTable;
class ImportWellPopup;

class WellPrepTab : public QWidget
{
  Q_OBJECT

public:
  explicit WellPrepTab(QWidget* parent = nullptr);

  CalibrationTargetTable* calibrationTargetTable() const;
  const EmphasisButton* openDataFileButton() const;

  const QComboBox* smoothingOptions() const;
  const QSpinBox* smoothingLength() const;
  const QPushButton* buttonApplySmoothing() const;

  const QComboBox* subsamplingOptions() const;
  const QSpinBox* subsamplingDistance() const;
  const QPushButton* buttonApplySubsampling() const;

  const QPushButton* buttonVPtoDT() const;
  const QPushButton* buttonDTtoTWT() const;
  QPushButton* buttonCropOutline() const;
  QPushButton* buttonCropBasement() const;
  const QPushButton* buttonExport() const;
  QPushButton* buttonToSAC() const;

  const QPushButton* buttonSelectAll() const;
  const QPushButton* buttonDeselectAll() const;

  ImportWellPopup* importWellPopup() const;

private:
  CalibrationTargetTable* calibrationTargetWellPrepTable_;

  EmphasisButton* openDataFileButton_;

  QComboBox* smoothingOptions_;
  QSpinBox* smoothingLength_;
  QPushButton* buttonApplySmoothing_;

  QComboBox* subsamplingOptions_;
  QSpinBox* subsamplingDistance_;
  QPushButton* buttonApplySubsampling_;

  QPushButton* buttonVPtoDT_;
  QPushButton* buttonDTtoTWT_;
  QPushButton* buttonCropOutline_;
  QPushButton* buttonCropBasement_;
  QPushButton* buttonExport_;
  QPushButton* buttonToSAC_;

  QPushButton* buttonSelectAll_;
  QPushButton* buttonDeselectAll_;

  ImportWellPopup* importWellPopup_;
};

} // namespace casaWizard
