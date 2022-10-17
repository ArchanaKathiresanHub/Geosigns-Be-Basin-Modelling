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
#include <QDoubleValidator>

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
   m_lineMinTCHP{new QLineEdit(this)},
   m_lineMaxTCHP{new QLineEdit(this)}
{
   m_lineMaxTCHP->setFixedWidth(48);
   m_lineMinTCHP->setFixedWidth(48);
   m_lineMinTCHP->setMaxLength(5);
   m_lineMaxTCHP->setMaxLength(5);
   m_lineMinTCHP->setValidator(new QDoubleValidator(-9999, 9999, 4, this));
   m_lineMaxTCHP->setValidator(new QDoubleValidator(-9999, 9999, 4, this));

   QGridLayout* layoutTCHPRange = new QGridLayout();
   QLabel* labelMinTCHP = new QLabel(this);
   labelMinTCHP->setText("Min A0:");
   HelpLabel* helpLabelMinTCHP = new HelpLabel(this, "Minimal value for the optimizing range of the Top Crust Heat Production / \"A0\"");
   layoutTCHPRange->addWidget(labelMinTCHP, 0, 0);
   layoutTCHPRange->addWidget(m_lineMinTCHP, 0, 1);
   layoutTCHPRange->addWidget(helpLabelMinTCHP, 0, 2);

   QLabel* labelMaxTCHP = new QLabel(this);
   labelMaxTCHP->setText("Max A0:");
   HelpLabel* helpLabelMaxTCHP = new HelpLabel(this, "Maximum value for the optimizing range of the Top Crust Heat Production / \"A0\"");
   layoutTCHPRange->addWidget(labelMaxTCHP, 1, 0);
   layoutTCHPRange->addWidget(m_lineMaxTCHP, 1, 1);
   layoutTCHPRange->addWidget(helpLabelMaxTCHP, 1, 2);

   layoutTCHPRange->setMargin(0);

   layoutRunOptions()->insertLayout(0, layoutTCHPRange);
   layoutCalibrationOptions()->insertWidget(1, m_pushImportTargets);
}

const QPushButton* ThermalInputTab::pushButtonImportTargets() const
{
   return m_pushImportTargets;
}

QLineEdit* ThermalInputTab::lineEditMinTCHP() const
{
   return m_lineMinTCHP;
}

QLineEdit* ThermalInputTab::lineEditMaxTCHP() const
{
   return m_lineMaxTCHP;
}

} // namespace thermal

} // namespace sac

} // namespace casaWizard
