//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ThermalInputTab.h"
#include <QPushButton>
#include <QLineEdit>

#include "view/sharedComponents/helpLabel.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

ThermalInputTab::ThermalInputTab(QWidget* parent) :
   SacInputTab(parent),
   m_pushImportTargets{new QPushButton("Import Targets", this)},
   m_lineMinHCP{new QLineEdit(this)},
   m_lineMaxHCP{new QLineEdit(this)}
{
   m_lineMaxHCP->setFixedWidth(48);
   m_lineMinHCP->setFixedWidth(48);
   m_lineMinHCP->setMaxLength(5);
   m_lineMaxHCP->setMaxLength(5);

   QGridLayout* layoutHCPRange = new QGridLayout();
   QLabel* labelMinHCP = new QLabel(this);
   labelMinHCP->setText("Min A0:");
   HelpLabel* helpLabelMinHCP = new HelpLabel(this, "minimal value to optimize Heat Crust Production (HCP) / \"A0\" for");
   layoutHCPRange->addWidget(labelMinHCP, 0, 0);
   layoutHCPRange->addWidget(m_lineMinHCP, 0, 1);
   layoutHCPRange->addWidget(helpLabelMinHCP, 0, 2);

   QLabel* labelMaxHCP = new QLabel(this);
   labelMaxHCP->setText("Max A0:");
   HelpLabel* helpLabelMaxHCP = new HelpLabel(this, "maximum value to optimize Heat Crust Production (HCP) / \"A0\" for");
   layoutHCPRange->addWidget(labelMaxHCP, 1, 0);
   layoutHCPRange->addWidget(m_lineMaxHCP, 1, 1);
   layoutHCPRange->addWidget(helpLabelMaxHCP, 1, 2);

   layoutHCPRange->setMargin(0);

   layoutRunOptions()->insertLayout(1, layoutHCPRange);
   layoutCalibrationOptions()->insertWidget(1, m_pushImportTargets);
}

const QPushButton* ThermalInputTab::pushButtonImportTargets() const
{
   return m_pushImportTargets;
}

QLineEdit* ThermalInputTab::lineEditMinHCP() const
{
   return m_lineMinHCP;
}

QLineEdit* ThermalInputTab::lineEditMaxHCP() const
{
   return m_lineMaxHCP;
}

} // namespace thermal

} // namespace sac

} // namespace casaWizard
