//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QLineEdit>
#include <QTableWidget>
#include <QVBoxLayout>

namespace casaWizard
{

class StringSelectionTable;
class CustomCheckbox;

namespace ua
{

class TargetImportLayout : public QVBoxLayout
{
     Q_OBJECT

public:
   TargetImportLayout(QWidget* parent = nullptr);
   void updateSurfaceTable(const QStringList& surfaces);

   QVector<bool> surfaceSelectionStates() const;
   bool temperatureTargetsSelected() const;
   bool vreTargetsSelected() const;
   QString depthInput() const;

private slots:
   void slotSelectAllSurfaces();

private:
   QLineEdit* m_lineEditZ;
   StringSelectionTable* m_surfaceTable;
   CustomCheckbox* m_checkBoxTemperature;
   CustomCheckbox* m_checkBoxVRe;
};

} // namespace ua

} // namespace casaWizard

