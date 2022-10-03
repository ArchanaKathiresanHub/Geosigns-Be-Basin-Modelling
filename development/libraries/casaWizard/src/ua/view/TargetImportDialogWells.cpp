//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "TargetImportDialogWells.h"
#include "TargetImportLayout.h"
#include "view/sharedComponents/customcheckbox.h"
#include "view/StringSelectionTable.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QDialogButtonBox>

namespace casaWizard
{

namespace ua
{

TargetImportDialogWells::TargetImportDialogWells(QWidget* parent):
   QDialog(parent),
   m_targetImportLayout{new TargetImportLayout()},
   m_wellsTable(new StringSelectionTable("Well name", parent))
{
   setWindowTitle("Import options");

   QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

   connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
   connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

   QPushButton* pushSelectAllWells = new QPushButton("Select all", parent);
   connect(pushSelectAllWells, SIGNAL(clicked()), this, SLOT(slotSelectAllWells()));

   QHBoxLayout* layoutWellSelect = new QHBoxLayout();
   layoutWellSelect->addWidget(new QLabel("Wells", parent));
   layoutWellSelect->addStretch(1);
   layoutWellSelect->addWidget(pushSelectAllWells);

   QVBoxLayout* wellsLayout = new QVBoxLayout();
   wellsLayout->addLayout(layoutWellSelect);
   wellsLayout->addWidget(m_wellsTable);

   QHBoxLayout* dialogLayout = new QHBoxLayout();
   dialogLayout->addLayout(wellsLayout);
   dialogLayout->addSpacing(5);
   dialogLayout->addLayout(m_targetImportLayout);

   QVBoxLayout* total = new QVBoxLayout(this);
   total->addLayout(dialogLayout);
   total->addWidget(buttons);
}

void TargetImportDialogWells::slotSelectAllWells()
{
   m_wellsTable->slotSelectAll();
}

void TargetImportDialogWells::updateWellsTable(const QStringList& wellNames)
{
   m_wellsTable->updateTable(wellNames);
}

void TargetImportDialogWells::updateSurfaceTable(const QStringList& surfaces)
{
   m_targetImportLayout->updateSurfaceTable(surfaces);
}

QVector<bool> TargetImportDialogWells::wellSelectionStates() const
{
   return m_wellsTable->selectionStates();
}

QVector<bool> TargetImportDialogWells::surfaceSelectionStates() const
{
   return m_targetImportLayout->surfaceSelectionStates();
}

bool TargetImportDialogWells::temperatureTargetsSelected() const
{
   return m_targetImportLayout->temperatureTargetsSelected();
}

bool TargetImportDialogWells::vreTargetsSelected() const
{
   return m_targetImportLayout->vreTargetsSelected();
}

QString TargetImportDialogWells::depthInput() const
{
   return m_targetImportLayout->depthInput();
}

} // namespace ua

} // namespace casaWizard
