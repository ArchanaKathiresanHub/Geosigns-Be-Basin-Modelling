//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "mapsTabLithology.h"

#include "view/sharedComponents/customcheckbox.h"
#include "view/sharedComponents/customtitle.h"
#include "view/sharedComponents/emphasisbutton.h"
#include "view/sharedComponents/helpLabel.h"

#include "model/well.h"
#include "view/assets/activeWellsTable.h"
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

MapsTabLithology::MapsTabLithology(QWidget* parent) :
   SacMapsTab(parent),
   m_lithofractionVisualisation{new LithofractionVisualisation(this)},
   m_smartGridding{new CustomCheckbox(this)}
{
   setTotalLayout();
}

void MapsTabLithology::setGridGenerationOptionsLayout()
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

void MapsTabLithology::setTotalLayout()
{
   QHBoxLayout* total = new QHBoxLayout(this);
   total->addLayout(setWellsAndOptionsLayout(), 1);
   total->addWidget(m_lithofractionVisualisation,5);
}

LithofractionVisualisation* MapsTabLithology::mapsVisualisation() const
{
   return m_lithofractionVisualisation;
}

CustomCheckbox* MapsTabLithology::smartGridding() const
{
   return m_smartGridding;
}

} // namespace sac

} // namespace casaWizard
