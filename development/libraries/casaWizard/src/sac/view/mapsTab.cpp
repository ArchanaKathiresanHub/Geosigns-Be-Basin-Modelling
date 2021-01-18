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
#include <QPushButton>
#include <QSpinBox>

namespace casaWizard
{

namespace sac
{

MapsTab::MapsTab(QWidget* parent) :
  QWidget(parent),
  createGridsButton_{new EmphasisButton("Create 2D lithofraction grids", this)},
  interpolationType_{new QComboBox(this)},
  iwdOptions_{new QWidget(this)},
  gridGenerationOptions_{new QWidget(this)},
  pValue_{new QSpinBox(this)},
  smoothingOptions_{new QWidget(this)},
  smoothingRadius_{new QSpinBox(this)},
  smoothingType_{new QComboBox(this)},
  threads_{new QSpinBox(this)},
  activeWellsTable_{new ActiveWellsTable(this)},
  lithofractionVisualisation_{new LithofractionVisualisation(this)}
{
  QVBoxLayout* wellsAndOptions = setWellsAndOptionsLayout();
  setTotalLayout(wellsAndOptions);

  connectSignalsAndSlots();

  slotInterpolationTypeChange(0);
  slotSmoothingTypeChange(0);
}

QVBoxLayout* MapsTab::setWellsAndOptionsLayout()
{
  setGridGenerationOptionsLayout();
  setDefaultGridGenerationOptions();

  CustomTitle* label1 = new CustomTitle("Wells", this);
  label1->setFixedHeight(15);
  CustomTitle* label2 = new CustomTitle("Gridding options", this);
  label2->setFixedHeight(15);

  QVBoxLayout* wellsAndOptions = new QVBoxLayout();
  wellsAndOptions->addWidget(label1,0);
  wellsAndOptions->addWidget(activeWellsTable_, 1);
  wellsAndOptions->addWidget(label2, 2);
  wellsAndOptions->addWidget(gridGenerationOptions_, 3);
  wellsAndOptions->setStretch(0,1);
  wellsAndOptions->setStretch(1,20);
  wellsAndOptions->setStretch(2,1);
  wellsAndOptions->setStretch(3,1);

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

void MapsTab::setTotalLayout(QVBoxLayout* wellsAndOptions)
{
  QHBoxLayout* total = new QHBoxLayout(this);
  total->addLayout(wellsAndOptions, 0);
  total->addWidget(lithofractionVisualisation_,1);
  total->setStretch(0,1);
  total->setStretch(1,5);
}

void MapsTab::connectSignalsAndSlots() const
{
  connect(interpolationType_, SIGNAL(currentIndexChanged(int)), this, SLOT(slotInterpolationTypeChange(int)));
  connect(smoothingType_,     SIGNAL(currentIndexChanged(int)), this, SLOT(slotSmoothingTypeChange(int)));
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

void MapsTab::updateBirdsView(const QVector<const Well*> wells)
{
  QVector<double> x;
  QVector<double> y;
  for (const Well *const well : wells)
  {
    x.append(well->x());
    y.append(well->y());
  }

  for (Grid2DPlot* plot: lithofractionVisualisation()->lithoFractionPlots())
  {
    plot->grid2DView()->setWellLocations(x, y);
  }

}

} // namespace sac

} // namespace casaWizard
