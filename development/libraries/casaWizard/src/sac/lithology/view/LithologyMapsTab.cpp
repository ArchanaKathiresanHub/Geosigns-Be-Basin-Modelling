//
// Copyright (C) 2012-2023 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "LithologyMapsTab.h"

#include "view/sharedComponents/customcheckbox.h"
#include "view/sharedComponents/emphasisbutton.h"
#include "view/sharedComponents/helpLabel.h"

#include "model/well.h"
#include "view/assets/activeWellsTable.h"

#include "assets/LithoGrid2DPlot.h"
#include "plots/lithoPercent2Dview.h"
#include "assets/lithofractionVisualisation.h"

#include <QComboBox>
#include <QLayout>

namespace casaWizard
{

namespace sac
{

namespace lithology
{

LithologyMapsTab::LithologyMapsTab(QWidget* parent) :
   SacMapsTab(parent),
   m_lithofractionVisualisation{new LithofractionVisualisation(this)},
   m_smartGridding{new CustomCheckbox(this)}
{
   setTotalLayout();
}

void LithologyMapsTab::setGridGenerationOptionsLayout()
{
   setIdwOptionsLayout();
   setSmoothingOptionsLayout();

   QGridLayout* optimizationOptionsLayout = new QGridLayout(gridGenerationOptions());

   optimizationOptionsLayout->addWidget(new QLabel("Interpolation: ", this),0,0);
   optimizationOptionsLayout->addWidget(interpolationType(),0,1);
   optimizationOptionsLayout->addWidget(idwOptions(),1,1);

   optimizationOptionsLayout->addWidget(new QLabel("Smoothing: ", this),2,0);
   optimizationOptionsLayout->addWidget(smoothingType(),2,1);
   optimizationOptionsLayout->addWidget(smoothingOptions(),3,1);

   optimizationOptionsLayout->addWidget(new QLabel("Smart Gridding: ", this),4,0);

   QHBoxLayout* smartGriddingLayout = new QHBoxLayout();
   smartGriddingLayout->addWidget(m_smartGridding);
   smartGriddingLayout->addWidget(new HelpLabel(this, "When checked, selected wells are only included and visualized \nfor gridding in formations where data is present"));
   smartGriddingLayout->setStretch(0,1);
   smartGriddingLayout->setStretch(1,0);

   optimizationOptionsLayout->addLayout(smartGriddingLayout,4,1);
   m_smartGridding->setCheckState(Qt::CheckState::Checked);

   optimizationOptionsLayout->addWidget(createGridsButton(),5,0,1,2);
   optimizationOptionsLayout->setMargin(0);
}

void LithologyMapsTab::setTotalLayout()
{
   QHBoxLayout* total = new QHBoxLayout(this);
   total->addLayout(setWellsAndOptionsLayout(), 1);
   total->addWidget(m_lithofractionVisualisation, 5);
}

LithofractionVisualisation* LithologyMapsTab::mapsVisualisation() const
{
   return m_lithofractionVisualisation;
}

CustomCheckbox* LithologyMapsTab::smartGridding() const
{
   return m_smartGridding;
}

} // namespace lithology

} // namespace sac

} // namespace casaWizard
