//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ThermalMapTab.h"

#include "view/sharedComponents/emphasisbutton.h"

#include "model/well.h"
#include "view/assets/activeWellsTable.h"
#include "assets/TCHPVisualisation.h"

#include <QLabel>
#include <QLayout>
#include <QComboBox>

namespace casaWizard
{

namespace sac
{

namespace thermal
{

ThermalMapTab::ThermalMapTab(QWidget* parent) :
   SacMapsTab(parent),
   m_TCHPVisualisation{new TCHPVisualisation(this)}
{
   setTotalLayout();
}

void ThermalMapTab::setGridGenerationOptionsLayout()
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

   optimizationOptionsLayout->addWidget(createGridsButton(),5,0,1,2);
   optimizationOptionsLayout->setMargin(0);
}

void ThermalMapTab::setTotalLayout()
{
   QHBoxLayout* total = new QHBoxLayout(this);
   total->addLayout(setWellsAndOptionsLayout(), 1);
   total->addWidget(m_TCHPVisualisation,5);
}

TCHPVisualisation* ThermalMapTab::mapsVisualisation() const
{
   return m_TCHPVisualisation;
}

} // namespace thermal

} // namespace sac

} // namespace casaWizard
