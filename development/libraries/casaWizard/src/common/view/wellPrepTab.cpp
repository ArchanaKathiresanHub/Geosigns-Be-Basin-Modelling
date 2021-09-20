//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "wellPrepTab.h"

#include "calibrationTargetTable.h"

#include "view/components/emphasisbutton.h"
#include "view/components/customtitle.h"
#include "view/importWellPopup.h"

#include <QComboBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QSpinBox>
#include <QVBoxLayout>

namespace casaWizard
{

WellPrepTab::WellPrepTab(QWidget* parent) :
  QWidget(parent),
  calibrationTargetWellPrepTable_{new CalibrationTargetTable(this)},
  openDataFileButton_{new EmphasisButton("Add data from file", this)},
  smoothingOptions_{new QComboBox(this)},
  smoothingLength_{new QSpinBox(this)},
  buttonApplySmoothing_{new QPushButton("Apply smoothing", this)},
  subsamplingOptions_{new QComboBox(this)},
  subsamplingDistance_{new QSpinBox(this)},
  buttonApplySubsampling_{new QPushButton("Apply subsampling", this)},
  buttonVPtoDT_{new QPushButton("Convert VP log to DT log", this)},
  buttonDTtoTWT_{new QPushButton("Convert DT log to TWT log", this)},
  buttonCropOutline_{new QPushButton("Remove locations outside basin outline", this)},
  buttonCropBasement_{new QPushButton("Remove data below basement", this)},
  buttonExport_{new QPushButton("Export selected wells", this)},
  buttonToSAC_{new QPushButton("Save selection for SAC", this)},
  buttonSelectAll_{new QPushButton("Select all",this)},
  buttonDeselectAll_{new QPushButton("Deselect all",this)},
  importWellPopup_{new ImportWellPopup()}
{  
  QVBoxLayout* optionsLayout = new QVBoxLayout();

  optionsLayout->addWidget(openDataFileButton_, 0, Qt::AlignTop);

  optionsLayout->addSpacing(20);
  optionsLayout->addWidget(new CustomTitle("Editing options", this), 0, Qt::AlignLeft);

  smoothingOptions_->setMinimumContentsLength(20);
  smoothingOptions_->setMinimumWidth(150);
  smoothingOptions_->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed));
  smoothingOptions_->addItem("None");
  QHBoxLayout* smoothingLayout = new QHBoxLayout();
  smoothingLayout->addWidget(new QLabel("Smoothing: ", this), 0, Qt::AlignLeft);
  smoothingLayout->addWidget(smoothingOptions_, 1, Qt::AlignRight);
  optionsLayout->addLayout(smoothingLayout);
  smoothingLength_->setMinimumWidth(150);
  smoothingLength_->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed));
  smoothingLength_->setMinimum(0);
  QHBoxLayout* smoothingLengthLayout = new QHBoxLayout();
  smoothingLengthLayout->addWidget(new QLabel("Length [m]: ", this), 0, Qt::AlignLeft);
  smoothingLengthLayout->addWidget(smoothingLength_, 1, Qt::AlignRight);
  optionsLayout->addLayout(smoothingLengthLayout);
  optionsLayout->addWidget(buttonApplySmoothing_);

  subsamplingOptions_->setMinimumContentsLength(20);
  subsamplingOptions_->setMinimumWidth(150);
  subsamplingOptions_->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed));
  subsamplingOptions_->addItem("None");
  QHBoxLayout* subsamplingLayout = new QHBoxLayout();
  subsamplingLayout->addWidget(new QLabel("Subsampling: ", this), 0, Qt::AlignLeft);
  subsamplingLayout->addWidget(subsamplingOptions_, 1, Qt::AlignRight);
  optionsLayout->addLayout(subsamplingLayout);
  subsamplingDistance_->setMinimumWidth(150);
  subsamplingDistance_->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed));
  subsamplingDistance_->setMinimum(0);
  QHBoxLayout* subsamplingDistanceLayout = new QHBoxLayout();
  subsamplingDistanceLayout->addWidget(new QLabel("Distance [m]: ", this), 0, Qt::AlignLeft);
  subsamplingDistanceLayout->addWidget(subsamplingDistance_, 1, Qt::AlignRight);
  optionsLayout->addLayout(subsamplingDistanceLayout);
  optionsLayout->addWidget(buttonApplySubsampling_);

  optionsLayout->addSpacing(20);

  optionsLayout->addWidget(new CustomTitle("Data type conversions", this), 0, Qt::AlignLeft);
  optionsLayout->addWidget(buttonVPtoDT_);
  optionsLayout->addWidget(buttonDTtoTWT_);

  optionsLayout->addSpacing(20);

  optionsLayout->addWidget(new CustomTitle("Cropping", this), 0, Qt::AlignLeft);
  optionsLayout->addWidget(buttonCropOutline_);
  optionsLayout->addWidget(buttonCropBasement_);

  optionsLayout->addSpacing(50);
  optionsLayout->addWidget(new QWidget(this), 1);

  optionsLayout->addWidget(buttonExport_);
  optionsLayout->addWidget(buttonToSAC_);


  QHBoxLayout* calibrationOptionsLayout = new QHBoxLayout();
  calibrationOptionsLayout->addWidget(new CustomTitle("Data locations", this), 0, Qt::AlignLeft);
  calibrationOptionsLayout->addWidget(new QWidget(this), 1);

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

const QComboBox* WellPrepTab::smoothingOptions() const
{
  return smoothingOptions_;
}

const QSpinBox* WellPrepTab::smoothingLength() const
{
  return smoothingLength_;
}

const QPushButton* WellPrepTab::buttonApplySmoothing() const
{
  return buttonApplySmoothing_;
}

const QComboBox* WellPrepTab::subsamplingOptions() const
{
  return subsamplingOptions_;
}

const QSpinBox* WellPrepTab::subsamplingDistance() const
{
  return subsamplingDistance_;
}

const QPushButton* WellPrepTab::buttonApplySubsampling() const
{
  return buttonApplySubsampling_;
}

const QPushButton* WellPrepTab::buttonVPtoDT() const
{
  return buttonVPtoDT_;
}

const QPushButton* WellPrepTab::buttonDTtoTWT() const
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

const QPushButton* WellPrepTab::buttonExport() const
{
  return buttonExport_;
}

QPushButton* WellPrepTab::buttonToSAC() const
{
  return buttonToSAC_;
}

const QPushButton* WellPrepTab::buttonSelectAll() const
{
  return buttonSelectAll_;
}

const QPushButton* WellPrepTab::buttonDeselectAll() const
{
  return buttonDeselectAll_;
}

ImportWellPopup* WellPrepTab::importWellPopup() const
{
  return importWellPopup_;
}

} // namespace casaWizard
