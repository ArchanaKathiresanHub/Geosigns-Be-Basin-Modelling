//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "wellPrepTab.h"

#include "calibrationTargetTable.h"

#include "view/sharedComponents/emphasisbutton.h"
#include "view/sharedComponents/customtitle.h"
#include "view/sharedComponents/helpLabel.h"
#include "view/importWellPopup.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QVBoxLayout>

namespace casaWizard
{

WellPrepTab::WellPrepTab(QWidget* parent) :
  QWidget(parent),
  calibrationTargetWellPrepTable_{new CalibrationTargetTable(this)},
  openDataFileButton_{new EmphasisButton("Add data from file", this)},
  smoothingLength_{new QSpinBox(this)},
  buttonApplySmoothing_{new QPushButton("Apply smoothing", this)},
  subsamplingDistance_{new QSpinBox(this)},
  buttonApplySubsampling_{new QPushButton("Apply subsampling", this)},
  scalingFactor_{new QDoubleSpinBox(this)},
  buttonApplyScaling_{new QPushButton("Apply scaling", this)},
  buttonApplyCutOff_{new QPushButton("Apply cut off ranges", this)},
  buttonVPtoDT_{new QPushButton("Convert VP log to DT log", this)},
  buttonDTtoTWT_{new QPushButton("Convert DT log to TWT log", this)},
  buttonCropOutline_{new QPushButton("Remove locations outside basin outline", this)},
  buttonCropBasement_{new QPushButton("Remove data below basement and above mudline", this)},
  buttonExportCSV_{new QPushButton("Export wells to CSV", this)},
  buttonExport_{new QPushButton("Export selected wells", this)},
  buttonToSAC_{new QPushButton("Save selection for SAC", this)},
  buttonDeleteSelection_{new QPushButton("Delete selection",this)},
  buttonSelectAll_{new QPushButton("Select all",this)},
  buttonDeselectAll_{new QPushButton("Deselect all",this)}
{  
  QVBoxLayout* optionsLayout = new QVBoxLayout();

  optionsLayout->addWidget(openDataFileButton_, 0, Qt::AlignTop);

  optionsLayout->addStretch(1);

  optionsLayout->addWidget(new CustomTitle("Editing options", this), 0, Qt::AlignLeft);

  QHBoxLayout* smoothingLayout = new QHBoxLayout();
  smoothingLayout->addWidget(new QLabel("Smoothing: ", this), 0, Qt::AlignLeft);
  optionsLayout->addLayout(smoothingLayout);
  smoothingLength_->setMinimumWidth(150);
  smoothingLength_->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed));
  smoothingLength_->setMinimum(1);
  smoothingLength_->setMaximum(1e5);
  smoothingLength_->setValue(20); //Default
  QHBoxLayout* smoothingLengthLayout = new QHBoxLayout();
  smoothingLengthLayout->addWidget(new QLabel("Length [m]: ", this), 0, Qt::AlignLeft);
  smoothingLengthLayout->addWidget(smoothingLength_, 1, Qt::AlignRight);
  optionsLayout->addLayout(smoothingLengthLayout);
  optionsLayout->addWidget(buttonApplySmoothing_);

  QHBoxLayout* subsamplingLayout = new QHBoxLayout();
  subsamplingLayout->addWidget(new QLabel("Subsampling: ", this), 0, Qt::AlignLeft);
  optionsLayout->addLayout(subsamplingLayout);
  subsamplingDistance_->setMinimumWidth(150);
  subsamplingDistance_->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed));
  subsamplingDistance_->setMinimum(1);
  subsamplingDistance_->setMaximum(1e5);
  subsamplingDistance_->setValue(10); //Default
  QHBoxLayout* subsamplingDistanceLayout = new QHBoxLayout();
  subsamplingDistanceLayout->addWidget(new QLabel("Distance [m]: ", this), 0, Qt::AlignLeft);
  subsamplingDistanceLayout->addWidget(subsamplingDistance_, 1, Qt::AlignRight);
  optionsLayout->addLayout(subsamplingDistanceLayout);
  optionsLayout->addWidget(buttonApplySubsampling_);

  QHBoxLayout* scalingLayout = new QHBoxLayout();
  scalingLayout->addWidget(new QLabel("Scaling: ", this), 0, Qt::AlignLeft);
  optionsLayout->addLayout(scalingLayout);
  scalingFactor_->setMinimumWidth(150);
  scalingFactor_->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed));
  scalingFactor_->setValue(1); //Default
  scalingFactor_->setMinimum(-1.0e5);
  scalingFactor_->setMaximum(1.0e5);
  QHBoxLayout* scalingFactorLayout = new QHBoxLayout();
  scalingFactorLayout->addWidget(new QLabel("Scaling factor: ", this), 0, Qt::AlignLeft);
  scalingFactorLayout->addWidget(scalingFactor_, 1, Qt::AlignRight);
  optionsLayout->addLayout(scalingFactorLayout);
  optionsLayout->addWidget(buttonApplyScaling_);

  QHBoxLayout* cutOffLayout = new QHBoxLayout();
  cutOffLayout->addWidget(new QLabel("Cut off: ", this), 0, Qt::AlignLeft);
  optionsLayout->addLayout(cutOffLayout);
  optionsLayout->addWidget(buttonApplyCutOff_);

  optionsLayout->addStretch(1);

  QHBoxLayout* dataTypeConversions = new QHBoxLayout();
  dataTypeConversions->addWidget(new CustomTitle("Data type conversions", this), Qt::AlignLeft);
  HelpLabel* helpLabeldataTypeConversion = new HelpLabel(this, "Use this section to convert data types for the selected wells. "
                                                               "To enable a DT to TWT conversion, first run an Original 1d run "
                                                               "on the Input Tab.");
  dataTypeConversions->addWidget(helpLabeldataTypeConversion);
  optionsLayout->addLayout(dataTypeConversions);

  optionsLayout->addWidget(buttonVPtoDT_);
  optionsLayout->addWidget(buttonDTtoTWT_);

  optionsLayout->addStretch(1);

  QHBoxLayout* cropping = new QHBoxLayout();
  cropping->addWidget(new CustomTitle("Cropping", this), Qt::AlignLeft);
  HelpLabel* helpLabelCropping = new HelpLabel(this, "Use this section to remove data outside of the basin model. To enable this "
                                                     "functionality, select a Project File in the Input Tab");
  cropping->addWidget(helpLabelCropping);
  optionsLayout->addLayout(cropping);

  optionsLayout->addWidget(buttonCropOutline_);
  optionsLayout->addWidget(buttonCropBasement_);

  optionsLayout->addStretch(2);

  optionsLayout->addWidget(new QWidget(this), 1);

  optionsLayout->addWidget(buttonExportCSV_);
  optionsLayout->addWidget(buttonExport_);
  optionsLayout->addWidget(buttonToSAC_);

  QHBoxLayout* calibrationOptionsLayout = new QHBoxLayout();
  calibrationOptionsLayout->addWidget(new CustomTitle("Data locations", this), 0, Qt::AlignLeft);
  calibrationOptionsLayout->addWidget(new QWidget(this), 1);

  calibrationOptionsLayout->addWidget(buttonDeleteSelection_, 0, Qt::AlignRight);
  calibrationOptionsLayout->addWidget(buttonSelectAll_, 0, Qt::AlignRight);
  calibrationOptionsLayout->addWidget(buttonDeselectAll_, 0, Qt::AlignRight);

  QVBoxLayout* calibrationsLayout = new QVBoxLayout();
  calibrationsLayout->addLayout(calibrationOptionsLayout, 0);
  calibrationsLayout->addWidget(calibrationTargetWellPrepTable_, 1);

  QHBoxLayout* horizontalLayoutTab = new QHBoxLayout(this);
  horizontalLayoutTab->addLayout(optionsLayout, 0);
  horizontalLayoutTab->addLayout(calibrationsLayout, 1);

  optionsLayout->setMargin(10);
  calibrationsLayout->setMargin(10);
}

CalibrationTargetTable* WellPrepTab::calibrationTargetTable() const
{
  return calibrationTargetWellPrepTable_;
}

const EmphasisButton* WellPrepTab::openDataFileButton() const
{
  return openDataFileButton_;
}

int WellPrepTab::smoothingLength() const
{
  return smoothingLength_->value();
}

const QPushButton* WellPrepTab::buttonApplySmoothing() const
{
  return buttonApplySmoothing_;
}

int WellPrepTab::subsamplingDistance() const
{
  return subsamplingDistance_->value();
}

double WellPrepTab::scalingFactor() const
{
  return scalingFactor_->value();
}

const QPushButton* WellPrepTab::buttonApplyScaling() const
{
  return buttonApplyScaling_;
}

const QPushButton* WellPrepTab::buttonApplySubsampling() const
{
  return buttonApplySubsampling_;
}

const QPushButton* WellPrepTab::buttonApplyCutOff() const
{
  return buttonApplyCutOff_;
}

const QPushButton* WellPrepTab::buttonVPtoDT() const
{
  return buttonVPtoDT_;
}

QPushButton* WellPrepTab::buttonDTtoTWT() const
{
  return buttonDTtoTWT_;
}

QPushButton* WellPrepTab::buttonCropOutline() const
{
  return buttonCropOutline_;
}

QPushButton* WellPrepTab::buttonCropBasement() const
{
  return buttonCropBasement_;
}

const QPushButton* WellPrepTab::buttonExportToCSV() const
{
  return buttonExportCSV_;
}

const QPushButton* WellPrepTab::buttonExport() const
{
  return buttonExport_;
}

QPushButton* WellPrepTab::buttonToSAC() const
{
  return buttonToSAC_;
}

const QPushButton* WellPrepTab::buttonDeleteSelection() const
{
  return buttonDeleteSelection_;
}

const QPushButton* WellPrepTab::buttonSelectAll() const
{
  return buttonSelectAll_;
}

const QPushButton* WellPrepTab::buttonDeselectAll() const
{
  return buttonDeselectAll_;
}

} // namespace casaWizard
