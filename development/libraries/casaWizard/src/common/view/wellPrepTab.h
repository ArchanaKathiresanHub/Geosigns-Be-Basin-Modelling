//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QWidget>

class QPushButton;
class QSpinBox;
class QDoubleSpinBox;
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

  int smoothingLength() const;
  const QPushButton* buttonApplySmoothing() const;

  int subsamplingDistance() const;
  const QPushButton* buttonApplySubsampling() const;

  double scalingFactor() const;
  const QPushButton* buttonApplyScaling() const;

  const QPushButton* buttonApplyCutOff() const;

  const QPushButton* buttonVPtoDT() const;
  QPushButton* buttonDTtoTWT() const;
  QPushButton* buttonCropOutline() const;
  QPushButton* buttonCropBasement() const;
  const QPushButton* buttonExportToCSV() const;
  const QPushButton* buttonExport() const;
  QPushButton* buttonToSAC() const;

  const QPushButton* buttonDeleteSelection() const;
  const QPushButton* buttonSelectAll() const;
  const QPushButton* buttonDeselectAll() const;

private:
  CalibrationTargetTable* calibrationTargetWellPrepTable_;

  EmphasisButton* openDataFileButton_;

  QSpinBox* smoothingLength_;
  QPushButton* buttonApplySmoothing_;

  QSpinBox* subsamplingDistance_;
  QPushButton* buttonApplySubsampling_;

  QDoubleSpinBox* scalingFactor_;
  QPushButton* buttonApplyScaling_;

  QPushButton* buttonApplyCutOff_;

  QPushButton* buttonVPtoDT_;
  QPushButton* buttonDTtoTWT_;
  QPushButton* buttonCropOutline_;
  QPushButton* buttonCropBasement_;
  QPushButton* buttonExportCSV_;
  QPushButton* buttonExport_;
  QPushButton* buttonToSAC_;

  QPushButton* buttonDeleteSelection_;
  QPushButton* buttonSelectAll_;
  QPushButton* buttonDeselectAll_;
};

} // namespace casaWizard
