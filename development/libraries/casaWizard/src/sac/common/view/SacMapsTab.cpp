//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SacMapsTab.h"

#include "view/assets/activeWellsTable.h"

#include "view/sharedComponents/customcheckbox.h"
#include "view/sharedComponents/customtitle.h"
#include "view/sharedComponents/emphasisbutton.h"
#include "view/sharedComponents/helpLabel.h"

#include <QComboBox>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace casaWizard
{

namespace sac
{

SacMapsTab::SacMapsTab(QWidget* parent):
   QWidget(parent),
   m_activeWellsTable{new ActiveWellsTable(this)},
   m_createGridsButton{new EmphasisButton("Create 2D grids", this)},
   m_buttonExportOptimized{new QPushButton("Export optimized", this)},
   m_buttonExportOptimizedToZycor{new QPushButton("Export to Zycor", this)},
   m_buttonRunOptimized{new QPushButton("Run optimized 3D", this)},
   m_interpolationType{new QComboBox(this)},
   m_idwOptions{new QWidget(this)},
   m_gridGenerationOptions{new QWidget(this)},
   m_pValue{new QSpinBox(this)},
   m_smoothingOptions{new QWidget(this)},
   m_smoothingRadius{new QSpinBox(this)},
   m_smoothingType{new QComboBox(this)},
   m_pushSelectAllWells{new QPushButton("Select All", this)},
   m_pushClearWellSelection{new QPushButton("Deselect All", this)}
{
   connectSignalsAndSlots();
   slotInterpolationTypeChange(0);
   slotSmoothingTypeChange(0);
}

void SacMapsTab::disableWellAtIndex(const int index)
{
   CustomCheckbox* itemCheckBox = static_cast<CustomCheckbox*>(m_activeWellsTable->cellWidget(index, 0)->children()[1]);
   itemCheckBox->setCheckState(Qt::CheckState::Unchecked);
   itemCheckBox->enable(false);

   QTableWidgetItem* toDisable = m_activeWellsTable->item(index, 1);
   toDisable->setForeground(Qt::red);
   toDisable->setFlags(toDisable->flags().setFlag(Qt::ItemIsEnabled, false));
}

void SacMapsTab::highlightWell(const QString& wellName)
{
   int wellIndex = -1;
   for (int i = 0; i < activeWellsTable()->rowCount(); i++)
   {
      if (activeWellsTable()->item(i, 1)->text() == wellName)
      {
         wellIndex = i;
         break;
      }
   }

   activeWellsTable()->selectRow(wellIndex);
}

void SacMapsTab::setDefaultGridGenerationOptions()
{
   m_interpolationType->insertItems(0, {"Inverse Distance Weighting", "Natural Neighbor"});
   m_smoothingType->insertItems(0, {"None", "Gaussian", "Moving Average"});

   m_pValue->setMinimum(1);
   m_pValue->setMaximum(100);

   m_smoothingRadius->setMinimum(0);
   m_smoothingRadius->setSingleStep(100);
   m_smoothingRadius->setMaximum(1e6);
}

void SacMapsTab::setIdwOptionsLayout()
{
   QHBoxLayout* idwOptionsLayout = new QHBoxLayout(idwOptions());
   idwOptionsLayout->addWidget(new QLabel(" P: ", this));
   idwOptionsLayout->addWidget(pValue());
   HelpLabel* helpLabelRun = new HelpLabel(this, "<img src= ':/IDWExplanation.png'>");
   idwOptionsLayout->addWidget(helpLabelRun);
   idwOptionsLayout->setStretch(0,1);
   idwOptionsLayout->setStretch(1,4);
   idwOptionsLayout->setMargin(0);
}

QVBoxLayout* SacMapsTab::setWellsAndOptionsLayout()
{
   setGridGenerationOptionsLayout();
   setDefaultGridGenerationOptions();

   CustomTitle* label1 = new CustomTitle("Wells", this);
   label1->setFixedHeight(15);
   CustomTitle* label2 = new CustomTitle("Gridding options", this);
   label2->setFixedHeight(15);
   QHBoxLayout* griddingOptions = new QHBoxLayout();
   griddingOptions->addWidget(label2);
   HelpLabel* helpGriddingOptions = new HelpLabel(this, "The gridding options determine how the optimized values at the well locations "
                                                        "are interpolated to result in complete maps. <br><br>The maps can be tweaked by adjusting: <br>"
                                                        " - The interpolation algorithm <br>"
                                                        " - The smoothing options <br>");
   griddingOptions->addWidget(helpGriddingOptions);

   CustomTitle* label3 = new CustomTitle("3D model", this);
   label3->setFixedHeight(15);

   QVBoxLayout* wellsAndOptions = new QVBoxLayout();

   QHBoxLayout* wellSelectionLayout = new QHBoxLayout;
   wellSelectionLayout->addWidget(label1);
   wellSelectionLayout->addStretch(1);
   wellSelectionLayout->addWidget(m_pushSelectAllWells);
   wellSelectionLayout->addWidget(m_pushClearWellSelection);

   wellsAndOptions->addLayout(wellSelectionLayout);
   wellsAndOptions->addWidget(activeWellsTable());
   wellsAndOptions->addLayout(griddingOptions);
   wellsAndOptions->addWidget(gridGenerationOptions());
   wellsAndOptions->addWidget(label3);
   QHBoxLayout* exportOptimized = new QHBoxLayout();
   exportOptimized->addWidget(buttonExportOptimized());
   HelpLabel* helpLabelExport = new HelpLabel(this, "Create a zip-file for import into BPA2-Basin");
   exportOptimized->addWidget(helpLabelExport);
   wellsAndOptions->addLayout(exportOptimized);
   QHBoxLayout* exportOptimizedToZycor = new QHBoxLayout();
   exportOptimizedToZycor->addWidget(buttonExportOptimizedToZycor());
   HelpLabel* helpLabelExportToZycor = new HelpLabel(this, "Export the optimized map(s) to the zycor format");
   exportOptimizedToZycor->addWidget(helpLabelExportToZycor);
   wellsAndOptions->addLayout(exportOptimizedToZycor);

   QHBoxLayout* runOptimized = new QHBoxLayout();
   runOptimized->addWidget(buttonRunOptimized());
   HelpLabel* helpLabelRun = new HelpLabel(this, "For plotting and QC purposes under 'Well log plots and Results' tab");

   runOptimized->addWidget(helpLabelRun);

   wellsAndOptions->addLayout(runOptimized);

   return wellsAndOptions;
}

void SacMapsTab::setSmoothingOptionsLayout()
{
   QVBoxLayout* smoothingOptionsLayout = new QVBoxLayout(smoothingOptions());

   QHBoxLayout* radiusOptionLayout = new QHBoxLayout();
   radiusOptionLayout->addWidget(new QLabel(" Radius[m]: ", this));
   radiusOptionLayout->addWidget(smoothingRadius());

   smoothingOptionsLayout->addLayout(radiusOptionLayout);
   smoothingOptionsLayout->setMargin(0);
}

ActiveWellsTable* SacMapsTab::activeWellsTable() const
{
   return m_activeWellsTable;
}

int SacMapsTab::numberOfActiveWells() const
{
   return m_activeWellsTable->rowCount();
}

QPushButton* SacMapsTab::createGridsButton() const
{
   return m_createGridsButton;
}

void SacMapsTab::slotInterpolationTypeChange(int interpolationType)
{
   m_idwOptions->setVisible(interpolationType == 0);
}

void SacMapsTab::slotSmoothingTypeChange(int smoothingType)
{
   m_smoothingOptions->setVisible(smoothingType > 0);
}

QSpinBox* SacMapsTab::smoothingRadius() const
{
   return m_smoothingRadius;
}

QSpinBox* SacMapsTab::pValue() const
{
   return m_pValue;
}

QComboBox* SacMapsTab::smoothingType() const
{
   return m_smoothingType;
}

QPushButton* SacMapsTab::buttonExportOptimized() const
{
   return m_buttonExportOptimized;
}

QPushButton* SacMapsTab::buttonExportOptimizedToZycor() const
{
   return m_buttonExportOptimizedToZycor;
}

QPushButton* SacMapsTab::buttonRunOptimized() const
{
   return m_buttonRunOptimized;
}

QComboBox* SacMapsTab::interpolationType() const
{
   return m_interpolationType;
}

QPushButton* SacMapsTab::pushSelectAllWells() const
{
   return m_pushSelectAllWells;
}

QPushButton* SacMapsTab::pushClearWellSelection() const
{
   return m_pushClearWellSelection;
}

QWidget* SacMapsTab::idwOptions() const
{
   return m_idwOptions;
}

QWidget* SacMapsTab::gridGenerationOptions() const
{
   return m_gridGenerationOptions;
}

QWidget* SacMapsTab::smoothingOptions() const
{
   return m_smoothingOptions;
}

void SacMapsTab::connectSignalsAndSlots()
{
   connect(m_interpolationType, SIGNAL(currentIndexChanged(int)), this, SLOT(slotInterpolationTypeChange(int)));
   connect(m_smoothingType,     SIGNAL(currentIndexChanged(int)), this, SLOT(slotSmoothingTypeChange(int)));
}

} // namespace sac

} // namespace casaWizard

