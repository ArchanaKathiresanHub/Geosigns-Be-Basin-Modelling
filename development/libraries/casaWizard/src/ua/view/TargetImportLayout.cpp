//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "TargetImportLayout.h"

#include "view/sharedComponents/customcheckbox.h"
#include "view/sharedComponents/helpLabel.h"
#include "view/StringSelectionTable.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>

namespace casaWizard
{

namespace ua
{

TargetImportLayout::TargetImportLayout(QWidget* parent):
   QVBoxLayout(parent),
   m_lineEditZ{new QLineEdit(parent)},
   m_surfaceTable(new StringSelectionTable("Surface name", parent)),
   m_checkBoxTemperature{new CustomCheckbox("", parent)},
   m_checkBoxVRe{new CustomCheckbox("", parent)}
{
   QPushButton* pushSelectAllSurfaces = new QPushButton("Select all", parent);
   connect(pushSelectAllSurfaces, SIGNAL(clicked()), this, SLOT(slotSelectAllSurfaces()));

   QHBoxLayout* layoutSurfaceSelect = new QHBoxLayout();
   layoutSurfaceSelect->addWidget(new QLabel("Surfaces", parent));
   layoutSurfaceSelect->addStretch(1);
   layoutSurfaceSelect->addWidget(pushSelectAllSurfaces);

   QHBoxLayout* layoutTemperatureCheck = new QHBoxLayout();
   layoutTemperatureCheck->addWidget(new QLabel("Temperature:", parent));
   m_checkBoxTemperature->setChecked(true);
   layoutTemperatureCheck->addWidget(m_checkBoxTemperature);

   HelpLabel* helpLabelZ = new HelpLabel(parent, "Enter multiple depth values by separating the entries with a comma, e.g.: '500, 1000, 1500, ...'");
   helpLabelZ->setSize(10);

   QHBoxLayout* layoutZ = new QHBoxLayout();
   layoutZ->addWidget(new QLabel("Z [m]:", parent));
   layoutZ->addStretch(1);
   layoutZ->addWidget(m_lineEditZ);
   layoutZ->addWidget(helpLabelZ);

   QHBoxLayout* layoutVReCheck = new QHBoxLayout();
   layoutVReCheck->addWidget(new QLabel("VRe:", parent));
   m_checkBoxVRe->setChecked(true);
   layoutVReCheck->addWidget(m_checkBoxVRe);

   addLayout(layoutSurfaceSelect);
   addWidget(m_surfaceTable);
   addLayout(layoutZ);
   addLayout(layoutTemperatureCheck);
   addLayout(layoutVReCheck);
   addSpacing(5);
}

void TargetImportLayout::updateSurfaceTable(const QStringList& surfaces)
{
   m_surfaceTable->updateTable(surfaces);
}

void TargetImportLayout::slotSelectAllSurfaces()
{
   m_surfaceTable->slotSelectAll();
}

QVector<bool> TargetImportLayout::surfaceSelectionStates() const
{
   return m_surfaceTable->selectionStates();
}

bool TargetImportLayout::temperatureTargetsSelected() const
{
   return m_checkBoxTemperature->isChecked();
}

bool TargetImportLayout::vreTargetsSelected() const
{
   return m_checkBoxVRe->isChecked();
}

QString TargetImportLayout::depthInput() const
{
   return m_lineEditZ->text();
}

} // namespace ua

} // namespace casaWizard
