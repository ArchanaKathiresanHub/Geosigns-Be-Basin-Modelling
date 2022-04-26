//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QDialog>
#include <QStringList>
#include <QTableWidget>
#include <QVector>

namespace casaWizard
{

class CustomCheckbox;
class StringSelectionTable;

namespace ua
{

class TargetImportLayout;

class TargetImportDialogWells : public QDialog
{
   Q_OBJECT

public:
   TargetImportDialogWells(QWidget* parent = nullptr);
   void updateWellsTable(const QStringList& wellNames);
   void updateSurfaceTable(const QStringList& surfaces);

   QString depthInput() const;
   QVector<bool> surfaceSelectionStates() const;
   bool temperatureTargetsSelected() const;
   bool vreTargetsSelected() const;
   QVector<bool> wellSelectionStates() const;

private slots:
   void slotSelectAllWells();

private:
   TargetImportLayout* m_targetImportLayout;
   StringSelectionTable* m_wellsTable;
};

} // namespace ua

} // namespace casaWizard

