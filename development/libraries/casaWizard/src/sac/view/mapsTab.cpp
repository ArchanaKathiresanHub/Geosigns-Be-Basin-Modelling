//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "mapsTab.h"

#include "../common/view/components/customcheckbox.h"
#include "../common/view/components/customtitle.h"
#include "../common/view/components/emphasisbutton.h"
#include "../common/view/components/helpLabel.h"

#include "model/well.h"
#include "view/activeWellsTable.h"
#include "view/grid2dplot.h"
#include "view/plot/lithoPercent2Dview.h"
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
  buttonExportOptimizedToZycor_{new QPushButton("Export to Zycor", this)},
  buttonRunOptimized_{new QPushButton("Run optimized 3D", this)},
  interpolationType_{new QComboBox(this)},
  iwdOptions_{new QWidget(this)},
  gridGenerationOptions_{new QWidget(this)},
  pValue_{new QSpinBox(this)},
  smoothingOptions_{new QWidget(this)},
  smoothingRadius_{new QSpinBox(this)},
  smoothingType_{new QComboBox(this)},
  smartGridding_{new CustomCheckbox(this)}
{
  setTotalLayout();
  connectSignalsAndSlots();

  slotInterpolationTypeChange(0);
  slotSmoothingTypeChange(0);
}

void MapsTab::disableWellAtIndex(const int index)
{
  CustomCheckbox* itemCheckBox = static_cast<CustomCheckbox*>(activeWellsTable_->cellWidget(index, 0)->children()[1]);
  itemCheckBox->setCheckState(Qt::CheckState::Unchecked);
  itemCheckBox->enable(false);

  QTableWidgetItem* toDisable = activeWellsTable_->item(index, 1);
  toDisable->setTextColor(Qt::red);
  toDisable->setFlags(toDisable->flags().setFlag(Qt::ItemIsEnabled, false));
}

QVBoxLayout* MapsTab::setWellsAndOptionsLayout()
{
  setGridGenerationOptionsLayout();
  setDefaultGridGenerationOptions();

  CustomTitle* label1 = new CustomTitle("Wells", this);
  label1->setFixedHeight(15);
  CustomTitle* label2 = new CustomTitle("Gridding options", this);
  label2->setFixedHeight(15);
  QHBoxLayout* griddingOptions = new QHBoxLayout();
  griddingOptions->addWidget(label2);
  HelpLabel* helpGriddingOptions = new HelpLabel(this, "The gridding options determine how the optimized lithofractions at the well locations "
                                                       "are interpolated to result in complete lithology maps. <br><br>The maps can be tweaked by adjusting: <br>"
                                                       " - The interpolation algorithm <br>"
                                                       " - The smoothing options <br>");
  griddingOptions->addWidget(helpGriddingOptions);

  CustomTitle* label3 = new CustomTitle("3D model", this);
  label3->setFixedHeight(15);

  QVBoxLayout* wellsAndOptions = new QVBoxLayout();
  wellsAndOptions->addWidget(label1);
  wellsAndOptions->addWidget(activeWellsTable_);
  wellsAndOptions->addLayout(griddingOptions);
  wellsAndOptions->addWidget(gridGenerationOptions_);
  wellsAndOptions->addWidget(label3);
  QHBoxLayout* exportOptimized = new QHBoxLayout();
  exportOptimized->addWidget(buttonExportOptimized_);
  HelpLabel* helpLabelExport = new HelpLabel(this, "Create a zip-file for import into BPA2-Basin");
  exportOptimized->addWidget(helpLabelExport);
  wellsAndOptions->addLayout(exportOptimized);
  QHBoxLayout* exportOptimizedToZycor = new QHBoxLayout();
  exportOptimizedToZycor->addWidget(buttonExportOptimizedToZycor_);
  HelpLabel* helpLabelExportToZycor = new HelpLabel(this, "Export the optimized lithofraction maps to the zycor format");
  exportOptimizedToZycor->addWidget(helpLabelExportToZycor);
  wellsAndOptions->addLayout(exportOptimizedToZycor);

  QHBoxLayout* runOptimized = new QHBoxLayout();
  runOptimized->addWidget(buttonRunOptimized_);
  HelpLabel* helpLabelRun = new HelpLabel(this, "For plotting and QC purposes under 'Well log plots and Results' tab");

  runOptimized->addWidget(helpLabelRun);

  wellsAndOptions->addLayout(runOptimized);

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
  optimizationOptionsLayout->addWidget(new QLabel("Smart Gridding: ", this),4,0);
  QHBoxLayout* smartGriddingLayout = new QHBoxLayout();
  smartGriddingLayout->addWidget(smartGridding_);
  smartGriddingLayout->addWidget(new HelpLabel(this, "When checked, selected wells are only included and visualized \nfor gridding in formations where data is present"));
  smartGriddingLayout->setStretch(0,1);
  smartGriddingLayout->setStretch(1,0);
  optimizationOptionsLayout->addLayout(smartGriddingLayout,4,1);

  smartGridding_->setCheckState(Qt::CheckState::Checked);
  optimizationOptionsLayout->addWidget(createGridsButton_,5,0,1,2);
  optimizationOptionsLayout->setMargin(0);
}

void MapsTab::setSmoothingOptionsLayout()
{
  QVBoxLayout* smoothingOptionsLayout = new QVBoxLayout(smoothingOptions_);

  QHBoxLayout* radiusOptionLayout = new QHBoxLayout();
  radiusOptionLayout->addWidget(new QLabel(" Radius[m]: ", this));
  radiusOptionLayout->addWidget(smoothingRadius_);

  smoothingOptionsLayout->addLayout(radiusOptionLayout);  

  smoothingOptionsLayout->setMargin(0);
}

void MapsTab::setIdwOptionsLayout()
{
  QHBoxLayout* idwOptionsLayout = new QHBoxLayout(iwdOptions_);
  idwOptionsLayout->addWidget(new QLabel(" P: ", this));
  idwOptionsLayout->addWidget(pValue_);
  HelpLabel* helpLabelRun = new HelpLabel(this, "<img src= ':/IDWExplanation.png'>");
  idwOptionsLayout->addWidget(helpLabelRun);
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

QPushButton*MapsTab::buttonExportOptimizedToZycor() const
{
  return buttonExportOptimizedToZycor_;
}

QPushButton* MapsTab::buttonRunOptimized() const
{
  return buttonRunOptimized_;
}

ActiveWellsTable* MapsTab::activeWellsTable() const
{
  return activeWellsTable_;
}

int MapsTab::numberOfActiveWells() const
{
  return activeWellsTable_->rowCount();
}

void MapsTab::highlightWell(const QString& wellName)
{
  int wellIndex = -1;
  for (int i = 0; i < activeWellsTable_->rowCount(); i++)
  {
    if (activeWellsTable_->item(i, 1)->text() == wellName)
    {
      wellIndex = i;
      break;
    }
  }

  activeWellsTable_->selectRow(wellIndex);
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

QSpinBox* MapsTab::pValue() const
{
  return pValue_;
}

QComboBox* MapsTab::smoothingType() const
{
  return smoothingType_;
}

CustomCheckbox* MapsTab::smartGridding() const
{
  return smartGridding_;
}

QComboBox* MapsTab::interpolationType() const
{
  return interpolationType_;
}

} // namespace sac

} // namespace casaWizard
