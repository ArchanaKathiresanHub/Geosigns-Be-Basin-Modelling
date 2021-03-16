//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "mapsTab.h"

#include "../common/view/components/customtitle.h"
#include "../common/view/components/emphasisbutton.h"

#include "model/well.h"
#include "view/activeWellsTable.h"
#include "view/grid2dplot.h"
#include "view/plot/grid2Dview.h"
#include "view/lithofractionVisualisation.h"

#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QSpinBox>

namespace casaWizard
{

namespace sac
{

MapsTab::MapsTab(QWidget* parent) :
  QWidget(parent),
  activeWellsTable_{new ActiveWellsTable(this)},
  lithofractionVisualisation_{new LithofractionVisualisation(this)},
  createGridsButton_{new EmphasisButton("Create 2D lithofraction grids", this)},
  buttonExportOptimized_{new QPushButton("Export optimized", this)},
  buttonRunOptimized_{new QPushButton("Run optimized", this)},
  buttonRunOriginal_{new QPushButton("Run original", this)},
  interpolationType_{new QComboBox(this)},
  iwdOptions_{new QWidget(this)},
  gridGenerationOptions_{new QWidget(this)},
  pValue_{new QSpinBox(this)},
  smoothingOptions_{new QWidget(this)},
  smoothingRadius_{new QSpinBox(this)},
  smoothingType_{new QComboBox(this)},
  threads_{new QSpinBox(this)}
{
  setTotalLayout();
  connectSignalsAndSlots();

  slotInterpolationTypeChange(0);
  slotSmoothingTypeChange(0);
}

void MapsTab::updateSelectedWells(const QVector<int> selectedWells)
{
  lithofractionVisualisation_->updateSelectedWells(selectedWells);
}

QVBoxLayout* MapsTab::setWellsAndOptionsLayout()
{
  setGridGenerationOptionsLayout();
  setDefaultGridGenerationOptions();

  CustomTitle* label1 = new CustomTitle("Wells", this);
  label1->setFixedHeight(15);
  CustomTitle* label2 = new CustomTitle("Gridding options", this);
  label2->setFixedHeight(15);
  CustomTitle* label3 = new CustomTitle("3D model", this);
  label3->setFixedHeight(15);

  QVBoxLayout* wellsAndOptions = new QVBoxLayout();
  wellsAndOptions->addWidget(label1);
  wellsAndOptions->addWidget(activeWellsTable_);
  wellsAndOptions->addWidget(label2);
  wellsAndOptions->addWidget(gridGenerationOptions_);
  wellsAndOptions->addWidget(label3);
  wellsAndOptions->addWidget(buttonExportOptimized_);
  wellsAndOptions->addWidget(buttonRunOptimized_);
  wellsAndOptions->addWidget(buttonRunOriginal_);

  return wellsAndOptions;
}

void MapsTab::setGridGenerationOptionsLayout()
{
  setIdwOptionsLayout();
  setSmoothingOptionsLayout();

  QGridLayout* optimizationOptionsLayout = new QGridLayout(gridGenerationOptions_);
  optimizationOptionsLayout->addWidget(new QLabel("Interpolation: ", this),0,0);
  optimizationOptionsLayout->addWidget(interpolationType_,0,1);
  optimizationOptionsLayout->addWidget(iwdOptions_,1,1);
  optimizationOptionsLayout->addWidget(new QLabel("Smoothing: ", this),2,0);
  optimizationOptionsLayout->addWidget(smoothingType_,2,1);
  optimizationOptionsLayout->addWidget(smoothingOptions_,3,1);
  optimizationOptionsLayout->addWidget(createGridsButton_,4,0,1,2);
  optimizationOptionsLayout->setMargin(0);
}

void MapsTab::setSmoothingOptionsLayout()
{
  QVBoxLayout* smoothingOptionsLayout = new QVBoxLayout(smoothingOptions_);

  QHBoxLayout* radiusOptionLayout = new QHBoxLayout();
  radiusOptionLayout->addWidget(new QLabel(" Radius[m]: ", this));
  radiusOptionLayout->addWidget(smoothingRadius_);

  QHBoxLayout* threadsOptionLayout = new QHBoxLayout();
  threadsOptionLayout->addWidget(new QLabel(" Threads: ", this));
  threadsOptionLayout->addWidget(threads_);

  smoothingOptionsLayout->addLayout(radiusOptionLayout);
  smoothingOptionsLayout->addLayout(threadsOptionLayout);

  smoothingOptionsLayout->setMargin(0);
}

void MapsTab::setIdwOptionsLayout()
{
  QHBoxLayout* idwOptionsLayout = new QHBoxLayout(iwdOptions_);
  idwOptionsLayout->addWidget(new QLabel(" P: ", this));
  idwOptionsLayout->addWidget(pValue_);
  idwOptionsLayout->setStretch(0,1);
  idwOptionsLayout->setStretch(1,4);
  idwOptionsLayout->setMargin(0);
}

void MapsTab::setDefaultGridGenerationOptions()
{
  interpolationType_->insertItems(0, {"Inverse Distance Weighting", "Natural Neighbor"});
  smoothingType_->insertItems(0, {"None", "Gaussian", "Moving Average"});

  pValue_->setMinimum(1);
  pValue_->setMaximum(100);

  threads_->setMinimum(1);
  threads_->setMaximum(1000);

  smoothingRadius_->setMinimum(0);
  smoothingRadius_->setSingleStep(100);
  smoothingRadius_->setMaximum(1e6);
}

void MapsTab::setTotalLayout()
{
  QHBoxLayout* total = new QHBoxLayout(this);
  total->addLayout(setWellsAndOptionsLayout(), 1);
  total->addWidget(lithofractionVisualisation_,5);
}

void MapsTab::connectSignalsAndSlots() const
{
  connect(interpolationType_, SIGNAL(currentIndexChanged(int)), this, SLOT(slotInterpolationTypeChange(int)));
  connect(smoothingType_,     SIGNAL(currentIndexChanged(int)), this, SLOT(slotSmoothingTypeChange(int)));
}

QPushButton* MapsTab::buttonExportOptimized() const
{
  return buttonExportOptimized_;
}

QPushButton* MapsTab::buttonRunOptimized() const
{
  return buttonRunOptimized_;
}

QPushButton* MapsTab::buttonRunOriginal() const
{
  return buttonRunOriginal_;
}

ActiveWellsTable* MapsTab::activeWellsTable() const
{
  return activeWellsTable_;
}

void MapsTab::slotInterpolationTypeChange(int interpolationType)
{
  iwdOptions_->setVisible(interpolationType == 0);
}

void MapsTab::slotSmoothingTypeChange(int smoothingType)
{
  smoothingOptions_->setVisible(smoothingType > 0);
}

LithofractionVisualisation* MapsTab::lithofractionVisualisation() const
{
  return lithofractionVisualisation_;
}

QSpinBox* MapsTab::smoothingRadius() const
{
  return smoothingRadius_;
}

QPushButton*MapsTab::createGridsButton() const
{
  return createGridsButton_;
}

QSpinBox* MapsTab::threads() const
{
  return threads_;
}

QSpinBox* MapsTab::pValue() const
{
  return pValue_;
}

QComboBox* MapsTab::smoothingType() const
{
  return smoothingType_;
}

QComboBox* MapsTab::interpolationType() const
{
  return interpolationType_;
}



} // namespace sac

} // namespace casaWizard
